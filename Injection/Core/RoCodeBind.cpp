#include "../tinyconsole.h"
#include "RoCodeBind.h"

HANDLE         g_hMapObject = 0;
StSHAREDMEMORY *g_pSharedData = 0;


typedef int (WSAAPI *tWS2_32_send)(SOCKET s, char *buf, int len, int flags);
typedef int (WSAAPI *tWS2_32_recv)(SOCKET s, char *buf, int len, int flags);

tWS2_32_send *pCConnection_s_wsSend = NULL;
tWS2_32_recv *pCConnection_s_wsRecv = NULL;

extern tWS2_32_recv OrigWS2_32_recv;
int WSAAPI ProxyWS2_32_recv(SOCKET s, char *buf, int len, int flags);

/*
	Create a shared memory.
*/
BOOL OpenSharedMemory(void)
{
	g_hMapObject = ::OpenFileMapping( FILE_MAP_ALL_ACCESS , FALSE, SHAREDMEMORY_OBJECTNAME );
	if( !g_hMapObject ){
		DEBUG_LOGGING_NORMAL( ("shared memory:Initialize Failed.") );
		return FALSE;
	}
	g_pSharedData = (StSHAREDMEMORY*)::MapViewOfFile(g_hMapObject,
		FILE_MAP_ALL_ACCESS,0,0,0);
	if(!g_pSharedData){
		DEBUG_LOGGING_NORMAL( ("shared memory:DataMap Failed.") );
		::CloseHandle( g_hMapObject);
		g_hMapObject = NULL;
		return FALSE;
	}
	return TRUE;
}
/*
	Release shared memory
*/
BOOL ReleaseSharedMemory(void)
{
	if(g_pSharedData){
		g_pSharedData->g_hROWindow = 0;
		::UnmapViewOfFile( g_pSharedData );
		g_pSharedData = NULL;
	}
	if(g_hMapObject){
		::CloseHandle( g_hMapObject);
		g_hMapObject = NULL;
	}
	return TRUE;
}

CPerformanceCounter g_PerformanceCounter(60);

CRoCodeBind* g_pRoCodeBind = NULL;

BOOL g_FreeMouseSw = FALSE;


static HRESULT CALLBACK TextureSearchCallback( DDPIXELFORMAT* pddpf,
                                               VOID* param )
{
    if( pddpf->dwFlags & (DDPF_LUMINANCE|DDPF_BUMPLUMINANCE|DDPF_BUMPDUDV) )
        return DDENUMRET_OK;
    if( pddpf->dwFourCC != 0 )
        return DDENUMRET_OK;
    if( pddpf->dwRGBBitCount != 16 )
        return DDENUMRET_OK;
    if(!(pddpf->dwFlags&DDPF_ALPHAPIXELS) )
        return DDENUMRET_OK;
	// get 16 bit with alphapixel format
    memcpy( (DDPIXELFORMAT*)param, pddpf, sizeof(DDPIXELFORMAT) );
    return DDENUMRET_CANCEL;
}

void CRoCodeBind::Init(IDirect3DDevice7* d3ddevice)
{
	SearchRagexeMemory();
	InitItemNameMap();
	InitPacketHandler();
	LoadIni();

	D3DDEVICEDESC7 ddDesc;
	d3ddevice->GetCaps( &ddDesc );

	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
	ddsd.dwSize          = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags         = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|
							DDSD_PIXELFORMAT|DDSD_TEXTURESTAGE;
	ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
	ddsd.dwWidth         = 512;
	ddsd.dwHeight        = 512;

	if( ddDesc.deviceGUID == IID_IDirect3DHALDevice ){
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	} else if( ddDesc.deviceGUID == IID_IDirect3DTnLHalDevice ){
		ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
	} else {
		ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	}

	d3ddevice->EnumTextureFormats( TextureSearchCallback, &ddsd.ddpfPixelFormat );
	if( ddsd.ddpfPixelFormat.dwRGBBitCount ){
		LPDIRECTDRAWSURFACE7 pddsRender = NULL;
		LPDIRECTDRAW7        pDD = NULL;

		d3ddevice->GetRenderTarget( &pddsRender );
		if( pddsRender ){
			pddsRender->GetDDInterface( (VOID**)&pDD );
			pddsRender->Release();
		}
		if( pDD ){
			if( SUCCEEDED( pDD->CreateSurface( &ddsd, &m_pddsFontTexture, NULL )) ){
				DEBUG_LOGGING_NORMAL(( "font texture created." ));
			} else {
				DEBUG_LOGGING_NORMAL(( "failed create a font texture." ));
			}
			pDD->Release();
		}
		if( m_pddsFontTexture ){
			LOGFONT logfont;
			logfont.lfHeight         = -10;
			logfont.lfWidth          = 0;
			logfont.lfEscapement     = 0;
			logfont.lfOrientation    = 0;
			logfont.lfWeight         = FW_REGULAR;
			//
			logfont.lfItalic         = FALSE;
			logfont.lfUnderline      = FALSE;
			logfont.lfStrikeOut      = FALSE;
			logfont.lfCharSet        = DEFAULT_CHARSET;
			logfont.lfOutPrecision = OUT_TT_ONLY_PRECIS;//OUT_DEFAULT_PRECIS; 
			logfont.lfClipPrecision  = CLIP_DEFAULT_PRECIS; 
			logfont.lfQuality = PROOF_QUALITY;//NONANTIALIASED_QUALITY; 
			logfont.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE; 
			_tcscpy_s(logfont.lfFaceName,_T("‚l‚r ‚oƒSƒVƒbƒN"));

			m_pSFastFont = new CSFastFont;
			m_pSFastFont->CreateFastFont(&logfont,d3ddevice,m_pddsFontTexture,0);
		}

	}

}

CRoCodeBind::~CRoCodeBind(void)
{
	if( m_pSFastFont )
		delete m_pSFastFont;
	if( m_pddsFontTexture ){
		m_pddsFontTexture->Release();
		m_pddsFontTexture = NULL;
	}
}

void *CRoCodeBind::GetPak(const char *name, unsigned int *size)
{
	if (m_CFileMgr__gfileMgr && m_functionRagexe_CFileMgr__GetPak)
	{
		void *address;
		address = m_functionRagexe_CFileMgr__GetPak(m_CFileMgr__gfileMgr,name, size);
		return address;
	}
	return NULL;
}
void CRoCodeBind::ReleasePak(void *handle)
{
	::VirtualFree(handle, 0, MEM_RELEASE);
}

void CRoCodeBind::InitItemNameMap()
{
	char *buf = NULL;
	char *p, *ptoken;

	unsigned int size;
	buf = (char*)GetPak("data\\idnum2itemdisplaynametable.txt",&size);

	if (!buf)return;
	p = buf;
	while (*p != '\0'){
		ptoken = p;
		while (*p != '\0' && *p != '\r' && *p != '\n'){
			p++;
		}
		if (*p == '\r')p++;
		if (*p == '\n')p++;

		if (*ptoken == '/' || *ptoken == '\0' || *ptoken == ' ' || *ptoken == '\r' || *ptoken == '\n')
			continue;
		//
		char numstr[10], *pname;
		pname = numstr;
		while (*ptoken != '#')*pname++ = *ptoken++;
		*pname++ = '\0';
		int itemid = atoi(numstr);

		char tempstr[256];
		char *pdname = tempstr;
		*ptoken++;
		while (*ptoken != '#'){
			if (*ptoken != '_'){
				*pdname++ = *ptoken++;
			}
			else{
				*pdname++ = ' ';
				ptoken++;
			}
		}
		*pdname++ = '\0';
		int datasize = (pdname - tempstr);

		m_ItemName[itemid] = tempstr;
}
	ReleasePak(buf);
}

const char *CRoCodeBind::GetItemNameByID(int id)
{
	static const char* pUnknownItem = "Unknown Item";

	if (m_ItemName[id].empty()){
		return pUnknownItem;
	}
	else{
		return m_ItemName[id].c_str();
	}
}

void CRoCodeBind::OneSyncProc(HRESULT Result,LPVOID lpvData,BOOL FreeMouse)
{
#ifdef JRO_CLIENT_STRUCTURE
	if (pCConnection_s_wsRecv){
		if (*pCConnection_s_wsRecv && OrigWS2_32_recv == 0){
			OrigWS2_32_recv = *pCConnection_s_wsRecv;
			*pCConnection_s_wsRecv = &ProxyWS2_32_recv;
			DEBUG_LOGGING_NORMAL(("Hook CConnection_s_wsRecv(%08X) = %08X old %08X",
				pCConnection_s_wsRecv, ProxyWS2_32_recv,OrigWS2_32_recv ));
		}
	}
#endif

	if( Result == DI_OK ){
		//
		//  FreeMouse
		//
		if( FreeMouse ){
			MouseDataStructure *p = (MouseDataStructure*)lpvData;

			POINT point;
			::GetCursorPos(&point);
			::ScreenToClient(m_hWnd, &point);

			if( ::GetActiveWindow() == m_hWnd ){
				point.x -= p->x_axis;
				point.y -= p->y_axis;
			}
			SetMouseCurPos((int)point.x, (int)point.y);
		}
	}


	// proc on frame
	if( g_pSharedData ){
		if( g_pSharedData->executeorder == 1){
			g_pSharedData->executeorder = 0;
			char filename[MAX_PATH];
			if( ::WideCharToMultiByte(CP_ACP,0,
				g_pSharedData->musicfilename,wcslen(g_pSharedData->musicfilename)+1,
				filename,sizeof(filename),
				NULL,NULL) ){
				//
				// Play Sound File
				//
				if( m_funcRagexe_PlayStream )
					m_funcRagexe_PlayStream(filename,0);
			}
		}

		// cpu cooler
		if( g_pSharedData->cpucoolerlevel ){
			int level = g_pSharedData->cpucoolerlevel;
			int CoolerLevelTable[4]={1,1,3,10};
			int ref = g_PerformanceCounter.GetMonitorRefreshRate();

			if( level < 0 )level = 0;
			else if( level > 3 )level = 3;

			ref /= CoolerLevelTable[level];
			if( ref ){
				::Sleep( 1000/ref );
			}
		}
	}

}

void CRoCodeBind::SetMouseCurPos(int x,int y)
{
	if (g_mouse){
		g_mouse->m_xPos = x;
		g_mouse->m_yPos = y;
	}
}

void vector3d::MatrixMult(struct vector3d& v, struct matrix& m)
{
	x = v.x * m.v11 + v.y * m.v21 + v.z * m.v31 + m.v41;
	y = v.x * m.v12 + v.y * m.v22 + v.z * m.v32 + m.v42;
	z = v.x * m.v13 + v.y * m.v23 + v.z * m.v33 + m.v43;
}

// CRenderer::ProjectVertex
void CRoCodeBind::ProjectVertex(vector3d& src,matrix& vtm,float *x,float *y,float *oow)
{
	if( !g_renderer && !*g_renderer )return;

	vector3d viewvect;
	viewvect.MatrixMult( src , vtm );

	float w = 1.0f / viewvect.z;

	*x = viewvect.x * w * (*g_renderer)->m_hpc + (*g_renderer)->m_xoffset;
	*y = viewvect.y * w * (*g_renderer)->m_vpc + (*g_renderer)->m_yoffset;
	*oow = w;
}

// CRenderer::ProjectVertex
void CRoCodeBind::ProjectVertex(vector3d& src,matrix& vtm,tlvertex3d *vert)
{
	if( !g_renderer && !*g_renderer )return;

	vector3d viewvect;
	viewvect.MatrixMult( src , vtm );

	float w = 1.0f / viewvect.z;

	vert->x = viewvect.x * w * (*g_renderer)->m_hpc + (*g_renderer)->m_halfWidth;
	vert->y = viewvect.y * w * (*g_renderer)->m_vpc + (*g_renderer)->m_halfHeight;
	vert->z = (1500 / (1500.0f - 10.0f)) * (( 1.0f / w ) - 10.0f) * w;
	vert->oow = w;
}

// ProjectVertexEx
// to move pointvector on camera view
void CRoCodeBind::ProjectVertexEx(vector3d& src, vector3d& pointvector, matrix& vtm, float *x, float *y, float *oow)
{
	if (!g_renderer && !*g_renderer)return;

	vector3d viewvect;
	viewvect.MatrixMult(src, vtm);
	viewvect += pointvector;

	float w = 1.0f / viewvect.z;

	*x = viewvect.x * w * (*g_renderer)->m_hpc + (*g_renderer)->m_xoffset;
	*y = viewvect.y * w * (*g_renderer)->m_vpc + (*g_renderer)->m_yoffset;
	*oow = w;
}

void CRoCodeBind::ProjectVertexEx(vector3d& src,vector3d& pointvector, matrix& vtm, tlvertex3d *vert)
{
	if (!g_renderer && !*g_renderer)return;

	vector3d viewvect;
	viewvect.MatrixMult(src, vtm);
	viewvect += pointvector;

	float w = 1.0f / viewvect.z;

	vert->x = viewvect.x * w * (*g_renderer)->m_hpc + (*g_renderer)->m_halfWidth;
	vert->y = viewvect.y * w * (*g_renderer)->m_vpc + (*g_renderer)->m_halfHeight;
	vert->z = (1500 / (1500.0f - 10.0f)) * ((1.0f / w) - 10.0f) * w;
	vert->oow = w;
}


void CRoCodeBind::LoadIni(void)
{
	if( g_pSharedData ){

		int sectionsize;
		char Sectionbuf[32768];
		char *pkey;
		char filename[MAX_PATH];

		if( ::WideCharToMultiByte(CP_ACP,0,
			g_pSharedData->configfilepath,wcslen(g_pSharedData->configfilepath)+1,
			filename,sizeof(filename),
			NULL,NULL) ){

		DEBUG_LOGGING_NORMAL( ("LoadIni startup") );
		DEBUG_LOGGING_NORMAL( ("%s",filename) );

			sectionsize = GetPrivateProfileSection(_T("M2E"),Sectionbuf,32768,filename);
			pkey = Sectionbuf;

			for(int ii = 0;ii < MAX_FLLORSKILLTYPE;ii++)
				m_M2ESkillColor[ii]=0;

			
			while(*pkey!='\0'){
				int index;
				DWORD color;

				char *ptemp;
				ptemp = pkey;

				std::string linestring(ptemp);

				pkey += linestring.length();

				sscanf_s(linestring.c_str(),"Skill%x=%x",&index,&color);
				m_M2ESkillColor[index] = color;
				pkey++;
			}
		}
	}
}

/*
   drawgage sample
   (x,y,  w,h) screenposition size
   value : 0 - 1000
   type != 0 : use sub bg
 */
