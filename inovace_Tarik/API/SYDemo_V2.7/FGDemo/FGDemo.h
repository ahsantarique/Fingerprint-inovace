// FGDemo.h : main header file for the FGDEMO application
//

#if !defined(AFX_FGDEMO_H__0BB8AE61_101D_46F1_9B3F_BFB8FB177BCD__INCLUDED_)
#define AFX_FGDEMO_H__0BB8AE61_101D_46F1_9B3F_BFB8FB177BCD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFGDemoApp:
// See FGDemo.cpp for the implementation of this class
//
#define LGN_CHINA	0x0804	//”Ô—‘ ÷–Œƒ(ºÚÃÂ)
extern int  g_iLanguage;	//”Ô—‘

class CFGDemoApp : public CWinApp
{
public:
	CFGDemoApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFGDemoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CFGDemoApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FGDEMO_H__0BB8AE61_101D_46F1_9B3F_BFB8FB177BCD__INCLUDED_)
