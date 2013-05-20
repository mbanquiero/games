// soundDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sound.h"
#include "soundDlg.h"
#include <mmsystem.h> 
#include "math.h"
#include "windows.h"


#define QUE_WAV		0
#define TIME_LAP	100


#define M_PI	3.141592654
#define MAX_SIZE	5000L*1024L
#define SAMPLE_RATE 44100

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int _cant_bloques = 0;

DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName);

struct RIFF
{
	char ID[4];
	DWORD Size;
	char Format[4];
	
};


//The "WAVE" format consists of two subchunks: "fmt " and "data":
//The "fmt " subchunk describes the sound data's format:
struct FormatChunk
{
	
	char Subchunk1ID[4];	//  Contains the letters "fmt "
	DWORD Subchunk1Size;	//	16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
	WORD AudioFormat;		//  PCM = 1 (i.e. Linear quantization) Values other than 1 indicate some form of compression.
	WORD NumChannels;		//      Mono = 1, Stereo = 2, etc.
	DWORD SampleRate;		//       8000, 44100, etc.
	DWORD ByteRate;			// == SampleRate * NumChannels * BitsPerSample/8
	WORD BlockAlign;		// == NumChannels * BitsPerSample/8 The number of bytes for one sample includingall channels. I wonder what happens when this number isn't an integer?
	WORD BitsPerSample;		//    8 bits = 8, 16 bits = 16, etc.
};


// The "data" subchunk contains the size of the data and the actual sound:
struct DataChunk
{
	char Subchunk2ID[4];	//      Contains the letters "data"
	DWORD Subchunk2Size;	//	NumSamples * NumChannels * BitsPerSample/8 This is the number of bytes in the data.
	// You can also think of this as the size
	// of the read of the subchunk following this  number.
	
};


// Genera el pcm a partir de un archivo WAV
int generateWav(char *fname,short **pcm)
{

	FILE *fp = fopen(fname,"rb");
	if(!fp)
		return 0;

	

	BYTE *bytes = new BYTE[MAX_SIZE];
	fread(bytes,1,MAX_SIZE,fp);
	fclose(fp);
	
	struct RIFF m_pRiff;
	struct FormatChunk m_pFmt;
	struct DataChunk m_pData;
	int pos = 0;
	memcpy(&m_pRiff,bytes,sizeof(struct RIFF));
	pos+=sizeof(struct RIFF);
	memcpy(&m_pFmt,bytes+pos,sizeof(struct FormatChunk));
	pos+=sizeof(struct FormatChunk);
	
	int extra_bytes = m_pFmt.Subchunk1Size - 16;
	if(extra_bytes)
		pos+=extra_bytes;
	
	memcpy(&m_pData,bytes+pos,sizeof(struct DataChunk));

	if(strncmp(m_pData.Subchunk2ID,"fact",4)==0)
	{
		pos+=8+m_pData.Subchunk2Size;
		memcpy(&m_pData,bytes+pos,sizeof(struct DataChunk));
	}


	pos+=sizeof(struct DataChunk);
	
	// Ahora viene la data pp dicha
	int rta = m_pData.Subchunk2Size/2;
	// la transformo a lo que necesito:
	int data_size = m_pData.Subchunk2Size;
	if(m_pFmt.BitsPerSample==8)
		data_size*=2;
	if(m_pFmt.NumChannels==1)
		data_size*=2;
	if(m_pFmt.SampleRate!=SAMPLE_RATE)
		data_size*=SAMPLE_RATE/m_pFmt.SampleRate;

	short *data = (short *)malloc(data_size);
	if(m_pFmt.BitsPerSample==16)
	{
		// me quedo con un solo canal , el otro esta al pedo
		int index = 0;
		short *pb = (short *)(bytes + pos);
		int cant_samples = m_pData.Subchunk2Size/m_pFmt.NumChannels/2;
		int F = SAMPLE_RATE/m_pFmt.SampleRate;
		for(int i=0;i<cant_samples;++i)
			// corrijo la dif. de muestreo repitiendo los samples
			for(int t=0;t<F;++t)
				data[index++] = pb[m_pFmt.NumChannels*i];
		rta = index;
	}
	else
	// viene de culo:
	if(m_pFmt.BitsPerSample==8)
	{
		int index = 0;
		int cant_samples = m_pData.Subchunk2Size/m_pFmt.NumChannels;
		int F = SAMPLE_RATE/m_pFmt.SampleRate;
		for(int i=0;i<cant_samples;++i)
		{
			// tomo el sample y lo paso a 16 bits
			short sample = (short)bytes[pos+m_pFmt.NumChannels*i] << 7;
			// corrijo la dif. de muestreo repitiendo los samples
			for(int t=0;t<F;++t)
				data[index++] = sample;

		}
		rta = index;

	}
		

	//short *data = (short *)malloc(m_pData.Subchunk2Size);
	//memcpy(data,bytes+pos,m_pData.Subchunk2Size);

	*pcm = data;
	delete bytes;
	// Retorna la cantidad de samples
	return  rta;
	
}