void CRoCodeBind::DrawGage(LPDIRECT3DDEVICE7 device, int x, int y, int w, int h, unsigned long value, DWORD color, int alpha,int type)
{
	int gage_range;
	DWORD gage_color;
	D3DTLVERTEX vp[6];

	for (int ii = 0; ii < 5; ii++){
		vp[ii].sx = 0.0f;
		vp[ii].sy = 0.0f;
		vp[ii].sz = 0.5f;
		vp[ii].rhw = 1.0f;
		vp[ii].tu = 0.0f;
		vp[ii].tv = 0.0f;
	}

	if (type){
		gage_color = D3DCOLOR_ARGB(0x80, 1, 1, 1);
		for (int ii = 0; ii < 4; ii++){
			vp[ii].color = gage_color;
		}
		vp[0].sx = (D3DVALUE)(x);
		vp[0].sy = (D3DVALUE)(y);
		vp[1].sx = (D3DVALUE)(x + w);
		vp[1].sy = (D3DVALUE)(y);
		vp[2].sx = (D3DVALUE)(x);
		vp[2].sy = (D3DVALUE)(y - 12 - 1);
		vp[3].sx = (D3DVALUE)(x + w);
		vp[3].sy = (D3DVALUE)(y - 12 - 1);
		device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, vp, 4, 0);
	}

	gage_color = D3DCOLOR_ARGB(0x00, 64, 64, 64);
	gage_color = gage_color | (alpha << 24);
	for (int ii = 0; ii < 4; ii++){
		vp[ii].color = gage_color;
	}
	vp[0].sx = (D3DVALUE)(x);
	vp[0].sy = (D3DVALUE)(y);
	vp[1].sx = (D3DVALUE)(x + w - 1);
	vp[1].sy = (D3DVALUE)(y);
	vp[2].sx = (D3DVALUE)(x);
	vp[2].sy = (D3DVALUE)(y + h - 1);
	vp[3].sx = (D3DVALUE)(x + w - 1);
	vp[3].sy = (D3DVALUE)(y + h - 1);
	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, vp, 4, 0);

	gage_color = D3DCOLOR_ARGB(0x00, 16, 24, 152);
	gage_color = gage_color | (alpha << 24);
	for (int ii = 0; ii < 5; ii++){
		vp[ii].color = gage_color;
	}
	vp[0].sx = (D3DVALUE)(x);
	vp[0].sy = (D3DVALUE)(y);
	vp[1].sx = (D3DVALUE)(x + w - 1);
	vp[1].sy = (D3DVALUE)(y);
	vp[2].sx = (D3DVALUE)(x + w - 1);
	vp[2].sy = (D3DVALUE)(y + h - 1);
	vp[3].sx = (D3DVALUE)(x);
	vp[3].sy = (D3DVALUE)(y + h - 1);
	vp[4].sx = vp[0].sx;
	vp[4].sy = vp[0].sy;
	device->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_TLVERTEX, vp, 5, 0);

	gage_color = color | (alpha << 24);

	gage_range = (int)((value*(w - 2)) / 1000);
	for (int ii = 0; ii < 4; ii++){
		vp[ii].color = gage_color;
	}
	vp[0].sx = (D3DVALUE)(x + 1);
	vp[0].sy = (D3DVALUE)(y + 1);
	vp[1].sx = (D3DVALUE)(x + 1 + gage_range);
	vp[1].sy = (D3DVALUE)(y + 1);
	vp[2].sx = (D3DVALUE)(x + 1);
	vp[2].sy = (D3DVALUE)(y + h - 1);
	vp[3].sx = (D3DVALUE)(x + 1 + gage_range);
	vp[3].sy = (D3DVALUE)(y + h - 1);
	device->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, vp, 4, 0);
}
/*
 gage draw sample
 hp: 0 - 1000
 sp: 0 - 1000
 */
void CRoCodeBind::DrawHPSPGage(IDirect3DDevice7 *d3ddev, int x, int y, int hp, int sp)
{
	D3DTLVERTEX vp[6];
	int gage_range;

	vp[0].rhw =
	vp[1].rhw =
	vp[2].rhw =
	vp[3].rhw = 1.0f;

	vp[0].color =
	vp[1].color =
	vp[2].color =
	vp[3].color = D3DCOLOR_ARGB(255, 16, 24, 152);
	//
	vp[0].sz =
	vp[1].sz =
	vp[2].sz =
	vp[3].sz = 0.5f;

	vp[0].sx = (float)(x);
	vp[0].sy = (float)(y);
	vp[0].tu = 0.0f;
	vp[0].tv = 0.0f;
	//
	vp[1].sx = (float)(x + 60);
	vp[1].sy = vp[0].sy;
	vp[1].tu = 0.0f;
	vp[1].tv = 0.0f;
	//
	vp[2].sx = vp[0].sx;
	vp[2].sy = (float)(y + 9);
	vp[2].tu = 0.0f;
	vp[2].tv = 0.0f;
	//
	vp[3].sx = vp[1].sx;
	vp[3].sy = vp[2].sy;
	vp[3].tu = 0.0f;
	vp[3].tv = 0.0f;
	//
	d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, vp, 4, 0);
	//
	for (int ii = 0; ii<2; ii++){
		int value;
		// HP SP
		vp[0].color =
		vp[1].color =
		vp[2].color =
		vp[3].color = D3DCOLOR_ARGB(255, 64, 64, 64);
		//
		vp[0].sx = (float)(x + 1);
		vp[0].sy = (float)(y + 1 + ii * 4);
		vp[1].sx = (float)(x + 1 + 58);
		vp[1].sy = vp[0].sy;
		vp[2].sx = vp[0].sx;
		vp[2].sy = (float)(y + 1 + 3 + ii * 4);
		vp[3].sx = vp[1].sx;
		vp[3].sy = vp[2].sy;
		d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, vp, 4, 0);

		// normal color

		if (ii == 0){
			value = hp;
			if (hp > (1000 / 4) ) {
				// dead zone color
				vp[0].color =
				vp[1].color =
				vp[2].color =
				vp[3].color = D3DCOLOR_ARGB(255, 24, 96, 216);
			} else {
				vp[0].color =
				vp[1].color =
				vp[2].color =
				vp[3].color = D3DCOLOR_ARGB(255, 232, 16, 16);
			}
		} else {
			vp[0].color =
			vp[1].color =
			vp[2].color =
			vp[3].color = D3DCOLOR_ARGB(255, 38, 236, 32);
			value = sp;
		}

		gage_range = (value * 58) / 1000;

		// HP SP
		vp[0].sx = (float)(x + 1);
		vp[0].sy = (float)(y + 1 + ii * 4);
		vp[1].sx = (float)(x + 1 + gage_range);
		vp[1].sy = vp[0].sy;
		vp[2].sx = vp[0].sx;
		vp[2].sy = (float)(y + 1 + 3 + ii * 4);
		vp[3].sx = vp[1].sx;
		vp[3].sy = vp[2].sy;
		d3ddev->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, vp, 4, 0);
	}
}


void CRoCodeBind::DrawSRHDebug(IDirect3DDevice7* d3ddevice)
{
	if( !g_pSharedData )return;
	if( !g_renderer )return;
	if( !*g_renderer )return;

	if (g_pSharedData->show_framerate){
		std::stringstream str;
		str << g_PerformanceCounter.GetFrameRate() << "fps : "<<(int)g_PerformanceCounter.GetTotalTick() << std::endl;

		m_pSFastFont->DrawText((LPSTR)str.str().c_str(), 0, 0,D3DCOLOR_ARGB(255,255,255,255),0,NULL);
	}

	if (g_pSharedData->objectinformation){
		std::stringstream str;
		CModeMgr *pcmode = g_pmodeMgr;
		//str.str("");
		CGameMode *p_gamemode = (CGameMode*)pcmode->m_curMode;

		BackupRenderState(d3ddevice);
		d3ddevice->SetTexture(0, NULL);
		d3ddevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_FALSE);
		d3ddevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		d3ddevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
		d3ddevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
		d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
		d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x00);
		d3ddevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		d3ddevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);


		if( p_gamemode && pcmode->m_curModeType == 1 
		 && p_gamemode->m_world && p_gamemode->m_view && p_gamemode->m_world->m_attr ){
			CView *pView = p_gamemode->m_view;

			C3dAttr *pAttr = p_gamemode->m_world->m_attr;

			if( p_gamemode->m_world->m_player ){
				std::stringstream putinfostr;
				C3dAttr *pattr = p_gamemode->m_world->m_attr;

				CPlayer *pPlayer = (CPlayer*)p_gamemode->m_world->m_player;

				// CRenderObject
				str << "m_BodyAni = "      << (int)pPlayer->m_BodyAni << "\n";
				str << "m_BodyAct = "      << (int)pPlayer->m_BodyAct << "\n";
				str << "m_BodyAniFrame = " << (int)pPlayer->m_BodyAniFrame << "\n";
				str << "m_sprRes = "       << std::hex << pPlayer->m_sprRes << "\n";
				str << "m_actRes = "       << std::hex << pPlayer->m_actRes << "\n\n";
				// CAbleToMakeEffect
				str << "m_efId = "         << pPlayer->m_efId << "\n";
				str << "m_Sk_Level = "     << pPlayer->m_Sk_Level << "\n\n";

				// CGameActor
				str << "m_moveDestX/Y = " << std::dec 
					<< pPlayer->m_moveDestX << ","
					<< pPlayer->m_moveDestY << "\n\n";
				str << "m_speed = " << pPlayer->m_speed << "\n\n";

				str << "m_clevel = " << pPlayer->m_clevel << "\n";
				str << "m_gid = " << std::hex << (unsigned long)pPlayer->m_gid << "\n";
				str << "m_job = " << std::hex << (unsigned long)pPlayer->m_job << "\n";
				str << "m_sex = " << std::hex << (unsigned long)pPlayer->m_sex << "\n\n";

				unsigned char *pdump = (unsigned char*)&pPlayer->m_gid;
				for(int ii = 0;ii < 64 ;ii++){
					str << std::setfill('0') << std::setw(2) << std::hex << (int)pdump[ii] << ",";
					if( (ii % 0x10)==0x0f ){
						str << std::endl;
					}
				}
				str << std::endl;

				matrix *pm = &p_gamemode->m_view->m_viewMatrix;
				str << "m_viewMatrix\n"
					<< pm->v11 << " , "
					<< pm->v12 << " , "
					<< pm->v13 << "\n"
					<< pm->v21 << " , "
					<< pm->v22 << " , "
					<< pm->v23 << "\n"
					<< pm->v31 << " , "
					<< pm->v32 << " , "
					<< pm->v33 << "\n"
					<< pm->v41 << " , "
					<< pm->v42 << " , "
					<< pm->v43 << std::endl;

				str << "CRenderer = " << *g_renderer << "\n"
					<< "m_hpc = " << (*g_renderer)->m_hpc << "\n"
					<< "m_vpc = " << (*g_renderer)->m_vpc << "\n"
					<< "m_hratio = " << (*g_renderer)->m_hratio << "\n"
					<< "m_vratio = " << (*g_renderer)->m_vratio << "\n"
					<< "m_aspectRatio = " << (*g_renderer)->m_aspectRatio << "\n"
					<< "m_screenXFactor = " << (*g_renderer)->m_screenXFactor << "\n"
					<< "m_screenYFactor = " << (*g_renderer)->m_screenYFactor << "\n"
					<< "m_xoffset = " << (*g_renderer)->m_xoffset << "\n"
					<< "m_yoffset = " << (*g_renderer)->m_yoffset << "\n"
					<< "m_width = " << (*g_renderer)->m_width << "\n"
					<< "m_height = " << (*g_renderer)->m_height << "\n"
					<< "m_halfWidth = " << (*g_renderer)->m_halfWidth << "\n"
					<< "m_halfHeight = " << (*g_renderer)->m_halfHeight << "\n"
					<< "m_curFrame = " << (*g_renderer)->m_curFrame << "\n"
					<< "m_bRGBBitCount = " << (*g_renderer)->m_bRGBBitCount << "\n"
					<< "m_fpsFrameCount = " << (*g_renderer)->m_fpsFrameCount << "\n"
					<< "m_dwScreenWidth = " << (*g_renderer)->m_dwScreenWidth << "\n"
					<< "m_dwScreenHeight = " << (*g_renderer)->m_dwScreenHeight << "\n"
					<< std::endl;

				//
				//  world position to cell position
				//
				long cx,cy;
				pattr->ConvertToCellCoor(pPlayer->m_pos.x,pPlayer->m_pos.z,cx,cy);
				//
				//
				//
				putinfostr << "(" << cx << "," << cy << ")" << std::endl;
				int sx,sy;
				float fx,fy,oow;
				ProjectVertex( pPlayer->m_pos,pView->m_viewMatrix,&fx,&fy,&oow);
				sx = (int)fx;
				sy = (int)fy;


				m_pSFastFont->DrawText((LPSTR)putinfostr.str().c_str(), sx, sy,D3DCOLOR_ARGB(255,255,255,255),2,NULL);

				// fake cast bar
				vector3d pointvecter(0, -13.5f, 0);
				ProjectVertexEx(pPlayer->m_pos, pointvecter, pView->m_viewMatrix, &fx, &fy, &oow);
				{
					static int sx = 0, sy = 0;
					sx = (int)fx - 30 + 60; // +60 side of the true cast bar
					sy = (int)fy - 3;
					DrawGage(d3ddevice, sx, sy, 60, 6, 800, D3DCOLOR_ARGB(0x00, 180, 0, 0), 0xff, 0);
				}
				// fake hp sp bar
				pointvecter.Set(0, 2.5, 0);
				ProjectVertexEx(pPlayer->m_pos, pointvecter, pView->m_viewMatrix, &fx, &fy, &oow);
				{
					static int sx = 0, sy = 0;
					sx = (int)fx - 30 + 60; // +60 side of the true hpsp bar
					sy = (int)fy - 6;
					DrawHPSPGage(d3ddevice, sx, sy, 200, 900);
				}
			}

			int skillnums = p_gamemode->m_world->m_skillList.size();
			str << " m_skillList size =" << skillnums << "\n";

			int itemnums = p_gamemode->m_world->m_itemList.size();
			str << " m_itemList size =" << itemnums << "\n";

			std::list<CItem*> itemList = p_gamemode->m_world->m_itemList;
			for( std::list<CItem*>::iterator it = itemList.begin() ; it != itemList.end() ; it++ )
			{
				CItem *pItem = *it;
				if( pItem ){
					long cx,cy;
					pAttr->ConvertToCellCoor(pItem->m_pos.x,pItem->m_pos.z,cx,cy);

					std::stringstream putinfostr;
					putinfostr << "(" << cx << "," << cy << ")" << std::endl;
					//putinfostr << pItem->m_itemName << std::endl;
					//putinfostr << "aid = " << pItem->m_aid << std::endl;
					putinfostr << "itemid = " << pItem->m_itemid2 << std::endl;

					int sx,sy;
					float fx,fy,oow;
					ProjectVertex( pItem->m_pos,pView->m_viewMatrix,&fx,&fy,&oow);
					sx = (int)fx;
					sy = (int)fy;

					m_pSFastFont->DrawText((LPSTR)putinfostr.str().c_str(), sx, sy,D3DCOLOR_ARGB(255,255,255,255),2,NULL);
				}
			}

//			if (g_pSharedData->test01){
				int actornums = p_gamemode->m_world->m_actorList.size();
				str << " m_actorList size =" << actornums << "\n";
				std::list<CGameActor*> actorList = p_gamemode->m_world->m_actorList;
				for (std::list<CGameActor*>::iterator it = actorList.begin(); it != actorList.end(); it++)
				{
					CGameActor *pGameActor = *it;
					if (pGameActor){
						long cx, cy;
						pAttr->ConvertToCellCoor(pGameActor->m_pos.x, pGameActor->m_pos.z, cx, cy);

						std::stringstream putinfostr;
						putinfostr << "(" << cx << "," << cy << ")" << std::endl;
						putinfostr << std::setfill('0') << std::setw(8) << std::hex << pGameActor->m_gid << std::endl;

						//	putinfostr << "dest(" << pGameActor->m_moveDestX << "," << pGameActor->m_moveDestY << ")" << std::endl;

						int sx, sy;
						float fx, fy, oow;
						ProjectVertex(pGameActor->m_pos, pView->m_viewMatrix, &fx, &fy, &oow);
						sx = (int)fx;
						sy = (int)fy;

						m_pSFastFont->DrawText((LPSTR)putinfostr.str().c_str(), sx, sy, D3DCOLOR_ARGB(255, 255, 255, 255), 2, NULL);
					}
				}
//			}
			
		}
		str << std::endl;
		m_pSFastFont->DrawText((LPSTR)str.str().c_str(), 0, 16,D3DCOLOR_ARGB(255,255,255,255),0,NULL);

		RestoreRenderState(d3ddevice);
	}
	m_pSFastFont->Flush();

}

void CRoCodeBind::BackupRenderState(IDirect3DDevice7* d3ddevice)
{
	d3ddevice->GetRenderState(D3DRENDERSTATE_ZENABLE, &_state_zenable);
	d3ddevice->GetRenderState(D3DRENDERSTATE_ZWRITEENABLE, &_state_zwriteenable);
	d3ddevice->GetRenderState(D3DRENDERSTATE_ZBIAS, &_state_zbias);
	d3ddevice->GetRenderState(D3DRENDERSTATE_FOGENABLE, &_state_fogenable);
	d3ddevice->GetRenderState(D3DRENDERSTATE_SPECULARENABLE, &_state_specularenable);
	d3ddevice->GetRenderState(D3DRENDERSTATE_ALPHAFUNC, &_state_alphafunc);
	d3ddevice->GetRenderState(D3DRENDERSTATE_ALPHAREF, &_state_alpharef);
	d3ddevice->GetRenderState(D3DRENDERSTATE_SRCBLEND, &_state_srcblend);
	d3ddevice->GetRenderState(D3DRENDERSTATE_DESTBLEND, &_state_destblend);
}

void CRoCodeBind::RestoreRenderState(IDirect3DDevice7* d3ddevice)
{
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZENABLE, _state_zenable);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, _state_zwriteenable);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZBIAS, _state_zbias);
	d3ddevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, _state_specularenable);
	d3ddevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, _state_specularenable);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, _state_alphafunc);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, _state_alpharef);
	d3ddevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, _state_srcblend);
	d3ddevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, _state_destblend);
}

