/*
C  = Client
Z  = Zone(map server)
A  = Account(login server)
H  = cHar(char server)
CZ = client to zone
ZC = zone to client
*/
enum PACKET_HEADER{
	HEADER_ZC_ACCEPT_ENTER										= 0x73,
	HEADER_ZC_REFUSE_ENTER										= 0x74,
	HEADER_ZC_NOTIFY_INITCHAR									= 0x75,
	HEADER_ZC_NOTIFY_UPDATECHAR									= 0x76,
	HEADER_ZC_NOTIFY_UPDATEPLAYER								= 0x77,
	HEADER_ZC_NOTIFY_STANDENTRY									= 0x78,
	HEADER_ZC_NOTIFY_NEWENTRY									= 0x79,
	HEADER_ZC_NOTIFY_ACTENTRY									= 0x7A,
	HEADER_ZC_NOTIFY_MOVEENTRY									= 0x7B,
	HEADER_ZC_NOTIFY_STANDENTRY_NPC								= 0x7C,
	HEADER_ZC_NOTIFY_TIME										= 0x7F,
	HEADER_ZC_NOTIFY_VANISH										= 0x80,
	HEADER_ZC_ACCEPT_QUIT										= 0x83,
	HEADER_ZC_REFUSE_QUIT										= 0x84,
	HEADER_ZC_NOTIFY_MOVE										= 0x86,
	HEADER_ZC_NOTIFY_PLAYERMOVE									= 0x87,
	HEADER_ZC_STOPMOVE											= 0x88,
	HEADER_ZC_NOTIFY_ACT										= 0x8A,
	HEADER_ZC_NOTIFY_ACT_POSITION								= 0x8B,
	HEADER_ZC_NOTIFY_CHAT										= 0x8D,
	HEADER_ZC_NOTIFY_PLAYERCHAT									= 0x8E,
	HEADER_ZC_NPCACK_MAPMOVE									= 0x91,
	HEADER_ZC_NPCACK_SERVERMOVE									= 0x92,
	HEADER_ZC_NPCACK_ENABLE										= 0x93,
	HEADER_ZC_ACK_REQNAME										= 0x95,
	HEADER_ZC_WHISPER											= 0x97,
	HEADER_ZC_ACK_WHISPER										= 0x98,
	HEADER_ZC_BROADCAST											= 0x9A,
	HEADER_ZC_CHANGE_DIRECTION									= 0x9C,
	HEADER_ZC_ITEM_ENTRY										= 0x9D,
	HEADER_ZC_ITEM_FALL_ENTRY									= 0x9E,
	HEADER_ZC_ITEM_PICKUP_ACK									= 0xA0,
	HEADER_ZC_ITEM_DISAPPEAR									= 0xA1,
	HEADER_ZC_NORMAL_ITEMLIST									= 0xA3,
	HEADER_ZC_EQUIPMENT_ITEMLIST								= 0xA4,
	HEADER_ZC_STORE_NORMAL_ITEMLIST								= 0xA5,
	HEADER_ZC_STORE_EQUIPMENT_ITEMLIST							= 0xA6,
	HEADER_ZC_USE_ITEM_ACK										= 0xA8,
	HEADER_ZC_REQ_WEAR_EQUIP_ACK								= 0xAA,
	HEADER_ZC_REQ_TAKEOFF_EQUIP_ACK								= 0xAC,
	HEADER_ZC_ITEM_THROW_ACK									= 0xAF,
	HEADER_ZC_PAR_CHANGE										= 0xB0,
	HEADER_ZC_LONGPAR_CHANGE									= 0xB1,
	HEADER_ZC_RESTART_ACK										= 0xB3,
	HEADER_ZC_SAY_DIALOG										= 0xB4,
	HEADER_ZC_WAIT_DIALOG										= 0xB5,
	HEADER_ZC_CLOSE_DIALOG										= 0xB6,
	HEADER_ZC_MENU_LIST											= 0xB7,
	HEADER_ZC_STATUS_CHANGE_ACK									= 0xBC,
	HEADER_ZC_STATUS											= 0xBD,
	HEADER_ZC_STATUS_CHANGE										= 0xBE,
	HEADER_ZC_EMOTION											= 0xC0,
	HEADER_ZC_USER_COUNT										= 0xC2,
	HEADER_ZC_SPRITE_CHANGE										= 0xC3,
	HEADER_ZC_SELECT_DEALTYPE									= 0xC4,
	HEADER_ZC_PC_PURCHASE_ITEMLIST								= 0xC6,
	HEADER_ZC_PC_SELL_ITEMLIST									= 0xC7,
	HEADER_ZC_PC_PURCHASE_RESULT								= 0xCA,
	HEADER_ZC_PC_SELL_RESULT									= 0xCB,
	HEADER_ZC_ACK_DISCONNECT_CHARACTER							= 0xCD,
	HEADER_ZC_SETTING_WHISPER_PC								= 0xD1,
	HEADER_ZC_SETTING_WHISPER_STATE								= 0xD2,
	HEADER_ZC_WHISPER_LIST										= 0xD4,
	HEADER_ZC_ACK_CREATE_CHATROOM								= 0xD6,
	HEADER_ZC_ROOM_NEWENTRY										= 0xD7,
	HEADER_ZC_DESTROY_ROOM										= 0xD8,
	HEADER_ZC_REFUSE_ENTER_ROOM									= 0xDA,
	HEADER_ZC_ENTER_ROOM										= 0xDB,
	HEADER_ZC_MEMBER_NEWENTRY									= 0xDC,
	HEADER_ZC_MEMBER_EXIT										= 0xDD,
	HEADER_ZC_CHANGE_CHATROOM									= 0xDF,
	HEADER_ZC_ROLE_CHANGE										= 0xE1,
	HEADER_ZC_REQ_EXCHANGE_ITEM									= 0xE5,
	HEADER_ZC_ACK_EXCHANGE_ITEM									= 0xE7,
	HEADER_ZC_ADD_EXCHANGE_ITEM									= 0xE9,
	HEADER_ZC_ACK_ADD_EXCHANGE_ITEM								= 0xEA,
	HEADER_ZC_CONCLUDE_EXCHANGE_ITEM							= 0xEC,
	HEADER_ZC_CANCEL_EXCHANGE_ITEM								= 0xEE,
	HEADER_ZC_EXEC_EXCHANGE_ITEM								= 0xF0,
	HEADER_ZC_EXCHANGEITEM_UNDO									= 0xF1,
	HEADER_ZC_NOTIFY_STOREITEM_COUNTINFO						= 0xF2,
	HEADER_ZC_ADD_ITEM_TO_STORE									= 0xF4,
	HEADER_ZC_DELETE_ITEM_FROM_STORE							= 0xF6,
	HEADER_ZC_CLOSE_STORE										= 0xF8,
	HEADER_ZC_ACK_MAKE_GROUP									= 0xFA,
	HEADER_ZC_GROUP_LIST										= 0xFB,
	HEADER_ZC_ACK_REQ_JOIN_GROUP								= 0xFD,
	HEADER_ZC_REQ_JOIN_GROUP									= 0xFE,
	HEADER_ZC_GROUPINFO_CHANGE									= 0x101,
	HEADER_ZC_ADD_MEMBER_TO_GROUP								= 0x104,
	HEADER_ZC_DELETE_MEMBER_FROM_GROUP							= 0x105,
	HEADER_ZC_NOTIFY_HP_TO_GROUPM								= 0x106,
	HEADER_ZC_NOTIFY_POSITION_TO_GROUPM							= 0x107,
	HEADER_ZC_NOTIFY_CHAT_PARTY									= 0x109,
	HEADER_ZC_MVP_GETTING_ITEM									= 0x10A,
	HEADER_ZC_MVP_GETTING_SPECIAL_EXP							= 0x10B,
	HEADER_ZC_MVP												= 0x10C,
	HEADER_ZC_THROW_MVPITEM										= 0x10D,
	HEADER_ZC_SKILLINFO_UPDATE									= 0x10E,
	HEADER_ZC_SKILLINFO_LIST									= 0x10F,
	HEADER_ZC_ACK_TOUSESKILL									= 0x110,
	HEADER_ZC_ADD_SKILL											= 0x111,
	HEADER_ZC_NOTIFY_SKILL										= 0x114,
	HEADER_ZC_NOTIFY_SKILL_POSITION								= 0x115,
	HEADER_ZC_NOTIFY_GROUNDSKILL								= 0x117,
	HEADER_ZC_STATE_CHANGE										= 0x119,
	HEADER_ZC_USE_SKILL											= 0x11A,
	HEADER_ZC_WARPLIST											= 0x11C,
	HEADER_ZC_ACK_REMEMBER_WARPPOINT							= 0x11E,
	HEADER_ZC_SKILL_ENTRY										= 0x11F,
	HEADER_ZC_SKILL_DISAPPEAR									= 0x120,
	HEADER_ZC_NOTIFY_CARTITEM_COUNTINFO							= 0x121,
	HEADER_ZC_CART_EQUIPMENT_ITEMLIST							= 0x122,
	HEADER_ZC_CART_NORMAL_ITEMLIST								= 0x123,
	HEADER_ZC_ADD_ITEM_TO_CART									= 0x124,
	HEADER_ZC_DELETE_ITEM_FROM_CART								= 0x125,
	HEADER_ZC_CARTOFF											= 0x12B,
	HEADER_ZC_ACK_ADDITEM_TO_CART								= 0x12C,
	HEADER_ZC_OPENSTORE											= 0x12D,
	HEADER_ZC_STORE_ENTRY										= 0x131,
	HEADER_ZC_DISAPPEAR_ENTRY									= 0x132,
	HEADER_ZC_PC_PURCHASE_ITEMLIST_FROMMC						= 0x133,
	HEADER_ZC_PC_PURCHASE_RESULT_FROMMC							= 0x135,
	HEADER_ZC_PC_PURCHASE_MYITEMLIST							= 0x136,
	HEADER_ZC_DELETEITEM_FROM_MCSTORE							= 0x137,
	HEADER_ZC_ATTACK_FAILURE_FOR_DISTANCE						= 0x139,
	HEADER_ZC_ATTACK_RANGE										= 0x13A,
	HEADER_ZC_ACTION_FAILURE									= 0x13B,
	HEADER_ZC_EQUIP_ARROW										= 0x13C,
	HEADER_ZC_RECOVERY											= 0x13D,
	HEADER_ZC_USESKILL_ACK										= 0x13E,
	HEADER_ZC_COUPLESTATUS										= 0x141,
	HEADER_ZC_OPEN_EDITDLG										= 0x142,
	HEADER_ZC_COMPASS											= 0x144,
	HEADER_ZC_SHOW_IMAGE										= 0x145,
	HEADER_ZC_AUTORUN_SKILL										= 0x147,
	HEADER_ZC_RESURRECTION										= 0x148,
	HEADER_ZC_ACK_GIVE_MANNER_POINT								= 0x14A,
	HEADER_ZC_NOTIFY_MANNER_POINT_GIVEN							= 0x14B,
	HEADER_ZC_MYGUILD_BASIC_INFO								= 0x14C,
	HEADER_ZC_ACK_GUILD_MENUINTERFACE							= 0x14E,
	HEADER_ZC_GUILD_INFO										= 0x150,
	HEADER_ZC_GUILD_EMBLEM_IMG									= 0x152,
	HEADER_ZC_MEMBERMGR_INFO									= 0x154,
	HEADER_ZC_ACK_REQ_CHANGE_MEMBERS							= 0x156,
	HEADER_ZC_ACK_OPEN_MEMBER_INFO								= 0x158,
	HEADER_ZC_ACK_LEAVE_GUILD									= 0x15A,
	HEADER_ZC_ACK_BAN_GUILD										= 0x15C,
	HEADER_ZC_ACK_DISORGANIZE_GUILD_RESULT						= 0x15E,
	HEADER_ZC_ACK_DISORGANIZE_GUILD								= 0x15F,
	HEADER_ZC_POSITION_INFO										= 0x160,
	HEADER_ZC_GUILD_SKILLINFO									= 0x162,
	HEADER_ZC_BAN_LIST											= 0x163,
	HEADER_ZC_OTHER_GUILD_LIST									= 0x164,
	HEADER_ZC_POSITION_ID_NAME_INFO								= 0x166,
	HEADER_ZC_RESULT_MAKE_GUILD									= 0x167,
	HEADER_ZC_ACK_REQ_JOIN_GUILD								= 0x169,
	HEADER_ZC_REQ_JOIN_GUILD									= 0x16A,
	HEADER_ZC_UPDATE_GDID										= 0x16C,
	HEADER_ZC_UPDATE_CHARSTAT									= 0x16D,
	HEADER_ZC_GUILD_NOTICE										= 0x16F,
	HEADER_ZC_REQ_ALLY_GUILD									= 0x171,
	HEADER_ZC_ACK_REQ_ALLY_GUILD								= 0x173,
	HEADER_ZC_ACK_CHANGE_GUILD_POSITIONINFO						= 0x174,
	HEADER_ZC_ACK_GUILD_MEMBER_INFO								= 0x176,
	HEADER_ZC_ITEMIDENTIFY_LIST									= 0x177,
	HEADER_ZC_ACK_ITEMIDENTIFY									= 0x179,
	HEADER_ZC_ITEMCOMPOSITION_LIST								= 0x17B,
	HEADER_ZC_ACK_ITEMCOMPOSITION								= 0x17D,
	HEADER_ZC_GUILD_CHAT										= 0x17F,
	HEADER_ZC_ACK_REQ_HOSTILE_GUILD								= 0x181,
	HEADER_ZC_MEMBER_ADD										= 0x182,
	HEADER_ZC_DELETE_RELATED_GUILD								= 0x184,
	HEADER_ZC_ADD_RELATED_GUILD									= 0x185,
	HEADER_ZC_ACK_ITEMREFINING									= 0x188,
	HEADER_ZC_NOTIFY_MAPINFO									= 0x189,
	HEADER_ZC_ACK_REQ_DISCONNECT								= 0x18B,
	HEADER_ZC_MONSTER_INFO										= 0x18C,
	HEADER_ZC_MAKABLEITEMLIST									= 0x18D,
	HEADER_ZC_ACK_REQMAKINGITEM									= 0x18F,
	HEADER_ZC_TALKBOX_CHATCONTENTS								= 0x191,
	HEADER_ZC_UPDATE_MAPINFO									= 0x192,
	HEADER_ZC_ACK_REQNAME_BYGID									= 0x194,
	HEADER_ZC_ACK_REQNAMEALL									= 0x195,
	HEADER_ZC_MSG_STATE_CHANGE									= 0x196,
	HEADER_ZC_NOTIFY_MAPPROPERTY								= 0x199,
	HEADER_ZC_NOTIFY_RANKING									= 0x19A,
	HEADER_ZC_NOTIFY_EFFECT										= 0x19B,
	HEADER_ZC_START_CAPTURE										= 0x19E,
	HEADER_ZC_TRYCAPTURE_MONSTER								= 0x1A0,
	HEADER_ZC_PROPERTY_PET										= 0x1A2,
	HEADER_ZC_FEED_PET											= 0x1A3,
	HEADER_ZC_CHANGESTATE_PET									= 0x1A4,
	HEADER_ZC_PETEGG_LIST										= 0x1A6,
	HEADER_ZC_PET_ACT											= 0x1AA,
	HEADER_ZC_PAR_CHANGE_USER									= 0x1AB,
	HEADER_ZC_SKILL_UPDATE										= 0x1AC,
	HEADER_ZC_MAKINGARROW_LIST									= 0x1AD,
	HEADER_ZC_NPCSPRITE_CHANGE									= 0x1B0,
	HEADER_ZC_SHOWDIGIT											= 0x1B1,
	HEADER_ZC_SHOW_IMAGE2										= 0x1B3,
	HEADER_ZC_CHANGE_GUILD										= 0x1B4,
	HEADER_ZC_GUILD_INFO2										= 0x1B6,
	HEADER_ZC_GUILD_ZENY_ACK									= 0x1B8,
	HEADER_ZC_DISPEL											= 0x1B9,
	HEADER_ZC_REPLY_REMAINTIME									= 0x1C1,
	HEADER_ZC_INFO_REMAINTIME									= 0x1C2,
	HEADER_ZC_BROADCAST2										= 0x1C3,
	HEADER_ZC_ADD_ITEM_TO_STORE2								= 0x1C4,
	HEADER_ZC_ADD_ITEM_TO_CART2									= 0x1C5,
	HEADER_ZC_USE_ITEM_ACK2										= 0x1C8,
	HEADER_ZC_SKILL_ENTRY2										= 0x1C9,
	HEADER_ZC_MONSTER_TALK										= 0x1CC,
	HEADER_ZC_AUTOSPELLLIST										= 0x1CD,
	HEADER_ZC_DEVOTIONLIST										= 0x1CF,
	HEADER_ZC_SPIRITS											= 0x1D0,
	HEADER_ZC_BLADESTOP											= 0x1D1,
	HEADER_ZC_COMBODELAY										= 0x1D2,
	HEADER_ZC_SOUND												= 0x1D3,
	HEADER_ZC_OPEN_EDITDLGSTR									= 0x1D4,
	HEADER_ZC_NOTIFY_MAPPROPERTY2								= 0x1D6,
	HEADER_ZC_SPRITE_CHANGE2									= 0x1D7,
	HEADER_ZC_NOTIFY_STANDENTRY2								= 0x1D8,
	HEADER_ZC_NOTIFY_NEWENTRY2									= 0x1D9,
	HEADER_ZC_NOTIFY_MOVEENTRY2									= 0x1DA,
	HEADER_ZC_NOTIFY_SKILL2										= 0x1DE,
	HEADER_ZC_ACK_ACCOUNTNAME									= 0x1E0,
	HEADER_ZC_SPIRITS2											= 0x1E1,
	HEADER_ZC_REQ_COUPLE										= 0x1E2,
	HEADER_ZC_START_COUPLE										= 0x1E4,
	HEADER_ZC_COUPLENAME										= 0x1E6,
	HEADER_ZC_ADD_MEMBER_TO_GROUP2								= 0x1E9,
	HEADER_ZC_CONGRATULATION									= 0x1EA,
	HEADER_ZC_NOTIFY_POSITION_TO_GUILDM							= 0x1EB,
	HEADER_ZC_GUILD_MEMBER_MAP_CHANGE							= 0x1EC,
	HEADER_ZC_NORMAL_ITEMLIST2									= 0x1EE,
	HEADER_ZC_CART_NORMAL_ITEMLIST2								= 0x1EF,
	HEADER_ZC_STORE_NORMAL_ITEMLIST2							= 0x1F0,
	HEADER_ZC_UPDATE_CHARSTAT2									= 0x1F2,
	HEADER_ZC_NOTIFY_EFFECT2									= 0x1F3,
	HEADER_ZC_REQ_EXCHANGE_ITEM2								= 0x1F4,
	HEADER_ZC_ACK_EXCHANGE_ITEM2								= 0x1F5,
	HEADER_ZC_REQ_BABY											= 0x1F6,
	HEADER_ZC_START_BABY										= 0x1F8,
	HEADER_ZC_REPAIRITEMLIST									= 0x1FC,
	HEADER_ZC_ACK_ITEMREPAIR									= 0x1FE,
	HEADER_ZC_HIGHJUMP											= 0x1FF,
	HEADER_ZC_FRIENDS_LIST										= 0x201,
	HEADER_ZC_DIVORCE											= 0x205,
	HEADER_ZC_FRIENDS_STATE										= 0x206,
	HEADER_ZC_REQ_ADD_FRIENDS									= 0x207,
	HEADER_ZC_ADD_FRIENDS_LIST									= 0x209,
	HEADER_ZC_DELETE_FRIENDS									= 0x20A,
	HEADER_ZC_STARSKILL											= 0x20E,
	HEADER_ZC_ACK_PVPPOINT										= 0x210,
	HEADER_ZC_ACK_STATUS_GM										= 0x214,
	HEADER_ZC_SKILLMSG											= 0x215,
	HEADER_ZC_BABYMSG											= 0x216,
	HEADER_ZC_BLACKSMITH_RANK									= 0x219,
	HEADER_ZC_ALCHEMIST_RANK									= 0x21A,
	HEADER_ZC_BLACKSMITH_POINT									= 0x21B,
	HEADER_ZC_ALCHEMIST_POINT									= 0x21C,
	HEADER_ZC_LESSEFFECT										= 0x21E,
	HEADER_ZC_NOTIFY_PKINFO										= 0x21F,
	HEADER_ZC_NOTIFY_CRAZYKILLER								= 0x220,
	HEADER_ZC_NOTIFY_WEAPONITEMLIST								= 0x221,
	HEADER_ZC_ACK_WEAPONREFINE									= 0x223,
	HEADER_ZC_TAEKWON_POINT										= 0x224,
	HEADER_ZC_TAEKWON_RANK										= 0x226,
	HEADER_ZC_GAME_GUARD										= 0x227,
	HEADER_ZC_STATE_CHANGE3										= 0x229,
	HEADER_ZC_NOTIFY_STANDENTRY3								= 0x22A,
	HEADER_ZC_NOTIFY_NEWENTRY3									= 0x22B,
	HEADER_ZC_NOTIFY_MOVEENTRY3									= 0x22C,
	HEADER_ZC_PROPERTY_HOMUN									= 0x22E,
	HEADER_ZC_CHANGESTATE_MER									= 0x230,
	HEADER_ZC_REQ_STORE_PASSWORD								= 0x23A,
	HEADER_ZC_RESULT_STORE_PASSWORD								= 0x23C,
	HEADER_ZC_MAIL_REQ_GET_LIST									= 0x240,
	HEADER_ZC_MAIL_REQ_OPEN										= 0x242,
	HEADER_ZC_MAIL_REQ_GET_ITEM									= 0x245,
	HEADER_ZC_MAIL_REQ_SEND										= 0x249,
	HEADER_ZC_MAIL_RECEIVE										= 0x24A,
	HEADER_ZC_AUCTION_RESULT									= 0x250,
	HEADER_ZC_AUCTION_ITEM_REQ_SEARCH							= 0x252,
	HEADER_ZC_STARPLACE											= 0x253,
	HEADER_ZC_ACK_MAIL_ADD_ITEM									= 0x255,
	HEADER_ZC_ACK_AUCTION_ADD_ITEM								= 0x256,
	HEADER_ZC_ACK_MAIL_DELETE									= 0x257,
	HEADER_ZC_MAKINGITEM_LIST									= 0x25A,
	HEADER_ZC_AUCTION_ACK_MY_SELL_STOP							= 0x25E,
	HEADER_ZC_AUCTION_WINDOWS									= 0x25F,
	HEADER_ZC_MAIL_WINDOWS										= 0x260,
	HEADER_ZC_ACK_MAIL_RETURN									= 0x274,
	HEADER_ZC_NOTIFY_PCBANG										= 0x278,
	HEADER_ZC_HUNTINGLIST										= 0x27A,
	HEADER_ZC_PCBANG_EFFECT										= 0x27B,
	HEADER_ZC_PROPERTY_MERCE									= 0x27D,
	HEADER_ZC_SHANDA_PROTECT									= 0x27E,
	HEADER_ZC_GANGSI_POINT										= 0x280,
	HEADER_ZC_GANGSI_RANK										= 0x282,
	HEADER_ZC_AID												= 0x283,
	HEADER_ZC_NOTIFY_EFFECT3									= 0x284,
	HEADER_ZC_DEATH_QUESTION									= 0x285,
	HEADER_ZC_PC_CASH_POINT_ITEMLIST							= 0x287,
	HEADER_ZC_PC_CASH_POINT_UPDATE								= 0x289,
	HEADER_ZC_NPC_SHOWEFST_UPDATE								= 0x28A,
	HEADER_ZC_MSG												= 0x291,
	HEADER_ZC_BOSS_INFO											= 0x293,
	HEADER_ZC_READ_BOOK											= 0x294,
	HEADER_ZC_EQUIPMENT_ITEMLIST2								= 0x295,
	HEADER_ZC_STORE_EQUIPMENT_ITEMLIST2							= 0x296,
	HEADER_ZC_CART_EQUIPMENT_ITEMLIST2							= 0x297,
	HEADER_ZC_CASH_TIME_COUNTER									= 0x298,
	HEADER_ZC_CASH_ITEM_DELETE									= 0x299,
	HEADER_ZC_ITEM_PICKUP_ACK2									= 0x29A,
	HEADER_ZC_MER_INIT											= 0x29B,
	HEADER_ZC_MER_PROPERTY										= 0x29C,
	HEADER_ZC_MER_SKILLINFO_LIST								= 0x29D,
	HEADER_ZC_MER_SKILLINFO_UPDATE								= 0x29E,
	HEADER_ZC_MER_PAR_CHANGE									= 0x2A2,
	HEADER_ZC_GAMEGUARD_LINGO_KEY								= 0x2A3,
	HEADER_ZC_REQ_CASH_PASSWORD									= 0x2AA,
	HEADER_ZC_RESULT_CASH_PASSWORD								= 0x2AC,
	HEADER_ZC_ALL_QUEST_LIST									= 0x2B1,
	HEADER_ZC_ALL_QUEST_MISSION									= 0x2B2,
	HEADER_ZC_ADD_QUEST											= 0x2B3,
	HEADER_ZC_DEL_QUEST											= 0x2B4,
	HEADER_ZC_UPDATE_MISSION_HUNT								= 0x2B5,
	HEADER_ZC_ACTIVE_QUEST										= 0x2B7,
	HEADER_ZC_ITEM_PICKUP_PARTY									= 0x2B8,
	HEADER_ZC_SHORTCUT_KEY_LIST									= 0x2B9,
	HEADER_ZC_EQUIPITEM_DAMAGED									= 0x2BB,
	HEADER_ZC_NOTIFY_PCBANG_PLAYING_TIME						= 0x2BC,
	HEADER_ZC_SRHEADERR2_INIT									= 0x2BF,
	HEADER_ZC_NPC_CHAT											= 0x2C1,
	HEADER_ZC_FORMATSTRING_MSG									= 0x2C2,
	HEADER_ZC_PARTY_JOIN_REQ_ACK								= 0x2C5,
	HEADER_ZC_PARTY_JOIN_REQ									= 0x2C6,
	HEADER_ZC_PARTY_CONFIG										= 0x2C9,
	HEADER_ZC_MEMORIALDUNGEON_SUBSCRIPTION_INFO					= 0x2CB,
	HEADER_ZC_MEMORIALDUNGEON_SUBSCRIPTION_NOTIFY				= 0x2CC,
	HEADER_ZC_MEMORIALDUNGEON_INFO								= 0x2CD,
	HEADER_ZC_MEMORIALDUNGEON_NOTIFY							= 0x2CE,
	HEADER_ZC_EQUIPMENT_ITEMLIST3								= 0x2D0,
	HEADER_ZC_STORE_EQUIPMENT_ITEMLIST3							= 0x2D1,
	HEADER_ZC_CART_EQUIPMENT_ITEMLIST3							= 0x2D2,
	HEADER_ZC_NOTIFY_BIND_ON_EQUIP								= 0x2D3,
	HEADER_ZC_ITEM_PICKUP_ACK3									= 0x2D4,
	HEADER_ZC_ISVR_DISCONNECT									= 0x2D5,
	HEADER_ZC_EQUIPWIN_MICROSCOPE								= 0x2D7,
	HEADER_ZC_CONFIG											= 0x2D9,
	HEADER_ZC_CONFIG_NOTIFY										= 0x2DA,
	HEADER_ZC_BATTLEFIELD_CHAT									= 0x2DC,
	HEADER_ZC_BATTLEFIELD_NOTIFY_CAMPINFO						= 0x2DD,
	HEADER_ZC_BATTLEFIELD_NOTIFY_POINT							= 0x2DE,
	HEADER_ZC_BATTLEFIELD_NOTIFY_POSITION						= 0x2DF,
	HEADER_ZC_BATTLEFIELD_NOTIFY_HP								= 0x2E0,
	HEADER_ZC_NOTIFY_ACT2										= 0x2E1,
	HEADER_ZC_MAPPROPERTY										= 0x2E7,
	HEADER_ZC_NORMAL_ITEMLIST3									= 0x2E8,
	HEADER_ZC_CART_NORMAL_ITEMLIST3								= 0x2E9,
	HEADER_ZC_STORE_NORMAL_ITEMLIST3							= 0x2EA,
	HEADER_ZC_ACCEPT_ENTER2										= 0x2EB,
	HEADER_ZC_NOTIFY_MOVEENTRY4									= 0x2EC,
	HEADER_ZC_NOTIFY_NEWENTRY4									= 0x2ED,
	HEADER_ZC_NOTIFY_STANDENTRY4								= 0x2EE,
	HEADER_ZC_NOTIFY_FONT										= 0x2EF,
	HEADER_ZC_PROGRESS											= 0x2F0,
	HEADER_ZC_PROGRESS_CANCEL									= 0x2F2,
	HEADER_ZC_IRMAIL_SEND_RES									= 0x2F4,
	HEADER_ZC_IRMAIL_NOTIFY										= 0x2F5,
	HEADER_ZC_SIMPLE_CASHSHOP_POINT_ITEMLIST					= 0x35D,
	HEADER_ZC_SKILL_POSTDELAY									= 0x43D,
	HEADER_ZC_SKILL_POSTDELAY_LIST								= 0x43E,
	HEADER_ZC_MSG_STATE_CHANGE2									= 0x43F,
	HEADER_ZC_MILLENNIUMSHIELD									= 0x440,
	HEADER_ZC_SKILLINFO_DELETE									= 0x441,
	HEADER_ZC_SKILL_SELECT_REQUEST								= 0x442,
	HEADER_ZC_SIMPLE_CASH_POINT_ITEMLIST						= 0x444,
	HEADER_ZC_QUEST_NOTIFY_EFFECT								= 0x446,
	HEADER_ZC_HACKSH_ERROR_MSG									= 0x449,
	HEADER_ZC_ES_RESULT											= 0x7D0,
	HEADER_ZC_ES_LIST											= 0x7D2,
	HEADER_ZC_ES_READY											= 0x7D5,
	HEADER_ZC_ES_GOTO											= 0x7D6,
	HEADER_ZC_REQ_GROUPINFO_CHANGE_V2							= 0x7D8,
	HEADER_ZC_SHORTCUT_KEY_LIST_V2								= 0x7D9,
	HEADER_ZC_HO_PAR_CHANGE										= 0x7DB,
	HEADER_ZC_SEEK_PARTY										= 0x7DD,
	HEADER_ZC_SEEK_PARTY_MEMBER									= 0x7DF,
	HEADER_ZC_ES_NOTI_MYINFO									= 0x7E0,
	HEADER_ZC_SKILLINFO_UPDATE2									= 0x7E1,
	HEADER_ZC_MSG_VALUE											= 0x7E2,
	HEADER_ZC_ITEMLISTWIN_OPEN									= 0x7E3,
	HEADER_ZC_MSG_SKILL											= 0x7E6,
	HEADER_ZC_BATTLE_FIELD_LIST									= 0x7EB,
	HEADER_ZC_JOIN_BATTLE_FIELD									= 0x7ED,
	HEADER_ZC_CANCEL_BATTLE_FIELD								= 0x7EF,
	HEADER_ZC_ACK_BATTLE_STATE_MONITOR							= 0x7F1,
	HEADER_ZC_BATTLE_NOTI_START_STEP							= 0x7F2,
	HEADER_ZC_BATTLE_JOIN_NOTI_DEFER							= 0x7F3,
	HEADER_ZC_BATTLE_JOIN_DISABLE_STATE							= 0x7F4,
	HEADER_ZC_NOTIFY_EXP										= 0x7F6,
	HEADER_ZC_NOTIFY_MOVEENTRY7									= 0x7F7,
	HEADER_ZC_NOTIFY_NEWENTRY5									= 0x7F8,
	HEADER_ZC_NOTIFY_STANDENTRY5								= 0x7F9,
	HEADER_ZC_DELETE_ITEM_FROM_BODY								= 0x7FA,
	HEADER_ZC_USESKILL_ACK2										= 0x7FB,
	HEADER_ZC_CHANGE_GROUP_MASTER								= 0x7FC,
	HEADER_ZC_PLAY_NPC_BGM										= 0x7FE,
	HEADER_ZC_DEFINE_CHECK										= 0x7FF,
	HEADER_ZC_PC_PURCHASE_ITEMLIST_FROMMC2						= 0x800,
	HEADER_ZC_PARTY_BOOKING_ACK_REGISTER						= 0x803,
	HEADER_ZC_PARTY_BOOKING_ACK_SEARCH							= 0x805,
	HEADER_ZC_PARTY_BOOKING_ACK_DELETE							= 0x807,
	HEADER_ZC_PARTY_BOOKING_NOTIFY_INSERT						= 0x809,
	HEADER_ZC_PARTY_BOOKING_NOTIFY_UPDATE						= 0x80A,
	HEADER_ZC_PARTY_BOOKING_NOTIFY_DELETE						= 0x80B,
	HEADER_ZC_SIMPLE_CASH_BTNSHOW								= 0x80D,
	HEADER_ZC_NOTIFY_HP_TO_GROUPM_R2							= 0x80E,
	HEADER_ZC_ADD_EXCHANGE_ITEM2								= 0x80F,
	HEADER_ZC_OPEN_BUYING_STORE									= 0x810,
	HEADER_ZC_FAILED_OPEN_BUYING_STORE_TO_BUYER					= 0x812,
	HEADER_ZC_MYITEMLIST_BUYING_STORE							= 0x813,
	HEADER_ZC_BUYING_STORE_ENTRY								= 0x814,
	HEADER_ZC_DISAPPEAR_BUYING_STORE_ENTRY						= 0x816,
	HEADER_ZC_ACK_ITEMLIST_BUYING_STORE							= 0x818,
	HEADER_ZC_FAILED_TRADE_BUYING_STORE_TO_BUYER				= 0x81A,
	HEADER_ZC_UPDATE_ITEM_FROM_BUYING_STORE						= 0x81B,
	HEADER_ZC_ITEM_DELETE_BUYING_STORE							= 0x81C,
	HEADER_ZC_EL_INIT											= 0x81D,
	HEADER_ZC_EL_PAR_CHANGE										= 0x81E,
	HEADER_ZC_BROADCAST4										= 0x81F,
	HEADER_ZC_COSTUME_SPRITE_CHANGE								= 0x820,
	HEADER_ZC_FAILED_TRADE_BUYING_STORE_TO_SELLER				= 0x824,
	HEADER_ZC_SEARCH_STORE_INFO_ACK								= 0x836,
	HEADER_ZC_SEARCH_STORE_INFO_FAILED							= 0x837,
	HEADER_ZC_ACK_BAN_GUILD_SSO									= 0x839,
	HEADER_ZC_OPEN_SEARCH_STORE_INFO							= 0x83A,
	HEADER_ZC_SSILIST_ITEM_CLICK_ACK							= 0x83D,
	HEADER_ZC_SEARCH_STORE_OPEN_INFO							= 0x83F,
	HEADER_ZC_SE_CASHSHOP_OPEN									= 0x845,
	HEADER_ZC_ACK_SE_CASH_ITEM_LIST								= 0x847,
	HEADER_ZC_SE_PC_BUY_CASHITEM_RESULT							= 0x849,
	HEADER_ZC_ITEM_FALL_ENTRY4									= 0x84B,
	HEADER_ZC_GPK_DYNCODE										= 0x851,
	HEADER_ZC_MACRO_ITEMPICKUP_FAIL								= 0x855,
	HEADER_ZC_NOTIFY_MOVEENTRY8									= 0x856,
	HEADER_ZC_NOTIFY_STANDENTRY7								= 0x857,
	HEADER_ZC_NOTIFY_NEWENTRY6									= 0x858,
	HEADER_ZC_EQUIPWIN_MICROSCOPE2								= 0x859,
	HEADER_ZC_START_COLLECTION									= 0x8B4,
	HEADER_ZC_TRYCOLLECTION										= 0x8B6,
	HEADER_ZC_ACK_SE_CASH_ITEM_LIST2							= 0x8C0,
	HEADER_ZC_SKILL_ENTRY3										= 0x8C7,
	HEADER_ZC_NOTIFY_ACT3										= 0x8C8,
	HEADER_ZC_PERSONAL_INFOMATION								= 0x8CB,
	HEADER_ZC_STOPMOVE_FORCE									= 0x8CD,
	HEADER_ZC_FAILED_GET_ITEM_FROM_ZONEDA						= 0x8CE,
	HEADER_ZC_SPIRITS_ATTRIBUTE									= 0x8CF,
	HEADER_ZC_REQ_WEAR_EQUIP_ACK2								= 0x8D0,
	HEADER_ZC_REQ_TAKEOFF_EQUIP_ACK2							= 0x8D1,
	HEADER_ZC_FASTMOVE											= 0x8D2,
	HEADER_ZC_SE_CASHSHOP_UPDATE								= 0x8D3,
	HEADER_ZC_CLEAR_DIALOG										= 0x8D6,
	HEADER_ZC_ACK_ENTRY_QUEUE_APPLY								= 0x8D8,
	HEADER_ZC_NOTIFY_ENTRY_QUEUE_APPLY							= 0x8D9,
	HEADER_ZC_ACK_ENTRY_QUEUE_CANCEL							= 0x8DB,
	HEADER_ZC_NOTIFY_ENTRY_QUEUE_ADMISSION						= 0x8DC,
	HEADER_ZC_REPLY_ACK_ENTRY_QUEUE_ADMISSION					= 0x8DE,
	HEADER_ZC_NOTIFY_LOBBY_ADMISSION							= 0x8DF,
	HEADER_ZC_REPLY_ACK_LOBBY_ADMISSION							= 0x8E1,
	HEADER_ZC_NAVIGATION_ACTIVE									= 0x8E2,
	HEADER_ZC_PARTY_RECRUIT_ACK_REGISTER						= 0x8E6,
	HEADER_ZC_PARTY_RECRUIT_ACK_DELETE							= 0x8EA,
	HEADER_ZC_PARTY_RECRUIT_NOTIFY_INSERT						= 0x8EC,
	HEADER_ZC_PARTY_RECRUIT_NOTIFY_UPDATE						= 0x8ED,
	HEADER_ZC_PARTY_RECRUIT_NOTIFY_DELETE						= 0x8EE,
	HEADER_ZC_PARTY_RECRUIT_VOLUNTEER_INFO						= 0x8F2,
	HEADER_ZC_PARTY_RECRUIT_RECALL_COST							= 0x8F6,
	HEADER_ZC_PARTY_RECRUIT_FAILED_RECALL						= 0x8F8,
	HEADER_ZC_PARTY_RECRUIT_REFUSE_VOLUNTEER					= 0x8FA,
	HEADER_ZC_EFST_SET_ENTER									= 0x8FF,
	HEADER_ZC_INVENTORY_ITEMLIST_NORMAL							= 0x900,
	HEADER_ZC_INVENTORY_ITEMLIST_EQUIP							= 0x901,
	HEADER_ZC_INVENTORY_TAB										= 0x908,
	HEADER_ZC_PARTY_RECRUIT_CANCEL_VOLUNTEER					= 0x909,
	HEADER_ZC_PARTY_RECRUIT_ADD_FILTERLINGLIST					= 0x90B,
	HEADER_ZC_PARTY_RECRUIT_SUB_FILTERLINGLIST					= 0x90C,
	HEADER_ZC_PREMIUM_CAMPAIGN_INFO								= 0x90D,
	HEADER_ZC_ENTRY_QUEUE_INIT									= 0x90E,
	HEADER_ZC_NOTIFY_NEWENTRY7									= 0x90F,
	HEADER_ZC_ACK_PARTY_NAME									= 0x911,
	HEADER_ZC_NOTIFY_MOVEENTRY9									= 0x914,
	HEADER_ZC_NOTIFY_STANDENTRY8								= 0x915,
	HEADER_ZC_PRNPC_STATE										= 0x91B,
	HEADER_ZC_PARTY_RECRUIT_CANCEL_VOLUNTEER_TO_PM				= 0x91C,
	HEADER_ZC_ACK_MERGE_ITEM									= 0x96F,
	HEADER_ZC_PARTY_RECRUIT_REFUSE_VOLUNTEER_TO_PM				= 0x971,
	HEADER_ZC_WAIT_DIALOG2										= 0x973,
	HEADER_ZC_HP_INFO											= 0x977,
	HEADER_ZC_ACK_BEFORE_WORLD_INFO								= 0x979,
	HEADER_ZC_ALL_QUEST_LIST2									= 0x97A,
	HEADER_ZC_PERSONAL_INFOMATION2								= 0x97B,
	HEADER_ZC_ACK_RANKING										= 0x97D,
	HEADER_ZC_UPDATE_RANKING_POINT								= 0x97E,
	HEADER_ZC_PERSONAL_INFOMATION_CHN							= 0x981,
	HEADER_ZC_FATIGUE_CHN										= 0x982,
	HEADER_ZC_MSG_STATE_CHANGE3									= 0x983,
	HEADER_ZC_EFST_SET_ENTER2									= 0x984,
	HEADER_ZC_SKILL_POSTDELAY_LIST2								= 0x985,
	HEADER_ZC_NOTIFY_CLAN_CONNECTINFO							= 0x988,
	HEADER_ZC_ACK_CLAN_LEAVE									= 0x989,
	HEADER_ZC_CLANINFO											= 0x98A,
	HEADER_ZC_NOTIFY_CLAN_CHAT									= 0x98E,
	HEADER_ZC_ITEM_PICKUP_ACK_V5								= 0x990,
	HEADER_ZC_INVENTORY_ITEMLIST_NORMAL_V5						= 0x991,
	HEADER_ZC_INVENTORY_ITEMLIST_EQUIP_V5						= 0x992,
	HEADER_ZC_CART_ITEMLIST_NORMAL_V5							= 0x993,
	HEADER_ZC_CART_ITEMLIST_EQUIP_V5							= 0x994,
	HEADER_ZC_STORE_ITEMLIST_NORMAL_V5							= 0x995,
	HEADER_ZC_STORE_ITEMLIST_EQUIP_V5							= 0x996,
	HEADER_ZC_EQUIPWIN_MICROSCOPE_V5							= 0x997,
	HEADER_ZC_ACK_WEAR_EQUIP_V5									= 0x999,
	HEADER_ZC_ACK_TAKEOFF_EQUIP_V5								= 0x99A,
	HEADER_ZC_MAPPROPERTY_R2									= 0x99B,
	HEADER_ZC_SKILL_ENTRY4										= 0x99F,
	HEADER_ZC_DISPATCH_TIMING_INFO_CHN							= 0x9A4,
	HEADER_ZC_BANKING_CHECK										= 0x9A6,
	HEADER_ZC_ACK_BANKING_DEPOSIT								= 0x9A8,
	HEADER_ZC_ACK_BANKING_WITHDRAW								= 0x9AA,
	HEADER_ZC_ACK_CASH_BARGAIN_SALE_ITEM_INFO					= 0x9AD,
	HEADER_ZC_ACK_APPLY_BARGAIN_SALE_ITEM						= 0x9AF,
	HEADER_ZC_ACK_REMOVE_BARGAIN_SALE_ITEM						= 0x9B1,
	HEADER_ZC_NOTIFY_BARGAIN_SALE_SELLING						= 0x9B2,
	HEADER_ZC_NOTIFY_BARGAIN_SALE_CLOSE							= 0x9B3,
	HEADER_ZC_OPEN_BARGAIN_SALE_TOOL							= 0x9B5,

};
#pragma pack(push,1)

