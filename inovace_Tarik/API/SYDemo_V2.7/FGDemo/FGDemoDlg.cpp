// FGDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FGDemo.h"
#include "FGDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////
#include "math.h"
#include "DlgUser.h"
#include "DlgNotepad.h"
#include "DlgFGDB.h"
#include "ImageChangeURUtoFPC.h"
#include "DLGCOM.h"

#define BLUE01	"SynoChip芯片测试程序 v2.700"
#define BLUE02	"SynoChip Demo v2.700"
#define	MAX_HDEV	10
//#define LGN_CHINA	0x0804	//语言(中文)
#define		IMG_FILE  "C:\\Finger.bmp"
//#define		IMG_FILE  "Finger.bmp"
#define		IMAGE_SIZE (256*288)
#define		IMAGE_W	256
#define		IMAGE_H	288

//#define GETIMG_BAD	1	//开启差指纹录取和搜索权限

pJsDevHandle gHead_DevHandle,gCur_DevHandle;
CFGDemoDlg* pMyDlg;
BOOL g_bStop;	//用于做取消操作
int  g_iLanguage;	//语言
BYTE gImageData[IMAGE_SIZE];
int  g_IamgeLen;
int  g_iIndex;//用户地址(索引页，用于录入指纹用)
BOOL g_bReplace;//记录当前录入是覆盖模式还是添加模式
int  g_iSelect;
int  g_iSelectIndex;
int  g_iSearchMode;	//0-搜索 1-高速搜索
BOOL g_bImgpos;//图像正反校正
BOOL g_bImgCont;//连续获取图像计数
BOOL g_bSearchCont;//连续搜索10次计数
BOOL g_bImgDemo;//图像处理演示

UINT J_gGenImg(LPVOID pParam);		//获取图像
UINT J_gGenImgCon(LPVOID pParam);	//连续获取图像
UINT J_gEnroll(LPVOID pParam);		//录入指纹
UINT J_gEnrollCon(LPVOID pParam);	//连续录入指纹
UINT J_gVerify(LPVOID pParam);		//单一比对
UINT J_gSearch(LPVOID pParam);		//指纹搜索
UINT J_gSearchCon(LPVOID pParam);	//连续搜索
UINT J_gLoadFingerMB(LPVOID pParam);//载入指纹库
UINT J_gSaveFingerMB(LPVOID pParam);//保存指纹库

#include "Shlwapi.h"
TCHAR szUserLogoPath[255] = {0};

TCHAR	gszIMGFILE[MAX_PATH]=IMG_FILE;
int		g_iIMGtype=0;//0-C 1-当前
BOOL	g_bEnrollIMG=TRUE;//注册时获取图像FALSE-老方式 TRUE-新方式
BOOL	g_bLogs = FALSE;//是否进行日志记录 2011-01-14
int g_nEN_NUM = 2; //用于不同传感器的录入——标准次数 2011-09-08
int g_nMaxTry = 2; //用于不同传感器的录入——最高次数

//---------------------------------------------------------------------
//获取文件名及路径
//输入：iMode 0-文件名及路径	1-文件名	2-路径
//输出：pstr[MAX_PATH]
int WINAPI J_GetPathName(char* pstr,int iMode);

/////////////////////////////////////////////////////////////////////////////
//辅助函数
char g_log[2048];	//日志缓冲区
int GetLOGTime(char *s)
{//获取系统时间 2011-01-14
	time_t	Time;
	struct tm *T;
	time(&Time);
	T = localtime(&Time);
	sprintf(s, "%02d%02d%02d %02d%02d%02d", T->tm_year-100, T->tm_mon + 1, T->tm_mday, T->tm_hour, T->tm_min, T->tm_sec);
	return(0);
}
int WriteRunLog(char *Log)
{//写Log数据 2011-01-14
	//TRACE(Log);//调试显示信息
	
	FILE   *fp;
	char    timestr[32], systemDir[256], logDir[256], day[8], LogFileName[256];
	BOOL    ret;
	
	
	
	ret = GetSystemDirectory(systemDir,  sizeof(systemDir));
	sprintf(logDir, "%s\\temp", systemDir);
	ret = CreateDirectory(logDir, NULL);		
	GetLOGTime(timestr);
	strncpy(day, timestr, 6);
	day[6] = '\0';
	//	sprintf(LogFileName, "%s\\uil_csp_%s.log", logDir, day);
	sprintf(LogFileName, "C:\\temp\\SYDemo_%s.txt",  day);
	
	if((fp=fopen(LogFileName, "at"))==NULL)
		return(-1);
	
	fseek(fp,0,SEEK_END);
	
	fprintf(fp, "\n[%s]: %s", timestr, Log); 
	fflush(fp);
	fclose(fp);
	return(0);
}
//////////////////////////////////////////////////////////////////////////
int strHexToInt(CString strSource,UINT *nVal)
{	
	int nTemp=0;
	
	CString strTemp;
	strTemp=strSource;
	for(char cc='G',dd='g';   cc<='Z',dd<='z';  cc++,dd++)    //判断输入的字符串是否合法
	{
		if(strTemp.Find(cc,0) !=-1  ||  strTemp.Find(dd,0) !=-1)
		{
			::MessageBox(NULL,"please input a valid hex data!","input error",MB_ICONEXCLAMATION);
			return -1;
		}
	}
	
	
	for(int i = 0;  i<(int)::strlen(strSource);  i++)
	{
		int nDecNum;
		switch(strSource[i])
		{
		case 'a':
		case 'A':	nDecNum = 10;	break;
		case 'b':
		case 'B':	nDecNum = 11;	break;
		case 'c':
		case 'C':   nDecNum = 12;   break;
		case 'd':
		case 'D':   nDecNum = 13;   break;
		case 'e':
		case 'E':   nDecNum = 14;   break;
		case 'f':
		case 'F':   nDecNum = 15;   break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':    nDecNum = strSource[i] - '0';     break;
		default:     return -1;			
        }
		nTemp += nDecNum * (int)::pow(16,::strlen(strSource)-i -1);
	}
	*nVal=nTemp;
	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFGDemoDlg dialog

CFGDemoDlg::CFGDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFGDemoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFGDemoDlg)
	m_strAddr	= _T("FFFFFFFF");
	m_strPwd	= _T("00000000");
	m_iDevType	= 0;
	m_iCom		= 0;
	m_iBaud		= -1;
	m_iDevHandle= -1;
	m_iLG		= 1;
	m_bShowBMP	= TRUE;
	m_bReplaceMB= FALSE;
	m_iBaud_x = -1;
	m_iPackSize_x = -1;
	m_iSLev_x = -1;
	m_iDevID = 0;
	m_iSensorType = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
CFGDemoDlg:: ~CFGDemoDlg()
{
	pJsDevHandle pTmp1,pTmp2;
	if(gHead_DevHandle!=NULL){
		for(pTmp1 = gHead_DevHandle;pTmp1!=NULL;){
			pTmp2 = pTmp1;
			pTmp1 = pTmp1->next;
			if(pTmp2->nDevType != DEVICE_COM)
				PSCloseDeviceEx(pTmp2->hHandle);
			delete pTmp2;
		}
	}
}

void CFGDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFGDemoDlg)
	DDX_Control(pDX, IDC_COMBO4, m_cbSensorType);
	DDX_Control(pDX, IDC_COMBO_Dev, m_ctlDevHandle);
	DDX_Control(pDX, IDC_LIST1, m_list);
	DDX_Control(pDX, IDC_EDIT_DevAddr, m_STctlAddr);
	DDX_Control(pDX, IDC_EDIT_DevPwd, m_STctlPwd);
	DDX_Control(pDX, IDC_BITMAP, m_bmp);
	DDX_Text(pDX, IDC_EDIT_DevAddr, m_strAddr);
	DDX_Text(pDX, IDC_EDIT_DevPwd, m_strPwd);
	DDX_CBIndex(pDX, IDC_COMBO_DevType, m_iDevType);
	DDX_CBIndex(pDX, IDC_COMBO_iCom, m_iCom);
	DDX_CBIndex(pDX, IDC_COMB_iBaud, m_iBaud);
	DDX_CBIndex(pDX, IDC_COMBO_Dev, m_iDevHandle);
	DDX_CBIndex(pDX, IDC_COMBO_LG, m_iLG);
	DDX_Check(pDX, IDC_CHECK1, m_bShowBMP);
	DDX_Check(pDX, IDC_CHECK2, m_bReplaceMB);
	DDX_CBIndex(pDX, IDC_COMBO1, m_iBaud_x);
	DDX_CBIndex(pDX, IDC_COMBO2, m_iPackSize_x);
	DDX_CBIndex(pDX, IDC_COMBO3, m_iSLev_x);
	DDX_CBIndex(pDX, IDC_EDIT_DevID, m_iDevID);
	DDX_CBIndex(pDX, IDC_COMBO4, m_iSensorType);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CFGDemoDlg, CDialog)
	//{{AFX_MSG_MAP(CFGDemoDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_LBUTTONDOWN()
	ON_CBN_SELCHANGE(IDC_COMBO_LG, OnSelchangeComboLg)
	ON_CBN_SELCHANGE(IDC_COMBO_Dev, OnSelchangeCOMBODev)
	ON_BN_CLICKED(IDC_CloseDev, OnCloseDev)
	ON_BN_CLICKED(IDC_CloseDev2, OnCloseDev2)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_BUTTON7, OnButton7)
	ON_BN_CLICKED(IDC_BUTTON8, OnButton8)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnClickList1)
	ON_BN_CLICKED(IDC_BUTTON9, OnButton9)
	ON_BN_CLICKED(IDC_BUTTON10, OnButton10)
	ON_BN_CLICKED(IDC_BUTTON11, OnButton11)
	ON_BN_CLICKED(IDC_BUTTON12, OnButton12)
	ON_BN_CLICKED(IDC_BUTTON13, OnButton13)
	ON_BN_CLICKED(IDC_BUTTON24, OnButton24)
	ON_BN_CLICKED(IDC_BUTTON25, OnButton25)
	ON_BN_CLICKED(IDC_BUTTON26, OnButton26)
	ON_BN_CLICKED(IDC_BUTTON18, OnButton18)
	ON_BN_CLICKED(IDC_BUTTON16, OnButton16)
	ON_BN_CLICKED(IDC_BUTTON14, OnButton14)
	ON_BN_CLICKED(IDC_BUTTON15, OnButton15)
	ON_BN_CLICKED(IDC_BUTTON19, OnButton19)
	ON_BN_CLICKED(IDC_BUTTON20, OnButton20)
	ON_BN_CLICKED(IDC_BUTTON21, OnButton21)
	ON_BN_CLICKED(IDC_BUTTON22, OnButton22)
	ON_BN_CLICKED(IDC_BUTTON23, OnButton23)
	ON_BN_CLICKED(IDC_BUTTON17, OnButton17)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, OnSelchangeCombo2)
	ON_CBN_SELCHANGE(IDC_COMBO3, OnSelchangeCombo3)
	ON_BN_CLICKED(IDC_BUTTON27, OnButton27)
	ON_BN_CLICKED(IDC_BUTTON28, OnButton28)
	ON_CBN_SELCHANGE(IDC_COMBO4, OnSelchangeCombo4)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFGDemoDlg message handlers

BOOL CFGDemoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	//...参数初始化
	pMyDlg = this;
	gHead_DevHandle = NULL;
	m_bDevOpen	= FALSE;
	m_blShowTip	= FALSE;
	m_bSpBtn	= FALSE;
	g_bImgCont	= FALSE;
	g_bSearchCont=FALSE;
	g_bStop		= TRUE;
	m_bRefBMP	= FALSE;
	g_bImgpos	= 0;
	g_iIndex	= 0;
	g_bReplace	= FALSE;
	g_iSelect	= -1;
	g_iSelectIndex	=-1;
	g_iSearchMode	= 0;
	g_iIMGtype		= 1;
	g_bEnrollIMG	= FALSE;
	g_bImgDemo	= FALSE;
	m_STctlAddr.JSetOnlyHexStr(true,8);
	m_STctlAddr.JSetColor(RGB(255,0,0));
	m_STctlPwd.JSetOnlyHexStr(true,8);
	m_STctlPwd.JSetColor(RGB(255,0,0));

	//...按钮
//	J_MyBtn(&m_sbtn1,IDOK,IDI_ICON3,CShadeButtonST::SHS_DIAGSHADE);
//	J_MyBtn(&m_sbtn2,IDCANCEL,IDI_ICON2,CShadeButtonST::SHS_DIAGSHADE);
//	J_MyBtn(&m_sbtn3,IDC_BUTTON1,0,CShadeButtonST::SHS_DIAGSHADE);
//	J_MyBtn(&m_sbtn4,IDC_CloseDev,IDI_ICON4,CShadeButtonST::SHS_DIAGSHADE);
//	J_MyBtn(&m_sbtn5,IDC_CloseDev2,IDI_ICON4,CShadeButtonST::SHS_DIAGSHADE);
//	int i=0;
//	for(i=IDC_BUTTON2; i<=IDC_BUTTON26; i++)//指令操作
//		J_MyBtn(i);

	//...列表
	DWORD dwStyle = m_list.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP;// |LVS_EX_CHECKBOXES| LVS_EX_SUBITEMIMAGES 
	m_list.SetExtendedStyle(dwStyle);
	m_list.InsertColumn(0,"FingerID",LVCFMT_CENTER,130);
