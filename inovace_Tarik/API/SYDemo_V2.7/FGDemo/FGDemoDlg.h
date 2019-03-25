// FGDemoDlg.h : header file
//

#if !defined(AFX_FGDEMODLG_H__E502968B_DE7D_4FDD_A4BF_254476CC1AE8__INCLUDED_)
#define AFX_FGDEMODLG_H__E502968B_DE7D_4FDD_A4BF_254476CC1AE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CFGDemoDlg dialog
#include "ToolEx\\XInfoTip.h"
#include "ToolEx\\J_EditEx.h"
#include "ToolEx\\ShadeButtonST.h"

//数据结构
typedef struct _JsDevHANDLE
{
	HANDLE hHandle;	//设备句柄
	int nDevType;	//设备类型	USB-0	COM-1	UDisk-2
	int iCom;		//串口号
	int iDevId;		//设备Id
	DWORD dwPwd;	//设备密码	0x00000000
	DWORD dwAddr;	//设备地址	0xFFFFFFFF
	CString	sDevName;	//设备信息	USB_No.0	Dev_COM1	UDisk_No.0
	CString sMsg_CH;	//硬件信息_CH	中文
	CString sMsg_EN;	//硬件信息_EN	英文
	int iBaud_x;		//波特率		$ 0 - 9600  1 - 19200   2 - 38400   3 - 57600   4 - 115200
	int iPacketSize_x;	//数据包大小	$ 0 - 32    1 - 64      2 - 128     3 - 256
	int iSecureLev_x;	//安全等级		$ 0 - 1     1 - 2       2 - 3       3 - 4       4 - 5
	int iMbMax;	//指纹库大小
	_JsDevHANDLE *next;//指向下一个的指针
//	int nPackSize;
}JsDevHandle,*pJsDevHandle;

#define LGN_CHINA	0x0804	//语言 中文(简体)
#define LGN_EN		0x0000	//语言 英文
/////////////////////////////////////////////
class CFGDemoDlg : public CDialog
{
// Construction
public:
	CFGDemoDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CFGDemoDlg();

// Dialog Data
	//{{AFX_DATA(CFGDemoDlg)
	enum { IDD = IDD_FGDEMO_DIALOG };
	CComboBox	m_cbSensorType;
	CComboBox	m_ctlDevHandle;
	CListCtrl	m_list;
	CJ_EditEx	m_STctlAddr;
	CJ_EditEx	m_STctlPwd;
	CStatic	m_bmp;
	CString	m_strAddr;
	CString	m_strPwd;
	int		m_iDevType;
	int		m_iCom;
	int		m_iBaud;
	int		m_iDevHandle;
	int		m_iLG;
	BOOL	m_bShowBMP;
	BOOL	m_bReplaceMB;
	int		m_iBaud_x;
	int		m_iPackSize_x;
	int		m_iSLev_x;
	int		m_iDevID;
	int		m_iSensorType;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFGDemoDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
////////////////////////////////////////////////////////////////
private:
	CXInfoTip m_tooltip;
	CShadeButtonST m_sbtn1;
	CShadeButtonST m_sbtn2;
	CShadeButtonST m_sbtn3;
	CShadeButtonST m_sbtn4;
	CShadeButtonST m_sbtn5;
protected:
	BOOL m_bSpBtn;		//是否显示特殊功能按钮
	BOOL m_bDevOpen;	//判断设备是否打开
	BOOL m_blShowTip;	//是否显示ToolTip
	CFont m_font;	//用于sMsg的字体
	BOOL m_bBlue;	//用于sMsg的颜色 TRUE-BLUE FALSE=RED
	BOOL m_bRefBMP; //用于刷新显示图像
	
public:
	void SaveUserInfo();
	void LoadUserDefine(HANDLE hl=NULL,BOOL bInit=FALSE);//配置文件
	void J_MyBtn(CShadeButtonST* msbtn,UINT nID,UINT iIcon=0,UINT shadeID=CShadeButtonST::SHS_METAL,COLORREF rgb=RGB(50,50,50));
	void J_EnableBtn(BOOL bEnable=1,BOOL bEcl=1,BOOL bOpen=TRUE);
	void J_EnableSet(BOOL bEnable=1,int nType=DEVICE_USB);//设备设置
	void J_SetLanuage(int iLanguage=LGN_CHINA);//设置语言 中文/英文
	int  J_GetDevInfo(HANDLE hHandle,int nAddr, CString& str_CH, CString& str_EN,int* iMbMax);//获取设备硬件信息
	int  J_GetBaudMul(int imbo=0,int way=0);//获取波特率倍数N,基于9600
	BOOL J_CheckDev(CString sDevInfo);//判断设备是否早已打开，用于避免同一设备的重复打开
	BOOL J_GetDevHandle(pJsDevHandle& pDevHandle,CString sDevName);//获取设备句柄
	BOOL J_GetCurDevHandle(pJsDevHandle& pDevHandle);//获取当前有效设备句柄
	BOOL J_CloseDev(CString sDevName);//关闭设备
	void J_SetMsg1(CString str,BOOL bBlue=TRUE);
	void J_SetMsg2(CString str_CH,CString str_EN,BOOL bBlue=TRUE);//自动选择语言
	void ShowDefaltImg();
	void ShowImage(char *filename=NULL); //显示图像
	void J_LoadInfo2List();//加载指纹索引到List列表
	BOOL J_IsExistId(int iIndex);
	void J_AfxMsgBox(CString str_CH, CString str_EN);
	void J_ShowDemoImage(int iMode=-1);// -1 原始(默认), 0 二值化图, 1 细化图, 2 特征点细化图 
	int  J_WriteReg(int iType,int nVal);
	void J_bImgPosN(BYTE *pImg=NULL, BOOL bImgPN=0);//图象正反矫正
	void J_SetFocus(UINT nID);