#pragma warning(push)
#pragma warning(disable : 4200)

struct PACKET_CZ_SAY_DIALOG {
	WORD  PacketType;
	WORD  PacketLength;
	DWORD AID;
	BYTE  Data[];
};

struct PACKET_CZ_MENU_LIST {
	WORD  PacketType;
	WORD  PacketLength;
	DWORD AID;
	BYTE  Data[];
};

struct PACKET_CZ_SKILL_POSTDELAY {
	WORD  PacketType;
	WORD  skill_id;
	DWORD tick;
};
struct PACKET_CZ_MSG_STATE_CHANGE{
	WORD  PacketType;
	WORD  type;
	DWORD id;
	BYTE flag;
	DWORD tick;
};
struct PACKET_CZ_MSG_STATE_CHANGE2 {
	WORD  PacketType;
	WORD  type;
	DWORD id;
	BYTE flag;
	DWORD tick;
	DWORD v1;
	DWORD v2;
	DWORD v3;
};
struct PACKET_CZ_NOTIFY_EFFECT2{
	WORD  PacketType;
	DWORD id;
	DWORD type;
};
struct MEMBER{
	DWORD id;
	BYTE nick_name[24];
	BYTE map_name[16];
	BYTE leader;
	BYTE offline;
};
struct PACKET_CZ_GROUP_LIST{
	WORD PacketType;
	WORD PacketLength;
	BYTE party_name[24];
	MEMBER party_list[];
};
#pragma warning(pop)

#pragma pack(pop)
