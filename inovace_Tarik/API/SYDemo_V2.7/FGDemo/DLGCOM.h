#if !defined(AFX_DLGCOM_H__D0EAFB7E_D22F_47CC_B8F4_BFE5DFD77743__INCLUDED_)
#define AFX_DLGCOM_H__D0EAFB7E_D22F_47CC_B8F4_BFE5DFD77743__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DLGCOM.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDLGCOM dialog

class CDLGCOM : public CDialog
{
// Construction
public:
	CDLGCOM(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDLGCOM)
	enum { IDD = IDD_DLGCOM };
	int		m_iCom;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDLGCOM)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDLGCOM)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCOM_H__D0EAFB7E_D22F_47CC_B8F4_BFE5DFD77743__INCLUDED_)
