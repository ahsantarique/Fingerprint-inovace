// DlgNotepad.cpp : implementation file
//

#include "stdafx.h"
#include "FGDemo.h"
#include "DlgNotepad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgNotepad dialog
CDlgNotepad::CDlgNotepad(HANDLE hHandle,UINT nAddr/*=0xFFFFFFFF*/,int iMode/*=0*/,CWnd* pParent /*=NULL*/)//iMode=0[读]	1[写]
	: CDialog(CDlgNotepad::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgNotepad)
	m_uPage = 0;
	//}}AFX_DATA_INIT
	m_hHandle	= hHandle;
	m_nAddr		= nAddr;
	m_iMode		= iMode;
	m_ret		= -1;
	m_content	= "";
}


void CDlgNotepad::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgNotepad)
	DDX_Text(pDX, IDC_EDIT1, m_uPage);
	DDV_MinMaxUInt(pDX, m_uPage, 0, 15);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgNotepad, CDialog)
	//{{AFX_MSG_MAP(CDlgNotepad)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgNotepad message handlers

BOOL CDlgNotepad::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if(g_iLanguage==LGN_CHINA)//中文
	{
		SetWindowText("读/写 记事本");
		GetDlgItem(IDC_STATIC1)->SetWindowText("页码: ");
		GetDlgItem(IDC_STATIC2)->SetWindowText("内容: ");
		GetDlgItem(IDC_STATIC3)->SetWindowText("页码范围:0~15; 内容:32bytes");
		GetDlgItem(IDC_BUTTON1)->SetWindowText("示范");
		GetDlgItem(IDCANCEL)->SetWindowText("退出");
	}
	else
	{
		SetWindowText("Read/Write Notepad");
		GetDlgItem(IDC_STATIC1)->SetWindowText("PageID: ");
		GetDlgItem(IDC_STATIC2)->SetWindowText("Content: ");
		GetDlgItem(IDC_STATIC3)->SetWindowText("Page range:0~15; Size:32bytes");
		GetDlgItem(IDC_BUTTON1)->SetWindowText("Demo");
		GetDlgItem(IDCANCEL)->SetWindowText("Cancel");
	}

	if(m_iMode==0){//读记事本	PS_ReadNotepad
		if(g_iLanguage==LGN_CHINA)//中文
			SetDlgItemText(IDOK,"读记事本");
		else
			SetDlgItemText(IDOK,"ReadNote");
	}
	else{ //写记事本	PS_WriteNotepad
		if(g_iLanguage==LGN_CHINA)//中文
			SetDlgItemText(IDOK,"写记事本");
		else
			SetDlgItemText(IDOK,"WriteNote");
		((CButton*)GetDlgItem(IDC_BUTTON1))->ShowWindow(SW_SHOW);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CDlgNotepad::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(nCtlColor == CTLCOLOR_EDIT)
	{
		pDC->SetTextColor(RGB(250,50,50));
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CDlgNotepad::OnOK() //OK
{
	UpdateData();
	CString sMsg="",sTmp,str;
	BYTE pContent[32]={0};
	int i,j,iLen=0,iH,iL;
	m_content = "";
	if( m_uPage<0 || m_uPage>15)
		return;
	if(m_iMode==0)//读记事本	PS_ReadNotepad
	{
		m_ret = PSReadInfo(m_hHandle,m_nAddr,m_uPage,pContent);
		for(i=0; i<32; i++)
		{
			sTmp.Format("%02X ",pContent[i]);
			sMsg += sTmp;
			m_content += sTmp;
			if( (i%8)==7 )
				m_content += "\r\n";
		}
		//
		if(m_ret!=PS_OK)
			J_AfxMsgBox("读记事本失败! ×","Read Noteoad Failed! ×");
		else
			SetDlgItemText(IDC_EDIT2,sMsg);
	}
	else //写记事本	PS_WriteNotepad
	{
		GetDlgItemText(IDC_EDIT2,sMsg);
		iLen = sMsg.GetLength();
		if( (iLen%3)==1 ){
			J_AfxMsgBox("输入格式错误，请重新输入!\r\n\r\n格式为: 00 FF (请补齐每位16进制数 00~FF)","Input format error, please re-enter! \r\n\r\n format: 00 FF (please each filled hexadecimal number 00 ~ FF)");
			return;
		}
		for(i=0,j=0; i<=iLen-2; i+=3)
		{
			if( i<(iLen-2) )
			{
				if(sMsg[i+2]!=' ')
				{
					J_AfxMsgBox("输入格式错误，请重新输入!\r\n\r\n格式为: 00 FF (每个16进制数之间用一个空格隔开)","Input format error, please re-enter! \r\n\r\n format: 00 FF (hexadecimal number between with a separate space)");
					return;
				}
			}
			iH = J_GetHex2Int(sMsg[i+0]);
			iL = J_GetHex2Int(sMsg[i+1]);
			if(iH==-1 || iL==-1){
				J_AfxMsgBox("输入非法字符，请重新输入!\r\n\r\n只允许输入16进制数,如0~9，a~f，A~F","Illegal importation of characters, please re-enter! \r\n\r\n only allows the importation of hexadecimal number, such as 0 ~ 9, a ~ f, A ~ F");
				return;
			}
			pContent[j++] = iH*16+iL;
		}
		m_ret = PSWriteInfo(m_hHandle,m_nAddr,m_uPage,pContent);
		if(m_ret!=PS_OK)
			J_AfxMsgBox("写记事本失败! ×","Write Noteoad Failed! ×");
		else{
			J_AfxMsgBox("写记事本操作成功! √","Write Notepad Success! √");
			m_content = sMsg;
		}
	}//*/
}

void CDlgNotepad::OnButton1() //示范
{
	CString str;
	str = "00 11 22 33 44 55 66 77 88 99 AA BB CC DD EE FF 12 23 56 78 9A BC DE F0 00 00 00 00 00 00 00 00";
	SetDlgItemText(IDC_EDIT2,str);
}
void CDlgNotepad::J_AfxMsgBox(CString str_CH, CString str_EN)
{
	if(g_iLanguage==LGN_CHINA)//中文
		AfxMessageBox(str_CH);
	else
		AfxMessageBox(str_EN);
}
int CDlgNotepad::J_GetHex2Int(BYTE bt)
{
	int ret=-1;
	switch(bt){
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			ret = bt-'0';	break;
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			ret = bt-'a'+10;break;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			ret = bt-'A'+10;break;
		default:
			ret = -1;		break;
	}
	return ret;
}