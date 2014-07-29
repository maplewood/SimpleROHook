﻿using System;
using System.IO;
using System.Xml;
using System.Xml.Serialization;
using System.ComponentModel;
using System.Windows.Forms;
using System.Security.Permissions;


namespace SimpleROHookCS
{

    public partial class MainForm : Form,IDisposable
    {
        private SRHSharedData m_SharedData = null;
        private NPCLogger m_npcLogger;

        protected override CreateParams CreateParams
        {
            [SecurityPermission(SecurityAction.Demand,
                Flags = SecurityPermissionFlag.UnmanagedCode)]
            get
            {
                const int WS_EX_TOOLWINDOW = 0x80;
                const long WS_POPUP = 0x80000000L;
                const int WS_VISIBLE = 0x10000000;
                const int WS_SYSMENU = 0x80000;
                const int WS_MAXIMIZEBOX = 0x10000;

                CreateParams cp = base.CreateParams;
                cp.ExStyle = WS_EX_TOOLWINDOW;
                cp.Style = unchecked((int)WS_POPUP) |
                    WS_VISIBLE | WS_SYSMENU | WS_MAXIMIZEBOX;

                cp.Width = 0;
                cp.Height = 0;

                return cp;
            }
        }

        public MainForm()
        {
            InitializeComponent();
            m_SharedData = new SRHSharedData();
        }

        public new void Dispose()
        {
            m_SharedData.Dispose();
            Dispose(true);
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TaskTray_notifyIcon.Visible = false;
            Application.Exit();
        }

        private void playMusicOnClientStreamPlayerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string musicfilename = m_SharedData.musicfilename;
            var openFileDialog = new OpenFileDialog();
            openFileDialog.Title = "Ragnarok Online BGM";
            openFileDialog.InitialDirectory = musicfilename;
            openFileDialog.CheckFileExists = true;
            openFileDialog.Multiselect = false;
            openFileDialog.FileName = musicfilename;
            openFileDialog.Filter = "mp3 file|*.mp3|wave file|*.wav";
            openFileDialog.ShowReadOnly = false;
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                m_SharedData.musicfilename = openFileDialog.FileName;
                m_SharedData.executeorder = true;
            }
        }

        private void UpdateCheckMenu()
        {
            playMusicOnClientStreamPlayerToolStripMenuItem.Enabled
                = (m_SharedData.g_hROWindow == 0)? false : true;
            packetLogToolStripMenuItem.Checked
                = m_SharedData.write_packetlog;
            freeMouseToolStripMenuItem.Checked
                = m_SharedData.freemouse;

            ground_zbias_ToolStripTrackBar.Value
                = m_SharedData.ground_zbias;
            Set_ZBiasValue_groundZBiasToolStripMenuItem(m_SharedData.ground_zbias);

            alphaLeveltoolStripTrackBar.Value
                = m_SharedData.alphalevel;
            Set_AlphaLevelValue_alphaLeveltoolStripMenuItem(m_SharedData.alphalevel);

            showM2EToolStripMenuItem.Checked
                = m_SharedData.m2e;
            showBBEtoolStripMenuItem.Checked
                = m_SharedData.bbe;
            showDeadCelltoolStripMenuItem.Checked
                = m_SharedData.deadcell;
            showChatScopetoolStripMenuItem.Checked
                = m_SharedData.chatscope;
            
            CPUCooler_toolStripTrackBar.Value =
                m_SharedData.cpucoolerlevel;
            Set_CPUCoolerText_toolStripMenuItem(m_SharedData.cpucoolerlevel);

            fixWindowModeVsyncWaitToolStripMenuItem.Checked
                = m_SharedData.fix_windowmode_vsyncwait;
            showFpsToolStripMenuItem.Checked
                = m_SharedData.show_framerate;
            showObjectInformationToolStripMenuItem.Checked
                = m_SharedData.objectinformation;
            kHzAudioModeonBootToolStripMenuItem.Checked
                = m_SharedData._44khz_audiomode;

            test01ToolStripMenuItem.Checked
                = m_SharedData.test01;
            test02ToolStripMenuItem.Checked
              = m_SharedData.test02;

            nPCLoggerToolStripMenuItem.Checked = m_npcLogger.Visible;
        }


        private void kHzAudioModeonBootToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData._44khz_audiomode = tsm.Checked;
        }

        private void packetLogToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.write_packetlog = tsm.Checked;
        }

        private void freeMouseToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.freemouse = tsm.Checked;
        }

        private void showBBEtoolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.bbe = tsm.Checked;
        }
        private void showDeadCelltoolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.deadcell = tsm.Checked;
        }
        private void showChatScopetoolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.chatscope = tsm.Checked;
        }
        private void showM2EToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.m2e = tsm.Checked;
        }

        private void showFpsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.show_framerate = tsm.Checked;
        }

        private void showObjectInformationToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.objectinformation = tsm.Checked;
        }

        private void fixWindowModeVsyncWaitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.fix_windowmode_vsyncwait = tsm.Checked;
        }

        private void aboutSimpleROHookToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var aboutbox = new SRHAboutBox();
            aboutbox.ShowDialog();
        }

        private void TaskTray_contextMenuStrip_Opening(object sender, CancelEventArgs e)
        {
            UpdateCheckMenu();
        }

        private void ground_zbias_ToolStripTrackBar_Update(object sender, EventArgs e)
        {
            ToolStripTrackBar tsTrackbar = (ToolStripTrackBar)sender;
            m_SharedData.ground_zbias = tsTrackbar.Value;

            Set_ZBiasValue_groundZBiasToolStripMenuItem(m_SharedData.ground_zbias);
        }
        private void Set_ZBiasValue_groundZBiasToolStripMenuItem(int value)
        {
            groundZBiasToolStripMenuItem.Text =
                String.Format("Ground Z Bias {0}", value);
        }

        private void CPUCooler_toolStripTrackBar_Update(object sender, EventArgs e)
        {
            ToolStripTrackBar tsTrackbar = (ToolStripTrackBar)sender;
            m_SharedData.cpucoolerlevel = tsTrackbar.Value;

            Set_CPUCoolerText_toolStripMenuItem(m_SharedData.cpucoolerlevel);
        }
        private void Set_CPUCoolerText_toolStripMenuItem(int value)
        {
            if (value==0)
            {
                CPUCoolerText_toolStripMenuItem.Text = "CPU Cooler OFF";
            }
            else
            {
                CPUCoolerText_toolStripMenuItem.Text =
                    String.Format("CPU Cooler Level {0}", value);
            }
        }

        private void alphaLeveltoolStripTrackBar_Update(object sender, EventArgs e)
        {
            ToolStripTrackBar tsTrackbar = (ToolStripTrackBar)sender;
            m_SharedData.alphalevel = tsTrackbar.Value;

            Set_AlphaLevelValue_alphaLeveltoolStripMenuItem(m_SharedData.alphalevel);
        }
        private void Set_AlphaLevelValue_alphaLeveltoolStripMenuItem(int value)
        {
            alphaLeveltoolStripMenuItem.Text =
                String.Format("Alpha Level {0}", value);
        }
        private void test01ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.test01 = tsm.Checked;
        }

        private void test02ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var tsm = (ToolStripMenuItem)sender;
            m_SharedData.test02 = tsm.Checked;
        }


        private void InitTaskTrayMenu()
        {
            ground_zbias_ToolStripTrackBar.SetMinMax(0, 16);
            ground_zbias_ToolStripTrackBar.SetTickFrequency(1);
            ground_zbias_ToolStripTrackBar.SetChangeValue(1, 4);

            alphaLeveltoolStripTrackBar.SetMinMax(0, 255);
            alphaLeveltoolStripTrackBar.SetTickFrequency(4);
            alphaLeveltoolStripTrackBar.SetChangeValue(1, 16);

            CPUCooler_toolStripTrackBar.SetMinMax(0, 3);
            CPUCooler_toolStripTrackBar.SetTickFrequency(1);
            CPUCooler_toolStripTrackBar.SetChangeValue(1, 4);
        }

        private void Window_Load(object sender, EventArgs e)
        {
            InitTaskTrayMenu();

            string curentdirstr = System.IO.Directory.GetCurrentDirectory() + "\\config.ini";
            m_SharedData.configfilepath = curentdirstr;
            if (File.Exists("config.xml"))
            {
                #region Load Config XML
                using (XmlReader reader = XmlReader.Create("config.xml"))
                {
                    var configration = new Config();
                    var serializer = new XmlSerializer(typeof(Config));

                    configration = (Config)serializer.Deserialize(reader);

                    m_SharedData.write_packetlog
                        = configration.write_packetlog;
                    m_SharedData.freemouse
                        = configration.freemouse;
                    m_SharedData.ground_zbias
                        = configration.ground_zbias;
                    m_SharedData.alphalevel
                        = configration.alphalevel;
                    m_SharedData.m2e
                        = configration.m2e;
                    m_SharedData.bbe
                        = configration.bbe;
                    m_SharedData.deadcell
                        = configration.deadcell;
                    m_SharedData.chatscope
                        = configration.chatscope;
                    m_SharedData.fix_windowmode_vsyncwait
                        = configration.fix_windowmode_vsyncwait;
                    m_SharedData.show_framerate
                        = configration.show_framerate;
                    m_SharedData.objectinformation
                        = configration.objectinformation;
                    m_SharedData._44khz_audiomode
                        = configration._44khz_audiomode;
                    m_SharedData.cpucoolerlevel
                        = configration.cpucoolerlevel;
                    m_SharedData.test01
                        = configration.test01;
                    m_SharedData.test02
                        = configration.test02;
                }
                #endregion
            }

            m_npcLogger = new NPCLogger();
            m_npcLogger.StartPosition = FormStartPosition.Manual;
            m_npcLogger.Bounds = Properties.Settings.Default.LoggerWinBounds;
            m_npcLogger.WindowState = Properties.Settings.Default.LoggerWinState;

            if ( Properties.Settings.Default.LoggerWinVisible)
                m_npcLogger.Show();

        }

        private void Window_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (m_npcLogger.WindowState == FormWindowState.Normal )
                Properties.Settings.Default.LoggerWinBounds = m_npcLogger.Bounds;
            else
                Properties.Settings.Default.LoggerWinBounds = m_npcLogger.RestoreBounds;
            Properties.Settings.Default.LoggerWinState = m_npcLogger.WindowState;

            Properties.Settings.Default.LoggerWinVisible = m_npcLogger.Visible;

            Properties.Settings.Default.Save();

            m_npcLogger.Hide();

            #region Save Config XML
            using (XmlTextWriter writer = new XmlTextWriter("config.xml", System.Text.Encoding.UTF8))
            {
                var configration = new Config();
                var serializer = new XmlSerializer(typeof(Config));

                configration.write_packetlog
                    = m_SharedData.write_packetlog;
                configration.freemouse
                    = m_SharedData.freemouse;
                configration.ground_zbias
                    = m_SharedData.ground_zbias;
                configration.alphalevel
                    = m_SharedData.alphalevel;
                configration.m2e
                    = m_SharedData.m2e;
                configration.bbe
                    = m_SharedData.bbe;
                configration.deadcell
                    = m_SharedData.deadcell;
                configration.chatscope
                    = m_SharedData.chatscope;
                configration.fix_windowmode_vsyncwait
                    = m_SharedData.fix_windowmode_vsyncwait;
                configration.show_framerate
                    = m_SharedData.show_framerate;
                configration.objectinformation
                    = m_SharedData.objectinformation;
                configration._44khz_audiomode
                    = m_SharedData._44khz_audiomode;
                configration.cpucoolerlevel
                    = m_SharedData.cpucoolerlevel;
                configration.test01
                    = m_SharedData.test01;
                configration.test02
                    = m_SharedData.test02;

                writer.Formatting = Formatting.Indented;
                serializer.Serialize(writer, configration);
            }
            #endregion


        }

        private void nPCLoggerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (m_npcLogger.Visible)
                m_npcLogger.Hide();
            else
                m_npcLogger.Show();

        }

        private void notifyIcon_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                m_npcLogger.TopMost = true;
                m_npcLogger.TopMost = false;
            }
        }


    }

    public class Config
    {
        public Config()
        {
            write_packetlog = false;
            freemouse = false;
            ground_zbias = 0;
            alphalevel = 0;
            m2e = true;
            bbe = true;
            deadcell = true;
            chatscope = true;
            fix_windowmode_vsyncwait = true;
            show_framerate = true;
            objectinformation = false;
            _44khz_audiomode = false;
            cpucoolerlevel = 0;
            test01 = false;
            test02 = false;
        }

        public bool write_packetlog { get; set; }
        public bool freemouse { get; set; }
        public int ground_zbias { get; set; }
        public int alphalevel { get; set; }
        public bool m2e { get; set; }
        public bool bbe { get; set; }
        public bool deadcell { get; set; }
        public bool chatscope { get; set; }
        public bool fix_windowmode_vsyncwait { get; set; }
        public bool show_framerate { get; set; }
        public bool objectinformation { get; set; }
        public bool _44khz_audiomode { get; set; }
        public int cpucoolerlevel { get; set; }
        public bool test01 { get; set; }
        public bool test02 { get; set; }

        // without serialize
        [System.Xml.Serialization.XmlIgnoreAttribute]
        public string configfilepath { get; set; }
        [System.Xml.Serialization.XmlIgnoreAttribute]
        public string musicfilename { get; set; }
        [System.Xml.Serialization.XmlIgnoreAttribute]
        public bool executeorder { get; set; }
    }

}