//	m_list.InsertColumn(1,"FingerMsg",LVCFMT_CENTER,91);
//	J_AddFGTemp();
	m_list.SetTextColor(RGB(50,50,50));

	//...提示框
	m_tooltip.Create(this);
	m_tooltip.SetShowDelay(500);
	m_tooltip.AddTool(GetDlgItem(IDOK),_T("打开设备"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON2),_T("获取一枚指纹图像"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON3),_T("连续获取指纹图像"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON4),_T("保存当前的指纹图像到PC"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON5),_T("从PC选择一枚指纹图像，下载到下位机"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON6),_T("录入一枚指纹模版到flash指纹库"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON7),_T("连续录入指纹模版到flash指纹库"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_CHECK1),_T("是否显示指纹图像"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_CHECK2),_T("在录入时，是否自动覆盖原有的序号而不提示"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON8),_T("选中指纹库的一个序号，进行单一比对"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON9),_T("获取一枚指纹，然后在flash指纹库中搜索"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON10),_T("进行连续搜索操作，按‘取消操作’结束"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON11),_T("进行连续高速搜索操作，应用高速搜索算法"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON12),_T("从flash指纹库中删除选中的指纹模版号"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON13),_T("删除flash数据库中的所有指纹模版"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON14),_T("打开读记事本窗口"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON15),_T("打开写记事本窗口"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON16),_T("获取一组随机数"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON17),_T("打开采集指纹库窗口"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON18),_T("读取flash数据库的指纹模版数"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON24),_T("从PC下载指纹模版到flash数据库。\r\n注：.mb为模版库文件 .dat为单枚指纹特征"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON25),_T("从flash数据库上传指纹模版库到PC"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON26),_T("从flash数据库上传指定序号的指纹模版到PC"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON27),_T("@ 下载图像（256*288）\r\n流程：从PC获取指纹图像->下载图像到设备->生成特征->存储特征到flash指纹模版库"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_BUTTON28),_T("@ 图像转换\r\nURU图像转换为FPC的图像格式,225X313-->256X288"),NULL);
	m_tooltip.AddTool(GetDlgItem(IDC_COMBO4),_T("传感器类型:\r\n  0、Normal Secsor (通用类型 BufID：1，2 512) 2次手指\r\n  1、AES1711 (按4次手指 BufID：1，2，3，4 1024)\r\n  2、TYPE1 (GC0307 0x29命令)\r\n  3、TYPE2 (AES1711 3次：需3~6次手指 1024)"),NULL);
	
	//...设置初始化
	J_EnableBtn(0,0);
	g_iLanguage = GetSystemDefaultLangID();
	if(g_iLanguage==LGN_CHINA)
		m_iLG = 0;
	else
		m_iLG = 1;
	UpdateData(0);
	LoadUserDefine();
	ShowDefaltImg();
	J_SetLanuage(g_iLanguage);
	J_SetIMGFILE(g_iIMGtype);

	//字体  
	m_bBlue = TRUE;
	LOGFONT logFont;   
	logFont.lfHeight=20;   
	logFont.lfWidth=0;   
	logFont.lfEscapement=0;   
	logFont.lfOrientation=0;   
	logFont.lfWeight=FW_BOLD;   
	logFont.lfItalic=0;
	logFont.lfQuality=1;
	logFont.lfUnderline=0;
	logFont.lfStrikeOut=0;   
	logFont.lfCharSet=ANSI_CHARSET;   
	logFont.lfOutPrecision=OUT_DEFAULT_PRECIS;   
	logFont.lfClipPrecision=OUT_DEFAULT_PRECIS;   
	logFont.lfQuality=PROOF_QUALITY;   
	logFont.lfPitchAndFamily=VARIABLE_PITCH|FF_ROMAN;   
	strcpy(logFont.lfFaceName,"Times   New   Roman");   
	m_font.CreateFontIndirect(&logFont);   

	OnSelchangeCombo4();
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFGDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFGDemoDlg::OnPaint() 
{
////////////////////////////
	if(m_bRefBMP)
		ShowImage();
	else
		ShowDefaltImg();
////////////////////////////
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFGDemoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CFGDemoDlg::PreTranslateMessage(MSG* pMsg) 
{
	int iNum=0,res;
	CString str;
	if( pMsg->message == WM_KEYDOWN )//秘籍
	{
		switch(pMsg->wParam)
		{
			case VK_RETURN://屏蔽回车
				return TRUE;
			case VK_ESCAPE://屏蔽Esc
				return TRUE;
			case VK_SPACE://空格键
				if(!g_bStop)
					return TRUE;
				break;
			case '1': // 
				if( ::GetKeyState(VK_CONTROL)<0 && ::GetKeyState(VK_MENU)<0 ){//Ctrl + Alt +1
					CString str;
					int nLen=0;
					PSGetCharLen(&nLen);
					str.Format("当前指纹特征模版每枚大小：%d !  ",nLen);
					AfxMessageBox(str);
				//	g_bEnrollIMG = !g_bEnrollIMG;
				//	if(g_bEnrollIMG)//注册时获取图像，使用新方法PSGetImage_Enroll
				//		AfxMessageBox("注意:注册时获取图像使用新方法 0x29 !");
				//	else//注册时获取图像，使用老方法PSGetImage
				//		AfxMessageBox("注意:注册时获取图像使用老方法 0x01 !");
					return TRUE;
				}
				else if(::GetKeyState(VK_CONTROL)<0)// Ctrl + 1
				{
					if(m_bDevOpen){
						m_bDevOpen = FALSE;
						J_EnableBtn(0,0);
						AfxMessageBox("注意:设备打开状态已置为 关闭!");
					}
					else{
						m_bDevOpen = TRUE;
						J_EnableBtn();
						AfxMessageBox("注意:设备打开状态已置为 打开!");
					}
					return TRUE;
				}
				break;
			case '2':	//Ctrl + 2 : 开启/关闭 ToolTip
				if( ::GetKeyState(VK_CONTROL)<0){
					m_blShowTip = !m_blShowTip;
					if(m_blShowTip)
						AfxMessageBox("你已开启ToolTip提示条! √");
					else
						AfxMessageBox("你已关闭ToolTip提示条! ※");
					return TRUE;
				}
				break;
			case '3':	//Ctrl + 3 : 读取连接的USB设备数
				if( ::GetKeyState(VK_CONTROL)<0)
				{
					res = PSGetUSBDevNum(&iNum);
					str.Format("连接FG_USB设备数: %d",iNum);
					AfxMessageBox(str);
					return TRUE;
				}
				break;
			case '4':	//Ctrl + 4 : 读取连接的UDisk设备数
				if( ::GetKeyState(VK_CONTROL)<0)
				{
					res = PSGetUDiskNum(&iNum);
					str.Format("连接的UDisk设备数: %d",iNum);
					AfxMessageBox(str);
					return TRUE;
				}
				break;
			case '5':	//Ctrl + 5 : 连续搜索10次计数 开启/关闭
				if( ::GetKeyState(VK_CONTROL)<0 )
				{
					//J_SetFocus(IDC_BUTTON2);

					g_bSearchCont = !g_bSearchCont;
					if(g_bSearchCont)
						AfxMessageBox("注意:连续搜索10次计数 已开启! √");
					else
						AfxMessageBox("注意:连续搜索10次计数 已关闭! ※");
					return TRUE;
				}
				break;
			case '6':	//Ctrl + 6 : 显示/隐藏 特殊功能按钮
				if( ::GetKeyState(VK_CONTROL)<0 )
				{
					m_bSpBtn = !m_bSpBtn;
					((CButton*)GetDlgItem(IDC_BUTTON11))->ShowWindow(m_bSpBtn);
					((CButton*)GetDlgItem(IDC_BUTTON27))->ShowWindow(m_bSpBtn);
					((CButton*)GetDlgItem(IDC_BUTTON28))->ShowWindow(m_bSpBtn);

					((CButton*)GetDlgItem(IDC_BUTTON26))->ShowWindow(!m_bSpBtn);
					((CButton*)GetDlgItem(IDC_BUTTON25))->ShowWindow(!m_bSpBtn);	

					return TRUE;
				}
				break;
			case '7' :	//初始化配置文件
				if( ::GetKeyState(VK_CONTROL)<0 )
				{
					LoadUserDefine(NULL,TRUE);
					return TRUE;
				}
				break;
			case '8' :	//图像正反矫正
				if( ::GetKeyState(VK_CONTROL)<0 )
				{
					g_bImgpos = !g_bImgpos;
					if(g_bImgpos)
						AfxMessageBox("注意:图像正反矫正 已启用! √");
					else
						AfxMessageBox("注意:图像正反矫正 已关闭! ※");
					return TRUE;
				}
				break;
			case '9':	//开启/关闭 连续获取图像计数	//g_bImgCont
				if( ::GetKeyState(VK_CONTROL)<0 )
				{
					g_bImgCont = !g_bImgCont;
					if(g_bImgCont)
						AfxMessageBox("注意:连续获取图像计数 已启用! √");
					else
						AfxMessageBox("注意:连续获取图像计数 已关闭! ※");
					return TRUE;
				}
				break;
			case 'W':	//Ctrl + W 设置临时图像路径	//g_iIMGtype
				if( ::GetKeyState(VK_CONTROL)<0 )
				{
					if(g_iIMGtype==0){
						//设置临时图像路径为：当前EXE文件夹
						g_iIMGtype = 1;
						J_SetIMGFILE(g_iIMGtype);
						SaveUserInfo();
						MessageBox("设置临时图像路径为：当前EXE文件夹","<特殊功能>",MB_OK);
					}
					else if(g_iIMGtype==1){
						//设置临时图像路径为：默认路径C
						g_iIMGtype = 0;
						J_SetIMGFILE(g_iIMGtype);
						SaveUserInfo();
						MessageBox("设置临时图像路径为：默认路径C","<特殊功能>",MB_OK);
					}
					return TRUE;
				}
				break;
			case 'R':	// Ctrl + R 开启、关闭 日志记录功能 、g_bLogs
				if( ::GetKeyState(VK_CONTROL)<0 )
				{
					g_bLogs = !g_bLogs;
					if(g_bLogs)
						AfxMessageBox("注意:日志记录功能 已启用! √");
					else
						AfxMessageBox("注意:日志记录功能 已关闭! ※");
					return TRUE;
				}
				break;
			case 'P':	// Ctrl + P 开启/关闭 图像处理演示
			/*	if( ::GetKeyState(VK_CONTROL)<0 )
				{
					g_bImgDemo = !g_bImgDemo;
					((CButton*)GetDlgItem(IDC_BUTTON20))->ShowWindow(g_bImgDemo);
					((CButton*)GetDlgItem(IDC_BUTTON21))->ShowWindow(g_bImgDemo);
					((CButton*)GetDlgItem(IDC_BUTTON22))->ShowWindow(g_bImgDemo);
					((CButton*)GetDlgItem(IDC_BUTTON23))->ShowWindow(g_bImgDemo);
					((CButton*)GetDlgItem(IDC_STATIC0007))->ShowWindow(g_bImgDemo);
					((CButton*)GetDlgItem(IDC_ET_BmpPath))->ShowWindow(g_bImgDemo);
					((CButton*)GetDlgItem(IDC_BUTTON19))->ShowWindow(g_bImgDemo);
					((CButton*)GetDlgItem(IDC_STATIC7))->ShowWindow(g_bImgDemo);

				//	((CButton*)GetDlgItem(IDC_STATIC9))->ShowWindow(!g_bImgDemo);

					if(g_bImgDemo)
						AfxMessageBox("注意:图像处理演示 已启用! √");
					else
						AfxMessageBox("注意:图像处理演示 已关闭! ※");

					return TRUE;
				}*/
				break;
			case 'H':	// Ctrl + Alt + H : 显示秘籍
				if( ::GetKeyState(VK_CONTROL)<0 && ::GetKeyState(VK_MENU)<0 )
				{
					AfxMessageBox("小强秘籍如下:\r\n\r\nCtrl + Alt + H : 密籍显示\r\nCtrl + 1 : 打开/关闭 设备打开状态标志\r\nCtrl + 2 : 打开/关闭 ToolTip提示条\r\nCtrl + 3 : 读取连接的USB设备数\r\nCtrl + 4 : 读取连接的UDisk设备数\r\nCtrl + 5 : 开启/关闭 连续搜索10次计数\r\nCtrl + 6 : 显示/隐藏 特殊功能按钮\r\nCtrl + 7 : 初始化配置文件\r\nCtrl + 8 : 图像正反矫正\r\nCtrl + 9 : 开启/关闭 连续获取图像计数\r\nCtrl + W : 设置临时图像路径\r\nCtrl + R : 开启/关闭 日志记录功能\r\nCtrl + Alt + 1 : 显示当前每枚指纹特征大小.");
					return TRUE;
				}
				break;
		}
	}
	if(m_blShowTip)
		m_tooltip.RelayEvent(pMsg); 

	return CDialog::PreTranslateMessage(pMsg);
}

HBRUSH CFGDemoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	UINT nID = pWnd->GetDlgCtrlID();
	if(  nID==IDC_STDevInfo )
		pDC->SetTextColor(RGB(100,120,100));
	else if(  nID==IDC_STDevOpen )
		pDC->SetTextColor(RGB(207,138,6));
	else if( nID==IDC_STATIC1 || nID==IDC_STATIC2 || nID==IDC_STATIC3 || nID==IDC_STATIC4 || nID==IDC_STATIC5 || nID==IDC_STATIC6 || nID==IDC_STATIC7 || nID==IDC_STATIC8 )
		pDC->SetTextColor(RGB(50,20,50));
	else if( nID==IDC_STATIC9 )
		pDC->SetTextColor(RGB(128,128,128));
	else if( nID==IDC_STMsg ){
		if(m_bBlue)
			pDC->SetTextColor(RGB(10,36,221));
		else
			pDC->SetTextColor(RGB(221,36,10));//RGB(207,138,26)
		pDC->SelectObject(&m_font);
	}
	else if( nID==IDC_ET_BmpPath )
		pDC->SetTextColor(RGB(50,40,20));
//	else if( nID==IDC_BITMAP)
//		pDC->SetBkColor(RGB(255,0,0));
	else if(nCtlColor == CTLCOLOR_EDIT)//编辑框_红色
	{
		pDC->SetTextColor(RGB(250,50,50));
	}//*/
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CFGDemoDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x, point.y));
	CDialog::OnLButtonDown(nFlags, point);
}
//////////////////////////////////////////////////////////////////////////////
//f辅助函数
void CFGDemoDlg::LoadUserDefine(HANDLE hl,BOOL bInit/*=FALSE*/)
{//bInit 初始化配置文件
	TCHAR   szFilePath[256] = {0};   
    TCHAR   szTemp[256] = {0};   
	TCHAR   szExePath[256] = {0};
	
	TCHAR   szDrive[255];
    TCHAR   szDir[255];
	TCHAR   szFname[255];
    TCHAR   szExt[5];
	
	CString str;
	::GetModuleFileName(GetModuleHandle(NULL),szExePath,256);
	_splitpath(szExePath,szDrive,szDir,szFname,szExt) ;
	_tcscat(szDrive,szDir);
    _tcscpy(szUserLogoPath,szDrive);
	_tcscat(szUserLogoPath,"Logo.bmp");
	_tcscat(szDrive,"info.ini");
	_tcscpy(szFilePath,szDrive);
	
	if(bInit){
		if(g_iLanguage==LGN_CHINA)// 语言 中文(简体)
			::WritePrivateProfileString("UserInfo","Title",BLUE01,szFilePath);
		else
			::WritePrivateProfileString("UserInfo","Title",BLUE02,szFilePath);
		::WritePrivateProfileString("UserInfo","HardwareInfo","Synochip Fingerprint",szFilePath);
		::WritePrivateProfileString("UserSetting","SensorType","0",szFilePath);
		::WritePrivateProfileString("UserSetting","TempBmpPath","1",szFilePath);
	}
	if (PathFileExists(szFilePath))
	{
		if(g_iLanguage==LGN_CHINA)// 语言 中文(简体)
			::GetPrivateProfileString("UserInfo","Title",BLUE01,szTemp,80,szFilePath);   
		else
			::GetPrivateProfileString("UserInfo","Title",BLUE02,szTemp,80,szFilePath); 
		::SetWindowText(AfxGetMainWnd()->GetSafeHwnd(),szTemp);

		if(hl==NULL){
			::GetPrivateProfileString("UserInfo","HardwareInfo","Synochip Fingerprint",szTemp,80,szFilePath);
			if(g_iLanguage==LGN_CHINA)// 语言 中文(简体)
				str.Format("欢迎使用%s 测试程序。\r\n请先打开设备...",szTemp);
			else
				str.Format("Welcome to use %s demo.\r\nPlease open device first...",szTemp);
			SetDlgItemText(IDC_STDevInfo,str);
		}

		m_iSensorType = ::GetPrivateProfileInt("UserSetting","SensorType",0,szFilePath);
		if(m_iSensorType<0 || m_iSensorType>3)
			m_iSensorType = 0;
		g_iIMGtype	  = ::GetPrivateProfileInt("UserSetting","TempBmpPath",0,szFilePath);
		if(g_iIMGtype!=0 && g_iIMGtype!=1)
			g_iIMGtype = 1;

		UpdateData(FALSE);
	}

	return;
}
void CFGDemoDlg::SaveUserInfo()
{
	TCHAR szDrive[MAX_PATH]={0};
	TCHAR szFilePath[MAX_PATH]={0};
	TCHAR szTmp[MAX_PATH]={0};
	
	J_GetPathName(szDrive,2);
	sprintf(szFilePath,"%s\\info.ini",szDrive);
	if (PathFileExists(szFilePath)){
		UpdateData();
		sprintf(szTmp,"%d",m_iSensorType);
		::WritePrivateProfileString("UserSetting","SensorType",szTmp,szFilePath);
		sprintf(szTmp,"%d",g_iIMGtype);
		::WritePrivateProfileString("UserSetting","TempBmpPath",szTmp,szFilePath);
	}
}