void CRoCodeBind::DrawOn3DMap(IDirect3DDevice7* d3ddevice)
{
	if (!g_pSharedData)return;
	if (!g_pmodeMgr)return;

	if (g_pmodeMgr->m_curMode && g_pmodeMgr->m_curModeType == 1){
		CGameMode *pGamemode = (CGameMode*)g_pmodeMgr->m_curMode;
		if (!pGamemode->m_world || !pGamemode->m_view)return;
		if (!pGamemode->m_world->m_player)return;

		BackupRenderState(d3ddevice);

		DrawM2E(d3ddevice);
		DrawBBE(d3ddevice);
		DrawST(d3ddevice);

		RestoreRenderState(d3ddevice);
	}
}

void CRoCodeBind::DrawM2E(IDirect3DDevice7* d3ddevice)
{
	if( g_pSharedData->m2e == FALSE )return;

	CGameMode *pGamemode = (CGameMode*)g_pmodeMgr->m_curMode;
	CWorld *pWorld = pGamemode->m_world;
	CView *pView = pGamemode->m_view;

	int zbias = g_pSharedData->ground_zbias;

	d3ddevice->SetTexture(0, NULL);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZBIAS, zbias);
	d3ddevice->SetRenderState(D3DRENDERSTATE_FOGENABLE    ,FALSE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE   ,FALSE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC,D3DCMP_GREATER);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAREF,0x00);
	d3ddevice->SetRenderState(D3DRENDERSTATE_SRCBLEND ,D3DBLEND_SRCALPHA );
	d3ddevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);

	if( pWorld && pView && pWorld->m_attr ){
		C3dAttr *pAttr = pWorld->m_attr;

		int skillnums = pWorld->m_skillList.size();
		std::list<CSkill*> skillList = pWorld->m_skillList;

		for( std::list<CSkill*>::iterator it = skillList.begin() ; it != skillList.end() ; it++ )
		{
			CSkill *pSkill = *it;

			if( pSkill && pSkill->m_job < 0x100 && m_M2ESkillColor[pSkill->m_job] ){
				DWORD color = m_M2ESkillColor[pSkill->m_job];
				//
				CPOLVERTEX vertex[4] =
				{
					{   0.0,  0.0,   0.0f,  1.0f, color },
					{ 100.0,  0.0,   0.0f,  1.0f, color },
					{   0.0,100.0,   0.0f,  1.0f, color },
					{ 100.0,100.0,   0.0f,  1.0f, color }
				};

				long cx,cy;
				CAttrCell *pCell;
				vector3d centerpos(pSkill->m_pos),polvect[4];

				pAttr->ConvertToCellCoor(centerpos.x,centerpos.z,cx,cy);
				pCell = pAttr->GetCell(cx, cy);
				polvect[0].Set(centerpos.x -2.4f, pCell->h1 ,centerpos.z -2.4f);
				polvect[1].Set(centerpos.x +2.4f, pCell->h2 ,centerpos.z -2.4f);
				polvect[2].Set(centerpos.x -2.4f, pCell->h3 ,centerpos.z +2.4f);
				polvect[3].Set(centerpos.x +2.4f, pCell->h4 ,centerpos.z +2.4f);

				for(int ii = 0; ii < 4; ii ++){
					tlvertex3d tlv3d;
					ProjectVertex( polvect[ii] , pView->m_viewMatrix,&tlv3d );

					vertex[ii].x = tlv3d.x;
					vertex[ii].y = tlv3d.y;
					vertex[ii].z = tlv3d.z;
					vertex[ii].rhw = tlv3d.oow;
				}
				d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_CPOLVERTEX, vertex, 4, 0);
			}
		}
	}
}

void CRoCodeBind::DrawBBE(IDirect3DDevice7* d3ddevice)
{
	CGameMode *pGamemode = (CGameMode*)g_pmodeMgr->m_curMode;
	CWorld *pWorld = pGamemode->m_world;
	CView *pView = pGamemode->m_view;

	int zbias = g_pSharedData->ground_zbias;
	int alphalevel = g_pSharedData->alphalevel;
	BOOL bbe = g_pSharedData->bbe;
	BOOL deadcell = g_pSharedData->deadcell;
	BOOL chatscope = g_pSharedData->chatscope;

	d3ddevice->SetTexture(0, NULL);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZBIAS, zbias);
	d3ddevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x00);
	d3ddevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	d3ddevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

	if (pWorld && pView && pWorld->m_attr){
		C3dAttr *pAttr = pWorld->m_attr;
		CPlayer *pPlayer = pGamemode->m_world->m_player;
		CPOLVERTEX vertex[4] =
		{
			{ 0.0, 0.0, 0.0f, 1.0f, 0 },
			{ 100.0, 0.0, 0.0f, 1.0f, 0 },
			{ 0.0, 100.0, 0.0f, 1.0f, 0 },
			{ 100.0, 100.0, 0.0f, 1.0f, 0 }
		};
		long cx, cy,wx,wy;

		pAttr->ConvertToCellCoor(
			pPlayer->m_pos.x,pPlayer->m_pos.z,
			cx, cy);
		wx = 20;
		wy = 20;

		for (int yy = cy - wy; yy <= cy + wy; yy++) {
			for (int xx = cx - wx; xx <= cx + wx; xx++) {
				if (xx >= 0 && yy >= 0 && xx < pAttr->m_width && yy < pAttr->m_height){
					DWORD color = 0;
					CAttrCell *pCell = pAttr->GetCell(xx, yy);

					if (!pCell->flag && bbe)
					{
						if ((xx % 40 == 0) || (yy % 40 == 0)) {
							color = 0x00FF0000;
						}
						else if ((xx % 40 < 5) || (yy % 40 < 5)) {
							color = 0x000000FF;
						}
					}
					if (pCell->flag){
						if (deadcell){
							color = 0x00ff00ff;
						}
					}

					if (chatscope){
						if ((xx - cx) >= -9 && (xx - cx) <= +9 && ((yy - cy) == -9 || (yy - cy) == +9)
							|| (yy - cy) >= -9 && (yy - cy) <= +9 && ((xx - cx) == -9 || (xx - cx) == +9)){
							color = 0x0000ff00;
						}
					}

					if (color) {
						vector3d polvect[4];
						vector2d wpos;

						color += alphalevel << 24;
						pAttr->GetWorldPos((float)xx, (float)yy, wpos);

						polvect[0].Set(wpos.x - 2.5f, pCell->h1, wpos.y - 2.5f);
						polvect[1].Set(wpos.x + 2.5f, pCell->h2, wpos.y - 2.5f);
						polvect[2].Set(wpos.x - 2.5f, pCell->h3, wpos.y + 2.5f);
						polvect[3].Set(wpos.x + 2.5f, pCell->h4, wpos.y + 2.5f);

						for (int ii = 0; ii < 4; ii++){
							tlvertex3d tlv3d;
							ProjectVertex(polvect[ii], pView->m_viewMatrix, &tlv3d);

							vertex[ii].x = tlv3d.x;
							vertex[ii].y = tlv3d.y;
							vertex[ii].z = tlv3d.z;
							vertex[ii].rhw = tlv3d.oow;
							vertex[ii].color = color;
						}
						d3ddevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_CPOLVERTEX, vertex, 4, 0);
					}
				}
			}
		}
	}
}

void CRoCodeBind::DrawST(IDirect3DDevice7* d3ddevice){
	if (g_pSharedData->test02 == FALSE)return;
	CGameMode *pGamemode = (CGameMode*)g_pmodeMgr->m_curMode;
	CWorld *pWorld = pGamemode->m_world;
	CView *pView = pGamemode->m_view;
	int zbias = g_pSharedData->ground_zbias;

	d3ddevice->SetTexture(0, NULL);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_TRUE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ZBIAS, zbias);
	d3ddevice->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);
	d3ddevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x00);
	d3ddevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	d3ddevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	//-----skill timer------
	int actornums = pGamemode->m_world->m_actorList.size();
	std::list<CGameActor*> actorList = pGamemode->m_world->m_actorList;
	std::list<s_skill_tm> temp_tmList = tmList;
	//my
	CPlayer *pPlayer = (CPlayer*)pGamemode->m_world->m_player;
	const float BARB = -17.5f;
	const int NAMEBX = -10;
	const int NAMEBY =  -9;
	const int TIMEBX =  80;
	const int TIMEBY =   3;
	const float SB   =  -1.8f;
	float yy = BARB;
	for (std::list<s_skill_tm>::iterator it = temp_tmList.begin(); it != temp_tmList.end(); it++){
		if (pPlayer->m_gid == it->id){
			if (it->e_tick > timeGetTime()){
				vector3d pointvecter;
				float fx, fy, oow;
				std::stringstream putname;
				std::stringstream puttime;
				pointvecter.Set(0, yy, 0);
				ProjectVertex(pPlayer->m_pos, pView->m_viewMatrix, &fx, &fy, &oow);
				int sx, sy;
				sx = (int)fx;
				sy = (int)fy;
				unsigned long value;
				float t1, t2;
				int tm, ts;
				t1 = (float)(it->e_tick - timeGetTime());
				t2 = (float)(it->tick);
				tm = ((int)t1 / 1000) / 60;
				ts = ((int)t1 / 1000) % 60;
				value = (unsigned long)((t1 / t2) * 1000);

				ProjectVertexEx(pPlayer->m_pos, pointvecter, pView->m_viewMatrix, &fx, &fy, &oow);
				{
					static int sx = 0, sy = 0;
					sx = (int)fx - 30; // +60 side of the true cast bar
					sy = (int)fy - 3;
					DrawGage(d3ddevice, sx, sy, 60, 6, value, D3DCOLOR_ARGB(0x00, 170, 170, 220), 0x96, 0);
					
					putname << it->name;
					if (tm > 0){
						puttime << std::setfill(' ') << std::setw(3) << tm << "'";
						puttime << std::setfill('0') << std::setw(2) << ts << "''";
					}
					if (tm <= 0){
						puttime << std::setfill(' ') << std::setw(4) << " ";
						puttime << std::setfill(' ') << std::setw(2) << ts << "''";
					}
					
					sx += NAMEBX;
					sy += NAMEBY;
					m_pSFastFont->DrawText((LPSTR)putname.str().c_str(), sx, sy, D3DCOLOR_ARGB(230, 255,255,255), 0, NULL);
					sx += TIMEBX;
					sy += TIMEBY;
					m_pSFastFont->DrawText((LPSTR)puttime.str().c_str(), sx, sy, D3DCOLOR_ARGB(150, 255, 255, 255), 1, NULL);


				}
				yy += SB;
			}
		}
	}

	//other
	for (std::list<CGameActor*>::iterator it = actorList.begin(); it != actorList.end(); it++)
	{
		float yy = BARB;
		CGameActor *pGameActor = *it;
		if (pGameActor){
			for (std::list<s_skill_tm>::iterator it = temp_tmList.begin(); it != temp_tmList.end(); it++){
				if (pGameActor->m_gid == it->id){
					if (it->e_tick > timeGetTime()){
						vector3d pointvecter;
						float fx, fy, oow;
						pointvecter.Set(0, yy, 0);
						ProjectVertex(pGameActor->m_pos, pView->m_viewMatrix, &fx, &fy, &oow);
						int sx, sy;
						std::stringstream putname;
						std::stringstream puttime;
						sx = (int)fx;
						sy = (int)fy;
						unsigned long value;
						float t1, t2;
						int tm, ts;
						t1 = (float)(it->e_tick - timeGetTime());
						t2 = (float)(it->tick);
						tm = ((int)t1 / 1000) / 60;
						ts = ((int)t1 / 1000) % 60;
						value = (unsigned long)((t1 / t2) * 1000);

						ProjectVertexEx(pGameActor->m_pos, pointvecter, pView->m_viewMatrix, &fx, &fy, &oow);
						{
							static int sx = 0, sy = 0;
							sx = (int)fx - 30; // +60 side of the true cast bar
							sy = (int)fy - 3;
							DrawGage(d3ddevice, sx, sy, 60, 6, value, D3DCOLOR_ARGB(0x00, 170, 170, 220), 0x96, 0);
							
							putname << it->name;
							if (tm > 0){
								puttime << std::setfill(' ') << std::setw(3) << tm << "'";
								puttime << std::setfill('0') << std::setw(2) << ts << "''";
							}
							if (tm <= 0){
								puttime << std::setfill(' ') << std::setw(4) << " ";
								puttime << std::setfill(' ') << std::setw(2) << ts << "''";
							}
							sx += NAMEBX;
							sy += NAMEBY;
							m_pSFastFont->DrawText((LPSTR)putname.str().c_str(), sx, sy, D3DCOLOR_ARGB(230, 255, 255, 255), 0, NULL);
							sx += TIMEBX;
							sy += TIMEBY;
							m_pSFastFont->DrawText((LPSTR)puttime.str().c_str(), sx, sy, D3DCOLOR_ARGB(150, 255, 255, 255), 1, NULL);


						}
						yy += SB;
					}

				}
			}
		}
	}
	//-----Delay-------
	yy = 4.0f;
	std::list<DELAY> tempList = dList;
	for (std::list<DELAY>::iterator it = tempList.begin(); it != tempList.end();it++){
		if (it->etick > timeGetTime()){
			vector3d pointvecter;
			float fx, fy, oow;
			pointvecter.Set(0, yy, 0);
			ProjectVertex(pPlayer->m_pos, pView->m_viewMatrix, &fx, &fy, &oow);
			int sx, sy;
			sx = (int)fx;
			sy = (int)fy;
			unsigned long value;
			float t1, t2;
			t1 = (float)it->etick - timeGetTime();
			t2 = (float)it->tick;
			value = (unsigned long)((t1 / t2) * 1000);

			ProjectVertexEx(pPlayer->m_pos, pointvecter, pView->m_viewMatrix, &fx, &fy, &oow);
			{
				static int sx = 0, sy = 0;
				sx = (int)fx - 30; // +60 side of the true cast bar
				sy = (int)fy - 3;
				DrawGage(d3ddevice, sx, sy, 60, 6, value, D3DCOLOR_ARGB(0x00, 180, 0, 0), 0xC8, 0);
			}
			yy += 1.0f;
		}
	}
	m_pSFastFont->Flush();
	//list erase
	for (std::list<DELAY>::iterator it = dList.begin(); it != dList.end();){
		if (it->etick < timeGetTime()){
			it = dList.erase(it);
		}
		it++;
	}
	for (std::list<s_skill_tm>::iterator it = tmList.begin(); it != tmList.end();){
		if (it->e_tick < timeGetTime()){
			it = tmList.erase(it);
		}
		it++;
	}


}

 int CRoCodeBind::GetPacketLength(int opcode)
{
	int result = -1;

	if( opcode < m_packetLenMap_table_index )
	{
		result = m_packetLenMap_table[ opcode ];
	}
	return result;
}

int CRoCodeBind::GetTreeData(p_std_map_packetlen* node)
{
	if ( node->parent == NULL || node->key >= ROPACKET_MAXLEN || node->key == 0)
		return m_packetLenMap_table_index;

	GetTreeData( node->left );

	if( m_packetLenMap_table_index < (int)node->key )
		 m_packetLenMap_table_index = node->key;

	m_packetLenMap_table[ node->key ] = node->value;
	//DEBUG_LOGGING_DETAIL( ("[ %08X ] = %d",node->key,node->value) );

	GetTreeData( node->right );

	return m_packetLenMap_table_index;
}

void CRoCodeBind::InitPacketHandler(void)
{
	m_packethandler[HEADER_ZC_SAY_DIALOG] = &CRoCodeBind::PacketHandler_Cz_Say_Dialog;
	m_packethandler[HEADER_ZC_MENU_LIST] = &CRoCodeBind::PacketHandler_Cz_Menu_List;
	m_packethandler[HEADER_ZC_SKILL_POSTDELAY] = &CRoCodeBind::PacketHandler_Cz_SKILL_POSTDELAY;
	m_packethandler[HEADER_ZC_MSG_STATE_CHANGE2] = &CRoCodeBind::PacketHandler_Cz_MSG_STATE_CHANGE2;
	m_packethandler[HEADER_ZC_MSG_STATE_CHANGE] = &CRoCodeBind::PacketHandler_Cz_MSG_STATE_CHANGE;
	m_packethandler[HEADER_ZC_NOTIFY_EFFECT2] = &CRoCodeBind::PacketHandler_Cz_NOTIFY_EFFECT2;

}

