// J_EditEx.cpp : implementation file
//

#include "stdafx.h"
#include "J_EditEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJ_EditEx
IMPLEMENT_DYNAMIC(CJ_EditEx, CEdit)

CJ_EditEx::CJ_EditEx()
{
	m_bltip		  = 1;
	m_bltipByEdit = 1;
	m_strTip	  = "";
	
	m_blDecStr = 0;
	m_blHexStr = 0;
	m_blflag0x = 0;
	m_LenLimitDec = 0;
	m_LenLimitHex = 0;

	m_clrText	  = ::GetSysColor(COLOR_BTNTEXT);//RGB(0,0,0);
	m_clrBk		  = ::GetSysColor(COLOR_BTNFACE);//RGB(255,255,255);
	m_clrBkMode	  = TRANSPARENT;
		
	//ToolTip
	pTip = new CToolTipCtrl;
	if(!pTip->Create(this))
	{
		TRACE("Unable to create ToolTip\n");
		delete pTip;
		pTip = NULL;
		MessageBox("Unable to create ToolTip\n");
	}
	else
	{
		pTip->AddTool(this,"");
		pTip->SetTipBkColor(RGB( 228 , 255 , 215 ));//RGB(237 , 248 , 217)//RGB(242 , 245 , 228)//
		pTip->SetTipTextColor(RGB( 202 , 0 , 38 ));//RGB(91 , 0 , 35)////RGB(0 , 150 , 0)
		pTip->SetDelayTime(100);
		pTip->SetMaxTipWidth(300);
		pTip->Activate(TRUE);
	}
}

CJ_EditEx::~CJ_EditEx()
{
	if (pTip!=NULL)
	{
		delete pTip;
		pTip = NULL;
	}
}


BEGIN_MESSAGE_MAP(CJ_EditEx, CEdit)
	//{{AFX_MSG_MAP(CJ_EditEx)
	ON_WM_MOUSEMOVE()
	ON_WM_CHAR()
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CJ_EditEx message handlers

BOOL CJ_EditEx::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(m_bltip)
	{
		if( pTip!=NULL)
			pTip->RelayEvent(pMsg);
	}

	return CEdit::PreTranslateMessage(pMsg);
}

void CJ_EditEx::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(m_bltip)
	{
		if (m_bltipByEdit)
		{
			GetWindowText(m_strTip);
			//设为智能模式，只有当文本内容超出显示框时，才出现提示条
			//CFont::考虑不同字体时的情况
			CFont *pFont = CEdit::GetFont();
			CClientDC dc(this);
			dc.SelectObject(pFont);
			CSize sz = dc.GetTextExtent(m_strTip);
			CRect rect;
			DWORD dwM = GetMargins();
			GetClientRect(rect);
			if ( ( rect.Width() - (int)(LOWORD(dwM)+HIWORD(dwM)) - 6 ) < sz.cx )
				pTip->AddTool(this,m_strTip);
			else
				pTip->AddTool(this,"");
		}
		else
		{
			pTip->AddTool(this,m_strTip);
		}
	}

	CEdit::OnMouseMove(nFlags, point);
}

HBRUSH CJ_EditEx::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	// TODO: Change any attributes of the DC here
	m_brushBack.Detach();
	m_brushBack.CreateSolidBrush(m_clrBk);

	pDC->SetBkMode(m_clrBkMode);
	pDC->SetBkColor(m_clrBk);
	pDC->SetTextColor(m_clrText);
//	CRect rt;
//	GetClientRect(&rt);
//	pDC->FillRect(CRect(0,0,10,20),&CBrush(RGB(255,0,0)));
	
	// TODO: Return a non-NULL brush if the parent's handler should not be called
//	return NULL;
	return (HBRUSH)m_brushBack.GetSafeHandle();
}