void CFGDemoDlg::J_MyBtn(CShadeButtonST* msbtn,UINT nID,UINT iIcon/*=0*/,UINT shadeID,COLORREF rgb/*=RGB(50,50,50)*/)
{
//	CShadeButtonST* pbtn=new CShadeButtonST;
//	pbtn->SubclassDlgItem(nID,this);
	msbtn->SubclassDlgItem(nID,this);
	if(0!=iIcon)
		msbtn->SetIcon(iIcon);
	msbtn->SetShade(shadeID,8,50,10,RGB(155,155,255)); //SHS_HARDBUMP //,8,20,5,RGB(55,55,255
	msbtn->SetColor(CButtonST::BTNST_COLOR_FG_IN, RGB(0, 178, 0));
	msbtn->SetColor(CButtonST::BTNST_COLOR_FG_OUT, rgb);
//效果四：设置按钮在不同状态下的底色和文字颜色
//提示：SetColor函数和OffsetColor函数的第一个参数表示按钮的各个状态，前景色就是文字的颜色，它们的取值表示：
        	//BTNST_COLOR_BK_IN	     鼠标放在按钮内时的背景色
            //BTNST_COLOR_FG_IN 	 鼠标放在按钮内时的前景色
            // 	 普通状态时的背景色	
            //BTNST_COLOR_FG_OUT 	 普通状态时的前景色
            //BTNST_COLOR_BK_FOCUS 	 按钮被按下后的背景色
            //BTNST_COLOR_FG_FOCUS 	 按钮被按下后的前景色
/*CShadeButtonST类 是CButtonST的另一个派生类。它的特点是支持更为多样化的背景，
	通过SetShade函数可以为按钮设置9种不通的背景效果。它包括四个文件，它们分别是
	CeXDib.h、CeXDib.cpp、ShadeButtonST.h和ShadeButtonST.cpp。在引用它时,同样要一并引入CButtonST的文件。
	使用：
	1.引入文件，添加文件
	2.导入文件后，在SdtAfx.h文件中添加#include "ShadeButtonST.h"
	3.添加成员变量 	CShadeButtonST	m_btn3;
	4.假设按钮ID为IDC_BUTTON3 添加Icon资源，ID设为IDI_ICON1
	5.在OnInitDialog函数中初始化按钮//*/
	//SHS_METAL  SHS_HARDBUMP  SHS_SOFTBUMP
	//SHS_NOISE  SHS_VBUMP     SHS_HBUMP
	//SHS_VSHADE SHS_HSHADE    SHS_DIAGSHADE
}
void CFGDemoDlg::J_EnableBtn(BOOL bEnable/*=1*/,BOOL bEcl/*=1*/,BOOL bOpen/*=TRUE*/)
{
	for(int i=IDC_BUTTON2; i<=IDC_BUTTON28; i++)//指令操作	IDC_CloseDev
		((CButton*)GetDlgItem(i))->EnableWindow(bEnable);
	((CButton*)GetDlgItem(IDC_CloseDev))->EnableWindow(bEnable); //关闭设备
	((CButton*)GetDlgItem(IDC_CloseDev2))->EnableWindow(bEnable);//关闭所有设备
	m_list.EnableWindow(bEnable);//列表
	((CButton*)GetDlgItem(IDC_CHECK1))->EnableWindow(bEnable);
	((CButton*)GetDlgItem(IDC_CHECK2))->EnableWindow(bEnable);

	((CButton*)GetDlgItem(IDC_BUTTON1))->EnableWindow(bEcl);//取消操作按钮
	
	//打开设备
	((CButton*)GetDlgItem(IDOK))->EnableWindow(bOpen);
	((CButton*)GetDlgItem(IDC_COMBO_Dev))->EnableWindow(bOpen);
	((CButton*)GetDlgItem(IDC_COMBO_DevType))->EnableWindow(bOpen);
	((CButton*)GetDlgItem(IDC_COMBO_iCom))->EnableWindow(bOpen);
	((CButton*)GetDlgItem(IDC_COMBO_LG))->EnableWindow(bOpen);
	((CButton*)GetDlgItem(IDC_EDIT_DevPwd))->EnableWindow(bOpen);
	((CButton*)GetDlgItem(IDC_EDIT_DevID))->EnableWindow(bOpen);
	((CButton*)GetDlgItem(IDC_EDIT_DevAddr))->EnableWindow(bOpen);

	J_EnableSet(bEnable,m_iDevType);
}
void CFGDemoDlg::J_EnableSet(BOOL bEnable/*=1*/,int nType/*=DEVICE_USB*/)//设备设置
{
	if(bEnable){
		((CButton*)GetDlgItem(IDC_COMBO3))->EnableWindow(1);
		if(nType==DEVICE_COM){
			((CButton*)GetDlgItem(IDC_COMBO1))->EnableWindow(1);
			((CButton*)GetDlgItem(IDC_COMBO2))->EnableWindow(1);
			return;
		}
		((CButton*)GetDlgItem(IDC_COMBO1))->EnableWindow(0);
		((CButton*)GetDlgItem(IDC_COMBO2))->EnableWindow(0);
		return;
	}
	((CButton*)GetDlgItem(IDC_COMBO1))->EnableWindow(0);
	((CButton*)GetDlgItem(IDC_COMBO2))->EnableWindow(0);
	((CButton*)GetDlgItem(IDC_COMBO3))->EnableWindow(0);
}
int CFGDemoDlg::J_GetDevInfo(HANDLE hHandle,int nAddr, CString& str_CH, CString& str_EN,int* iMbMax)
{
	int ret,i,iTmp;
	BYTE iParTable[512]={0};
	CString sMsg_CH,sMsg_EN,sTmp;
	Sleep(200);
	
	ret = PSReadInfPage(hHandle,nAddr,iParTable);
	if(ret != PS_OK)
		return ret;
	
	iTmp = (iParTable[4]<<8)+iParTable[5];		*iMbMax = iTmp;
	sTmp.Format("指纹库大小: %d\r\n",iTmp);		sMsg_CH = sTmp;
	sTmp.Format("DataBaseSize: %d\r\n",iTmp);	sMsg_EN = sTmp;
		
	iTmp = (iParTable[6]<<8)+iParTable[7];
	m_iSLev_x = iTmp-1;//获取安全等级
	sTmp.Format("安全等级: %d\r\n",iTmp);		sMsg_CH += sTmp;
	sTmp.Format("SecureLevel:  %d\r\n",iTmp);	sMsg_EN += sTmp;

	sMsg_CH += "设备地址: 0x";	
	sMsg_EN += "Address:    0x";
	for(i=8; i<8+4; i++)
	{
		sTmp.Format("%02X",iParTable[i]);
		sMsg_CH += sTmp;	sMsg_EN += sTmp;
	}
	sMsg_CH += "\r\n";		sMsg_EN += "\r\n";

	iTmp = (iParTable[12]<<8)+iParTable[13];
	m_iPackSize_x = iTmp;//获取数据包大小
	iTmp = (iParTable[14]<<8)+iParTable[15];
	m_iBaud_x = J_GetBaudMul(iTmp,1);//获取波特率

		
	sMsg_CH += "产品型号: ";
	sMsg_EN += "ProductSN:    ";
	for(i=28; i<28+8; i++)
	{
		sTmp.Format("%c",iParTable[i]);
		sMsg_CH += sTmp;	sMsg_EN += sTmp;
	}
	sMsg_CH += "\r\n";		sMsg_EN += "\r\n";
		
	sMsg_CH += "软件版本: ";
	sMsg_EN += "SoftwareVersion:";
	for(i=36; i<36+8; i++)
	{
		sTmp.Format("%c",iParTable[i]);
		sMsg_CH += sTmp;	sMsg_EN += sTmp;
	}
	sMsg_CH += "\r\n";		sMsg_EN += "\r\n";
	
//	sMsg_CH += "厂家名称: ";
//	sMsg_EN += "Manufacture:  ";
//	for(i=44; i<44+8; i++)
//	{
//		sTmp.Format("%c",iParTable[i]);
//		sMsg_CH += sTmp;	sMsg_EN += sTmp;
//	}
//	sMsg_CH += "\r\n";		sMsg_EN += "\r\n";
			
	sMsg_CH += "传感器名称: ";
	sMsg_EN += "SensorName:   ";
	for(i=52; i<52+8; i++)
	{
		sTmp.Format("%c",iParTable[i]);
		sMsg_CH += sTmp;	sMsg_EN += sTmp;
	}
	sMsg_CH += "\r\n";		sMsg_EN += "\r\n";

	str_CH = sMsg_CH;		str_EN = sMsg_EN;
	UpdateData(0);
	return 0;
}
void CFGDemoDlg::J_SetLanuage(int iLanguage/*=LGN_CHINA*/)
{
	CString sTmp,sDevName;
	pJsDevHandle pDevHandle=NULL;
	int iIndex = m_ctlDevHandle.GetCurSel();
	if(iIndex>=0){
		m_ctlDevHandle.GetLBText(iIndex,sDevName);
		J_GetDevHandle(pDevHandle,sDevName);
	}

	if(iLanguage==LGN_CHINA)// 语言 中文(简体)
	{
		SetWindowText(BLUE01);
		SetDlgItemText(IDC_STATIC1,"硬件信息");
		SetDlgItemText(IDC_STATIC2,"打开设备");
		SetDlgItemText(IDC_STATIC3,"信息显示");
		SetDlgItemText(IDC_STATIC4,"图像管理");
		SetDlgItemText(IDC_STATIC5,"指纹处理");
		SetDlgItemText(IDC_STATIC6,"辅助功能区");
		SetDlgItemText(IDC_STATIC7,"图像处理演示");
		SetDlgItemText(IDC_STATIC8,"设备配置");
//		SetDlgItemText(IDC_STATIC9,"杭州晟元芯片技术有限公司\thttp://www.synochip.com\r\nTel: +086 571 88271908\r\nFax: +086 571 88271901\r\n地址：杭州西湖区天目山路176号17幢203室\r\n邮编：310012");

		SetDlgItemText(IDC_STATIC0001,"设备类型: ");
		SetDlgItemText(IDC_STATIC0002,"串口号: ");
		SetDlgItemText(IDC_STATIC0003,"设备密码: ");
		SetDlgItemText(IDC_STATIC0004,"设备地址: ");
		SetDlgItemText(IDC_STATIC0005,"指纹库: ");
		SetDlgItemText(IDC_STATIC0006,"指纹处理: ");
		SetDlgItemText(IDC_STATIC0007,"演示图像: ");
		SetDlgItemText(IDC_STATIC0008,"打开设备列表: ");
		SetDlgItemText(IDC_STATIC0009,"波特率: ");
		SetDlgItemText(IDC_STATIC0010,"设备ID: ");
		SetDlgItemText(IDC_STATIC0011,"波特率设置");
		SetDlgItemText(IDC_STATIC0012,"数据包大小");
		SetDlgItemText(IDC_STATIC0013,"安全等级");
		SetDlgItemText(IDC_STATIC0014,"传感器类型");
		SetDlgItemText(IDC_CHECK1,"显示图像");
		SetDlgItemText(IDC_CHECK2,"保持覆盖");

		//按钮
		SetDlgItemText(IDC_CloseDev,"关闭设备");
		SetDlgItemText(IDC_CloseDev2,"关闭所有设备");
		SetDlgItemText(IDOK,"打开设备");
		SetDlgItemText(IDCANCEL,"退出程序");
		SetDlgItemText(IDC_BUTTON1,"取消操作");
		SetDlgItemText(IDC_BUTTON2,"获取图像");
		SetDlgItemText(IDC_BUTTON3,"连续获取图像");
		SetDlgItemText(IDC_BUTTON4,"保存图像");
		SetDlgItemText(IDC_BUTTON5,"下载图像");
		SetDlgItemText(IDC_BUTTON6,"录入指纹");
		SetDlgItemText(IDC_BUTTON7,"连续录入");
		SetDlgItemText(IDC_BUTTON8,"单一比对");
		SetDlgItemText(IDC_BUTTON9,"搜索");
		SetDlgItemText(IDC_BUTTON10,"连续搜索");
		SetDlgItemText(IDC_BUTTON11,"连续高速搜索");
		SetDlgItemText(IDC_BUTTON12,"单一删除");
		SetDlgItemText(IDC_BUTTON13,"清空指纹库");
		SetDlgItemText(IDC_BUTTON14,"读记事本");
		SetDlgItemText(IDC_BUTTON15,"写记事本");
		SetDlgItemText(IDC_BUTTON16,"获取随机数");
		SetDlgItemText(IDC_BUTTON17,"采集指纹库");
		SetDlgItemText(IDC_BUTTON18,"读有效模板数");
		SetDlgItemText(IDC_BUTTON20,"原始图像");
		SetDlgItemText(IDC_BUTTON21,"二值图");
		SetDlgItemText(IDC_BUTTON22,"细化图");
		SetDlgItemText(IDC_BUTTON23,"特征点细化图");
		SetDlgItemText(IDC_BUTTON24,"下载指纹库");
		SetDlgItemText(IDC_BUTTON25,"上传指纹库");
		SetDlgItemText(IDC_BUTTON26,"上传指定模版");
		SetDlgItemText(IDC_BUTTON27,"@ 下载图像");
		SetDlgItemText(IDC_BUTTON28,"@ 图像转换");

		//以下3个需作特殊处理
		SetDlgItemText(IDC_STMsg,"Info");
		if(pDevHandle==NULL){
			SetDlgItemText(IDC_STDevInfo,"欢迎使用Synochip Fingerprint 测试程序。\r\n请先打开设备...");
			SetDlgItemText(IDC_STDevOpen,">正在与 ? 通讯...");
		}
		else{
			sTmp.Format(">正在与 %s 通讯",sDevName);
			SetDlgItemText(IDC_STDevInfo,pDevHandle->sMsg_CH);
			SetDlgItemText(IDC_STDevOpen,sTmp);
		}
	}
	else	//语言 英文
	{
		SetWindowText(BLUE02);
		SetDlgItemText(IDC_STATIC1,"Hardware information");
		SetDlgItemText(IDC_STATIC2,"Open device");
		SetDlgItemText(IDC_STATIC3,"Information");
		SetDlgItemText(IDC_STATIC4,"Image Manage");
		SetDlgItemText(IDC_STATIC5,"Finger Manage");
		SetDlgItemText(IDC_STATIC6,"Assistant function");
		SetDlgItemText(IDC_STATIC7,"Deal with image");
		SetDlgItemText(IDC_STATIC8,"Device Manage");
//		SetDlgItemText(IDC_STATIC9,"hangzhou Synochip Technologies Co.,Ltd \thttp://www.synochip.com\r\nTel: +086 571 88271908\r\nFax: +086 571 88271901\r\nAddr: Rm 103, Building 17, No.176 Tianmushan Road, Hangzhou 310012, P.R.China");

		SetDlgItemText(IDC_STATIC0001,"Device Type:");
		SetDlgItemText(IDC_STATIC0002,"Com Num: ");
		SetDlgItemText(IDC_STATIC0003,"DevicePwd: ");
		SetDlgItemText(IDC_STATIC0004,"DeviceAddr: ");
		SetDlgItemText(IDC_STATIC0005,"Finger Info: ");
		SetDlgItemText(IDC_STATIC0006,"Finger manage: ");
		SetDlgItemText(IDC_STATIC0007,"ImagePath: ");
		SetDlgItemText(IDC_STATIC0008,"Opened Devide: ");
		SetDlgItemText(IDC_STATIC0009,"Baudrate: ");
		SetDlgItemText(IDC_STATIC0010,"DeviceID: ");
		SetDlgItemText(IDC_STATIC0011,"Baudrate");
		SetDlgItemText(IDC_STATIC0012,"Package Size");
		SetDlgItemText(IDC_STATIC0013,"Secure Level");
		SetDlgItemText(IDC_STATIC0014,"SensorType");
		SetDlgItemText(IDC_CHECK1,"Show Image");
		SetDlgItemText(IDC_CHECK2,"ReplaceMB");

		//按钮
		SetDlgItemText(IDC_CloseDev,"Close Device");
		SetDlgItemText(IDC_CloseDev2,"Close All Device");
		SetDlgItemText(IDOK,"Open Device");
		SetDlgItemText(IDCANCEL,"Exit demo");
		SetDlgItemText(IDC_BUTTON1,"Cancel operate");
		SetDlgItemText(IDC_BUTTON2,"Capture");
		SetDlgItemText(IDC_BUTTON3,"Con Capture");
		SetDlgItemText(IDC_BUTTON4,"Save Image");
		SetDlgItemText(IDC_BUTTON5,"Load Image");
		SetDlgItemText(IDC_BUTTON6,"Enroll");
		SetDlgItemText(IDC_BUTTON7,"Con Enroll");
		SetDlgItemText(IDC_BUTTON8,"Match");
		SetDlgItemText(IDC_BUTTON9,"Search");
		SetDlgItemText(IDC_BUTTON10,"Con Search");
		SetDlgItemText(IDC_BUTTON11,"Quick Search");
		SetDlgItemText(IDC_BUTTON12,"Delete One");
		SetDlgItemText(IDC_BUTTON13,"Empty");
		SetDlgItemText(IDC_BUTTON14,"Read Notepad");
		SetDlgItemText(IDC_BUTTON15,"WriteNotepad");
		SetDlgItemText(IDC_BUTTON16,"Random");
		SetDlgItemText(IDC_BUTTON17,"CaptureImgDB");
		SetDlgItemText(IDC_BUTTON18,"TemplateNum");
		SetDlgItemText(IDC_BUTTON20,"Source");
		SetDlgItemText(IDC_BUTTON21,"Bin Image");
		SetDlgItemText(IDC_BUTTON22,"Thin Image");
		SetDlgItemText(IDC_BUTTON23,"Template Image");
		SetDlgItemText(IDC_BUTTON24,"DownFingerDB");
		SetDlgItemText(IDC_BUTTON25,"UpFingerDB");
		SetDlgItemText(IDC_BUTTON26,"UpOneFinger");
		SetDlgItemText(IDC_BUTTON27,"@ DownImage");
		SetDlgItemText(IDC_BUTTON28,"@ ImageChange");

		//以下3个需作特殊处理
		SetDlgItemText(IDC_STMsg,"Info");
		if(pDevHandle==NULL){
			SetDlgItemText(IDC_STDevInfo,"Welcome to use Synochip Fingerprint demo.\r\nPlease open device first...");
			SetDlgItemText(IDC_STDevOpen,">Connecting to ?...");
		}
		else{
			sTmp.Format(">Connectint to %s",sDevName);
			SetDlgItemText(IDC_STDevInfo,pDevHandle->sMsg_EN);
			SetDlgItemText(IDC_STDevOpen,sTmp);
		}
	}
	LoadUserDefine(pDevHandle);
}
int CFGDemoDlg::J_GetBaudMul(int imbo,int way/*=0*/)//获取波特率倍数N,基于9600
{//way:方向 0:imbo->iBaud 1:iBaud->imbo 
	int iRes=1;
	if(way==0)
	{
		switch(imbo){
			case 0://9600
				iRes = 9600/9600;
				break;
			case 1://19200
				iRes = 19200/9600;
				break;
			case 2://38400
				iRes = 38400/9600;
				break;
			case 3://57600
				iRes = 57600/9600;
				break;
			case 4://115200
				iRes = 115200/9600;
				break;
			default:
				break;
		}
	}
	else{
		iRes = 0;
		switch(imbo){
			case 1://9600
				iRes = 0;
				break;
			case 2://19200
				iRes = 1;
				break;
			case 4://38400
				iRes = 2;
				break;
			case 6://57600
				iRes = 3;
				break;
			case 12://115200
				iRes = 4;
				break;
			default:
				break;
		}
	}
	return iRes;
}
BOOL CFGDemoDlg::J_CheckDev(CString sDevInfo)//判断设备是否早已打开，用于避免同一设备的重复打开
{
	int res = m_ctlDevHandle.FindString(-1,sDevInfo);
	if(CB_ERR==res)
		return FALSE;
	return TRUE;
}
BOOL CFGDemoDlg::J_GetDevHandle(pJsDevHandle& pDevHandle,CString sDevName)//获取设备句柄
{
	if(!J_CheckDev(sDevName))
		return FALSE;//设备不存在或未打开
	pJsDevHandle pTmp1;
	pTmp1 = gHead_DevHandle;
	while(pTmp1!=NULL){
		if(sDevName==pTmp1->sDevName){//就是这个了
			pDevHandle = pTmp1;
			return TRUE;
		}
		pTmp1 = pTmp1->next;//下一个
	}
	return FALSE;//未搜索到
}
BOOL CFGDemoDlg::J_CloseDev(CString sDevName)//关闭设备
{
	int iIndex = m_ctlDevHandle.FindString(-1,sDevName);
	if(iIndex==CB_ERR)
		return FALSE;//设备不存在或未打开

	pJsDevHandle pTmp1,pTmp2;
	if(gHead_DevHandle->sDevName == sDevName)//如果头就是
	{
		pTmp2 = gHead_DevHandle;
		gHead_DevHandle = gHead_DevHandle->next;
		PSCloseDeviceEx(pTmp2->hHandle);
		delete pTmp2;
		m_ctlDevHandle.DeleteString(iIndex);//删除列表框信息
		m_list.DeleteAllItems();
		return TRUE;
	}
	else{
		pTmp1 = gHead_DevHandle;//从头开始搜索
		while(pTmp1->next!=NULL){
			pTmp2 = pTmp1->next;
			if(sDevName==pTmp2->sDevName){//就是这个了
				pTmp1->next = pTmp2->next;
				PSCloseDeviceEx(pTmp2->hHandle);
				delete pTmp2;
				m_ctlDevHandle.DeleteString(iIndex);//删除列表框信息
				m_list.DeleteAllItems();
				return TRUE;
			}
			pTmp1 = pTmp1->next;//下一个
		}
	}
	
	return FALSE;
}
void CFGDemoDlg::J_SetMsg1(CString str,BOOL bBlue/*=TRUE*/)
{
	m_bBlue = bBlue;
	SetDlgItemText(IDC_STMsg,str);
	if(g_bLogs){
		sprintf(g_log,"%s\n",str.GetBuffer(0));
		WriteRunLog(g_log);
	}
}
void CFGDemoDlg::J_SetMsg2(CString str_CH,CString str_EN,BOOL bBlue/*=TRUE*/)//自动选择语言
{
	if(g_iLanguage==LGN_CHINA)
		J_SetMsg1(str_CH,bBlue);
	else
		J_SetMsg1(str_EN,bBlue);
}
void CFGDemoDlg::ShowDefaltImg() 
{
	HBITMAP hBitmapFP;
	if (PathFileExists(szUserLogoPath))
		hBitmapFP  = (HBITMAP)LoadImage( NULL,szUserLogoPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION);
	else
		hBitmapFP=LoadBitmap(AfxGetApp()->m_hInstance,MAKEINTRESOURCE(IDB_BITMAP1));
	((CStatic*)GetDlgItem(IDC_BITMAP))->SetBitmap(hBitmapFP);
	DeleteObject(hBitmapFP);
	m_bRefBMP = FALSE;
}
BOOL CFGDemoDlg::J_GetCurDevHandle(pJsDevHandle& pDevHandle)//获取当前有效设备句柄
{
	int iIndex = m_ctlDevHandle.GetCurSel();
	if(iIndex==CB_ERR)
		return FALSE;//设备不存在或未打开
	CString sDevName;
	m_ctlDevHandle.GetLBText(iIndex,sDevName);
	return J_GetDevHandle(pDevHandle,sDevName);
}
void CFGDemoDlg::ShowImage(char *filename) //显示图像
{
	CString strFile;
	if(filename==NULL)
		strFile = gszIMGFILE;//IMG_FILE;
	else
		strcpy((char*)(LPCTSTR)strFile,filename);
    HBITMAP hBitmapFP;
    hBitmapFP = (HBITMAP)LoadImage( GetModuleHandle(NULL),(LPCTSTR)strFile, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (hBitmapFP==NULL)
		hBitmapFP  = (HBITMAP)LoadImage( NULL,(LPCTSTR)strFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION);
	((CStatic*)pMyDlg->GetDlgItem(IDC_BITMAP))->SetBitmap(hBitmapFP);
	DeleteObject(hBitmapFP);	
	m_bRefBMP = TRUE;
}
void CFGDemoDlg::J_LoadInfo2List()//加载指纹索引到List列表
{
	UpdateData();
	m_list.DeleteAllItems();
	g_iSelect = -1;
	if(!J_GetCurDevHandle(gCur_DevHandle))
		return;//未选中要操作的设备，或列表中无打开的设备
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int		nMbMax	= gCur_DevHandle->iMbMax;//指纹库大小
	
	//如果是老版的程序，则不支持最新读模板索引功能，所以沿用从文件加载的老方法////////20090106
	if( nMbMax==120 || nMbMax==376 || nMbMax==888 )
	{
		FILE* fp;
		int iLen=0,nIndex=0;
		CString sIdx;
		if( (fp=fopen("FGTemplate.dat","rb"))==NULL )
			return;//打开指纹索引文件失败
		BYTE pIdx[2]={0};//2Byte表示一个索引页信息
		while(!feof(fp)){
			iLen = fread(pIdx,sizeof(BYTE),2,fp);
			if(iLen!=2){
				fclose(fp);
				return;
			}
			nIndex = pIdx[0]*256+pIdx[1];
			sIdx.Format("%d",nIndex);
			m_list.InsertItem(m_list.GetItemCount(),sIdx);
		}
		fclose(fp);
		return;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////

	int ret,iMbNums,iPage=0,iBase,iIndex,i,iIndexOffset;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return;
		}
	}

	ret = PSTemplateNum(hHandle,nAddr,&iMbNums);
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		return;//获取有效模板个数失败
	}
	if(iMbNums<=0){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		return;//有效模板个数为0，不加载
	}
	BYTE UserContent[32]={0};
	BYTE bt,b;
	CString sTmp;
	int  iMaxPage = ( (gCur_DevHandle->iMbMax)-1 )/256 + 1;	//iMaxPage∈[0,4)
	for(iPage=0; iPage<iMaxPage; iPage++)//读模版索引表	nPage,0,1,2,3对应模版从0-255，256-511，512-767，768-1023
	{
		ret = PSReadIndexTable(hHandle,nAddr,iPage,UserContent);
		if(ret!=PS_OK){
			if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
				PSCloseDeviceEx(hHandle);
			return;//读模版索引表失败
		}
		for(i=0; i<32; i++)
		{
			bt = UserContent[i];
			iBase = iPage*256 + i*8;
			if(iBase>=(gCur_DevHandle->iMbMax)){
				if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
					PSCloseDeviceEx(hHandle);
				return;//goto LoadFGIndex_END;//基数大于模板库，提前结束
			}
			if(bt==0x00)
				continue;
			//开始计算 bt含8个索引页信息
			for(b=0x01,iIndexOffset=0; iIndexOffset<8; b=b<<1,iIndexOffset++)
			{
				if(!(bt&b))
					continue;
				iIndex = iBase + iIndexOffset;
				sTmp.Format("%d",iIndex);
				m_list.InsertItem(iIndex,sTmp);
			}
		}
	}
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
//LoadFGIndex_END:
}
BOOL CFGDemoDlg::J_IsExistId(int iIndex)
{
	int i,iMax;
	CString str,sTmp;
	str.Format("%d",iIndex);
	iMax = m_list.GetItemCount();
	for(i=0; i<iMax; i++)
	{
		sTmp = m_list.GetItemText(i,0);
		if(sTmp==str)
			return TRUE;
	}
	return FALSE;
}
void CFGDemoDlg::J_bImgPosN(BYTE *pImg/*=NULL*/,BOOL bImgPN/*=0*/)//图象正反矫正
{
	if(pImg==NULL || !bImgPN)
		return;
	BYTE pBuf[IMAGE_SIZE]={0};
	for(int i=0; i<IMAGE_H; i++)
	{
		memcpy(pBuf+i*IMAGE_W,pImg+(IMAGE_H-1-i)*IMAGE_W,IMAGE_W*sizeof(BYTE));
	}
	memcpy(pImg,pBuf,IMAGE_SIZE);
	return;
}
//////////////////////////////////////////////////////////////////////////////////

