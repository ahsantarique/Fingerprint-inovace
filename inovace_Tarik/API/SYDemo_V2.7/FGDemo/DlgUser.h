#if !defined(AFX_DLGUSER_H__D7E5FD64_26B5_4101_B00E_FC1CDE455825__INCLUDED_)
#define AFX_DLGUSER_H__D7E5FD64_26B5_4101_B00E_FC1CDE455825__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgUser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgUser dialog

class CDlgUser : public CDialog
{
// Construction
public:
	CDlgUser(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgUser)
	enum { IDD = IDD_DLG_USER };
	UINT	m_nAddr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgUser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgUser)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGUSER_H__D7E5FD64_26B5_4101_B00E_FC1CDE455825__INCLUDED_)
