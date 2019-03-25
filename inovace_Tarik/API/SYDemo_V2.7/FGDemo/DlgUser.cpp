// DlgUser.cpp : implementation file
//

#include "stdafx.h"
#include "FGDemo.h"
#include "DlgUser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgUser dialog


CDlgUser::CDlgUser(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgUser::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgUser)
	m_nAddr = 0;
	//}}AFX_DATA_INIT
}


void CDlgUser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgUser)
	DDX_Text(pDX, IDC_EDIT1, m_nAddr);
	DDV_MinMaxUInt(pDX, m_nAddr, 0, 99999);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgUser, CDialog)
	//{{AFX_MSG_MAP(CDlgUser)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgUser message handlers

BOOL CDlgUser::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if(g_iLanguage==LGN_CHINA)//中文
	{
		SetWindowText("设置用户地址");
		GetDlgItem(IDC_STATIC1)->SetWindowText("设置");
		GetDlgItem(IDC_STATIC2)->SetWindowText("存放地址: ");
	}
	else
	{
		SetWindowText("User Address");
		GetDlgItem(IDC_STATIC1)->SetWindowText("Setting");
		GetDlgItem(IDC_STATIC2)->SetWindowText("Address: ");
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CDlgUser::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(nCtlColor == CTLCOLOR_EDIT)//编辑框_红色
	{
		pDC->SetTextColor(RGB(250,50,50));
	}
	// TODO: Return a different brush if the default is not desired
	return hbr;
}
