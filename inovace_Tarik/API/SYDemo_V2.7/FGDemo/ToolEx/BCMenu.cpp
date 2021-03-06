//*************************************************************************
// BCMenu.cpp : implementation file
// Version : 3.02
// Date : March 2002
// Author : Brent Corkum
// Email :  corkum@rocscience.com
// Latest Version : http://www.rocscience.com/~corkum/BCMenu.html
// 
// Bug Fixes and portions of code supplied by:
//
// Ben Ashley,Girish Bharadwaj,Jean-Edouard Lachand-Robert,
// Robert Edward Caldecott,Kenny Goers,Leonardo Zide,
// Stefan Kuhr,Reiner Jung,Martin Vladic,Kim Yoo Chul,
// Oz Solomonovich,Tongzhe Cui,Stephane Clog,Warren Stevens,
// Damir Valiulin,David Kinder,Marc Loiry
//
// You are free to use/modify this code but leave this header intact.
// This class is public domain so you are free to use it any of
// your applications (Freeware,Shareware,Commercial). All I ask is
// that you let me know so that if you have a real winner I can
// brag to my buddies that some of my code is in your app. I also
// wouldn't mind if you sent me a copy of your application since I
// like to play with new stuff.
//*************************************************************************

#include "stdafx.h"        // Standard windows header file
#include "BCMenu.h"        // BCMenu class declaration
#include <afxpriv.h>       //SK: makes A2W and other spiffy AFX macros work

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BCMENU_GAP 1
#ifndef OBM_CHECK
#define OBM_CHECK 32760 // from winuser.h
#endif

#if _MFC_VER <0x400
#error This code does not work on Versions of MFC prior to 4.0
#endif

static CPINFO CPInfo;
// how the menu's are drawn in win9x/NT/2000
UINT BCMenu::original_drawmode=BCMENU_DRAWMODE_ORIGINAL;
BOOL BCMenu::original_select_disabled=TRUE;
// how the menu's are drawn in winXP
UINT BCMenu::xp_drawmode=BCMENU_DRAWMODE_XP;
BOOL BCMenu::xp_select_disabled=FALSE;
BOOL BCMenu::xp_draw_3D_bitmaps=TRUE;

CImageList BCMenu::m_AllImages;
CArray<int,int&> BCMenu::m_AllImagesID;

enum Win32Type{
	Win32s,
	WinNT3,
	Win95,
	Win98,
	WinME,
	WinNT4,
	Win2000,
	WinXP
};


Win32Type IsShellType()
{
	Win32Type  ShellType;
	DWORD winVer;
	OSVERSIONINFO *osvi;
	
	winVer=GetVersion();
	if(winVer<0x80000000){/*NT */
		ShellType=WinNT3;
		osvi= (OSVERSIONINFO *)malloc(sizeof(OSVERSIONINFO));
		if (osvi!=NULL){
			memset(osvi,0,sizeof(OSVERSIONINFO));
			osvi->dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
			GetVersionEx(osvi);
			if(osvi->dwMajorVersion==4L)ShellType=WinNT4;
			else if(osvi->dwMajorVersion==5L&&osvi->dwMinorVersion==0L)ShellType=Win2000;
			else if(osvi->dwMajorVersion==5L&&osvi->dwMinorVersion==1L)ShellType=WinXP;
			free(osvi);
		}
	}
	else if  (LOBYTE(LOWORD(winVer))<4)
		ShellType=Win32s;
	else{
		ShellType=Win95;
		osvi= (OSVERSIONINFO *)malloc(sizeof(OSVERSIONINFO));
		if (osvi!=NULL){
			memset(osvi,0,sizeof(OSVERSIONINFO));
			osvi->dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
			GetVersionEx(osvi);
			if(osvi->dwMajorVersion==4L&&osvi->dwMinorVersion==10L)ShellType=Win98;
			else if(osvi->dwMajorVersion==4L&&osvi->dwMinorVersion==90L)ShellType=WinME;
			free(osvi);
		}
	}
	return ShellType;
}

static Win32Type g_Shell=IsShellType();

void BCMenuData::SetAnsiString(LPCSTR szAnsiString)
{
	USES_CONVERSION;
	SetWideString(A2W(szAnsiString));  //SK:  see MFC Tech Note 059
}

CString BCMenuData::GetString(void)//returns the menu text in ANSI or UNICODE
//depending on the MFC-Version we are using
{
	CString strText;
	if (m_szMenuText)
    {
#ifdef UNICODE
		strText = m_szMenuText;
#else
		USES_CONVERSION;
		strText=W2A(m_szMenuText);     //SK:  see MFC Tech Note 059
#endif    
    }
	return strText;
}

CTypedPtrArray<CPtrArray, HMENU> BCMenu::m_AllSubMenus;  // Stores list of all sub-menus

IMPLEMENT_DYNAMIC( BCMenu, CMenu )

/*
===============================================================================
BCMenu::BCMenu()
BCMenu::~BCMenu()
-----------------

Constructor and Destructor.

========================    3�  