// Genera un tono wave sin tipo para afinar 
int generateTone(int freq, double lengthMS, int sampleRate, 
				  double riseTimeMS, double gain,int **rta)
{
	int numSamples = ((double) sampleRate) * lengthMS / 1000.;
	int riseTimeSamples = ((double) sampleRate) * riseTimeMS / 1000.;
	

	if(gain > 1.)
		gain = 1.;
	if(gain < 0.)
		gain = 0.;

	int *pcm = (int *)malloc(numSamples*sizeof(int));
	
	for(int i = 0; i < numSamples; ++i)
	{
		double value = sin(2. * M_PI * freq * i / sampleRate);
		if(i < riseTimeSamples)
			value *= sin(i * M_PI / (2.0 * riseTimeSamples));
		if(i > numSamples - riseTimeSamples - 1)
			value *= sin(2. * M_PI * (i - (numSamples - riseTimeSamples) + riseTimeSamples)/ (4. * riseTimeSamples));
		
		pcm[i] = (int) (value * 32500.0 * gain);
		pcm[i] += (pcm[i]<<16);
	}
	
	*rta = pcm;
	return numSamples;
}



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
	hWaveOut = NULL;
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
	

	ant_k_freq = _k_freq = 1;
	m_freq.SetRange(0,100);
	m_freq.SetPos(100-_k_freq*50);

	_volumen = 1;
	_left = 0.5;
	_right = 0.5;
	m_vol.SetRange(0,100);
	m_vol.SetPos((1-_volumen)*100);
	m_left.SetRange(0,100);
	m_left.SetPos((1-_left)*100);
	m_right.SetRange(0,100);
	m_right.SetPos((1-_right)*100);



	// Cargo el wav que voy a usar de muestreo
	char wav = QUE_WAV;
	switch(wav)
	{
		case 0:
			_cant_samples = generateWav("jet.wav",&_pcm);
			break;
		case 1:
			_cant_samples = generateWav("engine1.wav",&_pcm);
			break;
		case 2:
			_cant_samples = generateWav("spaceship.wav",&_pcm)*0.5;
			break;
	}
			

	_index = 0;
	WAVEFORMATEX   Format; 
	Format.cbSize = sizeof(WAVEFORMATEX);
	Format.wFormatTag = WAVE_FORMAT_PCM;
	Format.nChannels = 2;
	Format.nSamplesPerSec = SAMPLE_RATE;
	Format.wBitsPerSample = 16;
	Format.nBlockAlign = 4;
	Format.nAvgBytesPerSec = Format.nSamplesPerSec*Format.nBlockAlign;

	if(waveOutOpen(&hWaveOut, WAVE_MAPPER, &Format, (DWORD)GetSafeHwnd() , 
			0L,CALLBACK_WINDOW)!=MMSYSERR_NOERROR)
	{       
		return TRUE;
	}


	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);

	// Genereo el timer 
	SetTimer(999,lap=TIME_LAP,NULL);


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