void CFGDemoDlg::OnSelchangeComboLg() //语言切换
{
	UpdateData();
	if(m_iLG==0)
		g_iLanguage = LGN_CHINA;
	else
		g_iLanguage = LGN_EN;
	J_SetLanuage(g_iLanguage);
	ShowDefaltImg();
}

void CFGDemoDlg::OnSelchangeCOMBODev() //设备切换
{
	UpdateData();
	int iIndex = m_ctlDevHandle.GetCurSel();
	CString sDevName,sTmp;
	m_ctlDevHandle.GetLBText(iIndex,sDevName);
	pJsDevHandle pDevHandle=NULL;
	if(!J_GetDevHandle(pDevHandle,sDevName))
		return;

	m_strAddr.Format("%08X",pDevHandle->dwAddr);
	m_strPwd.Format("%08X",pDevHandle->dwPwd);
	m_iBaud		= J_GetBaudMul(pDevHandle->iBaud_x,1);
	m_iCom		= pDevHandle->iCom-1;
	m_iDevID	= pDevHandle->iDevId;
	m_iDevType	= pDevHandle->nDevType;
	m_iBaud_x		= J_GetBaudMul(pDevHandle->iBaud_x,1);
	m_iPackSize_x	= pDevHandle->iPacketSize_x;
	m_iSLev_x		= pDevHandle->iSecureLev_x - 1;
	if(g_iLanguage==LGN_CHINA){//中文
		sTmp.Format(">正在与 %s 通讯.",sDevName);
		SetDlgItemText(IDC_STDevOpen,sTmp);
		SetDlgItemText(IDC_STDevInfo,pDevHandle->sMsg_CH);
	}
	else{//英文
		sTmp.Format(">Connecting to %s.",sDevName);
		SetDlgItemText(IDC_STDevOpen,sTmp);
		SetDlgItemText(IDC_STDevInfo,pDevHandle->sMsg_EN);
	}
	J_EnableBtn();
	J_LoadInfo2List();
	g_iIndex = 0;
	UpdateData(FALSE);
}

void CFGDemoDlg::OnOK() //打开设备
{
	UpdateData();

	if(m_strPwd.GetLength()!=8 || m_strAddr.GetLength()!=8)
	{
		AfxMessageBox("Password or Address size error! It should be 8 ×");
		return;
	}
	CString sDevInfo_CH,sDevInfo_EN,sDevOpen,sMsg,sDevName,sTmp1,strSensor;
	int nRet,res1,res2,iBaud=57600/9600,iMbMax,iUSBNums,iUDiskNums;
	UINT uPwd=0x00000000;//设备密码
	UINT nAddr=0xFFFFFFFF;//设备地址
	res1 = strHexToInt(m_strPwd,&uPwd);		//解析设备密码
	res2 = strHexToInt(m_strAddr,&nAddr);	//解析设备地址
	if(res1==-1 || res2==-1){
		AfxMessageBox("Invalid password or address! Please input again. ×");
		return;
	}
	HANDLE hHandle=NULL;
	pJsDevHandle pDevHandle = new JsDevHandle;
	pJsDevHandle pDevHTmp=gHead_DevHandle;
	PSGetUSBDevNum(&iUSBNums);
	if(iUSBNums<=0 && m_iDevType==DEVICE_USB){
		m_iDevType = DEVICE_UDisk;//2
		UpdateData(0);
	}
	PSGetUDiskNum(&iUDiskNums);
	if(iUDiskNums<=0 && m_iDevType==DEVICE_UDisk){
		m_iDevType = DEVICE_COM;//1
		UpdateData(0);
	}
	switch(m_iDevType)
	{
		case 0:	//DEVICE_USB
			//J_CheckDev
			sDevName.Format("USB_No.%d",m_iDevID);
			if(J_CheckDev(sDevName))
				J_CloseDev(sDevName);
			//	goto REOPEN;
			nRet = PSOpenDeviceEx(&hHandle,DEVICE_USB,m_iCom+1,iBaud,2,m_iDevID);
			if(nRet!=PS_OK)
				goto OPENFail;
			nRet = PSVfyPwd(hHandle,nAddr,(BYTE*)&uPwd);
			
			break;
		case 1:	//DEVICE_COM	需作特殊处理_打开后要关闭
			//////////////////////////////////////////////////////////////////////////
			{
				CDLGCOM dcom;
				if( dcom.DoModal()!=IDOK )
					goto OPENFail;
				m_iCom = dcom.m_iCom;
				UpdateData(0);
			}
			//////////////////////////////////////////////////////////////////////////
			sDevName.Format("Dev_COM%d",m_iCom+1);
			if(J_CheckDev(sDevName))
				J_CloseDev(sDevName);
			//	goto REOPEN;
			iBaud = 57600/9600;
			if(PSOpenDeviceEx(&hHandle,DEVICE_COM,m_iCom+1,iBaud)==PS_OK){ // = 57600/9600
				m_iBaud = 3;
				if(PSVfyPwd(hHandle,nAddr,(BYTE*)&uPwd)==PS_OK) goto OPENOK;
			}
			PSCloseDeviceEx(hHandle);
			iBaud = 115200/9600;
			if(PSOpenDeviceEx(&hHandle,DEVICE_COM,m_iCom+1,iBaud)==PS_OK){ // = 115200/9600
				m_iBaud = 4;
				if(PSVfyPwd(hHandle,nAddr,(BYTE*)&uPwd)==PS_OK) goto OPENOK;
			}
			PSCloseDeviceEx(hHandle);
			iBaud = 38400/9600;
			if(PSOpenDeviceEx(&hHandle,DEVICE_COM,m_iCom+1,iBaud)==PS_OK){ // = 38400/9600
				m_iBaud = 2;
				if(PSVfyPwd(hHandle,nAddr,(BYTE*)&uPwd)==PS_OK) goto OPENOK;
			}
			PSCloseDeviceEx(hHandle);
			iBaud = 19200/9600;
			if(PSOpenDeviceEx(&hHandle,DEVICE_COM,m_iCom+1,iBaud)==PS_OK){ // = 19200/9600
				m_iBaud = 1;
				if(PSVfyPwd(hHandle,nAddr,(BYTE*)&uPwd)==PS_OK) goto OPENOK;
			}
			PSCloseDeviceEx(hHandle);
			iBaud = 9600/9600;
			if(PSOpenDeviceEx(&hHandle,DEVICE_COM,m_iCom+1,iBaud)==PS_OK){ // = 9600/9600
				m_iBaud = 0;
				if(PSVfyPwd(hHandle,nAddr,(BYTE*)&uPwd)==PS_OK) goto OPENOK;
			}
			PSCloseDeviceEx(hHandle);
		/*	iBaud = 60;
			if(PSOpenDeviceEx(&hHandle,DEVICE_COM,m_iCom+1,iBaud)==PS_OK){ // = 60*9600/9600
				m_iBaud = 0;
				if(PSVfyPwd(hHandle,nAddr,(BYTE*)&uPwd)==PS_OK) goto OPENOK;
			}
			PSCloseDeviceEx(hHandle);//*/
			goto OPENFail;
			break;
		case 2:	//DEVICE_UDisk
			sDevName.Format("UDisk_No.%d",m_iDevID);
			if(J_CheckDev(sDevName))
				J_CloseDev(sDevName);
			//	goto REOPEN;
			nRet = PSOpenDeviceEx(&hHandle,DEVICE_UDisk,m_iCom+1,iBaud,2,m_iDevID);
			if(nRet!=PS_OK)
				goto OPENFail;
			nRet = PSVfyPwd(hHandle,nAddr,(BYTE*)&uPwd);
			break;
		default:
			delete pDevHandle;
			J_SetMsg1("Unknown Device Type! ×",FALSE);
			AfxMessageBox("Unknown Device Type! ×");
			return;
	}
	if(nRet==PS_OK)//判断是否打开成功
		goto OPENOK;

OPENFail:
	m_iDevType = DEVICE_USB;//0
	UpdateData(0);
	delete pDevHandle;
	J_SetMsg1("Open device Fail. ×\r\nPlease check the DevType, Address and Password!",FALSE);
	AfxMessageBox("Open device Fail. ×\r\nPlease check the DevType, Address and Password!");
	return;

REOPEN://重复打开_设备重新打开
	delete pDevHandle;
	J_SetMsg1("Open device reiteration. ×\r\nPlease close the same opened device first!",FALSE);
	AfxMessageBox("Open device reiteration. ×\r\nPlease close the same opened device first!");
	return;

OPENOK:
	//...链表节点生成
	if(m_iDevType==DEVICE_COM)//DEVICE_COM
		UpdateData(FALSE);
	J_GetDevInfo(hHandle,nAddr,sDevInfo_CH,sDevInfo_EN,&iMbMax);
	
	//如果Type=COM，则需作特殊处理，关闭已免独占
	if(m_iDevType==DEVICE_COM)//1
	{
		PSCloseDeviceEx(hHandle);
	}

	pDevHandle->dwAddr	= nAddr;
	pDevHandle->dwPwd	= uPwd;
	pDevHandle->hHandle	= hHandle;
	pDevHandle->iCom	= m_iCom+1;
	pDevHandle->iDevId	= m_iDevID;
	pDevHandle->nDevType= m_iDevType;
	pDevHandle->next	= NULL;
	pDevHandle->sDevName= sDevName;
	pDevHandle->sMsg_CH	= sDevInfo_CH;
	pDevHandle->sMsg_EN	= sDevInfo_EN;
	pDevHandle->iMbMax	= iMbMax;
	if(m_iDevType==DEVICE_COM)
		pDevHandle->iBaud_x	  = iBaud;
	else
		pDevHandle->iBaud_x	  = J_GetBaudMul(m_iBaud_x);
	pDevHandle->iPacketSize_x = m_iPackSize_x;
	pDevHandle->iSecureLev_x  = m_iSLev_x+1;
	//...链表节点保存
	if(gHead_DevHandle==NULL)
		gHead_DevHandle = pDevHandle;
	else{
		for(pDevHTmp=gHead_DevHandle;pDevHTmp->next!=NULL;)
			pDevHTmp = pDevHTmp->next;
		pDevHTmp->next = pDevHandle;
	}
	res1 = m_ctlDevHandle.AddString(sDevName);//加入下拉列表
	m_ctlDevHandle.SetCurSel(res1);

	//...成功				
	if(g_iLanguage == LGN_CHINA){//硬件信息
		SetDlgItemText(IDC_STDevInfo,sDevInfo_CH);
		sDevOpen.Format(">正在与 %s 通讯.",sDevName);
	}
	else{
		sDevOpen.Format(">Connecting to %s.",sDevName);
		SetDlgItemText(IDC_STDevInfo,sDevInfo_EN);
	}
	m_cbSensorType.GetLBText(m_iSensorType,strSensor.GetBuffer(0));
	sTmp1.Format("Open device %s Success! √\r\nSensor Type: %s",sDevName,strSensor);
	J_SetMsg1(sTmp1);
	SetDlgItemText(IDC_STDevOpen,sDevOpen);//设备打开
//	AfxMessageBox("Open device success! √");
	J_EnableBtn();
	((CButton*)GetDlgItem(IDC_BUTTON1))->EnableWindow(1);
	J_LoadInfo2List();
	return;
}

void CFGDemoDlg::OnCloseDev() //关闭一个设备
{
	int iIndex = m_ctlDevHandle.GetCurSel();
	if(iIndex==CB_ERR){
		J_SetMsg1("Close device failed. ×\r\nPlease select a device to close or you not have an opened device.",FALSE);
		AfxMessageBox("Close device failed. ×\r\nPlease select a device to close or you not have an opened device.");
		return;
	}
	CString sDevName,sTmp1;
	m_ctlDevHandle.GetLBText(iIndex,sDevName);//获取名称
	if(J_CloseDev(sDevName)){
		sTmp1.Format("Close device %s Success! √",sDevName);
		J_SetMsg1(sTmp1);
//		AfxMessageBox("Close device success! √");
	}
	else{
		sTmp1.Format("Close device %s Failed! ×",sDevName);
		J_SetMsg1(sTmp1,FALSE);
//		AfxMessageBox("Close device failed. ×");
	}
//	m_ctlDevHandle.DeleteString(iIndex);//删除列表框信息
	int iAll = m_ctlDevHandle.GetCount();
	if(iAll>0){
		iIndex = ( (iAll-1)>iIndex ? iIndex:(iAll-1) );
		m_ctlDevHandle.SetCurSel(iIndex);
		OnSelchangeCOMBODev();
	}
	else
		J_EnableBtn(0,0);
	UpdateData(FALSE);
//	m_list.DeleteAllItems();
	g_iSelect = -1;
//	OnSelchangeComboLg();
}

void CFGDemoDlg::OnCloseDev2() //关闭所有设备
{
	m_ctlDevHandle.ResetContent();	
	OnSelchangeComboLg();
	pJsDevHandle pTmp2;
	if(gHead_DevHandle!=NULL){
		while(gHead_DevHandle!=NULL){
			pTmp2 = gHead_DevHandle;
			gHead_DevHandle = gHead_DevHandle->next;
			PSCloseDeviceEx(pTmp2->hHandle);
			delete pTmp2;
		}
		J_SetMsg1("Close all devices Success! √");
//		AfxMessageBox("Close all devices Success! √");
	}
	else{
		J_SetMsg1("Close all devices Failed! ×\r\nNo device was opened.",FALSE);
//		AfxMessageBox("Close all devices Failed! ×\r\nNo device was opened.");
	}
	m_list.DeleteAllItems();
	g_iSelect = -1;
	J_EnableBtn(0,0);
}

