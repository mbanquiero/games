
#include "stdafx.h"
#include <mmsystem.h> 
#include "math.h"
#include "windows.h"
#include "sndmng.h"
#include "\msdev\games\clases\Vectores.h"

#define M_PI	3.141592654


// -----------------------------------
void CSoundManager::Create(HWND hWndNotify,char *fname)
{
	hWaveOut = NULL;
	m_hWnd = hWndNotify;
	tipo_mezcla = 2;

	ant_k_freq = k_freq = 1;
	volumen = 1;
	left = 0.5;
	right = 0.5;

	k_start = 0.3;
	k_decay = 0.5;
	k_end = 0.7;

	// Cargo el wav que voy a usar de muestreo
	_cant_samples = generateWav(fname,&_pcm);
	index = 0;
	WAVEFORMATEX   Format; 
	Format.cbSize = sizeof(WAVEFORMATEX);
	Format.wFormatTag = WAVE_FORMAT_PCM;
	Format.nChannels = 1;
	Format.nSamplesPerSec = SAMPLE_RATE;
	Format.wBitsPerSample = 16;
	Format.nBlockAlign = 2;
	Format.nAvgBytesPerSec = Format.nSamplesPerSec*Format.nBlockAlign;
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &Format, (DWORD)m_hWnd ,0L,CALLBACK_WINDOW);

	elapsed_time = 0;
	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);

	cant_bloques = 0;

	// Genero el timer 
	lap = .1;

}


void CSoundManager::WaveOut() 
{
	if(cant_bloques>=3)
		return;				// mejor espero

	// Avanzo el tiempo
	LARGE_INTEGER T1;   // address of current frequency
	QueryPerformanceCounter(&T1);
	elapsed_time = (double)(T1.LowPart - T0.LowPart) / (double)F.LowPart;
	T0 = T1;
	// quiero generar una onda que dure el mismo tiempo que el lap del timer. (que es el dt del timer)
	// y un 10% mas, de tal forma de evitar el clack entre buffer switchs
	// cant samples = SAMPLE_RATE * dt;		(dt ==1 )

	
	int cant_samples = SAMPLE_RATE*lap*1.1;
	short *pcm = new short[cant_samples];
	int m0 = _cant_samples*k_start;
	int mf = _cant_samples*k_end;
	int md = _cant_samples*k_decay;

	for(int j=0;j<cant_samples;j++)
	{
		float s = (float)j/(float)cant_samples;
		int ndx = index*k_freq;
		short sample = 0;
		switch(tipo_mezcla)
		{
			case 0:
				{
				// tono simple de 110 kz
					double t = (double)index/(double)SAMPLE_RATE;		// tiempo transcurrido
					double _freq = 110*k_freq;
					double _freq2 = 110*k_freq;
					double value1 = sin(2*M_PI*_freq*t);
					double value2 = sin(2*M_PI*_freq2*t);
					double value = s*value1 + (1-s)*value2;
					sample = (int) (value * 32500.0 * volumen);
				}
				break;
			case 1:
				{
					double t = (double)index/(double)SAMPLE_RATE;		// tiempo transcurrido
					double _freq = 440*k_freq;
					double _freq2 = 440*ant_k_freq;
					double value1 = sign(sin(M_PI/32*_freq*t)) + 0.3*cos(M_PI/37*_freq*t);
					double value2 = sign(sin(M_PI/32*_freq2*t)) + 0.3*cos(M_PI/37*_freq2*t);
					double value = s*value1 + (1-s)*value2;
					sample = (int) (value * 32500.0 * volumen);
				}
				break;
			case 2:
			default:
				{
					// muestra 1
					int m1 = m0 + ndx%(mf-m0);
					float vol = m1<=md?1:1-(float)(m1-md)/(float)(mf-md);
					// muestra 2
					int m2 = m0 + (ndx+md-m0)%(mf-m0);
					// combino ambas muestras (la suma de ambos volumens es 1)
					sample = (_pcm[m1]*vol + _pcm[m2]*(1-vol))*volumen;
				}
				break;
		}

		pcm[j] = sample;

		++index;

	}
	

	WAVEHDR *header = new WAVEHDR;
	memset(header,0,sizeof(WAVEHDR));
	header->lpData = (char *)pcm;
	header->dwBufferLength = sizeof(short)*cant_samples;
	header->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
	header->dwLoops = 1;

	if(waveOutPrepareHeader(hWaveOut, header, sizeof(WAVEHDR))==MMSYSERR_NOERROR)         
		if(waveOutWrite (hWaveOut, header, sizeof(WAVEHDR))==MMSYSERR_NOERROR)
			// meto el header en la lista de bloques pendientes
			bloques_pendientes[cant_bloques++] = header;

	ant_k_freq = k_freq;
}


void CSoundManager::Streaming()
{
	// revisa si hay bloques pendientes que se terminaron: 
	for(int i=0;i<cant_bloques;++i)
	{
		if(waveOutUnprepareHeader(hWaveOut, bloques_pendientes[i], sizeof(WAVEHDR))
			!=WAVERR_STILLPLAYING)
		{
			// ya termino de procesarse, (o dio un error, sea como sea: )
			// libero la memoria
			delete bloques_pendientes[i]->lpData;
			delete bloques_pendientes[i];
			// marco el bloque para borrar
			bloques_pendientes[i] = NULL;
		}
	}

	int ant_cant = cant_bloques;
	// pack de bloques
	cant_bloques = 0;
	for(i=0;i<ant_cant;++i)
		if(bloques_pendientes[i]!=NULL)
			bloques_pendientes[cant_bloques++] = bloques_pendientes[i];
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



/*
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
	// un solo canal, SAMPLE_RATE = 22100 hz, 16 bits x sample
	int data_size = m_pData.Subchunk2Size;
	if(m_pFmt.NumChannels!=1)
		data_size/=2;
	if(m_pFmt.SampleRate!=SAMPLE_RATE)
		data_size/=m_pFmt.SampleRate/SAMPLE_RATE;

	short *data = (short *)malloc(data_size);
	if(m_pFmt.BitsPerSample==16)
	{
		// me quedo con un solo canal , el otro esta al pedo
		int index = 0;
		short *pb = (short *)(bytes + pos);
		int cant_samples = m_pData.Subchunk2Size/m_pFmt.NumChannels/2;
		int F = m_pFmt.SampleRate/SAMPLE_RATE;
		// corrijo la dif. de muestreo salteando los samples
		for(int i=0;i<cant_samples;i+=F)
			data[index++] = pb[m_pFmt.NumChannels*i];
		rta = index;
	}

	*pcm = data;
	delete bytes;
	// Retorna la cantidad de samples
	return  rta;
	
}
*/


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
    // playback is complete.
    mciPlayParms.dwCallback = (DWORD) hWndNotify;
    mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms);
    //mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);

    return wDeviceID;
}