	void J_SetIMGFILE(int type=0);//设置默认临时文件路径

	
	int J_GetBmpDataFromFile(const LPCTSTR lpFileName,BYTE* pBuf,int *iWidth,int *iHeigh);//从BMP文件获取图像数据
	int J_GetBMPSize(CString file,int* image_x,int* image_y);//读图像尺寸
	int J_SaveBmpDataToFileEx8(const LPCTSTR lpFileName,BYTE* pBuf,int iWidth,int iHeight);//BMP图象数据保存为Bmp文件_
////////////////////////////////////////////////////////////////
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CFGDemoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelchangeComboLg();
	virtual void OnOK();
	afx_msg void OnSelchangeCOMBODev();
	afx_msg void OnCloseDev();
	afx_msg void OnCloseDev2();
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnButton4();
	afx_msg void OnButton5();
	afx_msg void OnButton6();
	afx_msg void OnButton7();
	afx_msg void OnButton8();
	afx_msg void OnClickList1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButton9();
	afx_msg void OnButton10();
	afx_msg void OnButton11();
	afx_msg void OnButton12();
	afx_msg void OnButton13();
	afx_msg void OnButton24();
	afx_msg void OnButton25();
	afx_msg void OnButton26();
	afx_msg void OnButton18();
	afx_msg void OnButton16();
	afx_msg void OnButton14();
	afx_msg void OnButton15();
	afx_msg void OnButton19();
	afx_msg void OnButton20();
	afx_msg void OnButton21();
	afx_msg void OnButton22();
	afx_msg void OnButton23();
	afx_msg void OnButton17();
	afx_msg void OnSelchangeCombo1();
	afx_msg void OnSelchangeCombo2();
	afx_msg void OnSelchangeCombo3();
	afx_msg void OnButton27();
	afx_msg void OnButton28();
	afx_msg void OnSelchangeCombo4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FGDEMODLG_H__E502968B_DE7D_4FDD_A4BF_254476CC1AE8__INCLUDED_)