void CFGDemoDlg::OnButton1() //取消操作
{
	g_bStop = TRUE;
	J_EnableBtn();
	if(g_iLanguage==LGN_CHINA)//中文
		J_SetMsg1("操作被取消!");
	else
		J_SetMsg1("The operation was canceled.");
}
UINT J_gGenImg(LPVOID pParam)
{ 
	int ret,i=0,ImgLen;
	BYTE ImgData[IMAGE_SIZE]={0}; 
	CString strinfo;
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}
	
	pMyDlg->J_EnableBtn(0,1,0);
	pMyDlg->ShowDefaltImg();
	if(g_iLanguage==LGN_CHINA)//中文
		pMyDlg->J_SetMsg1("请将手指平放在传感器上...");
	else
		pMyDlg->J_SetMsg1("Please put your finger to sensor.");
	clock_t start,finish;
	start = clock();
	ret = PSGetImage(hHandle,nAddr);
	finish = clock();
	while(ret==PS_NO_FINGER)
	{
		start = clock();
		ret = PSGetImage(hHandle,nAddr);
		finish= clock();
		if(g_bStop || (ret==-1) || (ret==-2) || (ret==1) )//收发数据包失败时退出
		{//取消操作
			if(!g_bStop)
				pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$
				PSCloseDeviceEx(hHandle);
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}
	g_bStop = TRUE;
	if(ret!=PS_OK){
		pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		pMyDlg->J_EnableBtn(1);
		return 1;
	}
//	if(pMyDlg->m_bShowBMP)//需显示图像
	{
		pMyDlg->J_SetMsg2("图像正在上传...","Finger Data transfering...");
		ret = PSUpImage(hHandle,nAddr,ImgData,&ImgLen);
		pMyDlg->J_bImgPosN(ImgData,g_bImgpos);//######图象正反矫正
		memcpy(gImageData,ImgData,IMAGE_SIZE);
		if(ret!=PS_OK){
			if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$
				PSCloseDeviceEx(hHandle);
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
		ret = PSImgData2BMP(ImgData,gszIMGFILE);//IMG_FILE);
		if(ret!=PS_OK){
			if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$
				PSCloseDeviceEx(hHandle);
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
		pMyDlg->ShowImage();
	}
	if(g_iLanguage==LGN_CHINA)//中文
		strinfo.Format("图像获取成功!\r\n获取图象耗时%d毫秒.",finish-start);
	else
		strinfo.Format("Capture Success! \r\nGet image used %d ms",finish-start);
	pMyDlg->J_SetMsg1(strinfo);
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	pMyDlg->J_EnableBtn(1);
//	pMyDlg->J_SetFocus(IDC_BUTTON2);
	return 0;
}
void CFGDemoDlg::OnButton2() //获取图像
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	g_bStop = FALSE;
	AfxBeginThread( J_gGenImg,NULL);//开始执行获取图像线程
}

UINT J_gGenImgCon(LPVOID pParam)//连续获取图像
{
	int ret,retGet,i=0,ImgLen,iCount=0;
	BYTE ImgData[IMAGE_SIZE]={0}; 
    clock_t start,finish;
	CString strinfo,sNum="";
	pMyDlg->J_EnableBtn(0,1,0);
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}
	pMyDlg->ShowDefaltImg();
	
	iCount = 0;
	if(g_iLanguage==LGN_CHINA)//中文
		pMyDlg->J_SetMsg1(">>请将手指平放在传感器上...");
	else
		pMyDlg->J_SetMsg1(">>Please put your finger to sensor.");
	while (!g_bStop)
	{		
/*		if(g_iLanguage==LGN_CHINA)//中文
			pMyDlg->J_SetMsg1("请将手指平放在传感器上...");
		else
			pMyDlg->J_SetMsg1("Please put your finger to sensor.");//*/
		if(g_bStop)
			break;
		start  = clock();
		retGet = PSGetImage(hHandle,nAddr);  //获取图象
		finish = clock();
		while ( (retGet==PS_NO_FINGER)&&(!g_bStop) )
		{
			if(g_bStop || (retGet==-1) || (retGet==-2) || (retGet==1))//收发数据包失败时退出
			{
				if(!g_bStop)
					pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
				if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$
					PSCloseDeviceEx(hHandle);
				pMyDlg->J_EnableBtn();
				return 1;
			}
			start = clock();
			retGet= PSGetImage(hHandle,nAddr);
			finish= clock();
		}
		if(g_bStop)
		{
			if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$
				PSCloseDeviceEx(hHandle);
			pMyDlg->J_EnableBtn();
			return 1;
		}
//		if(pMyDlg->m_bShowBMP)//需显示图像
		{
			pMyDlg->J_SetMsg2("图像正在上传...","Finger Data transfering...");
			ret = PSUpImage(hHandle,nAddr,ImgData,&ImgLen);//上传图象
			pMyDlg->J_bImgPosN(ImgData,g_bImgpos);//######图象正反矫正
			memcpy(gImageData,ImgData,IMAGE_SIZE);
			if(ret!=PS_OK){
				if(ret!=-1 && ret!=-2 && ret!=1){//·······不是收发包错误时，不退出，继续执行。20090112
					pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
					Sleep(1223);
					continue;
				}
				if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$$
					PSCloseDeviceEx(hHandle);
				pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
				pMyDlg->J_EnableBtn(1);
				return 1;
			}
			ret = PSImgData2BMP(ImgData,gszIMGFILE);//IMG_FILE);
			if(ret!=PS_OK){
				if(ret!=-1 && ret!=-2 && ret!=1){//·······不是收发包错误时，不退出，继续执行。20090112
					pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
					Sleep(1223);
					continue;
				}
				if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$$
					PSCloseDeviceEx(hHandle);
				pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
				pMyDlg->J_EnableBtn(1);
				return 1;
			}
			pMyDlg->ShowImage();
			//Sleep(500);
			pMyDlg->GetDlgItem(IDC_BUTTON4)->EnableWindow(1);
		}
		iCount++;
		if(g_bImgCont)
			sNum.Format("[Imgs:%d]",iCount);
		else
			sNum = "";
		if(retGet==PS_OK){
			if(g_iLanguage==LGN_CHINA)
				strinfo.Format("图像获取成功! %02XH\t%s\r\n获取图象耗时 %d 毫秒.\r\n>>请将手指平放在传感器上...",retGet,sNum,finish-start);
			else
				strinfo.Format("Capture Success! \t%s\r\nGet image used %d ms.\r\n>>Please put your finger to sensor.",sNum,finish-start);
			pMyDlg->J_SetMsg1(strinfo);
		}
		else{
			if(g_iLanguage==LGN_CHINA)//中文
				strinfo.Format("图像质量太差! %02XH\t%s\r\n获取图象耗时 %d 毫秒.\r\n>>请将手指平放在传感器上...",retGet,sNum,finish-start);
			else
				strinfo.Format("The Image is bad!\t%s\r\nGet image used %d ms.\r\n>>Please put your finger to sensor.",sNum,finish-start);
			pMyDlg->J_SetMsg1(strinfo);
		}	
//		Sleep(1223);
	}
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	pMyDlg->J_EnableBtn(1);	
	return 0;
}

void CFGDemoDlg::OnButton3() //连续获取图像
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	g_bStop = FALSE;
	AfxBeginThread( J_gGenImgCon,NULL);//开始执行获取图像线程
}

void CFGDemoDlg::OnButton4() //保存图像
{
	int ret;
	CString strFile,sFilter;	
	if(g_iLanguage==LGN_CHINA)//中文
		sFilter="位图文件(*.bmp)|*.bmp|所有文件(*.*)|*.*||";
	else
		sFilter="BMP File(*.bmp)|*.bmp|All File(*.*)|*.*||";
	CFileDialog fo(FALSE,"bmp","Finger.bmp",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,sFilter,NULL);
	if(fo.DoModal() != IDOK)
		return ;
	strFile = fo.GetFileName();
	ret= PSImgData2BMP(gImageData,(char*)(LPCTSTR)strFile);	//保存到文件
	if(ret!=PS_OK)
	{
		J_SetMsg1(PSErr2Str(ret),0);
		return;		
	}
	
	if(g_iLanguage==LGN_CHINA)//中文
		J_SetMsg1("保存图像成功！");	
	else
		J_SetMsg1("Success to save image");	
	return ;
}

void CFGDemoDlg::OnButton5() //加载图像
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;

	int ret;
	CString strFile,sFilter;
	if(g_iLanguage==LGN_CHINA)//中文
		sFilter="位图文件(*.bmp)|*.bmp|所有文件(*.*)|*.*||";
	else
		sFilter="BMP File(*.bmp)|*.bmp|All File(*.*)|*.*||";
	CFileDialog fo(TRUE,"bmp",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,sFilter,NULL);
	if(fo.DoModal() != IDOK)
		return ;
	strFile = fo.GetFileName();

	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn();
			return;
		}
	}
	ret= PSGetImgDataFromBMP(hHandle,(char*)(LPCTSTR)strFile,gImageData,&g_IamgeLen);	//图像数据装载到内存
    if(ret!=PS_OK)
	{
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		return ;		
	}
	if(g_iLanguage==LGN_CHINA)//中文
		J_SetMsg1("正在下载图像...");
	else
		J_SetMsg1("Downloading...");
	ret = PSDownImage(hHandle,nAddr,gImageData,g_IamgeLen);  //下载图象
	if(ret!=PS_OK)
	{
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		return ;		
	}
	ret= PSImgData2BMP(gImageData,gszIMGFILE);//IMG_FILE);//刷新时用到
	
	ShowImage((char*)(LPCTSTR)strFile);
	if(g_iLanguage==LGN_CHINA)//中文
		J_SetMsg1("下载图像成功！");	
	else
		J_SetMsg1("Success to download!");	
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	return ;	
}

UINT J_gEnroll(LPVOID pParam)//录入指纹
{ 
	pMyDlg->J_EnableBtn(0,1,0);
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int		nMBMax	= gCur_DevHandle->iMbMax;//指纹库大小
	int  ret=0,iFingerNum = 0,iBuffer;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}

	BOOL bOK = FALSE;
	pMyDlg->ShowDefaltImg();
	CString str,stp;
	iFingerNum = 1;
	while ( iFingerNum <= g_nMaxTry )
	{
		if(g_iLanguage==LGN_CHINA)//中文
			stp.Format("%d.请将手指平放在传感器上...",(iFingerNum-1)%g_nEN_NUM+1);
		else
			stp.Format("%d.Please put your finger to sensor...",(iFingerNum-1)%g_nEN_NUM+1);
		pMyDlg->J_SetMsg1(stp);	
		if(g_bEnrollIMG)
			ret = PSGetImage_Enroll(hHandle,nAddr);//............................................. 获取图像 Step_1
		else
			ret = PSGetImage(hHandle,nAddr);
		while(ret!=PS_OK)
		{
#ifdef GETIMG_BAD //差指纹录取和搜索权限
			if(ret==PS_FP_TOO_DRY || ret==PS_FP_TOO_WET || ret==PS_FP_DISORDER)
				break;
#endif
			if(ret!=PS_NO_FINGER){
				str.Format("%s\r\n%s",PSErr2Str(ret),stp);
				pMyDlg->J_SetMsg1(str,0);
			}

			if(g_bEnrollIMG)
				ret = PSGetImage_Enroll(hHandle,nAddr);
			else
				ret = PSGetImage(hHandle,nAddr);
			if(g_bStop || (ret==-1) || (ret==-2) || (ret==1) )//收发数据包失败时退出
			{
				if(!g_bStop)
					pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
				if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
					PSCloseDeviceEx(hHandle);
				pMyDlg->J_EnableBtn();
				return 1;
			}
		}
		if(g_bStop)
		{
			if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
				PSCloseDeviceEx(hHandle);
			pMyDlg->J_EnableBtn();
			return 1;
		}
		if(g_iLanguage==LGN_CHINA)//中文
			pMyDlg->J_SetMsg1("请拿开手指.");	
		else
			pMyDlg->J_SetMsg1("Please take off finger.");
		//Sleep(300);
		if(pMyDlg->m_bShowBMP)
		{
			if(g_iLanguage==LGN_CHINA)//中文
				pMyDlg->J_SetMsg1("图像录入成功! \r\n正在上传...");	
			else
				pMyDlg->J_SetMsg1("Get Image Success!\r\nImage transfering...");
			ret = PSUpImage(hHandle,nAddr,gImageData,&g_IamgeLen);
			pMyDlg->J_bImgPosN(gImageData,g_bImgpos);//######图象正反矫正
			if(ret!=PS_OK)
			{
				if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
					PSCloseDeviceEx(hHandle);
				pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
				pMyDlg->J_EnableBtn();
				return 1;//上传图像不正常，退出
			}
			ret = PSImgData2BMP(gImageData,gszIMGFILE);//IMG_FILE);
			if(ret!=PS_OK)
			{
				if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
					PSCloseDeviceEx(hHandle);
				pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
				pMyDlg->J_EnableBtn();
				return 1;//上传图像不正常，退出
			}
			pMyDlg->ShowImage();
		}
		iBuffer = (iFingerNum-1)%g_nEN_NUM+1;
		ret = PSGenChar(hHandle,nAddr,iBuffer);//.......................................生成模板 Step_2
		if(ret!=PS_OK)
		{//录入失败，需重新录入
			if(g_iLanguage==LGN_CHINA)//中文
				str.Format("%s\r\n请重新录入!",PSErr2Str(ret));
			else
				str.Format("%s\r\nPlease re-enroll!",PSErr2Str(ret));
			pMyDlg->J_SetMsg1(str,0);
			//Sleep(300);
			continue;
		}
		if( iFingerNum<g_nEN_NUM ){
			iFingerNum++;
			//Sleep(380);
			continue;//前几次
		}
		iFingerNum++;//最后一次
		ret = PSRegModule(hHandle,nAddr);//.............................................合并特征 Step_3
		if(ret!=PS_OK)
		{
			continue;//合并特征不正常
		}
		ret = PSStoreChar(hHandle,nAddr,1,g_iIndex);//..................................存放模板 Step_4
		if(ret!=PS_OK)
		{
			continue;//存放模板不正常
		}
		bOK = TRUE;
		break;
	}
	if(!bOK){
		pMyDlg->J_SetMsg2("录入失败!","Enroll failed!",0);
		goto END;
	}
	if(!g_bReplace)//添加模式
	{
		str.Format("%d",g_iIndex);
		pMyDlg->m_list.InsertItem(g_iIndex,str);
		//如果是老版的程序，则不支持最新读模板索引功能，所以沿用从文件加载的老方法////////20090106///
		if( nMBMax==120 || nMBMax==376 || nMBMax==888 )
		{
			FILE* fp;
			if( (fp=fopen("FGTemplate.dat","ab+"))!=NULL )
			{
				BYTE pIdx[2]={0};//2Byte表示一个索引页信息
				pIdx[0] = (BYTE)((g_iIndex&0xFF00)>>8);
				pIdx[1] = (BYTE)(g_iIndex&0x00FF);
				fseek(fp,0,SEEK_END);//SEEK_END = 2
				fwrite(pIdx,sizeof(BYTE),2,fp);
				fclose(fp);
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////
	}
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("用户添加成功!\r\nFingerID = %d.",g_iIndex);
	else
		str.Format("Success to enroll!\r\nFingerID = %d.",g_iIndex);
	g_iIndex++;	
	pMyDlg->J_SetMsg1(str);

END:
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	pMyDlg->J_EnableBtn();
	return 0;
}
void CFGDemoDlg::OnButton6() //录入指纹
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}

	CDlgUser dlg;
	dlg.m_nAddr = g_iIndex;
	if(dlg.DoModal()!=IDOK)
		return;
	g_iIndex	= dlg.m_nAddr;//用户地址(索引页)
	g_bReplace	= FALSE;
	CString str;
	if(J_IsExistId(g_iIndex))//已存在，是否覆盖
	{
		if(m_bReplaceMB)
			g_bReplace = TRUE;
		else{
			if(g_iLanguage==LGN_CHINA)//中文
				str="该地址已经存有用户，是否覆盖?";
			else
				str="The address has user! do you want to overwrite?";
			if ( AfxMessageBox(str, MB_YESNO) != IDYES )
				return;//不覆盖
			g_bReplace = TRUE;
		}
	}
	g_bStop  = FALSE;
	AfxBeginThread( J_gEnroll,NULL);//开始执行线程
}
UINT J_gEnrollCon(LPVOID pParam)//连续录入指纹
{ 
	int ret=0,iFingerNum = 0,iBuffer;
	BOOL bOK;
	CString str,stp;
	CDlgUser dlg;
	pMyDlg->J_EnableBtn(0,1,0);
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int		nMBMax	= gCur_DevHandle->iMbMax;//指纹库大小
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}

ReEnroll_GetAddr://获取用户地址
	dlg.m_nAddr = g_iIndex;
	if(dlg.DoModal()!=IDOK){
		goto ReEnroll_END;//按取消时，结束录入
	}
	g_iIndex = dlg.m_nAddr;//用户地址(索引页)

