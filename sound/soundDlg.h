// soundDlg.h : header file
//

#if !defined(AFX_SOUNDDLG_H__C88EF1F7_3555_45A9_9809_C8E30BD09B0F__INCLUDED_)
#define AFX_SOUNDDLG_H__C88EF1F7_3555_45A9_9809_C8E30BD09B0F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <mmsystem.h> 
#include "sndmng.h"

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog

class CSoundDlg : public CDialog
{
// Construction
public:
	CSoundDlg(CWnd* pParent = NULL);	// standard constructor

	CSoundManager sound;

// Dialog Data
	//{{AFX_DATA(CSoundDlg)
	enum { IDD = IDD_SOUND_DIALOG };
	CSliderCtrl	m_vol;
	CSliderCtrl	m_right;
	CSliderCtrl	m_left;
	CSliderCtrl	m_freq;
	int		m_mezclar;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSoundDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnWomDone(WPARAM,LPARAM);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMezclar();
	afx_msg void OnPlay();
	afx_msg void OnPlayWav();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUNDDLG_H__C88EF1F7_3555_45A9_9809_C8E30BD09B0F__INCLUDED_)