void CRoCodeBind::PacketProc(const char *packetdata)
{
	unsigned short opcode = *(unsigned short*)packetdata;
	unsigned int packetlength;
	packetlength = GetPacketLength(opcode);
	if (m_packetqueue_head < 4)return; // packet is illegal
	if (packetlength == -1){
		packetlength = *(unsigned int*)packetdata;
		packetlength >>= 16;
	}

	// switch packet handler
	if (m_packethandler[opcode])
		// call to packetproc function.( CRoCodeBind::PacketHandler_...(const char *packetdata) )
		(this->*m_packethandler[opcode])(packetdata);

	// output packet log
	if (g_pSharedData && g_pSharedData->write_packetlog){
		if (opcode == HEADER_ZC_NOTIFY_EFFECT2 || opcode == HEADER_ZC_PAR_CHANGE){
			std::stringstream str;
			str << "[" << std::setfill('0') << std::setw(8) << timeGetTime() << "]R ";
			for (int ii = 0; ii < (int)packetlength; ii++)
				str << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << (int)(unsigned char)packetdata[ii] << " ";
			str << std::flush;
			DEBUG_LOGGING_NORMAL((str.str().c_str()));
		}
	}
}

void CRoCodeBind::SendMessageToNPCLogger(const char *src, int size)
{
	char buffer[512];
	char *dst = buffer;
	while (size){
		if (*src == '^'){
			//
			if (src[1] == 'n'){
				if (src[2] == 'I'
				 && src[3] == 't'
				 && src[4] == 'e'
				 && src[5] == 'm'
				 && src[6] == 'I'
				 && src[7] == 'D'
				 && src[8] == '^'){
					//
					src += 9;
					size -= 9;
					//
					char numstr[10];
					unsigned char *pname;
					pname = (unsigned char *)numstr;
					while (*src >= '0' && *src <= '9'){
						*pname++ = *src++;
						size--;
					}
					*pname++ = '\0';
					int itemid;
					itemid = atoi(numstr);
					const char *itemname = GetItemNameByID(itemid);
					while (*itemname != '\0')*dst++ = *itemname++;
				}
			}
			else{
				*dst++ = *src++;
				size--;
			}
		}
		else{
			int c;
			int mbcode;
			c = (unsigned char)*src++;
			mbcode = c << 8 | (unsigned char)*src;
			if (_ismbclegal(mbcode)){
				*dst++ = c;
				c = *src++;
				size--;
			}
			*dst++ = c;
			size--;
		}
	}
	*dst++ = '\0';

	WCHAR wbuffer[512];
	int wsize = 0;
	if ((wsize = ::MultiByteToWideChar(CP_ACP, 0, buffer, -1, wbuffer, sizeof(wbuffer) / sizeof(WCHAR))) != NULL){
		//DEBUG_LOGGING_NORMAL(("sizeof wbuffer = %d wsize = %d", sizeof(wbuffer),wsize));
		HWND hNPCLoggerWnd;
		hNPCLoggerWnd = ::FindWindow(NULL, "NPCLogger");
		if (hNPCLoggerWnd){
			COPYDATASTRUCT data;
			data.dwData = COPYDATA_NPCLogger;
			data.cbData = wsize * sizeof(WCHAR);
			data.lpData = wbuffer;
			::SendMessage(hNPCLoggerWnd, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&data);
		}
	}
}

