// SelNroTile.cpp : implementation file
//

#include "stdafx.h"
#include "TileEditor.h"
#include "SelNroTile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSelNroTile dialog


CSelNroTile::CSelNroTile(CWnd* pParent /*=NULL*/)
	: CDialog(CSelNroTile::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSelNroTile)
	m_nro_tile = 0;
	//}}AFX_DATA_INIT
}


void CSelNroTile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelNroTile)
	DDX_Text(pDX, IDC_EDIT1, m_nro_tile);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelNroTile, CDialog)
	//{{AFX_MSG_MAP(CSelNroTile)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelNroTile message handlers