// Este mensaje se llama cuando se termino de ejecutar el sonido del buffer. 
void CSoundDlg::OnWomDone(WPARAM wParam,LPARAM lParam)
{
	// A waveform-audio data block has been played and can now be freed. 
	LPWAVEHDR header = (LPWAVEHDR)lParam;
	// tengo que llamar a esta funcion para liberar el header
	waveOutUnprepareHeader((HWAVEOUT) wParam, header, sizeof(WAVEHDR)); 
	// libero la memoria
	delete header->lpData;
	delete header;

	// sincronzacion del streaming : 
	// un bloque menos
	--_cant_bloques;


	char buffer[255];
	sprintf(buffer,"Cant bloques: %d (Saque)",_cant_bloques);
	SetDlgItemText(IDC_ST_1,buffer);
	RedrawWindow();

}




// Currada de la aluda del visual studio 6.0
// le vole la mitad de las cosas que se ve que cambiaron para el XP

// Plays a specified MIDI file by using MCI_OPEN and MCI_PLAY. Returns 
// as soon as playback begins. The window procedure function for the 
// specified window will be notified when playback is complete. 
// Returns 0L on success; otherwise, it returns an MCI error code.
DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName)
{
    UINT wDeviceID;
    DWORD dwReturn;
    MCI_OPEN_PARMS mciOpenParms;
    MCI_PLAY_PARMS mciPlayParms;
    MCI_STATUS_PARMS mciStatusParms;

	// le agregue estas lineas, si no, no funciona en windows xp
	ZeroMemory(&mciOpenParms,sizeof(mciOpenParms));
	ZeroMemory(&mciPlayParms,sizeof(mciPlayParms));
	ZeroMemory(&mciStatusParms,sizeof(mciStatusParms));

    // Open the device by specifying the device and filename.
    // MCI will attempt to choose the MIDI mapper as the output port.
    mciOpenParms.lpstrDeviceType = "sequencer";
    mciOpenParms.lpstrElementName = lpszMIDIFileName;
    if (dwReturn = mciSendCommand(NULL, MCI_OPEN,
        MCI_OPEN_TYPE | MCI_OPEN_ELEMENT,
        (DWORD)(LPVOID) &mciOpenParms))
    {
        // Failed to open device. Don't close it; just return error.
        return (dwReturn);
    }

    // The device opened successfully; get the device ID.
    wDeviceID = mciOpenParms.wDeviceID;

    // Check if the output port is the MIDI mapper.
    mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_STATUS, 
        MCI_STATUS_ITEM, (DWORD)(LPVOID) &mciStatusParms))
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return (dwReturn);
    }

    // Begin playback. The window procedure function for the parent 
    // window will be notified with an MM_MCINOTIFY message when 
    // playback is complete. At this time, the window procedure closes 
    // the device.
    mciPlayParms.dwCallback = (DWORD) hWndNotify;
    if (dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY, 
        (DWORD)(LPVOID) &mciPlayParms))
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return (dwReturn);
    }

    return (0L);
}

