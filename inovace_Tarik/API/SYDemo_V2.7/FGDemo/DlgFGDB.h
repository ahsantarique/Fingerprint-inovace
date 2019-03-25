#if !defined(AFX_DLGFGDB_H__8CCF0FF5_C981_4882_929F_7FE981A8C911__INCLUDED_)
#define AFX_DLGFGDB_H__8CCF0FF5_C981_4882_929F_7FE981A8C911__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgFGDB.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgFGDB dialog

class CDlgFGDB : public CDialog
{
// Construction
public:
	CDlgFGDB(HANDLE hHandle,UINT nAddr=0xFFFFFFFF,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgFGDB)
	enum { IDD = IDD_DIG_FGDB };
	UINT	m_nNumFPin;
	CString	m_FGDBPath;
	CString	m_RunNum;
	CString	m_SinNum;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgFGDB)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
/////////////////////////////////////////////
public:
	HANDLE m_hHandle;
	UINT   m_nAddr;
	//
	int  J_GetPathName(char* pstr,int iMode=2);
	int  J_AfxMsgBox(CString str_CH, CString str_EN, UINT nType = MB_OK, UINT nIDHelp = 0);
	void J_EnableBtn(BOOL bEnable=TRUE);
	int  J_GetImage();
	void ShowDefaltImg();
	void J_ShowImage();
	void J_bImgPosN(BYTE *pImg=NULL, BOOL bImgPN=0);//图象正反矫正
/////////////////////////////////////////////
// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgFGDB)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnButton1();
	afx_msg void OnButton2();
	afx_msg void OnButton3();
	afx_msg void OnButton5();
	afx_msg void OnButton4();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGFGDB_H__8CCF0FF5_C981_4882_929F_7FE981A8C911__INCLUDED_)
