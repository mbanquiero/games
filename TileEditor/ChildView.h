// ChildView.h : interface of the CChildView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDVIEW_H__ABE92C08_FD98_40D2_9E93_1CC150D13FDF__INCLUDED_)
#define AFX_CHILDVIEW_H__ABE92C08_FD98_40D2_9E93_1CC150D13FDF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CChildView window

#define TOOL_NADA			0
#define TOOL_PONER_TILE		1

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	CPoint origen;
	CPoint pt_ant;
	char eventoInterno;
	char tool_sel;

	int tile_sel,tile_sel2,tile_sel3,tile_sel4;
	int ox,oy;
	float ex,ey;
	BOOL hay_preview;
	float pex,pey;
	int pox,poy;
	int sel_i,sel_j;

	BOOL grilla;

	CFont m_font;
	BOOL running;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChildView)
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnFileSave();
	afx_msg void OnFileOpen();
	afx_msg void OnSaveTile();
	afx_msg void OnUpdateTile();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnEscape();
	afx_msg void OnToolPonerTile();
	afx_msg void OnPonerFuego();
	afx_msg void OnSelNroTile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDVIEW_H__ABE92C08_FD98_40D2_9E93_1CC150D13FDF__INCLUDED_)
