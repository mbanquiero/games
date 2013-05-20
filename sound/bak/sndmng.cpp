
#include "stdafx.h"
#include <mmsystem.h> 
#include "math.h"
#include "windows.h"
#include "sndmng.h"



// -----------------------------------
CSoundManager::CSoundManager(HWND hWndNotify)
{
	m_hWnd = hWndNotify;
	tipo_mezcla = 2;

	ant_k_freq = k_freq = 1;
	volumen = 1;
	left = 0.5;
	right = 0.5;

	// Cargo el wav que voy a usar de muestreo
	_cant_samples = generateWav("jet.wav",&_pcm);
	index = 0;

	WAVEFORMATEX   Format; 
	Format.cbSize = sizeof(WAVEFORMATEX);
	Format.wFormatTag = WAVE_FORMAT_PCM;
	Format.nChannels = 2;
	Format.nSamplesPerSec = SAMPLE_RATE;
	Format.wBitsPerSample = 16;
	Format.nBlockAlign = 4;
	Format.nAvgBytesPerSec = Format.nSamplesPerSec*Format.nBlockAlign;
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &Format, (DWORD)m_hWnd ,0L,CALLBACK_WINDOW);

	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);



}


void CSoundManager::WaveOut() 
{
	// Avanzo el tiempo
	LARGE_INTEGER T1;   // address of current frequency
	QueryPerformanceCounter(&T1);
	double elapsed_time = (double)(T1.LowPart - T0.LowPart) / (double)F.LowPart;
	T0 = T1;
	// quiero generar una onda que dure el mismo tiempo que el lap del timer. (que es el dt del timer)
	// y un 10% mas, de tal forma de evitar el clack entre buffer switchs
	// cant samples = SAMPLE_RATE * dt;		(dt ==1 )
	int cant_samples = SAMPLE_RATE*(lap*1.1)/1000.0;
	short *pcm = new short[2*cant_samples];
	int ramp = cant_samples*0.4;
	for(int j=0;j<cant_samples;j++)
	{
		int ndx = index*k_freq;
		switch(tipo_mezcla)
		{
			case 0:
				{
					// muestra simple
					short sample = _pcm[ndx%_cant_samples]*volumen;
					pcm[2*j] = sample*left;
					pcm[2*j+1] = sample*right;
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
						sample = _pcm[t]*volumen;
					}
					else
					{
						// mixer
						short sample2 = _pcm[t-_cant_samples+ramp]*volumen;
						float s = (float)(t-_cant_samples+ramp)/(float)ramp;
						sample = sample*(1-s) + sample2*s;
					}

					pcm[2*j] = sample*left;
					pcm[2*j+1] = sample*right;

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
					short sample = total/5.0*volumen;
					pcm[2*j] = sample*left;
					pcm[2*j+1] = sample*right;
				}
				break;
		}

		++index;

	}
	

	WAVEHDR *header = new WAVEHDR;
	memset(header,0,sizeof(WAVEHDR));
	header->lpData = (char *)pcm;
	header->dwBufferLength = sizeof(int)*cant_samples;
	header->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
	header->dwLoops = 1;

	if(waveOutPrepareHeader(hWaveOut, header, sizeof(WAVEHDR))==MMSYSERR_NOERROR)         
		if(waveOutWrite (hWaveOut, header, sizeof(WAVEHDR))==MMSYSERR_NOERROR)
			cant_bloques++;


	ant_k_freq = k_freq;
}



// Este mensaje se llama cuando se termino de ejecutar el sonido del buffer. 
void CSoundManager::OnWomDone(WPARAM wParam,LPARAM lParam)
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
	--cant_bloques;

}








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
