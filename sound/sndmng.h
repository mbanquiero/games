#pragma once

#include "stdafx.h"
#include <mmsystem.h> 
#include "math.h"
#include "windows.h"

#define TIME_LAP	100


#define MAX_SIZE	5000L*1024L
#define SAMPLE_RATE 44100


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



extern DWORD playMIDIFile(HWND hWndNotify, LPSTR lpszMIDIFileName);

// Genera el pcm a partir de un archivo WAV
extern int generateWav(char *fname,short **pcm);

extern int generateTone(int freq, double lengthMS, int sampleRate, 
				  double riseTimeMS, double gain,int **rta);


#define MAX_STREAMING		100

class CSoundManager
{
	public:
		int cant_bloques;
		WAVEHDR *bloques_pendientes[MAX_STREAMING];
		HWND m_hWnd;

		HWAVEOUT hWaveOut; 
		LARGE_INTEGER F,T0;
		short *_pcm;
		int _cant_samples;
		
		int index;
		float k_freq;
		float ant_k_freq;
		float lap;
		float volumen;
		float left;
		float right;
		char tipo_mezcla;
		double elapsed_time;

		float k_start;
		float k_decay;
		float k_end;


		void Create(HWND hWndNotify,char *fname);
		void WaveOut();			// Genera el sonido
		void OnWomDone(WPARAM wParam,LPARAM lParam);
		void Streaming();


};