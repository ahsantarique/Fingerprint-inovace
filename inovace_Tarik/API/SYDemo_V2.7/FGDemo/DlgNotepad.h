#if !defined(AFX_DLGNOTEPAD_H__BAEF48F0_CC28_4CB5_9E93_C9BFA9D9E1CC__INCLUDED_)
#define AFX_DLGNOTEPAD_H__BAEF48F0_CC28_4CB5_9E93_C9BFA9D9E1CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgNotepad.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgNotepad dialog

class CDlgNotepad : public CDialog
{
// Construction
public:
//	CDlgNotepad(CWnd* pParent = NULL);   // standard constructor
	CDlgNotepad(HANDLE hHandle,UINT nAddr=0xFFFFFFFF,int iMode=0,CWnd* pParent = NULL);   //iMode=0[读]	1[写]

// Dialog Data
	//{{AFX_DATA(CDlgNotepad)
	enum { IDD = IDD_DIGNotepad };
	UINT	m_uPage;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgNotepad)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
//////////////////////////////////////////////
public:
	HANDLE	m_hHandle;
	UINT	m_nAddr;
	int		m_iMode;//读0/写1 记事本
	int		m_ret;//结果
	CString m_content;//内容

	int J_GetHex2Int(BYTE bt);
	void J_AfxMsgBox(CString str_CH, CString str_EN);
//////////////////////////////////////////////

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgNotepad)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	afx_msg void OnButton1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGNOTEPAD_H__BAEF48F0_CC28_4CB5_9E93_C9BFA9D9E1CC__INCLUDED_)