ReEnroll_GOTO://新一轮录入
	if(g_bStop){
		goto ReEnroll_END;//停止
	}
	pMyDlg->ShowDefaltImg();
	g_bReplace = FALSE;
	if( pMyDlg->J_IsExistId(g_iIndex) )
	{
		if(pMyDlg->m_bReplaceMB)
			g_bReplace = TRUE;
		else
		{
			if(g_iLanguage==LGN_CHINA)//中文
				str.Format("开始录入指纹 FingerID=%d\r\n该地址已经存有用户，是否覆盖?",g_iIndex);
			else
				str.Format("Start Enroll! FingerID=%d\r\nThe address has user! do you want to overwrite?",g_iIndex);
			if ( AfxMessageBox(str, MB_YESNO) != IDYES )
				goto ReEnroll_GetAddr;//不覆盖,重新输入用户地址
			g_bReplace = TRUE;
		}
	}

	//开始录入指纹	
	bOK = FALSE;
	iFingerNum = 1;
	while( iFingerNum <= g_nMaxTry )
	{
		if(g_iLanguage==LGN_CHINA)//中文
			stp.Format("%d.请将手指平放在传感器上...",(iFingerNum-1)%g_nEN_NUM+1);
		else
			stp.Format("%d.Please put your finger to sensor...",(iFingerNum-1)%g_nEN_NUM+1);
		pMyDlg->J_SetMsg1(stp);	
		if(g_bEnrollIMG)
			ret = PSGetImage_Enroll(hHandle,nAddr);//............... 获取图像 Step_1
		else
			ret = PSGetImage(hHandle,nAddr);
		while(ret!=PS_OK)
		{
#ifdef GETIMG_BAD //差指纹录取和搜索权限
			if(ret==PS_FP_TOO_DRY || ret==PS_FP_TOO_WET || ret==PS_FP_DISORDER)
				break;
#endif
			if(ret!=PS_NO_FINGER){
				str.Format("%s\r\n%s",PSErr2Str(ret),stp);
				pMyDlg->J_SetMsg1(str,0);
			}
			if(g_bEnrollIMG)
				ret = PSGetImage_Enroll(hHandle,nAddr);
			else
				ret = PSGetImage(hHandle,nAddr);
			if(g_bStop || (ret==-1) || (ret==-2) || (ret==1))//收发数据包失败时退出
			{
				if(!g_bStop)
					pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
				goto ReEnroll_END;
			}
		}
		if(g_bStop){
			goto ReEnroll_END;//停止
		}
		if(g_iLanguage==LGN_CHINA)//中文
			pMyDlg->J_SetMsg1("请拿开手指.");	
		else
			pMyDlg->J_SetMsg1("Please take off finger.");
		//Sleep(300);
		if(pMyDlg->m_bShowBMP)
		{
			if(g_iLanguage==LGN_CHINA)//中文
				pMyDlg->J_SetMsg1("图像录入成功! \r\n正在上传...");	
			else
				pMyDlg->J_SetMsg1("Get Image Success!\r\nImage transfering...");
			ret = PSUpImage(hHandle,nAddr,gImageData,&g_IamgeLen);
			pMyDlg->J_bImgPosN(gImageData,g_bImgpos);//######图象正反矫正
			if(ret!=PS_OK)
			{
				if(ret!=-1 && ret!=-2 && ret!=1){//·······不是收发包错误时，不退出，继续执行。20090112
					pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
					goto ReEnroll_Step2;
				}
				pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
				goto ReEnroll_END;//上传图像不正常，退出
			}
			ret = PSImgData2BMP(gImageData,gszIMGFILE);//IMG_FILE);
			if(ret!=PS_OK)
			{
				if(ret!=-1 && ret!=-2 && ret!=1){//·······不是收发包错误时，不退出，继续执行。20090112
					pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
					goto ReEnroll_Step2;
				}
				pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
				goto ReEnroll_END;//上传图像不正常，退出
			}
			pMyDlg->ShowImage();
			pMyDlg->GetDlgItem(IDC_BUTTON4)->EnableWindow(1);
		}
ReEnroll_Step2:
		iBuffer = (iFingerNum-1)%g_nEN_NUM+1;
		ret = PSGenChar(hHandle,nAddr,iBuffer);//....................生成模板 Step_2
		if(ret!=PS_OK)
		{//录入失败，需重新录入
			if(g_iLanguage==LGN_CHINA)//中文
				str.Format("%s\r\n请重新录入!",PSErr2Str(ret));
			else
				str.Format("%s\r\nPlease re-enroll!",PSErr2Str(ret));
			pMyDlg->J_SetMsg1(str,0);
			//Sleep(300);
			continue;
		}
		if( iFingerNum<g_nEN_NUM ){
			iFingerNum++;
			//Sleep(380);
			continue;//前几次
		}
		iFingerNum++;//最后一次操作完成，开始合成
		ret = PSRegModule(hHandle,nAddr);//.........................合并特征 Step_3
		if(ret!=PS_OK) {
			continue;//合并特征不正常
		}
		ret = PSStoreChar(hHandle,nAddr,1,g_iIndex);//..............存放模板 Step_4
		if(ret!=PS_OK) {
			continue;//存放模板不正常
		}
		bOK = TRUE;
		break;
	}
	if(!bOK){
		pMyDlg->J_SetMsg2("录入失败!","Enroll failed!",0);
		Sleep(580);
		goto ReEnroll_GOTO;
	}
	if(!g_bReplace)//添加模式
	{
		str.Format("%d",g_iIndex);
		pMyDlg->m_list.InsertItem(g_iIndex,str);
		//如果是老版的程序，则不支持最新读模板索引功能，所以沿用从文件加载的老方法////////20090106///
		if( nMBMax==120 || nMBMax==376 || nMBMax==888 )
		{
			FILE* fp;
			if( (fp=fopen("FGTemplate.dat","ab+"))!=NULL )
			{
				BYTE pIdx[2]={0};//2Byte表示一个索引页信息
				pIdx[0] = (BYTE)((g_iIndex&0xFF00)>>8);
				pIdx[1] = (BYTE)(g_iIndex&0x00FF);
				fseek(fp,0,SEEK_END);//SEEK_END = 2
				fwrite(pIdx,sizeof(BYTE),2,fp);
				fclose(fp);
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////
	}
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("用户添加成功!\r\nFingerID = %d.",g_iIndex);
	else
		str.Format("Success to enroll!\r\nFingerID = %d.",g_iIndex);
	pMyDlg->J_SetMsg1(str);
	g_iIndex++;
	Sleep(580);
	goto ReEnroll_GOTO;//录入成功，开始新一轮录入

ReEnroll_END:
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	pMyDlg->J_EnableBtn();
	return 1;
}
void CFGDemoDlg::OnButton7() //连续录入
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	g_bStop  = FALSE;
	AfxBeginThread( J_gEnrollCon,NULL);//开始执行线程*/
}

UINT J_gVerify(LPVOID pParam)//单一比对
{ 
	if(g_iSelect<0){
		pMyDlg->J_SetMsg2("先请选定要比对的模板! ×","Please select the template first. ",0);
		return 1;
	}
	int iScore=0;
	clock_t start,finish;

	pMyDlg->J_EnableBtn(0,1,0);
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int  ret=1;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}

	pMyDlg->ShowDefaltImg();
	CString str;
	ret = PSLoadChar(hHandle,nAddr,CHAR_BUFFER_B,g_iSelect);
	if(ret!=PS_OK){
		pMyDlg->J_SetMsg1(PSErr2Str(ret));
		goto END;
	}
	pMyDlg->J_SetMsg2("请将手指平放在传感器上...","Please put your finger to sensor...");	
	ret = PSGetImage(hHandle,nAddr);
	while(ret!=PS_OK)
	{
#ifdef GETIMG_BAD
		if(ret==PS_FP_TOO_DRY || ret==PS_FP_TOO_WET || ret==PS_FP_DISORDER)
			break;//2010.12.23
#endif
		ret = PSGetImage(hHandle,nAddr);
		if(g_bStop || (ret==-1) || (ret==-2) || (ret==1) ){//收发数据包失败时退出
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto END;
		}
	}
	if(pMyDlg->m_bShowBMP)//需显示图像
	{
		pMyDlg->J_SetMsg2("图像录入成功! \r\n正在上传...","Get Image Success!\r\nImage transfering...");
		ret = PSUpImage(hHandle,nAddr,gImageData,&g_IamgeLen);//上传图象
		pMyDlg->J_bImgPosN(gImageData,g_bImgpos);//######图象正反矫正
		if(ret!=PS_OK)
		{
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto END;//上传图像不正常，退出
		}
		ret = PSImgData2BMP(gImageData,gszIMGFILE);//IMG_FILE);
		if(ret!=PS_OK)
		{
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto END;//上传图像不正常，退出
		}
		pMyDlg->ShowImage();
	}
	else
		pMyDlg->J_SetMsg2("图像录入成功!","Success to get image!");
	ret = PSGenChar(hHandle,nAddr,CHAR_BUFFER_A);//生成模板
	if(ret!=PS_OK){
		pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
		goto END;//生成模板失败，退出
	}
	start = clock();
	ret = PSMatch(hHandle,nAddr,&iScore);
	finish= clock();
	if(ret!=PS_OK || iScore<50){//比对失败
		if(g_iLanguage==LGN_CHINA)//中文
			str.Format("比对失败!   FingerID=%d\r\n用时: %d 毫秒.\r\n得分: %d",g_iSelect,finish-start,iScore);
		else
			str.Format("Deny!   FingerID=%d\r\nTime used: %d ms\r\nScore: %d",g_iSelect,finish-start,iScore);
		pMyDlg->J_SetMsg1(str,0);
		goto END;
	}
	//比对通过
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("比对通过!   FingerID=%d\r\n用时: %d 毫秒.\r\n得分: %d",g_iSelect,finish-start,iScore);
	else
		str.Format("Pass!   FingerID=%d\r\nTime used: %d ms\r\nScore: %d",g_iSelect,finish-start,iScore);
	pMyDlg->J_SetMsg1(str);

END:
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	pMyDlg->J_EnableBtn();
	return 0;
}
void CFGDemoDlg::OnButton8() //单一比对
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	if(g_iSelect<0){
		J_SetMsg2("先请选定要比对的模板! ×","Please select the template first. ",0);
		return;
	}
	g_bStop  = FALSE;
	AfxBeginThread( J_gVerify,NULL);//开始执行线程*/
}

void CFGDemoDlg::OnClickList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	g_iSelect = -1;
	POSITION posTemp;
	int iIndex;
	posTemp=m_list.GetFirstSelectedItemPosition();
	if(posTemp==NULL)
		return;
	iIndex = m_list.GetNextSelectedItem(posTemp);
	CString sID	= m_list.GetItemText(iIndex,0);
	g_iSelect	= atoi(sID);
	g_iSelectIndex = iIndex;
	
	*pResult = 0;
}
 
UINT J_gSearch(LPVOID pParam)//指纹搜索
{ 
	pMyDlg->J_EnableBtn(0,1,0);
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		iMBMax	= gCur_DevHandle->iMbMax;
	int		nDevType= gCur_DevHandle->nDevType;
	int ret=1;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}

	clock_t start,finish;
	int iPageID=0,iScore=0;

	pMyDlg->ShowDefaltImg();
	CString str;
	pMyDlg->J_SetMsg2("请将手指平放在传感器上...","Please put your finger to sensor...");	
	ret = PSGetImage(hHandle,nAddr);
	while(ret!=PS_OK){
#ifdef GETIMG_BAD
		if(ret==PS_FP_TOO_DRY || ret==PS_FP_TOO_WET || ret==PS_FP_DISORDER)
			break;//2010.12.23
#endif
		ret = PSGetImage(hHandle,nAddr);
		if(g_bStop || (ret==-1) || (ret==-2) || (ret==1) ){//收发数据包失败时退出
			if(!g_bStop)
				pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto Search_END;
		}
	}
	if(pMyDlg->m_bShowBMP)//需显示图像
	{
		pMyDlg->J_SetMsg2("图像录入成功! \r\n正在上传...","Get Image Success!\r\nImage transfering...");
		ret = PSUpImage(hHandle,nAddr,gImageData,&g_IamgeLen);//上传图象
		pMyDlg->J_bImgPosN(gImageData,g_bImgpos);//######图象正反矫正
		if(ret!=PS_OK)
		{
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto Search_END;//上传图像不正常，退出
		}
		ret = PSImgData2BMP(gImageData,gszIMGFILE);//IMG_FILE);
		if(ret!=PS_OK)
		{
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto Search_END;//上传图像不正常，退出
		}
		pMyDlg->ShowImage();
	}
	else
		pMyDlg->J_SetMsg2("图像录入成功!","Success to get image!");
	pMyDlg->J_SetMsg2("图像录入成功,搜索中...","Success to enroll,Searching...");
	ret = PSGenChar(hHandle,nAddr,CHAR_BUFFER_A);//生成模板
	if(ret!=PS_OK){
		pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
		goto Search_END;//生成模板不正常，退出
	}
	start = clock();
	ret = PSSearch(hHandle,nAddr,CHAR_BUFFER_A,0,iMBMax,&iPageID,&iScore);
//	ret = PSHighSpeedSearch(hHandle,nAddr,CHAR_BUFFER_A,0,iMBMax,&iPageID,&iScore);
	finish= clock();
	if(ret==PS_OK){
		if(g_iLanguage==LGN_CHINA)//中文
			str.Format("找到相同手指! √  FingerID=%d\r\n用时: %d 毫秒.\r\n得分: %d",iPageID,finish-start,iScore);
		else
			str.Format("Find same finger! √  FingerID=%d\r\nTime used: %d ms\r\nScore: %d",iPageID,finish-start,iScore);
		pMyDlg->J_SetMsg1(str);
	}
	else{
		if(g_iLanguage==LGN_CHINA)//中文
			str.Format("没有找到相同的手指!  × \r\n用时: %d 毫秒.\r\n",finish-start);
		else
			str.Format("Not Find same finger!  × \r\nTime used: %d ms\r\n",finish-start);
		pMyDlg->J_SetMsg1(str,0);
	}

Search_END:
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	pMyDlg->J_EnableBtn();
	return 0;
}
void CFGDemoDlg::OnButton9() //搜索
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	g_bStop = FALSE;
	AfxBeginThread( J_gSearch,NULL );	
}

UINT J_gSearchCon(LPVOID pParam)//连续搜索
{
	pMyDlg->J_EnableBtn(0,1,0);
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		iMBMax	= gCur_DevHandle->iMbMax;
	int		nDevType= gCur_DevHandle->nDevType;
	CString str;
	clock_t start,finish;
	int ret=1,iPageID=0,iScore=0;
	int iCnt=0,iSucc=0,iFail=0;

	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}

	pMyDlg->J_SetMsg2(">>请将手指平放在传感器上...",">>Please put your finger to sensor...");
	pMyDlg->ShowDefaltImg();
SEARCHCON_goto:
//	pMyDlg->J_SetMsg2("请将手指平放在传感器上...","Please put your finger to sensor...");
	if(g_bStop){
		goto END;
	}
	ret = PSGetImage(hHandle,nAddr);
	while(ret!=PS_OK){
#ifdef GETIMG_BAD
		if(ret==PS_FP_TOO_DRY || ret==PS_FP_TOO_WET || ret==PS_FP_DISORDER)
			break;//2010.12.23
#endif
		ret = PSGetImage(hHandle,nAddr);
		if(g_bStop || (ret==-1) || (ret==-2) || (ret==1) ){//收发数据包失败时退出
			if(!g_bStop)
				pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto END;
		}
	}
	if(pMyDlg->m_bShowBMP)//需显示图像
	{
		pMyDlg->J_SetMsg2("图像录入成功! \r\n正在上传...","Get Image Success!\r\nImage transfering...");
		ret = PSUpImage(hHandle,nAddr,gImageData,&g_IamgeLen);//上传图象
		pMyDlg->J_bImgPosN(gImageData,g_bImgpos);//######图象正反矫正
		if(ret!=PS_OK)
		{
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto END;//上传图像不正常，退出
		}
		ret = PSImgData2BMP(gImageData,gszIMGFILE);//IMG_FILE);
		if(ret!=PS_OK)
		{
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto END;//上传图像不正常，退出
		}
		pMyDlg->ShowImage();
		pMyDlg->GetDlgItem(IDC_BUTTON4)->EnableWindow(1);
	}
	else
		pMyDlg->J_SetMsg2("图像录入成功!","Success to get image!");
	pMyDlg->J_SetMsg2("图像录入成功,搜索中...","Success to enroll,Searching...");
	ret = PSGenChar(hHandle,nAddr,CHAR_BUFFER_A);//生成模板
	if(ret!=PS_OK){
		pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
		goto END;//生成模板不正常，退出
	}
	start = clock();
	if(0==g_iSearchMode)
		ret = PSSearch(hHandle,nAddr,CHAR_BUFFER_A,0,iMBMax,&iPageID,&iScore);//搜索
	else
		ret = PSHighSpeedSearch(hHandle,nAddr,CHAR_BUFFER_A,0,iMBMax,&iPageID,&iScore);//高速搜索
	finish= clock();
	if(ret==PS_OK){
		if(g_iLanguage==LGN_CHINA)//中文
			str.Format("找到相同手指! √  FingerID=%d\r\n用时: %d 毫秒.\t得分: %d\r\n>>请将手指平放在传感器上...",iPageID,finish-start,iScore);
		else
			str.Format("Find same finger! √  FingerID=%d\r\nTime used: %d ms\tScore: %d\r\n>>Please put your finger to sensor...",iPageID,finish-start,iScore);
		pMyDlg->J_SetMsg1(str);
	}
	else{
		if(g_iLanguage==LGN_CHINA)//中文
			str.Format("没有找到相同的手指!  × \r\n用时: %d 毫秒.\r\n>>请将手指平放在传感器上...",finish-start);
		else
			str.Format("Not Find same finger!  × \r\nTime used: %d ms.\r\n>>Please put your finger to sensor...",finish-start);
		pMyDlg->J_SetMsg1(str,0);
	}

	//连续搜索计数模块
	if(g_bSearchCont)
	{
		iCnt++;
		if(ret==PS_OK)
			iSucc++;
		else
			iFail++;
		if(iCnt>9){
			str.Format("Succ : %d   Failed : %d",iSucc,iFail);
			pMyDlg->J_SetMsg1(str);
			goto END;
		}
	}

//	Sleep(1223);
	goto SEARCHCON_goto;

END:
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	pMyDlg->J_EnableBtn();
	return 0;
}
void CFGDemoDlg::OnButton10() //连续搜索
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	g_bStop = FALSE;
	g_iSearchMode = 0;//连续搜索
	AfxBeginThread( J_gSearchCon,NULL );
}

void CFGDemoDlg::OnButton11() //连续高速搜索
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	g_bStop = FALSE;
	g_iSearchMode = 1;//连续高速搜索
	AfxBeginThread( J_gSearchCon,NULL );
}