void CRoCodeBind::PacketHandler_Cz_Say_Dialog(const char *packetdata)
{
	PACKET_CZ_SAY_DIALOG *p = (PACKET_CZ_SAY_DIALOG*)packetdata;
	int size = p->PacketLength - sizeof(PACKET_CZ_SAY_DIALOG);

	SendMessageToNPCLogger((const char*)p->Data, size);
}
void CRoCodeBind::PacketHandler_Cz_Menu_List(const char *packetdata)
{
	PACKET_CZ_MENU_LIST *p = (PACKET_CZ_MENU_LIST*)packetdata;
	int size = p->PacketLength - sizeof(PACKET_CZ_SAY_DIALOG);
	char buffer[512];
	const char *src = (const char*)p->Data;
	int index = 0;

	while (size){
		int mbcode;
		int dlength;
		char *dst = buffer;

		std::stringstream answerhead;
		answerhead << '[' << (1 + index) << "] " << std::flush;

		dlength = answerhead.str().length();

		memcpy(dst, answerhead.str().c_str(), dlength);
		dst += dlength;


		while (size && *src != ':' && *src != '\0'){
			int c;
			c = (unsigned char)*src++;
			// mbcode
			mbcode = c << 8 | (unsigned char)*src;
			if (_ismbclegal(mbcode)){
				*dst++ = c;
				c = *src++;
				size--;
				dlength++;
			}
			*dst++ = c;
			size--;
			dlength++;
		}
		if (*src == ':'){
			src++;
			size--;
		}
		*dst++ = '\0';
		if (dlength <= 5)break;


		SendMessageToNPCLogger((const char*)buffer, dlength);
		index++;
	}

}
void CRoCodeBind::PacketHandler_Cz_SKILL_POSTDELAY(const char *packetdata)
{
	if (g_pSharedData->test02 == FALSE)return;
	PACKET_CZ_SKILL_POSTDELAY* p = (PACKET_CZ_SKILL_POSTDELAY*)packetdata;
	DELAY d;
	d.PacketType=p->PacketType;
	d.skill_id=p->skill_id;
	d.tick=p->tick;
	d.etick = p->tick + timeGetTime();
	dList.push_back(d);

}
void CRoCodeBind::PacketHandler_Cz_MSG_STATE_CHANGE2(const char *packetdata){
	if (g_pSharedData->test02 == FALSE)return;
	PACKET_CZ_MSG_STATE_CHANGE2* p = (PACKET_CZ_MSG_STATE_CHANGE2*)packetdata;
	std::stringstream str;
	s_skill_tm tm;
	tm.type = p->type;
	tm.id = p->id;
	tm.tick = p->tick;
	tm.e_tick = p->tick + timeGetTime();
	tm.v1 = p->v1;

	for (std::list<s_skill_tm>::iterator it = tmList.begin(); it != tmList.end();){
		if (it->id == tm.id && it->type == tm.type){
			it = tmList.erase(it);
		}
		it++;
	}

	if (p->flag == 1){
		switch (tm.type)
		{
		case SI_BLANK:
			break;
		case SI_PROVOKE:
			break;
		case SI_ENDURE:
			str << "Endure";
			tm.name = str.str();
			tmList.push_back(tm);
			break;
		case SI_ENERGYCOAT:
			break;
		case SI_LOUD:
			break;
		case SI_ANGELUS:
			str << "Ange";
			str << tm.v1;
			tm.name = str.str();
			tmList.push_back(tm);
			break;
		case SI_BLESSING:
			str << "Bless";
			str << tm.v1;
			tm.name = str.str();
			tmList.push_back(tm);
			break;
		case SI_SIGNUMCRUCIS:
			break;
		case SI_INCREASEAGI:
			str << "IA";
			str << tm.v1;
			tm.name = str.str();

			tmList.push_back(tm);
			break;
		case SI_DECREASEAGI:
			break;
		case SI_SLOWPOISON:
			break;
		case SI_HIDING:
			break;
		case SI_TWOHANDQUICKEN:
			break;
		case SI_RIDING:
			break;
		case SI_AUTOCOUNTER:
			break;
		case SI_QUAGMIRE:
			break;
		case SI_ADRENALINE:
			break;
		case SI_WEAPONPERFECTION:
			break;
		case SI_OVERTHRUST:
			break;
		case SI_MAXIMIZEPOWER:
			break;
		case SI_IMPOSITIO:
			break;
		case SI_SUFFRAGIUM:
			str << "Suffra";
			str << tm.v1;
			tm.name = str.str();

			tmList.push_back(tm);
			break;
		case SI_ASPERSIO:
			str << "Aspe";
			str << tm.v1;
			tm.name = str.str();

			tmList.push_back(tm);
			break;
		case SI_BENEDICTIO:
			break;
		case SI_KYRIE:
			str << "Kyrie";
			str << "[" << tm.v1 << "]";
			tm.name = str.str();

			tmList.push_back(tm);
			break;
		case SI_MAGNIFICAT:
			str << "Magni";
			str << tm.v1;
			tm.name = str.str();

			tmList.push_back(tm);
			break;
		case SI_GLORIA:
			str << "Gloria";
			str << tm.v1;
			tm.name = str.str();

			tmList.push_back(tm);
			break;
		case SI_AETERNA:
			break;
		case SI_CLOAKING:
			break;
		case SI_ENCPOISON:
			break;
		case SI_POISONREACT:
			break;
		case SI_FALCON:
			break;
		case SI_CONCENTRATE:
			break;
		case SI_TRICKDEAD:
			break;
		case SI_BROKENARMOR:
			break;
		case SI_BROKENWEAPON:
			break;
		case SI_HALLUCINATION:
			break;
		case SI_WEIGHT50:
			break;
		case SI_WEIGHT90:
			break;
		case SI_ASPDPOTION0:
			break;
		case SI_ASPDPOTION1:
			break;
		case SI_ASPDPOTION2:
			break;
		case SI_ASPDPOTIONINFINITY:
			break;
		case SI_SPEEDPOTION1:
			break;
		case SI_SPLASHER:
			break;
		case SI_ANKLESNARE:
			break;
		case SI_ACTIONDELAY:
			break;
		case SI_BARRIER:
			break;
		case SI_STRIPWEAPON:
			break;
		case SI_STRIPSHIELD:
			break;
		case SI_STRIPARMOR:
			break;
		case SI_STRIPHELM:
			break;
		case SI_CP_WEAPON:
			break;
		case SI_CP_SHIELD:
			break;
		case SI_CP_ARMOR:
			break;
		case SI_CP_HELM:
			break;
		case SI_AUTOGUARD:
			break;
		case SI_REFLECTSHIELD:
			break;
		case SI_DEVOTION:
			break;
		case SI_PROVIDENCE:
			break;
		case SI_DEFENDER:
			break;
		case SI_MAGICROD:
			break;
		case SI_AUTOSPELL:
			break;
		case SI_SPEARQUICKEN:
			break;
		case SI_BDPLAYING:
			break;
		case SI_WHISTLE:
			break;
		case SI_ASSASSINCROSS:
			break;
		case SI_POEMBRAGI:
			break;
		case SI_APPLEIDUN:
			break;
		case SI_HUMMING:
			break;
		case SI_DONTFORGETME:
			break;
		case SI_FORTUNEKISS:
			break;
		case SI_SERVICEFORYOU:
			break;
		case SI_RICHMANKIM:
			break;
		case SI_ETERNALCHAOS:
			break;
		case SI_DRUMBATTLEFIELD:
			break;
		case SI_RINGNIBELUNGEN:
			break;
		case SI_ROKISWEIL:
			break;
		case SI_INTOABYSS:
			break;
		case SI_SIEGFRIED:
			break;
		case SI_BLADESTOP:
			break;
		case SI_EXPLOSIONSPIRITS:
			break;
		case SI_STEELBODY:
			break;
		case SI_EXTREMITYFIST:
			break;
		case SI_FIREWEAPON:
			break;
		case SI_WATERWEAPON:
			break;
		case SI_WINDWEAPON:
			break;
		case SI_EARTHWEAPON:
			break;
		case SI_STOP:
			break;
		case SI_UNDEAD:
			break;
		case SI_AURABLADE:
			break;
		case SI_PARRYING:
			break;
		case SI_CONCENTRATION:
			break;
		case SI_TENSIONRELAX:
			break;
		case SI_BERSERK:
			break;
		case SI_GOSPEL:
			break;
		case SI_ASSUMPTIO:
			break;
		case SI_BASILICA:
			break;
		case SI_LANDENDOW:
			break;
		case SI_MAGICPOWER:
			str << "Magni";
			str << tm.v1;
			tm.name = str.str();
			tmList.push_back(tm);
			break;
		case SI_EDP:
			break;
		case SI_TRUESIGHT:
			break;
		case SI_WINDWALK:
			break;
		case SI_MELTDOWN:
			break;
		case SI_CARTBOOST:
			break;
		case SI_CHASEWALK:
			break;
		case SI_REJECTSWORD:
			break;
		case SI_MARIONETTE:
			break;
		case SI_MARIONETTE2:
			break;
		case SI_MOONLIT:
			break;
		case SI_BLEEDING:
			break;
		case SI_JOINTBEAT:
			break;
		case SI_MINDBREAKER:
			break;
		case SI_MEMORIZE:
			break;
		case SI_FOGWALL:
			break;
		case SI_SPIDERWEB:
			break;
		case SI_BABY:
			break;
		case SI_AUTOBERSERK:
			break;
		case SI_RUN:
			break;
		case SI_BUMP:
			break;
		case SI_READYSTORM:
			break;
		case SI_READYDOWN:
			break;
		case SI_READYTURN:
			break;
		case SI_READYCOUNTER:
			break;
		case SI_DODGE:
			break;
		case SI_SPURT:
			break;
		case SI_SHADOWWEAPON:
			break;
		case SI_ADRENALINE2:
			break;
		case SI_GHOSTWEAPON:
			break;
		case SI_SPIRIT:
			break;
		case SI_PLUSATTACKPOWER:
			break;
		case SI_PLUSMAGICPOWER:
			break;
		case SI_DEVIL:
			break;
		case SI_KAITE:
			break;
		case SI_SWOO:
			break;
		case SI_KAIZEL:
			break;
		case SI_KAAHI:
			break;
		case SI_KAUPE:
			break;
		case SI_SMA:
			break;
		case SI_NIGHT:
			break;
		case SI_ONEHAND:
			break;
		case SI_WARM:
			break;
		case SI_SUN_COMFORT:
			break;
		case SI_MOON_COMFORT:
			break;
		case SI_STAR_COMFORT:
			break;
		case SI_GDSKILL_BATTLEORDER:
			break;
		case SI_GDSKILL_REGENERATION:
			break;
		case SI_PRESERVE:
			break;
		case SI_CHASEWALK2:
			break;
		case SI_INTRAVISION:
			break;
		case SI_DOUBLECAST:
			break;
		case SI_GRAVITATION:
			break;
		case SI_MAXOVERTHRUST:
			break;
		case SI_LONGING:
			break;
		case SI_HERMODE:
			break;
		case SI_TAROT:
			break;
		case SI_SHRINK:
			break;
		case SI_SIGHTBLASTER:
			break;
		case SI_WINKCHARM:
			break;
		case SI_CLOSECONFINE:
			break;
		case SI_CLOSECONFINE2:
			break;
		case SI_MADNESSCANCEL:
			break;
		case SI_GATLINGFEVER:
			break;
		case SI_EARTHSCROLL:
			break;
		case SI_UTSUSEMI:
			break;
		case SI_BUNSINJYUTSU:
			break;
		case SI_NEN:
			break;
		case SI_ADJUSTMENT:
			break;
		case SI_ACCURACY:
			break;
		case SI_FOODSTR:
			break;
		case SI_FOODAGI:
			break;
		case SI_FOODVIT:
			break;
		case SI_FOODDEX:
			break;
		case SI_FOODINT:
			break;
		case SI_FOODLUK:
			break;
		case SI_FOODFLEE:
			break;
		case SI_FOODHIT:
			break;
		case SI_FOODCRI:
			break;
		case SI_EXPBOOST:
			break;
		case SI_LIFEINSURANCE:
			break;
		case SI_ITEMBOOST:
			break;
		case SI_BOSSMAPINFO:
			break;
		case SI_FOOD_STR_CASH:
			break;
		case SI_FOOD_AGI_CASH:
			break;
		case SI_FOOD_VIT_CASH:
			break;
		case SI_FOOD_DEX_CASH:
			break;
		case SI_FOOD_INT_CASH:
			break;
		case SI_FOOD_LUK_CASH:
			break;
		case SI_MERC_FLEEUP:
			break;
		case SI_MERC_ATKUP:
			break;
		case SI_MERC_HPUP:
			break;
		case SI_MERC_SPUP:
			break;
		case SI_MERC_HITUP:
			break;
		case SI_SLOWCAST:
			break;
		case SI_CRITICALWOUND:
			break;
		case SI_MOVHASTE_HORSE:
			break;
		case SI_DEF_RATE:
			break;
		case SI_MDEF_RATE:
			break;
		case SI_INCHEALRATE:
			break;
		case SI_S_LIFEPOTION:
			break;
		case SI_L_LIFEPOTION:
			break;
		case SI_INCCRI:
			break;
		case SI_PLUSAVOIDVALUE:
			break;
		case SI_ATKER_BLOOD:
			break;
		case SI_TARGET_BLOOD:
			break;
		case SI_ARMOR_PROPERTY:
			break;
		case SI_HELLPOWER:
			break;
		case SI_STEAMPACK:
			break;
		case SI_INVINCIBLE:
			break;
		case SI_CASH_PLUSONLYJOBEXP:
			break;
		case SI_PARTYFLEE:
			break;
		case SI_ANGEL_PROTECT:
			break;
		case SI_ENDURE_MDEF:
			break;
		case SI_ENCHANTBLADE:
			break;
		case SI_DEATHBOUND:
			break;
		case SI_REFRESH:
			break;
		case SI_GIANTGROWTH:
			break;
		case SI_STONEHARDSKIN:
			break;
		case SI_VITALITYACTIVATION:
			break;
		case SI_FIGHTINGSPIRIT:
			break;
		case SI_ABUNDANCE:
			break;
		case SI_REUSE_MILLENNIUMSHIELD:
			break;
		case SI_REUSE_CRUSHSTRIKE:
			break;
		case SI_REUSE_REFRESH:
			break;
		case SI_REUSE_STORMBLAST:
			break;
		case SI_VENOMIMPRESS:
			break;
		case SI_EPICLESIS:
			break;
		case SI_ORATIO:
			break;
		case SI_LAUDAAGNUS:
			break;
		case SI_LAUDARAMUS:
			break;
		case SI_CLOAKINGEXCEED:
			break;
		case SI_HALLUCINATIONWALK:
			break;
		case SI_HALLUCINATIONWALK_POSTDELAY:
			break;
		case SI_RENOVATIO:
			break;
		case SI_WEAPONBLOCKING:
			break;
		case SI_WEAPONBLOCKING_POSTDELAY:
			break;
		case SI_ROLLINGCUTTER:
			break;
		case SI_EXPIATIO:
			break;
		case SI_POISONINGWEAPON:
			break;
		case SI_TOXIN:
			break;
		case SI_PARALYSE:
			break;
		case SI_VENOMBLEED:
			break;
		case SI_MAGICMUSHROOM:
			break;
		case SI_DEATHHURT:
			break;
		case SI_PYREXIA:
			break;
		case SI_OBLIVIONCURSE:
			break;
		case SI_LEECHESEND:
			break;
		case SI_DUPLELIGHT:
			break;
		case SI_FROSTMISTY:
			break;
		case SI_FEARBREEZE:
			break;
		case SI_ELECTRICSHOCKER:
			break;
		case SI_MARSHOFABYSS:
			break;
		case SI_RECOGNIZEDSPELL:
			break;
		case SI_STASIS:
			break;
		case SI_WUGRIDER:
			break;
		case SI_WUGDASH:
			break;
		case SI_WUGBITE:
			break;
		case SI_CAMOUFLAGE:
			break;
		case SI_ACCELERATION:
			break;
		case SI_HOVERING:
			break;
		case SI_SPHERE_1:
			break;
		case SI_SPHERE_2:
			break;
		case SI_SPHERE_3:
			break;
		case SI_SPHERE_4:
			break;
		case SI_SPHERE_5:
			break;
		case SI_MVPCARD_TAOGUNKA:
			break;
		case SI_MVPCARD_MISTRESS:
			break;
		case SI_MVPCARD_ORCHERO:
			break;
		case SI_MVPCARD_ORCLORD:
			break;
		case SI_OVERHEAT_LIMITPOINT:
			break;
		case SI_OVERHEAT:
			break;
		case SI_SHAPESHIFT:
			break;
		case SI_INFRAREDSCAN:
			break;
		case SI_MAGNETICFIELD:
			break;
		case SI_NEUTRALBARRIER:
			break;
		case SI_NEUTRALBARRIER_MASTER:
			break;
		case SI_STEALTHFIELD:
			break;
		case SI_STEALTHFIELD_MASTER:
			break;
		case SI_MANU_ATK:
			break;
		case SI_MANU_DEF:
			break;
		case SI_SPL_ATK:
			break;
		case SI_SPL_DEF:
			break;
		case SI_REPRODUCE:
			break;
		case SI_MANU_MATK:
			break;
		case SI_SPL_MATK:
			break;
		case SI_STR_SCROLL:
			break;
		case SI_INT_SCROLL:
			break;
		case SI_LG_REFLECTDAMAGE:
			break;
		case SI_FORCEOFVANGUARD:
			break;
		case SI_BUCHEDENOEL:
			break;
		case SI_AUTOSHADOWSPELL:
			break;
		case SI_SHADOWFORM:
			break;
		case SI_RAID:
			break;
		case SI_SHIELDSPELL_DEF:
			break;
		case SI_SHIELDSPELL_MDEF:
			break;
		case SI_SHIELDSPELL_REF:
			break;
		case SI_BODYPAINT:
			break;
		case SI_EXEEDBREAK:
			break;
		case SI_ADORAMUS:
			break;
		case SI_PRESTIGE:
			break;
		case SI_INVISIBILITY:
			break;
		case SI_DEADLYINFECT:
			break;
		case SI_BANDING:
			break;
		case SI_EARTHDRIVE:
			break;
		case SI_INSPIRATION:
			break;
		case SI_ENERVATION:
			break;
		case SI_GROOMY:
			break;
		case SI_RAISINGDRAGON:
			break;
		case SI_IGNORANCE:
			break;
		case SI_LAZINESS:
			break;
		case SI_LIGHTNINGWALK:
			break;
		case SI_ACARAJE:
			break;
		case SI_UNLUCKY:
			break;
		case SI_CURSEDCIRCLE_ATKER:
			break;
		case SI_CURSEDCIRCLE_TARGET:
			break;
		case SI_WEAKNESS:
			break;
		case SI_CRESCENTELBOW:
			break;
		case SI_NOEQUIPACCESSARY:
			break;
		case SI_STRIPACCESSARY:
			break;
		case SI_MANHOLE:
			break;
		case SI_POPECOOKIE:
			break;
		case SI_FALLENEMPIRE:
			break;
		case SI_GENTLETOUCH_ENERGYGAIN:
			break;
		case SI_GENTLETOUCH_CHANGE:
			break;
		case SI_GENTLETOUCH_REVITALIZE:
			break;
		case SI_BLOODYLUST:
			break;
		case SI_SWINGDANCE:
			break;
		case SI_SYMPHONYOFLOVERS:
			break;
		case SI_PROPERTYWALK:
			break;
		case SI_SPELLFIST:
			break;
		case SI_NETHERWORLD:
			break;
		case SI_VOICEOFSIREN:
			break;
		case SI_DEEPSLEEP:
			break;
		case SI_SIRCLEOFNATURE:
			break;
		case SI_COLD:
			break;
		case SI_GLOOMYDAY:
			break;
		case SI_SONGOFMANA:
			break;
		case SI_CLOUDKILL:
			break;
		case SI_DANCEWITHWUG:
			break;
		case SI_RUSHWINDMILL:
			break;
		case SI_ECHOSONG:
			break;
		case SI_HARMONIZE:
			break;
		case SI_STRIKING:
			break;
		case SI_WARMER:
			break;
		case SI_MOONLITSERENADE:
			break;
		case SI_SATURDAYNIGHTFEVER:
			break;
		case SI_SITDOWN_FORCE:
			break;
		case SI_ANALYZE:
			break;
		case SI_LERADSDEW:
			break;
		case SI_MELODYOFSINK:
			break;
		case SI_WARCRYOFBEYOND:
			break;
		case SI_UNLIMITEDHUMMINGVOICE:
			break;
		case SI_SPELLBOOK1:
			break;
		case SI_SPELLBOOK2:
			break;
		case SI_SPELLBOOK3:
			break;
		case SI_FREEZE_SP:
			break;
		case SI_GN_TRAINING_SWORD:
			break;
		case SI_GN_REMODELING_CART:
			break;
		case SI_GN_CARTBOOST:
			break;
		case SI_FIXEDCASTINGTM_REDUCE:
			break;
		case SI_THORNTRAP:
			break;
		case SI_BLOODSUCKER:
			break;
		case SI_SPORE_EXPLOSION:
			break;
		case SI_DEMONIC_FIRE:
			break;
		case SI_FIRE_EXPANSION_SMOKE_POWDER:
			break;
		case SI_FIRE_EXPANSION_TEAR_GAS:
			break;
		case SI_BLOCKING_PLAY:
			break;
		case SI_MANDRAGORA:
			break;
		case SI_ACTIVATE:
			break;
		case SI_SECRAMENT:
			break;
		case SI_ASSUMPTIO2:
			break;
		case SI_TK_SEVENWIND:
			break;
		case SI_LIMIT_ODINS_RECALL:
			break;
		case SI_STOMACHACHE:
			break;
		case SI_MYSTERIOUS_POWDER:
			break;
		case SI_MELON_BOMB:
			break;
		case SI_BANANA_BOMB_SITDOWN_POSTDELAY:
			break;
		case SI_PROMOTE_HEALTH_RESERCH:
			break;
		case SI_ENERGY_DRINK_RESERCH:
			break;
		case SI_EXTRACT_WHITE_POTION_Z:
			break;
		case SI_VITATA_500:
			break;
		case SI_EXTRACT_SALAMINE_JUICE:
			break;
		case SI_BOOST500:
			break;
		case SI_FULL_SWING_K:
			break;
		case SI_MANA_PLUS:
			break;
		case SI_MUSTLE_M:
			break;
		case SI_LIFE_FORCE_F:
			break;
		case SI_VACUUM_EXTREME:
			break;
		case SI_SAVAGE_STEAK:
			break;
		case SI_COCKTAIL_WARG_BLOOD:
			break;
		case SI_MINOR_BBQ:
			break;
		case SI_SIROMA_ICE_TEA:
			break;
		case SI_DROCERA_HERB_STEAMED:
			break;
		case SI_PUTTI_TAILS_NOODLES:
			break;
		case SI_BANANA_BOMB:
			break;
		case SI_SUMMON_AGNI:
			break;
		case SI_SPELLBOOK4:
			break;
		case SI_SPELLBOOK5:
			break;
		case SI_SPELLBOOK6:
			break;
		case SI_SPELLBOOK7:
			break;
		case SI_ELEMENTAL_AGGRESSIVE:
			break;
		case SI_RETURN_TO_ELDICASTES:
			break;
		case SI_BANDING_DEFENCE:
			break;
		case SI_SKELSCROLL:
			break;
		case SI_DISTRUCTIONSCROLL:
			break;
		case SI_ROYALSCROLL:
			break;
		case SI_IMMUNITYSCROLL:
			break;
		case SI_MYSTICSCROLL:
			break;
		case SI_BATTLESCROLL:
			break;
		case SI_ARMORSCROLL:
			break;
		case SI_FREYJASCROLL:
			break;
		case SI_SOULSCROLL:
			break;
		case SI_CIRCLE_OF_FIRE:
			break;
		case SI_CIRCLE_OF_FIRE_OPTION:
			break;
		case SI_FIRE_CLOAK:
			break;
		case SI_FIRE_CLOAK_OPTION:
			break;
		case SI_WATER_SCREEN:
			break;
		case SI_WATER_SCREEN_OPTION:
			break;
		case SI_WATER_DROP:
			break;
		case SI_WATER_DROP_OPTION:
			break;
		case SI_WIND_STEP:
			break;
		case SI_WIND_STEP_OPTION:
			break;
		case SI_WIND_CURTAIN:
			break;
		case SI_WIND_CURTAIN_OPTION:
			break;
		case SI_WATER_BARRIER:
			break;
		case SI_ZEPHYR:
			break;
		case SI_SOLID_SKIN:
			break;
		case SI_SOLID_SKIN_OPTION:
			break;
		case SI_STONE_SHIELD:
			break;
		case SI_STONE_SHIELD_OPTION:
			break;
		case SI_POWER_OF_GAIA:
			break;
		case SI_PYROTECHNIC:
			break;
		case SI_PYROTECHNIC_OPTION:
			break;
		case SI_HEATER:
			break;
		case SI_HEATER_OPTION:
			break;
		case SI_TROPIC:
			break;
		case SI_TROPIC_OPTION:
			break;
		case SI_AQUAPLAY:
			break;
		case SI_AQUAPLAY_OPTION:
			break;
		case SI_COOLER:
			break;
		case SI_COOLER_OPTION:
			break;
		case SI_CHILLY_AIR:
			break;
		case SI_CHILLY_AIR_OPTION:
			break;
		case SI_GUST:
			break;
		case SI_GUST_OPTION:
			break;
		case SI_BLAST:
			break;
		case SI_BLAST_OPTION:
			break;
		case SI_WILD_STORM:
			break;
		case SI_WILD_STORM_OPTION:
			break;
		case SI_PETROLOGY:
			break;
		case SI_PETROLOGY_OPTION:
			break;
		case SI_CURSED_SOIL:
			break;
		case SI_CURSED_SOIL_OPTION:
			break;
		case SI_UPHEAVAL:
			break;
		case SI_UPHEAVAL_OPTION:
			break;
		case SI_TIDAL_WEAPON:
			break;
		case SI_TIDAL_WEAPON_OPTION:
			break;
		case SI_ROCK_CRUSHER:
			break;
		case SI_ROCK_CRUSHER_ATK:
			break;
		case SI_FIRE_INSIGNIA:
			break;
		case SI_WATER_INSIGNIA:
			break;
		case SI_WIND_INSIGNIA:
			break;
		case SI_EARTH_INSIGNIA:
			break;
		case SI_EQUIPED_FLOOR:
			break;
		case SI_GUARDIAN_RECALL:
			break;
		case SI_MORA_BUFF:
			break;
		case SI_REUSE_LIMIT_G:
			break;
		case SI_REUSE_LIMIT_H:
			break;
		case SI_NEEDLE_OF_PARALYZE:
			break;
		case SI_PAIN_KILLER:
			break;
		case SI_G_LIFEPOTION:
			break;
		case SI_VITALIZE_POTION:
			break;
		case SI_LIGHT_OF_REGENE:
			break;
		case SI_OVERED_BOOST:
			break;
		case SI_SILENT_BREEZE:
			break;
		case SI_ODINS_POWER:
			break;
		case SI_STYLE_CHANGE:
			break;
		case SI_SONIC_CLAW_POSTDELAY:
			break;
		case SI_SILVERVEIN_RUSH_POSTDELAY:
			break;
		case SI_MIDNIGHT_FRENZY_POSTDELAY:
			break;
		case SI_GOLDENE_FERSE:
			break;
		case SI_ANGRIFFS_MODUS:
			break;
		case SI_TINDER_BREAKER:
			break;
		case SI_TINDER_BREAKER_POSTDELAY:
			break;
		case SI_CBC:
			break;
		case SI_CBC_POSTDELAY:
			break;
		case SI_EQC:
			break;
		case SI_MAGMA_FLOW:
			break;
		case SI_GRANITIC_ARMOR:
			break;
		case SI_PYROCLASTIC:
			break;
		case SI_VOLCANIC_ASH:
			break;
		case SI_SPIRITS_SAVEINFO1:
			break;
		case SI_SPIRITS_SAVEINFO2:
			break;
		case SI_MAGIC_CANDY:
			break;
		case SI_SEARCH_STORE_INFO:
			break;
		case SI_ALL_RIDING:
			break;
		case SI_ALL_RIDING_REUSE_LIMIT:
			break;
		case SI_MACRO:
			break;
		case SI_MACRO_POSTDELAY:
			break;
		case SI_BEER_BOTTLE_CAP:
			break;
		case SI_OVERLAPEXPUP:
			break;
		case SI_PC_IZ_DUN05:
			break;
		case SI_CRUSHSTRIKE:
			break;
		case SI_MONSTER_TRANSFORM:
			break;
		case SI_SIT:
			break;
		case SI_ONAIR:
			break;
		case SI_MTF_ASPD:
			break;
		case SI_MTF_RANGEATK:
			break;
		case SI_MTF_MATK:
			break;
		case SI_MTF_MLEATKED:
			break;
		case SI_MTF_CRIDAMAGE:
			break;
		case SI_REUSE_LIMIT_MTF:
			break;
		case SI_MACRO_PERMIT:
			break;
		case SI_MACRO_PLAY:
			break;
		case SI_SKF_CAST:
			break;
		case SI_SKF_ASPD:
			break;
		case SI_SKF_ATK:
			break;
		case SI_SKF_MATK:
			break;
		case SI_REWARD_PLUSONLYJOBEXP:
			break;
		case SI_HANDICAPSTATE_NORECOVER:
			break;
		case SI_SET_NUM_DEF:
			break;
		case SI_SET_NUM_MDEF:
			break;
		case SI_SET_PER_DEF:
			break;
		case SI_SET_PER_MDEF:
			break;
		case SI_PARTYBOOKING_SEARCH_DEALY:
			break;
		case SI_PARTYBOOKING_REGISTER_DEALY:
			break;
		case SI_PERIOD_TIME_CHECK_DETECT_SKILL:
			break;
		case SI_KO_JYUMONJIKIRI:
			break;
		case SI_MEIKYOUSISUI:
			break;
		case SI_ATTHASTE_CASH:
			break;
		case SI_EQUIPPED_DIVINE_ARMOR:
			break;
		case SI_EQUIPPED_HOLY_ARMOR:
			break;
		case SI_2011RWC:
			break;
		case SI_KYOUGAKU:
			break;
		case SI_IZAYOI:
			break;
		case SI_ZENKAI:
			break;
		case SI_KG_KAGEHUMI:
			break;
		case SI_KYOMU:
			break;
		case SI_KAGEMUSYA:
			break;
		case SI_ZANGETSU:
			break;
		case SI_PHI_DEMON:
			break;
		case SI_GENSOU:
			break;
		case SI_AKAITSUKI:
			break;
		case SI_TETANY:
			break;
		case SI_GM_BATTLE:
			break;
		case SI_GM_BATTLE2:
			break;
		case SI_2011RWC_SCROLL:
			break;
		case SI_ACTIVE_MONSTER_TRANSFORM:
			break;
		case SI_MYSTICPOWDER:
			break;
		case SI_ECLAGE_RECALL:
			break;
		case SI_ENTRY_QUEUE_APPLY_DELAY:
			break;
		case SI_REUSE_LIMIT_ECL:
			break;
		case SI_M_LIFEPOTION:
			break;
		case SI_ENTRY_QUEUE_NOTIFY_ADMISSION_TIME_OUT:
			break;
		case SI_UNKNOWN_NAME:
			break;
		case SI_ON_PUSH_CART:
			break;
		case SI_HAT_EFFECT:
			break;
		case SI_FLOWER_LEAF:
			break;
		case SI_RAY_OF_PROTECTION:
			break;
		case SI_GLASTHEIM_ATK:
			break;
		case SI_GLASTHEIM_DEF:
			break;
		case SI_GLASTHEIM_HEAL:
			break;
		case SI_GLASTHEIM_HIDDEN:
			break;
		case SI_GLASTHEIM_STATE:
			break;
		case SI_GLASTHEIM_ITEMDEF:
			break;
		case SI_GLASTHEIM_HPSP:
			break;
		case SI_HOMUN_SKILL_POSTDELAY:
			break;
		case SI_ALMIGHTY:
			break;
		case SI_GVG_GIANT:
			break;
		case SI_GVG_GOLEM:
			break;
		case SI_GVG_STUN:
			break;
		case SI_GVG_STONE:
			break;
		case SI_GVG_FREEZ:
			break;
		case SI_GVG_SLEEP:
			break;
		case SI_GVG_CURSE:
			break;
		case SI_GVG_SILENCE:
			break;
		case SI_GVG_BLIND:
			break;
		case SI_CLIENT_ONLY_EQUIP_ARROW:
			break;
		case SI_CLAN_INFO:
			break;
		case SI_JP_EVENT01:
			break;
		case SI_JP_EVENT02:
			break;
		case SI_JP_EVENT03:
			break;
		case SI_JP_EVENT04:
			break;
		case SI_TELEPORT_FIXEDCASTINGDELAY:
			break;
		case SI_GEFFEN_MAGIC1:
			break;
		case SI_GEFFEN_MAGIC2:
			break;
		case SI_GEFFEN_MAGIC3:
			break;
		case SI_QUEST_BUFF1:
			break;
		case SI_QUEST_BUFF2:
			break;
		case SI_QUEST_BUFF3:
			break;
		case SI_REUSE_LIMIT_RECALL:
			break;
		case SI_SAVEPOSITION:
			break;
		case SI_HANDICAPSTATE_ICEEXPLO:
			break;
		case SI_FENRIR_CARD:
			break;
		case SI_REUSE_LIMIT_ASPD_POTION:
			break;
		case SI_MAXPAIN:
			break;
		case SI_PC_STOP:
			break;
		case SI_FRIGG_SONG:
			break;
		case SI_OFFERTORIUM:
			break;
		case SI_TELEKINESIS_INTENSE:
			break;
		case SI_MOONSTAR:
			break;
		case SI_STRANGELIGHTS:
			break;
		case SI_FULL_THROTTLE:
			break;
		case SI_REBOUND:
			break;
		case SI_UNLIMIT:
			break;
		case SI_KINGS_GRACE:
			break;
		case SI_ITEM_ATKMAX:
			break;
		case SI_ITEM_ATKMIN:
			break;
		case SI_ITEM_MATKMAX:
			break;
		case SI_ITEM_MATKMIN:
			break;
		case SI_SUPER_STAR:
			break;
		case SI_HIGH_RANKER:
			break;
		case SI_DARKCROW:
			break;
		case SI_2013_VALENTINE1:
			break;
		case SI_2013_VALENTINE2:
			break;
		case SI_2013_VALENTINE3:
			break;
		case SI_ILLUSIONDOPING:
			break;
		case SI_WIDEWEB:
			break;
		case SI_CHILL:
			break;
		case SI_BURNT:
			break;
		case SI_PCCAFE_PLAY_TIME:
			break;
		case SI_TWISTED_TIME:
			break;
		case SI_FLASHCOMBO:
			break;
		case SI_JITTER_BUFF1:
			break;
		case SI_JITTER_BUFF2:
			break;
		case SI_JITTER_BUFF3:
			break;
		case SI_JITTER_BUFF4:
			break;
		case SI_JITTER_BUFF5:
			break;
		case SI_JITTER_BUFF6:
			break;
		case SI_JITTER_BUFF7:
			break;
		case SI_JITTER_BUFF8:
			break;
		case SI_JITTER_BUFF9:
			break;
		case SI_JITTER_BUFF10:
			break;
		case SI_CUP_OF_BOZA:
			break;
		case SI_B_TRAP:
			break;
		case SI_E_CHAIN:
			break;
		case SI_E_QD_SHOT_READY:
			break;
		case SI_C_MARKER:
			break;
		case SI_H_MINE:
			break;
		case SI_H_MINE_SPLASH:
			break;
		case SI_P_ALTER:
			break;
		case SI_HEAT_BARREL:
			break;
		case SI_ANTI_M_BLAST:
			break;
		case SI_SLUGSHOT:
			break;
		case SI_SWORDCLAN:
			break;
		case SI_ARCWANDCLAN:
			break;
		case SI_GOLDENMACECLAN:
			break;
		case SI_CROSSBOWCLAN:
			break;
		case SI_PACKING_ENVELOPE1:
			break;
		case SI_PACKING_ENVELOPE2:
			break;
		case SI_PACKING_ENVELOPE3:
			break;
		case SI_PACKING_ENVELOPE4:
			break;
		case SI_PACKING_ENVELOPE5:
			break;
		case SI_PACKING_ENVELOPE6:
			break;
		case SI_PACKING_ENVELOPE7:
			break;
		case SI_PACKING_ENVELOPE8:
			break;
		case SI_PACKING_ENVELOPE9:
			break;
		case SI_PACKING_ENVELOPE10:
			break;
		case SI_GLASTHEIM_TRANS:
			break;
		case SI_ZONGZI_POUCH_TRANS:
			break;
		case SI_HEAT_BARREL_AFTER:
			break;
		case SI_DECORATION_OF_MUSIC:
			break;
		case SI_OVERSEAEXPUP:
			break;
		case SI_CLOWN_N_GYPSY_CARD:
			break;
		case SI_OPEN_NPC_MARKET:
			break;
		case SI_BEEF_RIB_STEW:
			break;
		case SI_PORK_RIB_STEW:
			break;
		case SI_CHUSEOK_MONDAY:
			break;
		case SI_CHUSEOK_TUESDAY:
			break;
		case SI_CHUSEOK_WEDNESDAY:
			break;
		case SI_CHUSEOK_THURSDAY:
			break;
		case SI_CHUSEOK_FRIDAY:
			break;
		case SI_CHUSEOK_WEEKEND:
			break;
		case SI_ALL_LIGHTGUARD:
			break;
		case SI_ALL_LIGHTGUARD_COOL_TIME:
			break;
		case SI_MTF_MHP:
			break;
		case SI_MTF_MSP:
			break;
		case SI_MTF_PUMPKIN:
			break;
		case SI_MTF_HITFLEE:
			break;
		case SI_MTF_CRIDAMAGE2:
			break;
		case SI_MTF_SPDRAIN:
			break;
		case SI_ACUO_MINT_GUM:
			break;
		case SI_S_HEALPOTION:
			break;
		case SI_REUSE_LIMIT_S_HEAL_POTION:
			break;
		case SI_PLAYTIME_STATISTICS:
			break;
		case SI_GN_CHANGEMATERIAL_OPERATOR:
			break;
		case SI_GN_MIX_COOKING_OPERATOR:
			break;
		case SI_GN_MAKEBOMB_OPERATOR:
			break;
		case SI_GN_S_PHARMACY_OPERATOR:
			break;
		case SI_SO_EL_ANALYSIS_DISASSEMBLY_OPERATOR:
			break;
		case SI_SO_EL_ANALYSIS_COMBINATION_OPERATOR:
			break;
		case SI_NC_MAGICDECOY_OPERATOR:
			break;
		case SI_GUILD_STORAGE:
			break;
		case SI_GC_POISONINGWEAPON_OPERATOR:
			break;
		case SI_WS_WEAPONREFINE_OPERATOR:
			break;
		case SI_BS_REPAIRWEAPON_OPERATOR:
			break;
		case SI_GET_MAILBOX:
			break;
		case SI_JUMPINGCLAN:
			break;
		case SI_JP_OTP:
			break;
		case SI_MAX:
			break;
		default:
			break;
		}
	}
}
void CRoCodeBind::PacketHandler_Cz_MSG_STATE_CHANGE(const char *packetdata){
	if (g_pSharedData->test02 == FALSE)return;
	PACKET_CZ_MSG_STATE_CHANGE* p = (PACKET_CZ_MSG_STATE_CHANGE*)packetdata;

	for (std::list<s_skill_tm>::iterator it = tmList.begin(); it != tmList.end();){
		if (it->id == p->id && it->type == p->type){
			it = tmList.erase(it);
		}
		it++;
	}
}
//Special skill timer
void CRoCodeBind::PacketHandler_Cz_NOTIFY_EFFECT2(const char *packetdata){
	if (g_pSharedData->test02 == FALSE)return;
	PACKET_CZ_NOTIFY_EFFECT2* p = (PACKET_CZ_NOTIFY_EFFECT2*)packetdata;
	std::stringstream str;
	s_skill_tm tm;
	tm.id = p->id;
	switch (p->type)
	{
	case 0x58:
		tm.name = "Isilla";
		tm.type = -1;
		tm.tick = 5000;
		tm.e_tick = tm.tick + timeGetTime();
		tmList.push_back(tm);
		break;
	default:
		break;
	}
}
void CRoCodeBind::PacketQueueProc(char *buf,int len)
{
	if( len > 0 ){
		int now_subMode = -1;
		if( m_packetqueue_head + len >= PACKETQUEUE_BUFFERSIZE ){
			DEBUG_LOGGING_NORMAL( ("packet buffer has overflowed.\n") );
			return;
		}

		memcpy( &m_packetqueuebuffer[m_packetqueue_head] , buf,
			len);
		m_packetqueue_head += len;
		while( m_packetqueue_head > 1 ){
			unsigned short opcode = *(unsigned short*)m_packetqueuebuffer;
			unsigned int packetlength;
			BOOL isReceivedGID = FALSE;

			if (g_pmodeMgr){
				CMode *p_mode = g_pmodeMgr->m_curMode;
				if (p_mode){
					m_CMode_old_subMode = m_CMode_subMode;
					m_CMode_subMode = p_mode->m_subMode;
					DEBUG_LOGGING_DETAIL(("CMode.m_subMode = %08X->%08X m_subModeCnt = %08X\n",
						m_CMode_old_subMode, m_CMode_subMode, p_mode->m_subModeCnt));
				}
			}

			if (m_CMode_subMode != m_CMode_old_subMode && m_CMode_subMode == LSM_WAITRESP_FROM_CHSVR ){
				packetlength = 4;
				isReceivedGID = TRUE;
			}else{
				packetlength = GetPacketLength( opcode );
				if( packetlength == -1 ){
					if( m_packetqueue_head < 4 )break;
					packetlength = *(unsigned int*)m_packetqueuebuffer;
					packetlength >>= 16;
				}
			}
			if( m_packetqueue_head >= packetlength ){
				if (isReceivedGID){
					m_gid = *(int*)m_packetqueuebuffer;
					DEBUG_LOGGING_NORMAL(("GID = %08X", m_gid));
				}else{
					DEBUG_LOGGING_DETAIL(("Opcode %04X size = %d / %d\n", opcode, packetlength, m_packetqueue_head));
					PacketProc(m_packetqueuebuffer);
				}

				m_packetqueue_head -= packetlength;
				if( m_packetqueue_head ){
					memmove( m_packetqueuebuffer, &m_packetqueuebuffer[packetlength],
						m_packetqueue_head );
				}
			}else{
				break;
			}
		}
	}
}