void CJ_EditEx::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	if(nChar==8)//退格键 VK_BACK
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
		return;
	}
	
	int nLen=0;
	int nStartChar,nEndChar;
	GetSel(nStartChar,nEndChar);
	CString str;
	if(m_blHexStr)//16进制
	{
		//先获取输入光标的位置
		POINT caret;
		::GetCaretPos(&caret);
		caret.x = LOWORD(CharFromPos(caret));
		GetWindowText(str);
		if( nChar=='x' || nChar=='X' )
		{
			if( m_blflag0x )
			{
				if( str[0]!='0' || caret.x!=1 )// || (str[1]=='X' || str[1]=='x' ))
					return;
				else if ( str.GetLength()>=2 )
				{
					if( str[1]=='X' || str[1]=='x' )
						return;
				}
			}
			else
				return;
		}
		else if( !((nChar>='0' && nChar<='9') || (nChar>='a' && nChar<='f') || (nChar>='A' && nChar<='F')) )
		{
			return;
		}	
		if( m_LenLimitHex>1 )//________位数判断
		{
			nLen = str.GetLength();
			if( m_blflag0x && nLen>=2)//str[0]=='0' && (str[1]=='X' || str[1]=='x') )
			{
				if( str[0]=='0' && (str[1]=='X' || str[1]=='x') )
					nLen = str.GetLength()-2;
			}
			else
				nLen = str.GetLength();
			nLen = nLen - (nEndChar-nStartChar);
			if(nLen>=m_LenLimitHex )
				return;
		}
	//	if( nChar>='a' && nChar<='f' )
	//		nChar -= ('a'-'A');
	}
	else if(m_blDecStr)//10进制
	{
		if(!isdigit(nChar))
			return;
		else if( m_LenLimitDec>1 )//________位数判断
		{
			GetWindowText(str);
			nLen = str.GetLength() - (nEndChar-nStartChar);;
			if( nLen>=m_LenLimitDec )
				return;
		}
	}
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}
////-------------------------------------------------------------------------------------------------////
CString CJ_EditEx::JSetStrTip(CString strTip)//输入提示信息
{
	CString strRes = m_strTip;
	m_strTip = strTip;
	return strRes;
}

bool CJ_EditEx::JEnableTip(bool bl_tip,bool bl_tipByEdit)//提示信息
{
	bool bl_res;
	bl_res	= m_bltip;
	m_bltip	= bl_tip;
	m_bltipByEdit = bl_tipByEdit;
	return bl_res;
}
bool CJ_EditEx::JSetOnlyDecStr(bool bl_DecStr,int nLengthLimit)//是否指显示10进制数
{
	if(J_IsMutilLine())
		return 0;
	//bool bl_res;
	//bl_res = m_blDecStr;
	m_blDecStr = bl_DecStr;
	m_blHexStr = 0;
	m_blflag0x = 0;
	m_LenLimitDec = nLengthLimit;
	m_LenLimitHex = 0;
	//return bl_res;
	return 1;
}

bool CJ_EditEx::JSetOnlyHexStr(bool bl_HexStr,int nLengthLimit,bool bl_flag0x)//是否指显示16进制数
{
//	if(J_IsMutilLine())
//		return 0;
//	bool bl_res;
//	bl_res = m_blHexStr;
	m_blHexStr = bl_HexStr;
	m_blDecStr = 0;	
	m_blflag0x = bl_flag0x;
	m_LenLimitHex = nLengthLimit;
	m_LenLimitDec = 0;
//	return bl_res;
	return 1;
}


COLORREF CJ_EditEx::JSetColor(COLORREF crText,COLORREF crBk,int iBkMode)//设置显示颜色
{
	COLORREF crRes = m_clrText;
	m_clrText	= crText;
	m_clrBk		= crBk;
	m_clrBkMode = iBkMode;
	return crRes;
}
BOOL CJ_EditEx::J_IsMutilLine()//是否是多行Edit
{
	DWORD dwStyle = GetStyle();
	if( (dwStyle & ES_MULTILINE) )
		return 1;
	return 0;
}

void CJ_EditEx::OnChange() 
{
	RedrawWindow();
}
////------------------------------------------------------------------------------------------////