void CFGDemoDlg::OnButton12() //删除指纹
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	if(g_iSelect<0){
		J_SetMsg2("先请选定要删除的模板! ×","Please select the template first. ",0);
		return;
	}
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int		nMBMax	= gCur_DevHandle->iMbMax;//指纹库大小
	
	CString str;
	int ret=1;
	if(g_iLanguage==LGN_CHINA)//中文
		str = "你确实要删除指定的用户吗?";
	else
		str = "Do you really want to delete the user?";
	if(AfxMessageBox(str, MB_YESNO) != IDYES)
		return;//舍不得删除，所以返回

	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return;
		}
	}
	ret = PSDelChar(hHandle,nAddr,g_iSelect,1);
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1( PSErr2Str(ret) );
		return;
	}
	m_list.DeleteItem(g_iSelectIndex);
	//如果是老版的程序，则不支持最新读模板索引功能，所以沿用从文件加载的老方法////////20090106///
	if( nMBMax==120 || nMBMax==376 || nMBMax==888 )
	{
		FILE* fp;
		if( (fp=fopen("FGTemplate.dat","wb"))!=NULL )
		{
			BYTE pIdx[2]={0};//2Byte表示一个索引页信息
			int nIdx=0;
			CString sIdx;
			for(int i=0; i<m_list.GetItemCount(); i++){
				sIdx = m_list.GetItemText(i,0);
				nIdx = atoi(sIdx);
				pIdx[0] = (BYTE)((nIdx&0xFF00)>>8);
				pIdx[1] = (BYTE)(nIdx&0x00FF);
				fwrite(pIdx,sizeof(BYTE),2,fp);
			}
			fclose(fp);
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("删除指定地址成功!  (FingerID=%d)",g_iSelect);
	else
		str.Format("Delete the address success!  (FingerID=%d)",g_iSelect);
	g_iSelect = -1;
	J_SetMsg1(str);
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
}

void CFGDemoDlg::OnButton13() //清空指纹库
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int		nMBMax	= gCur_DevHandle->iMbMax;//指纹库大小
	
	CString str;
	int ret=1;
	if(g_iLanguage==LGN_CHINA)//中文
		str = "你确实要清空所有用户吗?";
	else
		str = "Do you really want to clear all user?";
	if(AfxMessageBox(str, MB_YESNO) != IDYES)
		return;//舍不得删除，所以返回

	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return;
		}
	}
	ret = PSEmpty(hHandle,nAddr);
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1( PSErr2Str(ret) );
		return;
	}
	m_list.DeleteAllItems();
	g_iIndex = 0;
	//如果是老版的程序，则不支持最新读模板索引功能，所以沿用从文件加载的老方法////////20090106///
	if( nMBMax==120 || nMBMax==376 || nMBMax==888 )
	{
		FILE* fp;
		if( (fp=fopen("FGTemplate.dat","wb"))!=NULL )
		{
			fclose(fp);
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	J_SetMsg2("删除成功!","Success to clear!");
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
}
UINT J_gLoadFingerMB(LPVOID pParam)//载入指纹库
{
	pMyDlg->J_EnableBtn(0,1,0);
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		iMBMax	= gCur_DevHandle->iMbMax;
	int		nDevType= gCur_DevHandle->nDevType;
	int		nMBMax	= gCur_DevHandle->iMbMax;//指纹库大小
	int ret,tmpLen,iCount=0,iIndex;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}

	int nCharLen=512;
	FILE *fp;
	BYTE tmpBuf[CHAR_LEN_AES1711+1]={0};
	CString sFileName,str;
	CFileDialog dlg(TRUE,"*.mb","TmpFG",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"Template file(*.mb)|*.mb|Character file(*.dat)|*.dat||",NULL);

	if(g_iIndex>=iMBMax){
		pMyDlg->J_SetMsg2("地址超出范围! ×","Address out of range! ×");
		goto END;
	}

	if(IDOK!=dlg.DoModal())
		goto END;
	sFileName = dlg.GetPathName();
	if( (fp=fopen(sFileName,"rb"))==NULL ){
		pMyDlg->J_SetMsg2("打开指纹模板库文件 失败! ×","Open Template file failed! × ",0);
		goto END;
	}
	if(g_bStop){
		fclose(fp);
		goto END;
	}
	iCount=0;
	iIndex = g_iIndex;
	ret = PSGetCharLen(&nCharLen);
	pMyDlg->J_SetMsg2("正在加载指纹模板库,请耐心等待...","Loading fingerprint template library, please be patient ...");
	while(!feof(fp)){//模板数据载入指纹库
		if(g_bStop ){
			fclose(fp);
			goto END;
		}
		if(g_iIndex>=iMBMax){
			break;//指纹库满，无法继续加载
		}
		tmpLen = fread(tmpBuf,sizeof(BYTE),nCharLen,fp);
		if(tmpLen!=nCharLen)
			continue;
		ret = PSDownChar(hHandle,nAddr,CHAR_BUFFER_A,tmpBuf,tmpLen);
		if(ret!=PS_OK){
			fclose(fp);
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto END;
		}
		ret = PSStoreChar(hHandle,nAddr,CHAR_BUFFER_A,g_iIndex);
		if(ret!=PS_OK){
			fclose(fp);
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			goto END;
		}
		if(!(pMyDlg->J_IsExistId(g_iIndex))){//如果不存在，作添加模式
			str.Format("%d",g_iIndex);
			pMyDlg->m_list.InsertItem(g_iIndex,str);
			//如果是老版的程序，则不支持最新读模板索引功能，所以沿用从文件加载的老方法////////20090106///
			if( nMBMax==120 || nMBMax==376 || nMBMax==888 )
			{
				FILE* fp;
				if( (fp=fopen("FGTemplate.dat","ab+"))!=NULL )
				{
					BYTE pIdx[2]={0};//2Byte表示一个索引页信息
					pIdx[0] = (BYTE)((g_iIndex&0xFF00)>>8);
					pIdx[1] = (BYTE)(g_iIndex&0x00FF);
					fseek(fp,0,SEEK_END);//SEEK_END = 2
					fwrite(pIdx,sizeof(BYTE),2,fp);
					fclose(fp);
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////
		}
		g_iIndex++;
		iCount++;
	}
	fclose(fp);
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("加载成功!  \r\n起始PageID=%d, 共加载%d",iIndex,iCount);
	else
		str.Format("Load Template success!  \r\nStart PageID=%d, total load %d ",iIndex,iCount);
	pMyDlg->J_SetMsg1(str);

END:
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	pMyDlg->J_EnableBtn();
	return 0;
}
void CFGDemoDlg::OnButton24() //载入指纹库
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}

	CDlgUser dlg;
	dlg.m_nAddr = g_iIndex;
	if(dlg.DoModal()!=IDOK)
		return;
	g_iIndex	= dlg.m_nAddr;//用户地址(索引页)
	CString str;
	if(J_IsExistId(g_iIndex))//已存在，是否覆盖
	{
		if(g_iLanguage==LGN_CHINA)//中文
			str="该地址已经存有用户，是否覆盖?\r\n(特别提示: 该地址为起始地址，其后地址均保持覆盖模式.)";
		else
			str="The address has user! do you want to overwrite?\r\n(Special tips: The address is the starting address,then the follow address are to coverage model.)";
		if ( AfxMessageBox(str, MB_YESNO) != IDYES )
			return;//不覆盖
	}
	g_bStop = FALSE;
	AfxBeginThread( J_gLoadFingerMB,NULL );//载入指纹库线程
}
UINT J_gSaveFingerMB(LPVOID pParam)//保存指纹库
{
	pMyDlg->J_EnableBtn(0,1,0);
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	
	CString sFileName,str;

	if(g_bStop){
		pMyDlg->J_EnableBtn();
		return 1;
	}
	CFileDialog dlg(FALSE,"*.mb","FGTemplet",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"Template file(*.mb)|*.mb||",NULL);
	if(IDOK!=dlg.DoModal()){
		pMyDlg->J_EnableBtn();
		return 1;
	}
	sFileName = dlg.GetPathName();
	FILE* fp;
	BYTE pBuf[CHAR_LEN_AES1711+1]={0};
	int ret,iLen,iWriteNum=0,iIndex=0,iNums=0,i=0;
	iNums = pMyDlg->m_list.GetItemCount();
	if(iNums<=0){
		pMyDlg->J_SetMsg2("指纹库为空!","Fingerprint database is empty!",0);
		pMyDlg->J_EnableBtn();
		return 1;
	}
	if( (fp=fopen(sFileName,"wb"))==NULL ){
		pMyDlg->J_SetMsg2("创建指纹模板库文件 失败! ×","Create Template file failed! × ",0);
		pMyDlg->J_EnableBtn();
		return 1;
	}
	if(g_bStop){
		fclose(fp);
		pMyDlg->J_EnableBtn();
		return 1;
	}

	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return 1;
		}
	}
	pMyDlg->J_SetMsg2("正在保存指纹模板库,请耐心等待...","Saving fingerprint template library, please be patient ...");
	for(i=0; i<iNums; i++)//开始检索LIST列表，做保存操作
	{
		if(g_bStop){
			if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
				PSCloseDeviceEx(hHandle);
			fclose(fp);
			pMyDlg->J_EnableBtn();
			return 1;
		}
		str = pMyDlg->m_list.GetItemText(i,0);
		iIndex = atoi(str);
		ret = PSLoadChar(hHandle,nAddr,CHAR_BUFFER_A,iIndex);
		if(ret!=PS_OK){
			if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
				PSCloseDeviceEx(hHandle);
			fclose(fp);
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			pMyDlg->J_EnableBtn();
			return 1;
		}
		ret = PSUpChar(hHandle,nAddr,CHAR_BUFFER_A,pBuf,&iLen);
		if(ret!=PS_OK){
			if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
				PSCloseDeviceEx(hHandle);
			fclose(fp);
			pMyDlg->J_SetMsg1(PSErr2Str(ret),0);
			pMyDlg->J_EnableBtn();
			return 1;
		}
		iWriteNum += fwrite(pBuf,sizeof(BYTE),iLen,fp);
	}
	fclose(fp);
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("保存成功!  共保存模板%d个.\r\n文件路径: %s\r\n文件大小: %dbytes",iNums,sFileName,iWriteNum);
	else
		str.Format("Save Template success!  Total Save: %d.\r\nFile Path: %s\r\nFile Size: %dbytes",iNums,sFileName,iWriteNum);
	pMyDlg->J_SetMsg2("保存成功!","Save Template success!");
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	AfxMessageBox(str);
	pMyDlg->J_EnableBtn();
	return 0;
}
void CFGDemoDlg::OnButton25() //保存指纹库
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	int iNums = m_list.GetItemCount();
	if(iNums<=0){
		J_SetMsg2("指纹库为空!","Fingerprint database is empty!",0);
		return;
	}
	g_bStop = FALSE;
	AfxBeginThread( J_gSaveFingerMB,NULL );//保存指纹库线程
}

void CFGDemoDlg::OnButton26() //保存单个指纹
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	if(g_iSelect<0){
		J_SetMsg2("先请选定要保存的模板! ×","Please select the template first. ",0);
		return;
	}
	//开始保存
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	
	CString sFileName,str,sTmp;
	CFileDialog dlg(FALSE,"*.dat","FGChar",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,"Character file(*.dat)|*.dat||",NULL);
	if(IDOK!=dlg.DoModal())
		return;
	sFileName = dlg.GetPathName();
	FILE* fp;
	BYTE pBuf[CHAR_LEN_AES1711+1]={0};
	int ret,iLen,iWriteNum=0;
	if( (fp=fopen(sFileName,"wb"))==NULL ){
		J_SetMsg2("创建指纹特征文件 失败! ×","Create Character file failed! × ",0);
		return;
	}

	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return ;
		}
	}

	ret = PSLoadChar(hHandle,nAddr,CHAR_BUFFER_A,g_iSelect);
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		fclose(fp);
		J_SetMsg1(PSErr2Str(ret),0);
		return;
	}
	ret = PSUpChar(hHandle,nAddr,CHAR_BUFFER_A,pBuf,&iLen);
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		fclose(fp);
		J_SetMsg1(PSErr2Str(ret),0);
		return;
	}
	iWriteNum += fwrite(pBuf,sizeof(BYTE),iLen,fp);
	fclose(fp);
	if(g_iLanguage==LGN_CHINA){//中文
		str.Format("保存特征成功! (FingerID=%d) \r\n文件路径: %s\r\n文件大小: %dbytes",g_iSelect,sFileName,iWriteNum);
		sTmp.Format("保存特征成功! \r\n(FingerID=%d)",g_iSelect);
	}
	else{
		str.Format("Save Character success! (FingerID=%d) \r\nFile Path: %s\r\nFile Size: %dbytes",g_iSelect,sFileName,iWriteNum);
		sTmp.Format("Save Character success! \r\n(FingerID=%d)",g_iSelect);
	}
//	J_SetMsg2("保存成功!","Save Character success!");
	J_SetMsg1(sTmp);
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	AfxMessageBox(str);
}

void CFGDemoDlg::OnButton18() //读有效模板个数
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int ret,iMbNum=0;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return ;
		}
	}

	CString str;
	ret = PSTemplateNum(hHandle,nAddr,&iMbNum);
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg2("读有效模板个数失败! ×","Fail to read valid template number! ×",0);
		//J_SetMsg1(PSErr2Str(ret),0);
		return;
	}
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("有效模板个数为 %d 个!",iMbNum);
	else
		str.Format("Valid template number: %d",iMbNum);
	J_SetMsg1(str);
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
}

void CFGDemoDlg::OnButton16() //获取随机数
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int ret,i;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return ;
		}
	}

	CString str,sTmp;
	BYTE iRandom[4]={0};
	ret = PSGetRandomData(hHandle,nAddr,iRandom);
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg2("获取随机数失败! ×","Fail to get random data! ×",0);
		//J_SetMsg1(PSErr2Str(ret),0);
		return;
	}
	if(g_iLanguage==LGN_CHINA)//中文
		str = "获取到的随机数为: ";
	else
		str = "The random data is: ";
	for(i=0; i<4; i++){
		sTmp.Format("%02X",iRandom[i]);
		str += sTmp;
	}
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	J_SetMsg1(str);
}

void CFGDemoDlg::OnButton14() //读记事本
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int ret;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return ;
		}
	}

	CString sContent;
	CDlgNotepad dlg(hHandle,nAddr,0);
	dlg.DoModal();
	ret = dlg.m_ret;
	if(ret!=PS_OK)
		J_SetMsg2("读记事本操作失败! ×","Read Notepad Failed! ×",0);
	//	J_SetMsg1(PSErr2Str(ret),0);
	else
		J_SetMsg2("读记事本操作成功!","Read Notepad Success!");
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
}

void CFGDemoDlg::OnButton15() //写记事本
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;	
	int ret;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return ;
		}
	}

	CString sContent;
	CDlgNotepad dlg(hHandle,nAddr,1);
	dlg.DoModal();
	ret = dlg.m_ret;
	if(ret!=PS_OK)
		J_SetMsg2("写记事本操作失败! ×","Write Notepad Failed! ×",0);
	//	J_SetMsg1(PSErr2Str(ret),0);
	else
		J_SetMsg2("写记事本操作成功!","Write Notepad Success!");
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
}

void CFGDemoDlg::OnButton19() //演示图像路径
{
	CString sPath;
	CFileDialog FileDlg(TRUE,"*.bmp","",OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"Bmp File (*.bmp)|*.bmp|",NULL); 
	if (FileDlg.DoModal()!=IDOK)
		return;
	sPath = FileDlg.GetPathName();
	if (sPath == "C:\\Finger.bmp"){
		J_AfxMsgBox("该图片被程序使用，请选择别的图片","The image is useing,please select another image!");
		return;
	}
    SetDlgItemText(IDC_ET_BmpPath,sPath);
	ShowImage((char*)(LPCTSTR)sPath);
}
void CFGDemoDlg::J_AfxMsgBox(CString str_CH, CString str_EN)
{
	if(g_iLanguage==LGN_CHINA)//中文
		AfxMessageBox(str_CH);
	else
		AfxMessageBox(str_EN);
}
void CFGDemoDlg::J_ShowDemoImage(int iMode/*=-1*/)// -1 原始(默认), 0 二值化图, 1 细化图, 2 特征点细化图  
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	CString sPath("");
	GetDlgItemText(IDC_ET_BmpPath,sPath);
	if (sPath.IsEmpty())
	{
		J_AfxMsgBox("请先选择图片","Please select the image");
		return;
	}
	if(iMode<0 || iMode>2){//原始图像(默认)
		ShowImage((char*)(LPCTSTR)sPath);
		return;
	}
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int ret;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return ;
		}
	}

	J_EnableBtn(0,0,0);	
	ret= PSGetImgDataFromBMP(hHandle,(char*)(LPCTSTR)sPath,gImageData,&g_IamgeLen);	//图像数据装载到内存
    if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		J_EnableBtn();
		return ;		
	}
	J_SetMsg2("正在下载图像...","Downloading...");	
	ret = PSDownImage(hHandle,nAddr,gImageData,g_IamgeLen);  //下载图象
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		J_EnableBtn();
		return ;		
	}
	
	//图像处理
	ret = PSGenBinImage(hHandle,nAddr,iMode);
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		J_EnableBtn();
		return ;		
	}
	J_SetMsg2("生成图像成功，正在上传...","Success to get image,Transfering...");
	ret = PSUpImage(hHandle,nAddr,gImageData,&g_IamgeLen);  //上传图像
	pMyDlg->J_bImgPosN(gImageData,g_bImgpos);//######图象正反矫正
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		J_EnableBtn();
		return ;	
	}
	ret= PSImgData2BMP(gImageData,gszIMGFILE);//IMG_FILE);
	if(ret!=PS_OK){
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		J_EnableBtn();
		return;
	}
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	J_SetMsg2("处理完成！","Complete!");
	ShowImage(gszIMGFILE);//IMG_FILE);//在界面上显示图像
	J_EnableBtn();
}

void CFGDemoDlg::OnButton20() //原始图像
{
	J_ShowDemoImage();
}

void CFGDemoDlg::OnButton21() //二值化图像
{
	J_ShowDemoImage(0);
}

void CFGDemoDlg::OnButton22() //细化图
{
	J_ShowDemoImage(1);
}

void CFGDemoDlg::OnButton23() //特征点细化图
{
	J_ShowDemoImage(2);
}

void CFGDemoDlg::OnButton17() //采集指纹库
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	int		ret = -1;
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return;
		}
	}

	CDlgFGDB dlg(hHandle,nAddr);
	dlg.DoModal();

	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
}

void CFGDemoDlg::OnSelchangeCombo1() //波特率设置
{
	int nVal,nTmp,ret;
	nTmp = J_GetBaudMul(m_iBaud_x);
	UpdateData();
	nVal = J_GetBaudMul(m_iBaud_x);
	ret = J_WriteReg(4,nVal);
	if(ret!=PS_OK){
		m_iBaud_x = J_GetBaudMul(nTmp,1);
		UpdateData(0);
		J_SetMsg2("设置波特率失败!","Failure of the baud rate settings.",0);
		return;
	}
	CString str;
	int ibef = 9600*nTmp;
	int iCur = 9600*nVal;
	m_iBaud  = J_GetBaudMul(nVal,1);
	UpdateData(0);
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("设置波特率成功!\r\n当前波特率: %d  (%d->%d)",iCur,ibef,iCur);
	else
		str.Format("Baud rate setting success!\r\nCurrent Baudrate: %d  (%d->%d)",iCur,ibef,iCur);
	J_SetMsg1(str);		
}

void CFGDemoDlg::OnSelchangeCombo2() //数据包大小
{
	int nVal,nTmp,ret;
	nTmp = m_iPackSize_x;
	UpdateData();
	nVal = m_iPackSize_x;
	ret = J_WriteReg(6,nVal);
	if(ret!=PS_OK){
		m_iPackSize_x = nTmp;
		UpdateData(0);
		J_SetMsg2("设置数据包大小失败!","Set packet size failure.",0);
		return;
	}
	CString str;
	int ibef = 32*(0x1<<nTmp);
	int iCur = 32*(0x1<<nVal);
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("设置数据包大小成功!\r\n当前数据包大小: %d  (%d->%d)",iCur,ibef,iCur);
	else
		str.Format("Set packet size success!\r\nCurrent packet size: %d  (%d->%d)",iCur,ibef,iCur);
	J_SetMsg1(str);	
}

void CFGDemoDlg::OnSelchangeCombo3() //安全等级
{
	int nVal,nTmp,ret;
	nTmp = m_iSLev_x + 1;
	UpdateData();
	nVal = m_iSLev_x + 1;
	ret = J_WriteReg(5,nVal);
	if(ret!=PS_OK){
		m_iSLev_x = nTmp-1;
		UpdateData(0);
		J_SetMsg2("设置安全等级失败!","Set the security level of failure.",0);
		return;
	}
	CString str;
	if(g_iLanguage==LGN_CHINA)//中文
		str.Format("设置安全等级成功!\r\n当前安全等级: %d  (%d->%d)",nVal,nTmp,nVal);
	else
		str.Format("Set the security level of success!\r\nCurrent secure level: %d  (%d->%d)",nVal,nTmp,nVal);
	J_SetMsg1(str);
}

