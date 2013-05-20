
#pragma once
#define DIRECTINPUT_VERSION 0x0800

#include <mmsystem.h>
#include "\Archivos de programa\Microsoft DirectX SDK (April 2006)\include\d3dx9.h"
#include "\Archivos de programa\Microsoft DirectX SDK (April 2006)\include\dinput.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 

class CDirectInputMouse
{
	public:
		LPDIRECTINPUT8       g_pDI;
		LPDIRECTINPUTDEVICE8 g_pMouse;
		DIMOUSESTATE2 dims2;      // DirectInput mouse state structure
		HWND m_hWnd;
		BOOL    bExclusive;
		BOOL    bForeground;
		
		CDirectInputMouse();
		~CDirectInputMouse();
		void FreeDirectInput();
		
		HRESULT CreateDevice(HWND wnd);
		HRESULT ReadImmediateData();
	
};



