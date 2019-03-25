
#ifndef _XPOPUPTIP_H_INCLUDE_
#define _XPOPUPTIP_H_INCLUDE_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include "Afxtempl.h"
class CXInfoTip : public CWnd
{
protected:
	///////////////////////////////////////////////////////////////////////////
	// Tool information structure
	///////////////////////////////////////////////////////////////////////////
	typedef struct
	{
		CString	szText;											// Tooltip text
		HICON	hIcon;											// Tooltip icon
	} TipToolInfo;

	// Timer identifiers
	enum
	{
		timerShow			= 100,								// Show timer
		timerHide			= 101								// Hide timer
	};

	LPCTSTR		m_szClass;										// Window class

	int			m_nShowDelay;									// Show delay

	CPoint		m_ptOrigin;										// Popup origin

	CString		m_szText;										// Tip text

	UINT		m_nTimer;										// Show/hide timer

	HICON		m_hIcon;										// Tip icon
	CSize		m_IconSize;										// Tip icon size

	CFont		*m_pFont;										// Tip font

	CMap<HWND, HWND, TipToolInfo, TipToolInfo>	m_ToolMap;		// Tools map

public:
	CXInfoTip();
	virtual ~CXInfoTip();

	BOOL Create(CWnd *parent);

	void AddTool(CWnd *pWnd, LPCTSTR szTooltipText, HICON hIcon = NULL);
	void RemoveTool(CWnd *pWnd);

	void Show(CString szText, CPoint *pt = NULL);
	void Hide() { ShowWindow(SW_HIDE); };

	// Sets the delay for the tooltip
	void SetShowDelay(int nDelay) { m_nShowDelay = nDelay; };

	void SetIcon(HICON hIcon);

	// Sets the tooltip font
	void SetFont(CFont *pFont) 
	{ 
		m_pFont = pFont; 
		if (IsWindow(m_hWnd))
			RedrawWindow();
	};

	void RelayEvent(LPMSG lpMsg);

protected:
	BOOL GetWindowRegion(CDC *pDC, HRGN* hRegion, CSize* Size = NULL);


protected:
	//{{AFX_MSG(CXInfoTip)
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif 