typedef int (__stdcall *Func_CRagConnection__GetPacketSize)(DWORD opcode);

void CRoCodeBind::SearchRagexeMemory(void)
{
	// CZ_UIYourItemWnd::SendMsg CZ_REQ_WEAR_EQUIP handler
	// Marker '1' CModeMgr g_modeMgr (C++ Class Instance)
	// Marker '2' CModeMgr::GetGameMode
	// Marker '3' CRagConnection::instanceR
	// Marker '4' CRagConnection::GetPacketSize
	// Marker '5' CRagConnection::SendPacket
	// Marker '6' UIWindowMgr g_windowMgr (C++ Class Instance)
	// Marker '7' UIWindowMgr::DeleteWindow
	// based RagFree.exe iRO vc6
	CSearchCode UIYourItemWnd__SendMsg_0A9Handler_TypeA(
		"b9*1******"		// mov     ecx,dword L008208d0 ; CModeMgr__g_modeMgr
		"e8*2******"		// call    near F0052c0e0 ; CModeMgr__GetGameMode
		"668b536c"			// mov     dx,[ebx+06ch]
		"668b4370"			// mov     ax,[ebx+070h]
		"8d4dec"			// lea     ecx,[ebp-014h]
		"66c745eca900"		// mov     word [ebp-014h],word 000a9h
		"51"				// push    ecx
		"68a9000000"		// push    dword 0000000a9h
		"668955ee"			// mov     [ebp-012h],dx
		"668945f0"			// mov     [ebp-010h],ax
		"e8*3******"		// call    near F004190f0 ; CRagConnection__instanceR
		"8bc8"				// mov     ecx,eax
		"e8*4******"		// call    near F00419030 ; CRagConnection__GetPacketSize
		"50"				// push    eax
		"e8********"		// call    near F004190f0 ; CRagConnection__instanceR
		"8bc8"				// mov     ecx,eax
		"e8*5******"		// call    near F00418f00 ; CRagConnection__SendPacket
		"68**000000"		// push    dword 00000008ah
		"b9*6******"		// mov     ecx,dword L007e7220 ; UIWindowMgr__g_windowMgr
		"e8*7******"		// call    near F00502390 ; UIWindowMgr__DeleteWindow
		);
	// F2P_Ragexe.exe iRO vc9
	CSearchCode UIYourItemWnd__SendMsg_0A9Handler_TypeB(
		"b9*1******"		// mov     ecx,dword L00812088 ; CModeMgr__g_modeMgr
		"e8*2******"		// call    near F0051eeb0 ; CModeMgr__GetGameMode
		"668b4e**"			// mov     cx,[esi+06ch]
		"668b56**"			// mov     dx,[esi+070h]
		"b8a9000000"		// mov     eax,dword 0000000a9h
		"6689442408"		// mov     [esp+008h],ax
		"8d442408"			// lea     eax,[esp+008h]
		"50"				// push    eax
		"68a9000000"		// push    dword 0000000a9h
		"66894c2412"		// mov     [esp+012h],cx
		"6689542414"		// mov     [esp+014h],dx
		"e8*3******"		// call    near F006335a0 ; CRagConnection__instanceR
		"8bc8"				// mov     ecx,eax
		"e8*4******"		// call    near F006336d0 ; CRagConnection__GetPacketSize
		"50"				// push    eax
		"e8********"		// call    near F006335a0 ; CRagConnection__instanceR
		"8bc8"				// mov     ecx,eax
		"e8*5******"		// call    near F00633550 ; CRagConnection__SendPacket
		"688a000000"		// push    dword 00000008ah
		"b9*6******"		// mov     ecx,dword L0083df40 ; UIWindowMgr__g_windowMgr
		"e8*7******"		// call    near F004f8270 ; UIWindowMgr__DeleteWindow
		);
	// based 2011-12-01aRagexe.exe iRO vc9
	CSearchCode UIYourItemWnd__SendMsg_0A9Handler_TypeC(
		"b9*1******"		// mov     ecx,dword L008371f8 ; CModeMgr__g_modeMgr
		"e8*2******"		// call    near F0054a7a0 ; CModeMgr__GetGameMode
		"668b4e**"			// mov     cx,[esi+07ch]
		"668b96**000000"	// mov     dx,[esi+000000080h]
		"b8a9000000"		// mov     eax,dword 0000000a9h
		"6689442408"		// mov     [esp+008h],ax
		"8d442408"			// lea     eax,[esp+008h]
		"50"				// push    eax
		"68a9000000"		// push    dword 0000000a9h
		"66894c2412"		// mov     [esp+012h],cx
		"6689542414"		// mov     [esp+014h],dx
		"e8*3******"		// call    near F0065d4d0 ; CRagConnection__instanceR
		"8bc8"				// mov     ecx,eax
		"e8*4******"		// call    near F0065cf90 ; CRagConnection__GetPacketSize
		"50"				// push    eax
		"e8********"		// call    near F0065d4d0 ; CRagConnection__instanceR
		"8bc8"				// mov     ecx,eax
		"e8*5******"		// call    near F0065d3f0 ; CRagConnection__SendPacket
		"688a000000"		// push    dword 00000008ah
		"b9*6******"		// mov     ecx,dword L008626b8 ; UIWindowMgr__g_windowMgr
		"e8*7******"		// call    near F00523770 ; UIWindowMgr__DeleteWindow
		);
	// 2014-03-14aRagexe.exe jRO
	CSearchCode UIYourItemWnd__SendMsg_0A9Handler_TypeD(
		"b9*1******"		// mov     ecx,dword L00812088 ; CModeMgr__g_modeMgr
		"e8*2******"		// call    near F0051eeb0 ; CModeMgr__GetGameMode
		"668b****"			// mov     cx,[esi+06ch]
		"668b****"			// mov     dx,[esi+070h]
		"b9a9000000"		// mov     eax,dword 0000000a9h
		"6689****"			// mov     [esp+008h],ax
		"8d****"			// lea     eax,[esp+008h]
		"51"				// push    eax
		"68a9000000"		// push    dword 0000000a9h
		"668955**"			// mov     [ebp-012h],dx
		"668945**"			// mov     [ebp-010h],ax
		"e8*3******"		// call    near F006335a0 ; CRagConnection__instanceR
		"8bc8"				// mov     ecx,eax
		"e8*4******"		// call    near F006336d0 ; CRagConnection__GetPacketSize
		"50"				// push    eax
		"e8********"		// call    near F006335a0 ; CRagConnection__instanceR
		"8bc8"				// mov     ecx,eax
		"e8*5******"		// call    near F00633550 ; CRagConnection__SendPacket
		"688a000000"		// push    dword 00000008ah
		"b9*6******"		// mov     ecx,dword L0083df40 ; UIWindowMgr__g_windowMgr
		"e8*7******"		// call    near F004f8270 ; UIWindowMgr__DeleteWindow
		);
	// 2014-03-18 Ragexe.exe iRO link time 20140226 155100
	CSearchCode UIYourItemWnd__SendMsg_0A9Handler_TypeE(
		"b9*1******"        //         mov     ecx,dword L0099f930 ; CModeMgr__g_modeMgr
		"e8*2******"        //         call    near F005a5eb0 ; CModeMgr__GetGameMode
		"668b96********"    //         mov     dx,[esi+00000008ch]
		"668b86********"    //         mov     ax,[esi+000000090h]
		"b9a9000000"        //         mov     ecx,dword 0000000a9h
		"66894d**"          //         mov     [ebp-034h],cx
		"8d4d**"            //         lea     ecx,[ebp-034h]
		"51"                //         push    ecx
		"68a9000000"        //         push    dword 0000000a9h
		"668955**"          //         mov     [ebp-032h],dx
		"668945**"          //         mov     [ebp-030h],ax
		"e8*3******"        //         call    near F00716fc0 ; CRagConnection__instanceR
		"8bc8"              //         mov     ecx,eax
		"e8*4******"        //         call    near F007168e0 ; CRagConnection__GetPacketSize
		"50"                //         push    eax
		"e8********"        //         call    near F00716fc0 ; CRagConnection__instanceR
		"8bc8"              //         mov     ecx,eax
		"e8*5******"        //         call    near F00716ee0 ; CRagConnection__SendPacket
		"688a000000"        //         push    dword 00000008ah
		"b9*6******"        //         mov     ecx,dword L009d21c0 ; UIWindowMgr__g_windowMgr
		"e8*7******"        //         call    near F00573a40 ; UIWindowMgr__DeleteWindow
		);

	CSearchCode CMouse_Init_vc6(
		"a1********"		// mov     eax,[ g_hInstance ]
		"53"				// push    ebx
		"56"				// push    esi
		"33db"				// xor     ebx,ebx
		"57"				// push    edi
		"8bf1"				// mov     esi,ecx
		"53"				// push    ebx
		"56"				// push    esi
		"6800070000"		// push    dword 000000700h
		"50"				// push    eax
		);
	CSearchCode CMouse_Init_vc9(
		"a1********"		// mov     eax,[ g_hInstance ]
		"53"				// push    ebx
		"56"				// push    esi
		"33db"				// xor     ebx,ebx
		"53"				// push    ebx
		"8bf1"				// mov     esi,ecx
		"56"				// push    esi
		"6800070000"		// push    dword 000000700h
		"50"				// push    eax
		);
	CSearchCode winmain_init_CMouse_Init_call(
		"b9*1******"		// mov     ecx,g_mouse
		"e8*2******"		// call    near CMouse__Init
		"a1*3******"		// mov     eax, g_renderer__CRenderer
		);

	CSearchCode funcPlayStrem_based_HighPrest_exe(
		"55"                //   push    ebp
		"8bec"              //   mov     ebp,esp
		"a1*1******"        //   mov     eax,[L006f2534] ; g_soundMode
		"53"                //   push    ebx
		"85c0"              //   test    eax,eax
		"56"                //   push    esi
		"0f84c5000000"      //   jz      near C0040afa7
		"8b5d08"            //   mov     ebx,dword [ebp+008h] ;+streamFileName
		"be********"        //   mov     esi,dword L0074e7c4
		"8bc3"              //   mov     eax,ebx
		"8a10"              //   mov     dl,[eax]
		"8aca"              //   mov     cl,dl
		"3a16"              //   cmp     dl,[esi]
		"751c"              //   jnz     C0040af10                            
		"84c9"              //   test    cl,cl
		);
	CSearchCode funcPlayStrem_based_RagFree_exe(
		"55"                //   push    ebp
		"8bec"              //   mov     ebp,esp
		"81ec00010000"      //   sub     esp,dword 000000100h
		"a1*1******"        //   mov     eax,[L00775d44] ; g_soundMode
		"53"                //   push    ebx
		"85c0"              //   test    eax,eax
		"56"                //   push    esi
		"0f8417010000"      //   jz      near C0041b6bf
		"8b5d08"            //   mov     ebx,dword [ebp+008h] ;+streamFileName
		"be********"        //   mov     esi,dword L007cd9b4
		"8bc3"              //   mov     eax,ebx
		"8a10"              //   mov     dl,[eax]
		"8aca"              //   mov     cl,dl
		"3a16"              //   cmp     dl,[esi]
		"751c"              //   jnz     C0041b5d6                            
		"84c9"              //   test    cl,cl
		);
	CSearchCode funcPlayStrem_based_2011111201aRagexe_exe(
		"81ec04010000"      //   sub     esp,dword 000000104h
		"a1*1******"        //   mov     eax,[L00845990] ; g_soundMode
		"33c4"              //   xor     eax,esp
		"89842400010000"    //   mov     dword [esp+000000100h],eax
		"833d********00"    //   cmp     dword [L0084459c],byte +000h
		"56"                //   push    esi
		"8bb4240c010000"    //   mov     esi,dword [esp+00000010ch] ;+streamFileName
		"0f8406010000"      //   jz      near C0065fd9f
		"b9********"        //   mov     ecx,dword L008e348c
		"8bc6"              //   mov     eax,esi
		"8a10"              //   mov     dl,[eax]
		"3a11"              //   cmp     dl,[ecx]
		"751a"              //   jnz     C0065fcc0                                  
		"84d2"              //   test    dl,dl
		);
	CSearchCode funcPlayStrem_based_20140226_155100iRagexe_exe(
		// F00719260
		"55"                //   push    ebp
		"8bec"              //   mov     ebp,esp
		"81ec********"      //   sub     esp,dword 000000104h
		"a1*1******"        //   mov     eax,[L009b63d0] ; g_soundMode
		"33c5"              //   xor     eax,ebp
		"8945**"            //   mov     dword [ebp-004h],eax
		"833d********00"    //   cmp     dword [L00aa1610],byte +000h
		"56"                //   push    esi
		"8b75**"            //   mov     esi,dword [ebp+008h]
		"0f84********"      //   jz      near C00719393
		);

	CSearchCode initCConnection_20140318iRagexe( //  winmain...
//		"56"                 //  push    esi
//		"68********"         //  push    dword S00920704; "ws2_32.dll"
//		"ff15********"       //  call    dword[L008e619c]; ds:LoadLibraryA
//		"8b35********"       //  mov     esi, dword[L008e6194]; ds:GetProcAddress
//		"68********"         //  push    dword L009206e8 ; "send"
//		"50"                 //  push    eax
//		"a3********"         //  mov[L00a68674], eax; CConnection_s_wsmodule
//		"ffd6"               //  call    esi; GetProcAddress
//		"a3********"         //  mov[L00a68678], eax; CConnection_s_wsSend
//		"a1********"         //  mov     eax, [L00a68674]; CConnection_s_wsmodule
//		"68********"         //  push    dword S0092078c; "recv"
//		"50"                 //  push    eax; hModule
		"ffd6"               //  call    esi; GetProcAddress
		"833d*1******00"     //  cmp     dword[L00a68678], byte + 000h; CConnection_s_wsSend
		"8b35********"       //  mov     esi, dword[L008e66e8]; ds:MessageBoxA
		"a3*2******"         //  mov[L00a68670], eax; CConnection_s_wsRecv
		"751c"               //  jnz     C007170db
		"8b0d********"       //  mov     ecx, dword[WS2_32.dll_13]; ds:send
		"6a00"               //  push    byte + 000h; uType
	);

	CSearchCode strings_event_grf(0, "event.grf");
	LPBYTE strings_event_grf_address = NULL;
	CSearchCode addPak_event_grf(
		"68*1******"         //  push    dword *"event.grf"
		"b9*2******"         //  mov     ecx, dword CFileMgr::g_fileMgr
		"e8*3******"         //  call    near CFileMgr::AddPak
	);

	CSearchCode strings_readfolder(0, "readfolder");
	LPBYTE strings_readfolder_address = NULL;
	CSearchCode set_g_readFolderFirst(
		"68*1******"         //  push    dword *"readfolder"
		"8bce"               //  mov     ecx, esi
		"e8********"         //  call    near XMLElement::FindChild(char const *)
		"85c0"               //  test    eax, eax
		"7407"               //  jz      C005a43ce
		"c605*2******01"     //  mov     byte[g_readFolderFirst], byte 001h // bool g_readFolderFirst
	);
	PBOOL pg_readFolderFirst = NULL;

	CSearchCode subfunction_CFileMgr__Open(
		"803d*1******00"     //  cmp     byte[ g_readFolderFirst ], byte 000h
		"53"                 //  push    ebx
		"8b5d08"             //  mov     ebx, dword[ebp + 008h]
		"57"                 //  push    edi
		"8b7d0c"             //  mov     edi, dword[ebp + 00ch]
		"57"                 //  push    edi
		"53"                 //  push    ebx
		"7419"               //  jz      C005c79ac
		"e8*2******"         //  call    near CFileMgr::GetFile
		"85c0"               //  test    eax, eax
		"7522"               //  jnz     C005c79be;; goto
		"57"                 //  push    edi
		"53"                 //  push    ebx
		"8bce"               //  mov     ecx, esi
		"e8*3******"         //  call    near CFileMgr::GetPak
	);


	LPBYTE pRagexeBase;
	MEMORY_BASIC_INFORMATION mbi,mbi_data;
	DWORD temp_eax, temp_ecx, temp_edx, temp_esp;

	pRagexeBase = (LPBYTE)::GetModuleHandle(NULL);
	pRagexeBase += 0x1000;

	if (::VirtualQuery(pRagexeBase, &mbi, sizeof(mbi)) &&
		::VirtualQuery((LPBYTE)mbi.BaseAddress + mbi.RegionSize, &mbi_data, sizeof(mbi_data)))
	{
		DWORD ptr_CMouse_Init = 0;

		p_std_map_packetlen *packetLenMap = 0;

		DEBUG_LOGGING_NORMAL( ("MEMORY_BASIC_INFORMATION lpAddres:%08X",pRagexeBase) );
		DEBUG_LOGGING_NORMAL( ("mbi.AllocationBase = %08X",mbi.AllocationBase) );
		DEBUG_LOGGING_NORMAL( ("mbi.BaseAddress    = %08X",mbi.BaseAddress) );
		DEBUG_LOGGING_NORMAL( ("mbi.RegionSize     = %08X",mbi.RegionSize) );
		DEBUG_LOGGING_NORMAL(("MEMORY_BASIC_INFORMATION lpAddres:%08X", (LPBYTE)mbi.BaseAddress + mbi.RegionSize));
		DEBUG_LOGGING_NORMAL(("mbi_data.AllocationBase = %08X", mbi_data.AllocationBase));
		DEBUG_LOGGING_NORMAL(("mbi_data.BaseAddress    = %08X", mbi_data.BaseAddress));
		DEBUG_LOGGING_NORMAL(("mbi_data.RegionSize     = %08X", mbi_data.RegionSize));

		mbi.RegionSize += mbi_data.RegionSize;

		// get s_CMouse instance
		for (UINT ii = 0; ii < mbi.RegionSize - 1000; ii++)
		{
			LPBYTE pBase = (LPBYTE)mbi.BaseAddress;

			if (initCConnection_20140318iRagexe.PatternMatcher(&pBase[ii]))
			{
				pCConnection_s_wsSend = (tWS2_32_send*)initCConnection_20140318iRagexe.GetImmediateDWORD(&pBase[ii], '1');
				pCConnection_s_wsRecv = (tWS2_32_recv*)initCConnection_20140318iRagexe.GetImmediateDWORD(&pBase[ii], '2');
				DEBUG_LOGGING_NORMAL(("Find s_wsSend,s_wsRecv baseaddress = %08X send = %08X | recv =%08X",
					&pBase[ii], pCConnection_s_wsSend, pCConnection_s_wsRecv));

				break;
			}
		}

		// snatch the packetLenMap
		for( UINT ii = 0; ii < mbi.RegionSize - 1000 ; ii++ )
		{
			LPBYTE pBase = (LPBYTE)mbi.BaseAddress;

			if( UIYourItemWnd__SendMsg_0A9Handler_TypeA.PatternMatcher( &pBase[ii] ) )
			{
				DWORD GetPacketSizeAddr;
				GetPacketSizeAddr = 
					UIYourItemWnd__SendMsg_0A9Handler_TypeA.Get4BIndexDWORD( &pBase[ii] , '4' );
				__asm push 0x0A9
				__asm call GetPacketSizeAddr
				__asm mov temp_esp,esp

				p_std_map_packetlen *plen = (p_std_map_packetlen*)
					*(DWORD*) (*(DWORD*)(temp_esp-19*4) + 4);

				g_pmodeMgr = (CModeMgr*)UIYourItemWnd__SendMsg_0A9Handler_TypeA
					.GetImmediateDWORD( &pBase[ii], '1' );

				DEBUG_LOGGING_NORMAL( ("TypeA GetPacketSizeAddr %08X",GetPacketSizeAddr) );
				DEBUG_LOGGING_NORMAL( (" esp = %08X",temp_esp) );
				while(1)
				{
					if( plen->key > 0xffff || (plen->key == 0 && plen->value == 0) ){
						packetLenMap = plen;
						DEBUG_LOGGING_NORMAL( ("packetLenMap = %08X",packetLenMap) );
						break;
					}
					plen = plen->parent;
				}
				break;
			}else
			if( UIYourItemWnd__SendMsg_0A9Handler_TypeB.PatternMatcher( &pBase[ii] ) )
			{
				DWORD GetPacketSizeAddr;
				GetPacketSizeAddr = 
					UIYourItemWnd__SendMsg_0A9Handler_TypeB.Get4BIndexDWORD( &pBase[ii] , '4' );
				__asm push 0x0A9
				__asm call GetPacketSizeAddr
				__asm mov temp_eax,eax
				__asm mov temp_ecx,ecx
				__asm mov temp_edx,edx
				p_std_map_packetlen *plen;
				if( temp_eax == temp_edx ){
					plen = (p_std_map_packetlen*)temp_ecx;
				}else{
					plen = (p_std_map_packetlen*)temp_edx;
				}

				g_pmodeMgr = (CModeMgr*)UIYourItemWnd__SendMsg_0A9Handler_TypeB
					.GetImmediateDWORD( &pBase[ii], '1' );

				DEBUG_LOGGING_NORMAL( ("TypeB GetPacketSizeAddr     = %08X eax = %08X ecx = %08X edx =%08X",
					GetPacketSizeAddr,temp_eax,temp_ecx,temp_edx) );
				while(1)
				{
					if( plen->key > 0xffff || (plen->key == 0 && plen->value == 0) ){
						packetLenMap = plen;
						DEBUG_LOGGING_NORMAL( ("packetLenMap = %08X",packetLenMap) );
						break;
					}
					plen = plen->parent;
				}
				break;
			}else
			if( UIYourItemWnd__SendMsg_0A9Handler_TypeC.PatternMatcher( &pBase[ii] ) )
			{
				DWORD GetPacketSizeAddr;
				Func_CRagConnection__GetPacketSize GetPacketSize;
				GetPacketSizeAddr = 
					UIYourItemWnd__SendMsg_0A9Handler_TypeC.Get4BIndexDWORD( &pBase[ii] , '4' );
				GetPacketSize = (Func_CRagConnection__GetPacketSize)GetPacketSizeAddr;
				//GetPacketSize(0x0A9);
				__asm push 0x0A9
				__asm call GetPacketSizeAddr
				__asm mov temp_ecx,ecx
				p_std_map_packetlen *plen = (p_std_map_packetlen*)temp_ecx;

				g_pmodeMgr = (CModeMgr*)UIYourItemWnd__SendMsg_0A9Handler_TypeC
					.GetImmediateDWORD( &pBase[ii], '1' );

				DEBUG_LOGGING_NORMAL( ("TypeC GetPacketSizeAddr     = %08X ecx = %08X",GetPacketSizeAddr,temp_ecx) );
				while(1)
				{
					if( plen->key > 0xffff || (plen->key == 0 && plen->value == 0) ){
						packetLenMap = plen;
						DEBUG_LOGGING_NORMAL( ("packetLenMap = %08X",packetLenMap) );
						break;
					}
					plen = plen->parent;
				}
				break;
			}else
			if( UIYourItemWnd__SendMsg_0A9Handler_TypeD.PatternMatcher( &pBase[ii] ) )
			{
				DWORD GetPacketSizeAddr;
				Func_CRagConnection__GetPacketSize GetPacketSize;
				GetPacketSizeAddr = 
					UIYourItemWnd__SendMsg_0A9Handler_TypeD.Get4BIndexDWORD( &pBase[ii] , '4' );
				GetPacketSize = (Func_CRagConnection__GetPacketSize)GetPacketSizeAddr;
				//GetPacketSize(0x0A9);
				__asm push 0x0A9
				__asm call GetPacketSizeAddr
				__asm mov temp_ecx,ecx
				p_std_map_packetlen *plen = (p_std_map_packetlen*)temp_ecx;

				g_pmodeMgr = (CModeMgr*)UIYourItemWnd__SendMsg_0A9Handler_TypeD
					.GetImmediateDWORD( &pBase[ii], '1' );

				DEBUG_LOGGING_NORMAL( ("TypeD GetPacketSizeAddr     = %08X ecx = %08X",GetPacketSizeAddr,temp_ecx) );
				while(1)
				{
					if( plen->key > 0xffff || (plen->key == 0 && plen->value == 0) ){
						packetLenMap = plen;
						DEBUG_LOGGING_NORMAL( ("packetLenMap = %08X",packetLenMap) );
						break;
					}
					plen = plen->parent;
				}
				break;
			}else
			if( UIYourItemWnd__SendMsg_0A9Handler_TypeE.PatternMatcher( &pBase[ii] ) )
			{
				DWORD GetPacketSizeAddr;
				Func_CRagConnection__GetPacketSize GetPacketSize;
				GetPacketSizeAddr = 
					UIYourItemWnd__SendMsg_0A9Handler_TypeE.Get4BIndexDWORD( &pBase[ii] , '4' );
				GetPacketSize = (Func_CRagConnection__GetPacketSize)GetPacketSizeAddr;
				//GetPacketSize(0x0A9);
				__asm push 0x0A9
				__asm call GetPacketSizeAddr
				__asm mov temp_ecx,ecx
				p_std_map_packetlen *plen = (p_std_map_packetlen*)temp_ecx;

				g_pmodeMgr = (CModeMgr*)UIYourItemWnd__SendMsg_0A9Handler_TypeE
					.GetImmediateDWORD( &pBase[ii], '1' );

				DEBUG_LOGGING_NORMAL( ("TypeE GetPacketSizeAddr     = %08X ecx = %08X",GetPacketSizeAddr,temp_ecx) );
				while(1)
				{
					if( plen->key > 0xffff || (plen->key == 0 && plen->value == 0) ){
						packetLenMap = plen;
						DEBUG_LOGGING_NORMAL( ("packetLenMap = %08X",packetLenMap) );
						break;
					}
					plen = plen->parent;
				}
				break;
			}
		}

		// get CMouse instance
		for( UINT ii = 0; ii < mbi.RegionSize - CMouse_Init_vc6.GetSize() ; ii++ )
		{
			LPBYTE pBase = (LPBYTE)mbi.BaseAddress;
			if( CMouse_Init_vc6.PatternMatcher( &pBase[ii] )
			 || CMouse_Init_vc9.PatternMatcher( &pBase[ii] )
				)
			{
				ptr_CMouse_Init = (DWORD)( &pBase[ii] );
				DEBUG_LOGGING_NORMAL( ("find CMouse::Init = %08X",pBase + ii) );
				break;
			}
		}
		if( ptr_CMouse_Init )
		{
			for( int ii = mbi.RegionSize - winmain_init_CMouse_Init_call.GetSize(); ii >= 0 ; ii-- )
			{
				LPBYTE pBase = (LPBYTE)mbi.BaseAddress;
				if( winmain_init_CMouse_Init_call.PatternMatcher( &pBase[ii] ) )
				{
					DEBUG_LOGGING_NORMAL( ("find CMouse::Init call : %08X",pBase + ii) );
					if( winmain_init_CMouse_Init_call.NearJmpAddressMatcher( &pBase[ii],'2',ptr_CMouse_Init ) )
					{
						g_mouse = (CMouse*)winmain_init_CMouse_Init_call.GetImmediateDWORD(&pBase[ii],'1');
						g_renderer = (CRenderer**)winmain_init_CMouse_Init_call.GetImmediateDWORD(&pBase[ii],'3');
						
						DEBUG_LOGGING_NORMAL( ("find g_mouse = %08X",g_mouse) );
						DEBUG_LOGGING_NORMAL( ("find *g_renderer = %08X",g_renderer) );
						break;
					}
				}
			}
		}

		// get the address of PlayStream function 
		for( UINT ii = 0; ii < mbi.RegionSize - funcPlayStrem_based_HighPrest_exe.GetSize() ; ii++ )
		{
			LPBYTE pBase = (LPBYTE)mbi.BaseAddress;

			if( funcPlayStrem_based_20140226_155100iRagexe_exe.PatternMatcher( &pBase[ii] )	)
			{
				m_funcRagexe_PlayStream = (tPlayStream)&pBase[ii];
				DWORD pPlayStream;
				pPlayStream = (DWORD)&pBase[ii];
				DEBUG_LOGGING_NORMAL( ("based_20140226_155100iRagexe : %08X",&pBase[ii]) );
				DEBUG_LOGGING_NORMAL( ("g_soundMode == %08X",(char*)funcPlayStrem_based_20140226_155100iRagexe_exe.GetImmediateDWORD(&pBase[ii],'1')) );
				DEBUG_LOGGING_NORMAL( ("void PlayStream(const char *streamFileName) = %08X",pPlayStream) );
				break;
			}
			if( funcPlayStrem_based_HighPrest_exe.PatternMatcher( &pBase[ii] )	)
			{
				m_funcRagexe_PlayStream = (tPlayStream)&pBase[ii];
				DWORD pPlayStream;
				pPlayStream = (DWORD)&pBase[ii];
				DEBUG_LOGGING_NORMAL( ("based_HighPrest_exe : %08X",&pBase[ii]) );
				DEBUG_LOGGING_NORMAL( ("g_soundMode == %08X",(char*)funcPlayStrem_based_HighPrest_exe.GetImmediateDWORD(&pBase[ii],'1')) );
				DEBUG_LOGGING_NORMAL( ("void PlayStream(const char *streamFileName) = %08X",pPlayStream) );
				break;
			}
			if( funcPlayStrem_based_RagFree_exe.PatternMatcher( &pBase[ii] )	)
			{
				m_funcRagexe_PlayStream = (tPlayStream)&pBase[ii];
				DWORD pPlayStream;
				pPlayStream = (DWORD)&pBase[ii];
				DEBUG_LOGGING_NORMAL( ("based_RagFree_exe : %08X",&pBase[ii]) );
				DEBUG_LOGGING_NORMAL( ("g_soundMode == %08X",(char*)funcPlayStrem_based_RagFree_exe.GetImmediateDWORD(&pBase[ii],'1')) );
				DEBUG_LOGGING_NORMAL( ("void PlayStream(const char *streamFileName,int playflag) = %08X",pPlayStream) );
				break;
			}
			if( funcPlayStrem_based_2011111201aRagexe_exe.PatternMatcher( &pBase[ii] )	)
			{
				m_funcRagexe_PlayStream = (tPlayStream)&pBase[ii];
				DWORD pPlayStream;
				pPlayStream = (DWORD)&pBase[ii];
				DEBUG_LOGGING_NORMAL( ("based_2011111201aRagexe_exe : %08X",&pBase[ii]) );
				DEBUG_LOGGING_NORMAL( ("g_soundMode == %08X",(char*)funcPlayStrem_based_2011111201aRagexe_exe.GetImmediateDWORD(&pBase[ii],'1')) );
				DEBUG_LOGGING_NORMAL( ("void PlayStream(const char *streamFileName,int playflag) = %08X",pPlayStream) );
				break;
			}
		}

		// find strings
		for (UINT ii = 0; ii < mbi.RegionSize - 1000; ii++)
		{
			LPBYTE pBase = (LPBYTE)mbi.BaseAddress;

			if ( !strings_event_grf_address && strings_event_grf.PatternMatcher(&pBase[ii]))
			{
				strings_event_grf_address = &pBase[ii];
				DEBUG_LOGGING_NORMAL(("find 'event.grf' : %08X", strings_event_grf_address));
			}
			if (!strings_readfolder_address && strings_readfolder.PatternMatcher(&pBase[ii]))
			{
				strings_readfolder_address = &pBase[ii];
				DEBUG_LOGGING_NORMAL(("find 'readfolderf' : %08X", strings_readfolder_address));
			}
		}

		// get the address of CFileMgr::g_fileMgr
		for (UINT ii = 0; ii < mbi.RegionSize - 1000; ii++)
		{
			LPBYTE pBase = (LPBYTE)mbi.BaseAddress;

			if (addPak_event_grf.PatternMatcher(&pBase[ii]) &&
				addPak_event_grf.GetImmediateDWORD(&pBase[ii], '1') == (DWORD)strings_event_grf_address)
			{
				m_CFileMgr__gfileMgr = (void*)addPak_event_grf.GetImmediateDWORD(&pBase[ii], '2');
				DEBUG_LOGGING_NORMAL(("find CFileMgr::gfileMgr : %08X", m_CFileMgr__gfileMgr));
				break;
			}
		}

		// get the address of g_readFolderFirst
		for (UINT ii = 0; ii < mbi.RegionSize - 1000; ii++)
		{
			LPBYTE pBase = (LPBYTE)mbi.BaseAddress;

			if (set_g_readFolderFirst.PatternMatcher(&pBase[ii]) &&
				set_g_readFolderFirst.GetImmediateDWORD(&pBase[ii], '1') == (DWORD)strings_readfolder_address)
			{
				pg_readFolderFirst = (PBOOL)set_g_readFolderFirst.GetImmediateDWORD(&pBase[ii], '2');
				DEBUG_LOGGING_NORMAL(("find g_readFolderFirst : %08X", pg_readFolderFirst));
				break;
			}
		}

		// get the address of g_readFolderFirst
		for (UINT ii = 0; ii < mbi.RegionSize - 1000; ii++)
		{
			LPBYTE pBase = (LPBYTE)mbi.BaseAddress;

			if (subfunction_CFileMgr__Open.PatternMatcher(&pBase[ii]) &&
				subfunction_CFileMgr__Open.GetImmediateDWORD(&pBase[ii], '1') == (DWORD)pg_readFolderFirst)
			{
				m_functionRagexe_CFileMgr__GetPak = (tCFileMgr__GetPak)subfunction_CFileMgr__Open.GetNearJmpAddress(&pBase[ii], '3');
				DEBUG_LOGGING_NORMAL(("find CFileMgr::GetFile : %08X", subfunction_CFileMgr__Open.GetNearJmpAddress(&pBase[ii], '2') ));
				DEBUG_LOGGING_NORMAL(("find CFileMgr::GetPak : %08X", m_functionRagexe_CFileMgr__GetPak));
				break;
			}
		}
		{
			void *address = NULL;
			unsigned int size = 0;

			DEBUG_LOGGING_NORMAL(("call CFileMgr::GetPak"));
			address = GetPak("data\\idnum2itemdisplaynametable.txt", &size);

			DEBUG_LOGGING_NORMAL(("load data\\idnum2itemdisplaynametable.txt %08X size of %d",
				address, size));

			DEBUG_LOGGING_NORMAL(("release data\\idnum2itemdisplaynametable.txt"));
			ReleasePak(address);
		}

		// put packetlengthmap
		if( packetLenMap ){
			int packetnums = GetTreeData( packetLenMap->parent );
			if( packetnums ){
				std::stringstream onelinestr;
				for(int ii = 0;ii < packetnums ;ii++){
					if( (ii % 0x40)==0 ){
						onelinestr << "# 0x" << std::setfill('0') << std::setw(4) << std::hex << ii;
						DEBUG_LOGGING_NORMAL(( onelinestr.str().c_str() ));
						onelinestr.str("");
					}
					if( (ii % 0x10)==0 ){
						onelinestr << " ";
					}
					onelinestr << std::setfill(' ') << std::setw(4) << std::dec << m_packetLenMap_table[ii] << ",";
					if( (ii % 0x10)==0x0f ){
						DEBUG_LOGGING_NORMAL(( onelinestr.str().c_str() ));
						onelinestr.str("");
					}
				}
				DEBUG_LOGGING_NORMAL(( onelinestr.str().c_str() ));
				DEBUG_LOGGING_NORMAL(( "" ));
			}
		}

	}
}

