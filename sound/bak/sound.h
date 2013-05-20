// sound.h : main header file for the SOUND application
//

#if !defined(AFX_SOUND_H__C251ADE3_AB3E_4AEC_B58B_4E7893CADACC__INCLUDED_)
#define AFX_SOUND_H__C251ADE3_AB3E_4AEC_B58B_4E7893CADACC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSoundApp:
// See sound.cpp for the implementation of this class
//

class CSoundApp : public CWinApp
{
public:
	CSoundApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSoundApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUND_H__C251ADE3_AB3E_4AEC_B58B_4E7893CADACC__INCLUDED_)