//iType 4-波特率  5-安全等级  6-包大小
int  CFGDemoDlg::J_WriteReg(int iType,int nVal)
{
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return -1;
	}
	int		ret = -1;
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn(1);
			return -1;
		}
	}

	J_EnableBtn(0,1,0);
	////////////////////
	ret = PSWriteReg(hHandle,nAddr,iType,nVal);
	////////////////////

	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);

	//信息修改
	if(ret==PS_OK){
		CString sDevInfo_CH,sDevInfo_EN;
		int iMbMax;
		switch(iType){
			case 4://波特率
				gCur_DevHandle->iBaud_x = nVal;
				if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
					PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
				break;
			case 5://安全等级
				gCur_DevHandle->iSecureLev_x = nVal;
				break;
			case 6://包大小
				gCur_DevHandle->iPacketSize_x = nVal;
				if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
					PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
				break;
		}
		J_GetDevInfo(hHandle,nAddr,sDevInfo_CH,sDevInfo_EN,&iMbMax);
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		gCur_DevHandle->sMsg_CH = sDevInfo_CH;
		gCur_DevHandle->sMsg_EN = sDevInfo_EN;
		if(g_iLanguage==LGN_CHINA)//中文
			SetDlgItemText(IDC_STDevInfo,sDevInfo_CH);
		else//英文
			SetDlgItemText(IDC_STDevInfo,sDevInfo_EN);
	}

	J_EnableBtn();
	return ret;
}

void CFGDemoDlg::J_SetFocus(UINT nID)
{
	CButton* pBtn = (CButton*)(GetDlgItem(nID));
	pBtn->SetFocus();
}

void CFGDemoDlg::OnButton27() //加载图像并生成特征存储
{//加载图像->生成特征->存储特征
	UpdateData();
	if(!J_GetCurDevHandle(gCur_DevHandle)){
		J_SetMsg2("未选择要操作的设备! ×","Please choose a device to operate. ",0);
		return;
	}
	UINT	nAddr	= gCur_DevHandle->dwAddr;
	HANDLE	hHandle	= gCur_DevHandle->hHandle;
	int		nDevType= gCur_DevHandle->nDevType;
	int		nMBMax	= gCur_DevHandle->iMbMax;//指纹库大小

	int ret;
	CString strFile,sFilter,str;
	if(g_iLanguage==LGN_CHINA)//中文
		sFilter="位图文件(*.bmp)|*.bmp|所有文件(*.*)|*.*||";
	else
		sFilter="BMP File(*.bmp)|*.bmp|All File(*.*)|*.*||";
	CFileDialog fo(TRUE,"bmp",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,sFilter,NULL);
	if(fo.DoModal() != IDOK)
		return ;
	strFile = fo.GetFileName();

	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$
	{
		ret = PSOpenDeviceEx(&hHandle,nDevType,gCur_DevHandle->iCom,gCur_DevHandle->iBaud_x,gCur_DevHandle->iPacketSize_x,gCur_DevHandle->iDevId);
		if(ret!=PS_OK){
			pMyDlg->J_EnableBtn();
			return;
		}
	}
	ret= PSGetImgDataFromBMP(hHandle,(char*)(LPCTSTR)strFile,gImageData,&g_IamgeLen);	//图像数据装载到内存
    if(ret!=PS_OK)
	{
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		return ;		
	}
	if(g_iLanguage==LGN_CHINA)//中文
		J_SetMsg1("正在下载图像...");
	else
		J_SetMsg1("Downloading...");
	ret = PSDownImage(hHandle,nAddr,gImageData,g_IamgeLen);  //下载图象
	if(ret!=PS_OK)
	{
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		return ;		
	}

	//显示图像
	ret = PSImgData2BMP(gImageData,gszIMGFILE);//IMG_FILE);
	ShowImage(gszIMGFILE);//IMG_FILE);
	//ShowImage((char*)(LPCTSTR)strFile);

	//生成特征
	ret = PSGenChar(hHandle,nAddr,1);
	if(ret!=PS_OK)
	{
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		return ;		
	}

	//存储特征
	CDlgUser dlg;
	dlg.m_nAddr = g_iIndex;
	if(dlg.DoModal()!=IDOK)
		goto END;
	g_iIndex	= dlg.m_nAddr;//用户地址(索引页)
	g_bReplace	= FALSE;
	if(J_IsExistId(g_iIndex))//已存在，是否覆盖
	{
		if(m_bReplaceMB)
			g_bReplace = TRUE;
		else{
			if(g_iLanguage==LGN_CHINA)//中文
				str="该地址已经存有用户，是否覆盖?";
			else
				str="The address has user! do you want to overwrite?";
			if ( AfxMessageBox(str, MB_YESNO) != IDYES )
				goto END;//不覆盖
			g_bReplace = TRUE;
		}
	}
	ret = PSStoreChar(hHandle,nAddr,1,g_iIndex);
	if(ret!=PS_OK)
	{
		if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
			PSCloseDeviceEx(hHandle);
		J_SetMsg1(PSErr2Str(ret),0);
		return ;		
	}
	
	//成功
	if(!g_bReplace)//添加模式
	{
		str.Format("%d",g_iIndex);
		pMyDlg->m_list.InsertItem(g_iIndex,str);
		//如果是老版的程序，则不支持最新读模板索引功能，所以沿用从文件加载的老方法////////20090106///
		if( nMBMax==120 || nMBMax==376 || nMBMax==888 )
		{
			FILE* fp;
			if( (fp=fopen("FGTemplate.dat","ab+"))!=NULL )
			{
				BYTE pIdx[2]={0};//2Byte表示一个索引页信息
				pIdx[0] = (BYTE)((g_iIndex&0xFF00)>>8);
				pIdx[1] = (BYTE)(g_iIndex&0x00FF);
				fseek(fp,0,SEEK_END);//SEEK_END = 2
				fwrite(pIdx,sizeof(BYTE),2,fp);
				fclose(fp);
			}
		}
		/////////////////////////////////////////////////////////////////////////////////////////////
	}
	g_iIndex++;

	if(g_iLanguage==LGN_CHINA)//中文
		J_SetMsg1("下载图像成功！");	
	else
		J_SetMsg1("Success to download!");	

END:
	if(nDevType==DEVICE_COM)//串口需作特殊处理$$$$$$$$$$$$$$$$$$
		PSCloseDeviceEx(hHandle);
	return ;	
}

void CFGDemoDlg::OnButton28() 
{//URU图像转换为FPC的图像格式,225X313-->256X288
	CString strFile1,strFile2,sFilter,str;
	if(g_iLanguage==LGN_CHINA)//中文
		sFilter="位图文件(*.bmp)|*.bmp|所有文件(*.*)|*.*||";
	else
		sFilter="BMP File(*.bmp)|*.bmp|All File(*.*)|*.*||";
	CFileDialog fo(TRUE,"bmp",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,sFilter,NULL);
	if(fo.DoModal() != IDOK)
		return ;
	strFile1 = fo.GetFileName();//获取图像路径
	strFile2 = "FPC_"+strFile1;

	int ret,Img_x=0,Img_y=0;
	ret = J_GetBMPSize(strFile1,&Img_x,&Img_y);	//分析图像尺寸
	if(ret){
		AfxMessageBox("Err：加载图像错误! .1");
		return;
	}
	if(Img_x!=225 || Img_y!=313){
		str.Format("Err：图像尺寸错误(不是225*313) .1\r\n\r\n此图像尺寸为:%d * %d",Img_x,Img_y);
		AfxMessageBox(str);
		return;
	}

	BYTE pImg[256*288]={0};
	ret = J_GetBmpDataFromFile(strFile1,pImg,&Img_x,&Img_y);//加载图像1数据
	if(ret){
		AfxMessageBox("Err：加载图像数据错误! .1");
		return;
	}
//	J_SaveBmpDataToFileEx8(strFile2,pImg,225,313);
//	return;

	ImageChangeURUtoFPC(pImg);//转换

	CFileDialog dlg(FALSE,"bmp",strFile2,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,sFilter,NULL);
	if(dlg.DoModal() != IDOK)
		return ;
	strFile2 = dlg.GetFileName();
	ret = J_SaveBmpDataToFileEx8(strFile2,pImg,256,288);

	AfxMessageBox("转换成功!");
}


////////////////////////////////////////////////////////////////////////////////////////////
//从BMP文件获取图像数据
int CFGDemoDlg::J_GetBmpDataFromFile(const LPCTSTR lpFileName,BYTE* pBuf,int *iWidth,int *iHeigh)
{
	FILE* fp;
	int iw,ih,i,j,iPixel,iCompression,iOff,iRes=0;
	BYTE pHead[54]={0},pTmp[5]={0};
	BYTE *pData=NULL;
	if( (fp=fopen(lpFileName,"rb+"))==NULL )
		return -1;
	fread(pHead,sizeof(BYTE),54,fp);
	iw = pHead[18]+(pHead[19]<<8)+(pHead[20]<<16)+(pHead[21]<<24);	*iWidth = iw;
	ih = pHead[22]+(pHead[23]<<8)+(pHead[24]<<16)+(pHead[25]<<24);	*iHeigh = ih;
	iPixel = pHead[28]+(pHead[29]<<8);
	iCompression = pHead[30]+(pHead[31]<<8)+(pHead[32]<<16)+(pHead[33]<<24);
//	if(iw!=152 || ih!=200){
//		fclose(fp);
//		return -1;
//	}
	if(iPixel==8){//8 - 256 色位图 
		fseek(fp,1078*sizeof(BYTE),SEEK_SET); //设置文件指针
		for(i=0; i<ih; i++){
			fread(pBuf+iw*(ih-i-1),sizeof(BYTE),iw,fp);
			if(iw%4)//有填充
				fread(pTmp,sizeof(BYTE),4-iw%4,fp);
		}
		//fread(pBuf,sizeof(BYTE),iw*ih,fp);
		iRes = 0;
	}
	else if(iPixel==16){//16 - 16bit 高彩色位图 
		iRes =  -1;
	}
	else if(iPixel==24){//24 - 24bit 真彩色位图 
		iRes = -1;
	}
	else if(iPixel==32 && iCompression==BI_RGB)//32 - 32bit 增强型真彩色位图
	{//当biCompression成员的值是BI_RGB时，它也没有调色板，32位中有24位用于存放RGB值，顺序是：最高位—保留，红8位、绿8位、蓝8位。
		pData = new BYTE[iw*ih*4];
		fseek(fp,54*sizeof(BYTE),SEEK_SET);
		fread(pData,sizeof(BYTE),iw*ih*4,fp);
		for(i=0; i<ih; i++)
		{
			for(j=0; j<iw; j++)
			{
				iOff = i*iw*4+j*4;
				pBuf[i*iw+j] = (pData[iOff+0]+pData[iOff+1]+pData[iOff+2])/3;
			}
		}
		delete pData;
		pData = NULL;
		iRes = 0;
	}
	else
		iRes = -1;

	fclose(fp);
	return 0;
}
int CFGDemoDlg::J_GetBMPSize(CString file,int* image_x,int* image_y)
{
  	CFile fp;
	int succ = fp.Open(file,CFile::modeRead);
	if(!succ)
		return -1;
	unsigned char head[54];
	fp.Read(head,54*sizeof(unsigned char));
	int X,Y;
	X=head[18]+(head[19]<<8)+(head[20]<<16)+(head[21]<<24);
	Y=head[22]+(head[23]<<8)+(head[24]<<16)+(head[25]<<24);
	*image_x = X;
	*image_y = Y;
	 
	fp.Close();
	return 0;
}
//BMP图象数据保存为Bmp文件_[8_256*288]
int CFGDemoDlg::J_SaveBmpDataToFileEx8(const LPCTSTR lpFileName,BYTE* pBuf,int iWidth,int iHeight)
{//					   0	1	 2	  3	   4	5    6    7    8    9	 a	  b	   c	d    e    f
	BYTE pHead[54+1024] = {0x42,0x4D,		//BM
		0x36,0x24,0x01,0x00,	//用字节表示的整个文件的大小 256*288+1078 = 74806 = 0x12436
		0x00,0x00,0x00,0x00,	//保留，必须设置为0
		0x36,0x04,0x00,0x00,	//从文件开始到位图数据开始之间的数据(bitmap data)之间的偏移量 54+1024 = 1078 = 0x436
		0x28,0x00,0x00,0x00,	//??
		0x00,0x01,0x00,0x00,	//位图的宽度	256 = 0x100
		0x20,0x01,0x00,0x00,	//位图的高度	288 = 0x120
		0x01,0x00,	//1
		0x08,0x00,	//1-单色  4-16色  8-256色  16-16bit  24-24bit  32-32bit	
		0x00,0x00,0x00,0x00,	//0-不压缩
		0x00,0x20,0x01,0x00,	//位图数据大小 4的倍数	256*288 = 73728 = 0x12000
		0x00,0x00,0x00,0x00,	//用象素/米表示的水平分辨率
		0x00,0x00,0x00,0x00,	//用象素/米表示的垂直分辨率
		0x00,0x00,0x00,0x00,	//位图使用的颜色数。如8-比特/象素表示为100h或者 256.
		0x00,0x00,0x00,0x00};//指定重要的颜色数。当该域的值等于颜色数时（或者等于0时），表示所有颜色都一样重要												 //03H
	FILE* fp;
	int i;
	DWORD dwBMSize=0;
	dwBMSize = (((iWidth * 8+31)/32) * 4) * iHeight;
	if( (fp=fopen(lpFileName,"wb+"))==NULL )
		return -1;
	if(iWidth!=256 || iHeight!=288)//调整pHead数据
	{
		DWORD dw = dwBMSize + 0x436;//File Size 用字节表示的整个文件的大小
		pHead[2] = (BYTE)(dw&0xFF);
		pHead[3] = (BYTE)((dw>>8)&0xFF);
		pHead[4] = (BYTE)((dw>>16)&0xFF);
		pHead[5] = (BYTE)((dw>>24)&0xFF);
		dw = iWidth;
		pHead[18] = (BYTE)(dw&0xFF);
		pHead[19] = (BYTE)((dw>>8)&0xFF);
		pHead[20] = (BYTE)((dw>>16)&0xFF);
		pHead[21] = (BYTE)((dw>>24)&0xFF);
		dw = iHeight;
		pHead[22] = (BYTE)(dw&0xFF);
		pHead[23] = (BYTE)((dw>>8)&0xFF);
		pHead[24] = (BYTE)((dw>>16)&0xFF);
		pHead[25] = (BYTE)((dw>>24)&0xFF);
		dw = dwBMSize;//iWidth*iHeight;	//Bitmap Data Size 用字节数表示的位图数据的大小。该数必须是4的倍数
		pHead[34] = (BYTE)(dw&0xFF);
		pHead[35] = (BYTE)((dw>>8)&0xFF);
		pHead[36] = (BYTE)((dw>>16)&0xFF);
		pHead[37] = (BYTE)((dw>>24)&0xFF);
	}
	for(i=0; i<256; i++)//写调色板
	{
		pHead[54+4*i+0] = i;
		pHead[54+4*i+1] = i;
		pHead[54+4*i+2] = i;
		pHead[54+4*i+3] = 0;
	}
	
	BYTE pTmp[5]={0xFF,0xFF,0xFF,0xFF,0xFF};
	fwrite(pHead,sizeof(BYTE),54+1024,fp);//写头
	for(i=iHeight-1; i>=0; i--)//写图像数据
	{
		fwrite(pBuf+i*iWidth,sizeof(BYTE),iWidth,fp);
		if(iWidth%4)//不能被4整除，需填充
			fwrite(pTmp,sizeof(BYTE),4-(iWidth%4),fp);
	}
	
	fclose(fp);
	return 0;
}

//---------------------------------------------------------------------
//获取文件名及路径
//输入：iMode 0-文件名及路径	1-文件名	2-路径
//输出：pstr[MAX_PATH]
int WINAPI J_GetPathName(char* pstr,int iMode)
{
	if(pstr==NULL || iMode<0 || iMode>2 )
		return -1;
	char pBuf[MAX_PATH] = {0};
	char *pName;
	int iLen=0,iTmp=0;
	GetModuleFileName(NULL,(LPTSTR)pBuf,MAX_PATH);
	if(0==iMode){
		iLen = strlen(pBuf);
		memcpy(pstr,pBuf,iLen+1);//0-文件名及路径
		return 0;
	}
	pName = strrchr(pBuf,(int)('\\'));
	iLen = strlen(pName);
	if(1==iMode){//1-文件名
		memcpy(pstr,(pName+1),iLen);
		return 0;
	}
	if(2==iMode){//2-路径
		iTmp = iLen;
		iLen = strlen(pBuf);
		memcpy(pstr,pBuf,iLen-iTmp);
		return 0;
	}
	return -1;
}

void CFGDemoDlg::J_SetIMGFILE(int type)//设置默认临时文件路径
{
	TCHAR ps[MAX_PATH]={0};
	memset(gszIMGFILE,0,MAX_PATH);
	switch(type){
		case 0://默认路径
			sprintf(gszIMGFILE,"%s",IMG_FILE);
			break;

		case 1://当前文件夹路径
			J_GetPathName(ps,2);
			sprintf(gszIMGFILE,"%s\\Finger.bmp",ps);
			break;

		default:
			sprintf(gszIMGFILE,"%s",IMG_FILE);
			break;
	}
	
	return;
}

void CFGDemoDlg::OnSelchangeCombo4() 
{
	// TODO: 切换传感器类型
	UpdateData();
	g_bEnrollIMG = FALSE;
	switch(m_iSensorType){
		case 0: // Normal Secsor (通用类型 BufID：1，2 512) 2次手指
			g_nEN_NUM = 2;
			g_nMaxTry = 2;
			PSSetCharLen(CHAR_LEN_NORMAL);
			break;
		case 1: // AES1711 (按4次手指 BufID：1，2，3，4 1024)
			g_nEN_NUM = 4;
			g_nMaxTry = 4;
			PSSetCharLen(CHAR_LEN_AES1711);
			break;
		case 2:	// TYPE1 (GC0307 0x29命令)
			g_bEnrollIMG = TRUE;//x029
			g_nEN_NUM = 2;
			g_nMaxTry = 2;
			PSSetCharLen(CHAR_LEN_NORMAL);
			break;
		case 3: // TYPE2 (AES1711 3次：需3~6次手指 1024)
			g_nEN_NUM = 3;
			g_nMaxTry = 3*2;
			PSSetCharLen(CHAR_LEN_AES1711);
			break;
		default:
			g_nEN_NUM = 2;
			g_nMaxTry = 2;
			PSSetCharLen(CHAR_LEN_NORMAL);
			break;
	}
	SaveUserInfo();
}