void CSoundDlg::OnTimer(UINT nIDEvent) 
{
	// Avanzo el tiempo
	LARGE_INTEGER T1;   // address of current frequency
	QueryPerformanceCounter(&T1);
	double elapsed_time = (double)(T1.LowPart - T0.LowPart) / (double)F.LowPart;
	T0 = T1;
	if(nIDEvent==999)
	{
		// quiero generar una onda que dure el mismo tiempo que el lap del timer. (que es el dt del timer)
		// y un 10% mas, de tal forma de evitar el clack entre buffer switchs
		// cant samples = SAMPLE_RATE * dt;		(dt ==1 )
		int cant_samples = SAMPLE_RATE*(lap*1.1)/1000.0;
		short *pcm = new short[2*cant_samples];
		int ramp = cant_samples*0.4;
		for(int j=0;j<cant_samples;j++)
		{

			/*
			double value;
			double t = (double)_index/(double)SAMPLE_RATE;		// tiempo transcurrido
			float fndx = _index*k_freq;
			if(fndx>=_cant_samples)
				fndx = (((int)fndx)%_cant_samples) + (fndx-(int)fndx);

			int ndx = fndx;
			if(ndx==0)
				// justo la primer muestra
				value = (float)_pcm[ndx];
			else
			{
				// bilinear sampling
				float fmod = fndx-ndx;
				value = ((float)_pcm[ndx]*fmod) + ((float)_pcm[ndx-1]*(1-fmod));
			}

			short sample = value;
			pcm[2*j] = sample;
			pcm[2*j+1] = 0;
			*/

			/*
			double t = (double)_index/(double)SAMPLE_RATE;		// tiempo transcurrido
			double _freq = 440*_k_freq;
			double value = 0.7*sin(2.*M_PI*_freq*t) + 0.25*sin(M_PI/8.0*_freq*t);
			pcm[2*j] = (int) (value * 32500.0 * 0.2);
			pcm[2*j + 1] = 0;
			*/


			int ndx = _index*_k_freq;
			switch(m_mezclar)
			{
				case 0:
					{
						// muestra simple
						short sample = _pcm[ndx%_cant_samples]*_volumen;
						pcm[2*j] = sample*_left;
						pcm[2*j+1] = sample*_right;
					}
					break;
				case 1:
					// mixer c/rampa
					{
						// muestra actual
						int t = ndx%_cant_samples;
						short sample;

						if(t<_cant_samples-ramp)
						{
							sample = _pcm[t]*_volumen;
						}
						else
						{
							// mixer
							short sample2 = _pcm[t-_cant_samples+ramp]*_volumen;
							float s = (float)(t-_cant_samples+ramp)/(float)ramp;
							sample = sample*(1-s) + sample2*s;
						}

						pcm[2*j] = sample*_left;
						pcm[2*j+1] = sample*_right;

					}
					break;
				case 2:
					{
						// muestra circular
						int i0 = 1000;
						float total = 0;
						int ds = _cant_samples/5;
						for(int i=0;i<5;++i)
							total += _pcm[i0 + abs((ndx+ds*(i-2))%_cant_samples)];
						short sample = total/5.0*_volumen;
						pcm[2*j] = sample*_left;
						pcm[2*j+1] = sample*_right;
					}
					break;
			}

			++_index;

		}
		

		WAVEHDR *header = new WAVEHDR;
		memset(header,0,sizeof(WAVEHDR));
		header->lpData = (char *)pcm;
		header->dwBufferLength = sizeof(int)*cant_samples;
		header->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
		header->dwLoops = 1;

		if(waveOutPrepareHeader(hWaveOut, header, sizeof(WAVEHDR))==MMSYSERR_NOERROR)         
			if(waveOutWrite (hWaveOut, header, sizeof(WAVEHDR))==MMSYSERR_NOERROR)
				_cant_bloques++;

		char buffer[255];
		sprintf(buffer,"Cant bloques: %d (Agregue)",_cant_bloques);
		SetDlgItemText(IDC_ST_1,buffer);

		sprintf(buffer,"%d ms",(int)(1000.0*elapsed_time));
		SetDlgItemText(IDC_ST_2,buffer);

		ant_k_freq = _k_freq;

		RedrawWindow();

	}
	CDialog::OnTimer(nIDEvent);
}

void CSoundDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	int pos = 100-m_freq.GetPos();
	_k_freq = (float)pos / 50.;

	pos = m_vol.GetPos();
	_volumen = 1 - (float)pos / 100.;

	pos = m_left.GetPos();
	_left = 1 - (float)pos / 100.;

	pos = m_right.GetPos();
	_right = 1 - (float)pos / 100.;

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CSoundDlg::OnMezclar() 
{
	UpdateData(TRUE);
}

void CSoundDlg::OnPlay() 
{
	// TODO: Add your control notification handler code here
	playMIDIFile(GetSafeHwnd(),"Shook_Me_All_Night.mid");
	GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);
	
}

void CSoundDlg::OnPlayWav() 
{
	PlaySound("Explosion.wav",NULL,SND_ASYNC);
}

