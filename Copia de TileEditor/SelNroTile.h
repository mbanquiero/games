#if !defined(AFX_SELNROTILE_H__371F6945_8552_45E2_86ED_CAB2A32944A5__INCLUDED_)
#define AFX_SELNROTILE_H__371F6945_8552_45E2_86ED_CAB2A32944A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelNroTile.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelNroTile dialog

class CSelNroTile : public CDialog
{
// Construction
public:
	CSelNroTile(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSelNroTile)
	enum { IDD = IDD_SEL_NRO_TILE };
	int		m_nro_tile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelNroTile)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelNroTile)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELNROTILE_H__371F6945_8552_45E2_86ED_CAB2A32944A5__INCLUDED_)
