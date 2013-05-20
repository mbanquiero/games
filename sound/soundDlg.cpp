// soundDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sound.h"
#include "soundDlg.h"
#include "sndmng.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define QUE_WAV		0

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg dialog

CSoundDlg::CSoundDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSoundDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundDlg)
	m_mezclar = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSoundDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundDlg)
	DDX_Control(pDX, IDC_SLIDER4, m_vol);
	DDX_Control(pDX, IDC_SLIDER3, m_right);
	DDX_Control(pDX, IDC_SLIDER2, m_left);
	DDX_Control(pDX, IDC_SLIDER1, m_freq);
	DDX_Radio(pDX, IDC_MEZCLAR, m_mezclar);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSoundDlg, CDialog)
	//{{AFX_MSG_MAP(CSoundDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(MM_WOM_DONE , OnWomDone)
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_MEZCLAR, OnMezclar)
	ON_BN_CLICKED(IDC_MEZCLAR2, OnMezclar)
	ON_BN_CLICKED(IDC_MEZCLAR3, OnMezclar)
	ON_BN_CLICKED(IDC_PLAY, OnPlay)
	ON_BN_CLICKED(IDC_PLAY_WAV, OnPlayWav)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSoundDlg message handlers

BOOL CSoundDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	sound.Create(m_hWnd,"plane.wav");

	m_freq.SetRange(0,100);
	m_freq.SetPos(100-sound.k_freq*50);

	m_vol.SetRange(0,100);
	m_vol.SetPos((1-sound.volumen)*100);
	m_left.SetRange(0,100);
	m_left.SetPos((1-sound.left)*100);
	m_right.SetRange(0,100);
	m_right.SetPos((1-sound.right)*100);

	SetTimer(999,TIME_LAP,NULL);



	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSoundDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSoundDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}




void CSoundDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent==999)
	{
		sound.WaveOut();
		char buffer[255];
		sprintf(buffer,"Cant bloques: %d (Agregue)",sound.cant_bloques);
		SetDlgItemText(IDC_ST_1,buffer);

		sprintf(buffer,"%d ms",(int)(1000.0*sound.elapsed_time));
		SetDlgItemText(IDC_ST_2,buffer);

		RedrawWindow();
	}
	
	CDialog::OnTimer(nIDEvent);
}

void CSoundDlg::OnWomDone(WPARAM wparam,LPARAM lparam)
{
	sound.OnWomDone(wparam,lparam);
}


void CSoundDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int pos = 100-m_freq.GetPos();
	sound.k_freq = (float)pos / 50.;

	pos = m_vol.GetPos();
	sound.volumen = 1 - (float)pos / 100.;

	pos = m_left.GetPos();
	sound.left = 1 - (float)pos / 100.;

	pos = m_right.GetPos();
	sound.right = 1 - (float)pos / 100.;

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CSoundDlg::OnMezclar() 
{
	UpdateData(TRUE);
}

void CSoundDlg::OnPlay() 
{
	playMIDIFile(GetSafeHwnd(),"Shook_Me_All_Night.mid");
	GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);
	
}

void CSoundDlg::OnPlayWav() 
{
	PlaySound("Explosion.wav",NULL,SND_ASYNC);
}

