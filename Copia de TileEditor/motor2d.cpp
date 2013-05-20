#include "stdafx.h"

#include <mmsystem.h>
#include "\Archivos de programa\Microsoft DirectX SDK (April 2006)\include\d3dx9.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include "\msdev\games\clases\vectores.h"
#include "motor2d.h"



/////////////////////////////////////////////////////////////////////////////
// Motor 2d 
/////////////////////////////////////////////////////////////////////////////

void DXEngine2d::Create()
{
	init = FALSE;

	g_pD3D = NULL;
	g_pd3dDevice = NULL;
	g_pVB = NULL;
	g_pQuad = NULL;
	g_pEffect = NULL;			// D3DX effect interface
	hay_shader = FALSE;
	g_pLevel = NULL;			// Buffer para todo el nivel

	cant_bmp = 0;

	// tiles:
	memset(C,0,sizeof(C));
	tile_cant_fil = tile_cant_col = 0;

	// tamaño de pantalla en filas x columnas
	cant_fil = 25;
	cant_col = 40;
	// en pixels
	screen_dx = 480;
	screen_dy = 300;

	vel_h = vel_v = 0;

	status = P_STATUS_UNKNOWN;
	sentido = 0;
	sprite_sel = 0;

	vel_cinta = 50;

	flag_tubo = FALSE;
	timer_fuego = 0;

	cant_items = 0;

	cant_vidas = 5;

	timer_quema = timer_cadena = timer_choco = timer_caida = 0;

	pos_seg_x = pos_seg_y = -1;		// no tiene pos. segura todavia
}


// inicializa el Direct X
HRESULT DXEngine2d::DXInit( HWND hWnd)
{

	if(g_pD3D)
		return S_OK;

	if(hWnd==NULL)
		hWnd = AfxGetMainWnd()->m_hWnd;
	m_hWnd = hWnd;

	g_pVB = NULL;
	g_pQuad = NULL;

	// Create the D3D object.
	if( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
	{
		AfxMessageBox("Necesita DIRECT X");
		return E_FAIL;
	}

	// Set up the structure used to create the D3DDevice
	//D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof(d3dpp) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_2_SAMPLES;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	//d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE	;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	// ojo que si quiero dibujarn en una textura no anda el zbuffer si le pongo multisample
	
	// Pruebo con zbuffer de 24 bits 
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	if(FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
									  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
									  &d3dpp, &g_pd3dDevice ) ) )
	{

		// pruebo con otro tipo de buffer y sin antialias
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
		if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd,
									  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
									  &d3dpp, &g_pd3dDevice ) ) )
		{

			// pruebo sin hardware acceleration layer (HAL)
			if( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, m_hWnd,
									  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
									  &d3dpp, &g_pd3dDevice ) ) )
			{
				// no tengo lo que hacer
				AfxMessageBox("Necesita DIRECT X");
				return E_FAIL;
			}
		}
	}

    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE /*3DCULL_CCW */);
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);			// z-buffer habilitado ?
	g_pd3dDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS , TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_SHADEMODE,D3DSHADE_GOURAUD);
    g_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, FALSE );


	// D3DTADDRESS_WRAP
	//g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);		
	//g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);		
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);		// D3DTEXF_LINEAR
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_NONE);

	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	// habilito las tranparencias
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	//g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_BLENDDIFFUSEALPHA);
	// Color Final = (Source * A) + (Dest * (1-A))
	g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);		// Source * A
	g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);	// Dest * (1-A)
	g_pd3dDevice->SetRenderState(D3DRS_BLENDOP,D3DBLENDOP_ADD);			// Suma ambos terminos
	g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );


    // Texto
	D3DXCreateFont( g_pd3dDevice ,            // D3D device
                         14,					// Height
                         0,                     // Width
                         FW_LIGHT,               // Weight
                         0,                     // MipLevels, 0 = autogen mipmaps
                         FALSE,                 // Italic
                         DEFAULT_CHARSET,       // CharSet
                         OUT_DEFAULT_PRECIS,    // OutputPrecision
                         DEFAULT_QUALITY,       // Quality
                         DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
                         "Arial",              // pFaceName
                         &g_pFont);              // ppFont

	D3DXCreateFont( g_pd3dDevice ,22,8,FW_BOLD,0,FALSE,
		DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE, 
                         "Lucida Console",&g_pFontb);
	g_pFontb->PreloadGlyphs('0','9');
	g_pFontb->PreloadGlyphs('a','z');
	g_pFontb->PreloadGlyphs('A','Z');
	
	// font auxiliar
	D3DXCreateFont( g_pd3dDevice ,16,7,FW_NORMAL,0,FALSE,
		DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE, 
                         "Lucida Console",&g_pFont2);
	g_pFont2->PreloadGlyphs('0','9');
	g_pFont2->PreloadGlyphs('a','z');
	g_pFont2->PreloadGlyphs('A','Z');


	// Sprite para las cotas
	D3DXCreateSprite(g_pd3dDevice,&pSprite);
	// lines varios
	D3DXCreateLine(g_pd3dDevice, &ppLine);

	// Material x defecto
	ZeroMemory( &mtrl_std, sizeof(mtrl_std) );
	mtrl_std.Diffuse.r = mtrl_std.Ambient.r = 1;
	mtrl_std.Diffuse.g = mtrl_std.Ambient.g = 1;
	mtrl_std.Diffuse.b = mtrl_std.Ambient.b = 1;
	mtrl_std.Diffuse.a = mtrl_std.Ambient.a = 1;
	g_pd3dDevice->SetMaterial( &mtrl_std);
	g_pd3dDevice->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL);

	// Material transparente
	ZeroMemory( &mtrl_tx, sizeof(mtrl_tx) );
	mtrl_tx.Diffuse.r = mtrl_tx.Ambient.r = 1;
	mtrl_tx.Diffuse.g = mtrl_tx.Ambient.g = 1;
	mtrl_tx.Diffuse.b = mtrl_tx.Ambient.b = 1;
	mtrl_tx.Diffuse.a = mtrl_tx.Ambient.a = 0.5;

	// Material negro
	ZeroMemory( &mtrl_negro, sizeof(mtrl_negro) );
	mtrl_negro.Diffuse.r = mtrl_negro.Ambient.r = 0;
	mtrl_negro.Diffuse.g = mtrl_negro.Ambient.g = 0;
	mtrl_negro.Diffuse.b = mtrl_negro.Ambient.b = 0;
	mtrl_negro.Diffuse.a = mtrl_negro.Ambient.a = 1;


	// Resolucion de pantalla
	sdx = d3dpp.BackBufferWidth*0.35;
	sdy = d3dpp.BackBufferHeight*0.35;

	
	return S_OK;
}




void DXEngine2d::DXCleanTextures()
{
	// Libera las Texturas
	for(int i=0;i<cant_texturas;++i)
		SAFE_RELEASE(g_pTexture[i]);
	cant_texturas = 0;
}

// Carga la textura si no esta, devuelve el nro de textura
int DXEngine2d::cargar_textura(char *filename,int K)
{
	// primero busco que la textura no este repetida
	rtrim(filename);
	int rta = -1;
	int i =0;
	while(i<cant_bmp && rta==-1)
		if(strcmp(bmp_fname[i],filename)==0)
			rta = i;
		else
			++i;

	if(rta==-1)
	{
		// textura nueva, la cargo y la agrego en la lista
		strcpy(bmp_fname[cant_bmp],filename);
		bmp_k[cant_bmp] = K;
		rta = cant_bmp++;
	}
	else
		// actualizo el K
		bmp_k[rta] = K;

	return rta;
}


HRESULT DXEngine2d::DXLoadTextures()
{
	// Primero libero cualquier textura anterior
	DXCleanTextures();

	// Use D3DX to create a texture from a file based image
	// Voy a cargar las texturas

	for(int i=0;i<cant_bmp;++i)
	{
		char *ftexture = bmp_fname[i];
		char fname_aux[MAX_PATH];
		//sacarPath(ftexture,fname_aux);
		sprintf(fname_aux,"texturas\\%s",ftexture);

		// si el nombre del bmp empieza con + , es porque tiene alpha channel (simulado)
		BOOL alpha = FALSE;
		if(ftexture[0]=='+')
		{
			alpha = TRUE;
			if(FAILED( D3DXCreateTextureFromFileEx( g_pd3dDevice, fname_aux, 
							D3DX_DEFAULT,    // default width
                            D3DX_DEFAULT,    // default height
                            1,    // no mip mapping		--> si le pusiera mip-maping despues no lo puedo cambiar.
							// en ese caso deberia cambiar toda la cadena de bitmaps....
                            NULL,    // regular usage
                            D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                            D3DPOOL_MANAGED,    // typical memory handling
                            D3DX_DEFAULT,    // no filtering
                            D3DX_DEFAULT,    // no mip filtering
                            0,
							//D3DCOLOR_ARGB(255,255, 0, 255),    // MAGIC COLOR 
                            NULL,    // no image info struct
                            NULL,    // not using 256 colors
							&g_pTexture[cant_texturas])))
				g_pTexture[cant_texturas] = NULL;
		}
		else
		if(ftexture[0]=='-' || ftexture[0]=='*')
		{
			// - indica alpha chanel + automatic mipmaping
			// * indica alpha chanel + sin mipmaping
			sprintf(fname_aux,"texturas\\%s",ftexture+1);
			alpha = TRUE;
			if(FAILED( D3DXCreateTextureFromFileEx( g_pd3dDevice, fname_aux, 
							D3DX_DEFAULT,    // default width
                            D3DX_DEFAULT,    // default height
                            ftexture[0]=='*'?1:0,    // automatic mipmaping: ojo....en este caso tengo cambiar toda la cadena de bitmaps.... o sin mip maping
                            NULL,    // regular usage
                            D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                            D3DPOOL_MANAGED,    // typical memory handling
                            D3DX_DEFAULT,    // no filtering
                            D3DX_DEFAULT,    // no mip filtering
                            0,
							//D3DCOLOR_ARGB(255,0, 0, 0),    // MAGIC COLOR 
                            NULL,    // no image info struct
                            NULL,    // not using 256 colors
							&g_pTexture[cant_texturas])))
				g_pTexture[cant_texturas] = NULL;
		}
		else
		if(bmp_k[i]==-1)
		{
			// Textura sprite (sin MIPMAPING)
			if(FAILED( D3DXCreateTextureFromFileEx( g_pd3dDevice, fname_aux, 
							D3DX_DEFAULT,    // default width
                            D3DX_DEFAULT,    // default height
                            1,    // no mip mapping
                            NULL,    // regular usage
                            D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                            D3DPOOL_MANAGED,    // typical memory handling
                            D3DX_DEFAULT,    // no filtering
                            D3DX_DEFAULT,    // no mip filtering
                            0,
                            NULL,    // no image info struct
                            NULL,    // not using 256 colors
							&g_pTexture[cant_texturas])))
				g_pTexture[cant_texturas] = NULL;
		}
		else
		{
			// abro el bmp comun y corriente (con mipmaing
			if(FAILED( D3DXCreateTextureFromFile( g_pd3dDevice, fname_aux, &g_pTexture[cant_texturas])))
				g_pTexture[cant_texturas] = NULL;
		}

		if(alpha)
		{
			// Initialize the alpha channel
			D3DLOCKED_RECT lockedRect;

			if(SUCCEEDED(g_pTexture[cant_texturas]->LockRect(0, &lockedRect, NULL, D3DLOCK_DISCARD )))
			{
				D3DSURFACE_DESC desc;
				g_pTexture[cant_texturas]->GetLevelDesc(0,&desc);
				int m_dwWidth = desc.Width;
				int m_dwHeight = desc.Height;
				int pitch = lockedRect.Pitch / sizeof(DWORD);
				for( int y=0; y < m_dwHeight; y++ )
				{	
					//int dwOffset = y*m_dwWidth;
					int dwOffset = y*pitch;
					float grad;
					if(y<m_dwHeight/4.0)
						grad = 4.0*(float)y/(float)m_dwHeight;
					else
					if(y>3.0*m_dwHeight/4.0)
						grad = 4.0*(1.0-((float)y)/(float)m_dwHeight);
					else
						grad = 1.0;
					for( int x=0; x < m_dwWidth; x++ )
					{

						//DWORD color = *(((DWORD *)lockedRect.pBits)+(dwOffset+x));
						BYTE *color = (BYTE *)(((DWORD *)lockedRect.pBits)+(dwOffset+x));
						BYTE b = color[0];
						BYTE g = color[1];
						BYTE r = color[2];


						*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB((BYTE)(255.0*(1-grad*0.75)),r,g,b);

						//if(color==0xFFFF00ff)
							// MAGIC COLOR
							//*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(0,0,0,0);
					}
				}
				g_pTexture[cant_texturas]->UnlockRect(0);
			}
		}


		// paso a la siguiente textura
		++cant_texturas;

	}

	// si todo salio bien cant_texturas==cant_bmp
    return S_OK;
}


HRESULT SetAlphaChannel(LPDIRECT3DTEXTURE9  g_pTexture,BYTE r0,BYTE g0,BYTE b0,BYTE ds)
{
	// Initialize the alpha channel
	// tengo que hacer el reemplazo en TODOS los mipmaps generados para 
	// esta textura
	D3DLOCKED_RECT lockedRect;
	UINT cant_mipmaps = g_pTexture->GetLevelCount();
	for(UINT i=0;i<cant_mipmaps;++i)
	if(SUCCEEDED(g_pTexture->LockRect(i, &lockedRect, NULL, D3DLOCK_DISCARD )))
	{
		D3DSURFACE_DESC desc;
		g_pTexture->GetLevelDesc(i,&desc);
		int m_dwWidth = desc.Width;
		int m_dwHeight = desc.Height;
		int pitch = lockedRect.Pitch / sizeof(DWORD);
		for( int y=0; y < m_dwHeight; y++ )
		{	
			int dwOffset = y*pitch;
			for( int x=0; x < m_dwWidth; x++ )
			{

				BYTE *color = (BYTE *)(((DWORD *)lockedRect.pBits)+(dwOffset+x));
				BYTE b = color[0];
				BYTE g = color[1];
				BYTE r = color[2];

				if(abs(b-b0)<ds && abs(g-g0)<ds && abs(r-r0)<ds)		// es el mask transparente
					*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(0,r,g,b);
				else
					*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(255,r,g,b);

			}
		}
		g_pTexture->UnlockRect(i);
	}
    return S_OK;
}


HRESULT SetAlphaChannelGradient(LPDIRECT3DTEXTURE9  g_pTexture,BYTE r0,BYTE g0,BYTE b0,BYTE ds0,BYTE ds1)
{
	// Initialize the alpha channel
	// tengo que hacer el reemplazo en TODOS los mipmaps generados para 
	// esta textura
	D3DLOCKED_RECT lockedRect;
	UINT cant_mipmaps = g_pTexture->GetLevelCount();
	for(UINT i=0;i<cant_mipmaps;++i)
	if(SUCCEEDED(g_pTexture->LockRect(i, &lockedRect, NULL, D3DLOCK_DISCARD )))
	{
		D3DSURFACE_DESC desc;
		g_pTexture->GetLevelDesc(i,&desc);
		int m_dwWidth = desc.Width;
		int m_dwHeight = desc.Height;
		int pitch = lockedRect.Pitch / sizeof(DWORD);
		for( int y=0; y < m_dwHeight; y++ )
		{	
			int dwOffset = y*pitch;
			for( int x=0; x < m_dwWidth; x++ )
			{

				BYTE *color = (BYTE *)(((DWORD *)lockedRect.pBits)+(dwOffset+x));
				BYTE b = color[0];
				BYTE g = color[1];
				BYTE r = color[2];


				if(abs(b-b0)<ds0 && abs(g-g0)<ds0 && abs(r-r0)<ds0)		// es el mask transparente
					*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(0,r,g,b);
				else
				if(abs(b-b0)<ds1 && abs(g-g0)<ds1 && abs(r-r0)<ds1)		// es el mask gradiente
				{
					BYTE gradiente = 255*(abs(b-b0) + abs(g-g0) + abs(r-r0))/(3.*(float)ds1);

					*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(gradiente,r,g,b);
				}
				else
					// es pocap
					*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(255,r,g,b);

			}
		}
		g_pTexture->UnlockRect(i);
	}
    return S_OK;
}


/*
HRESULT SetAlphaChannel(LPDIRECT3DTEXTURE9  g_pTexture,BYTE alpha,BYTE mask_r,BYTE mask_g,BYTE mask_b)
{
	// Initialize the alpha channel
	// tengo que hacer el reemplazo en TODOS los mipmaps generados para 
	// esta textura
	D3DLOCKED_RECT lockedRect;
	UINT cant_mipmaps = g_pTexture->GetLevelCount();
	for(UINT i=0;i<cant_mipmaps;++i)
	if(SUCCEEDED(g_pTexture->LockRect(i, &lockedRect, NULL, D3DLOCK_DISCARD )))
	{
		D3DSURFACE_DESC desc;
		g_pTexture->GetLevelDesc(i,&desc);
		int m_dwWidth = desc.Width;
		int m_dwHeight = desc.Height;
		int pitch = lockedRect.Pitch / sizeof(DWORD);
		for( int y=0; y < m_dwHeight; y++ )
		{	
			int dwOffset = y*pitch;
			for( int x=0; x < m_dwWidth; x++ )
			{

				BYTE *color = (BYTE *)(((DWORD *)lockedRect.pBits)+(dwOffset+x));
				BYTE b = color[0];
				BYTE g = color[1];
				BYTE r = color[2];
				if(abs(b-mask_b)<10 && abs(g-mask_g)<10 && abs(r-mask_r)<10)		// es el mask transparente
					*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(0,r,g,b);
				else
				*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(alpha,r,g,b);

			}
		}
		g_pTexture->UnlockRect(i);
	}
    return S_OK;
}
*/

HRESULT SetAlphaChannel(LPDIRECT3DTEXTURE9  g_pTexture,BYTE alpha)
{
	// Initialize the alpha channel
	// tengo que hacer el reemplazo en TODOS los mipmaps generados para 
	// esta textura
	D3DLOCKED_RECT lockedRect;
	UINT cant_mipmaps = g_pTexture->GetLevelCount();
	for(UINT i=0;i<cant_mipmaps;++i)
	if(SUCCEEDED(g_pTexture->LockRect(i, &lockedRect, NULL, D3DLOCK_DISCARD )))
	{
		D3DSURFACE_DESC desc;
		g_pTexture->GetLevelDesc(i,&desc);
		int m_dwWidth = desc.Width;
		int m_dwHeight = desc.Height;
		int pitch = lockedRect.Pitch / sizeof(DWORD);
		for( int y=0; y < m_dwHeight; y++ )
		{	
			int dwOffset = y*pitch;
			for( int x=0; x < m_dwWidth; x++ )
			{

				BYTE *color = (BYTE *)(((DWORD *)lockedRect.pBits)+(dwOffset+x));
				BYTE b = color[0];
				BYTE g = color[1];
				BYTE r = color[2];
				*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(alpha,r,g,b);

			}
		}
		g_pTexture->UnlockRect(i);
	}
    return S_OK;
}

HRESULT DXEngine2d::LoadFx(char *fx_file)
{
	// primero busco el archivo en el directorio local
	ID3DXBuffer *pBuffer = NULL;
	HRESULT hr = D3DXCreateEffectFromFile( g_pd3dDevice, fx_file,
			NULL, NULL, D3DXFX_NOT_CLONEABLE, NULL, &g_pEffect, &pBuffer);
	if( FAILED(hr) )
	{
		char *saux = (char*)pBuffer->GetBufferPointer();
		AfxMessageBox(saux);
	}
	return hr;
}


// Libero toda la memoria de los objetos usados
void DXEngine2d::DXCleanup()
{

	if(g_pD3D==NULL)
		return;

	SAFE_RELEASE(pSprite);
	SAFE_RELEASE(ppLine);
    SAFE_RELEASE(g_pFont);
    SAFE_RELEASE(g_pFontb);
    SAFE_RELEASE(g_pFont2);

	// borro el buffer de vertices
	SAFE_RELEASE(g_pVB);
	SAFE_RELEASE(g_pQuad);
	SAFE_RELEASE(g_pLevel);

	// el dipositivo 
	SAFE_RELEASE(g_pd3dDevice );
	SAFE_RELEASE(g_pD3D);
	SAFE_RELEASE(g_pEffect);

}





// Render states
void DXEngine2d::SaveRenderStates()
{
	g_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &ant_zenable);
	g_pd3dDevice->GetRenderState( D3DRS_CULLMODE, &ant_cullmode);
	g_pd3dDevice->GetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, &ant_dmsource);
	g_pd3dDevice->GetRenderState( D3DRS_ALPHABLENDENABLE,&ant_alpha);
}

void DXEngine2d::RestoreRenderStates()
{

	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, ant_zenable);
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, ant_cullmode);
	g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, ant_dmsource);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,ant_alpha);

}





/////////////////////////////////////////////////////////////////////////////
// niveles (tiles)
/////////////////////////////////////////////////////////////////////////////
HRESULT DXEngine2d::LoadLevel(char tdx,char tdy)
{
	// Cargo el nivel:
	atlas = cargar_textura("atlas.bmp",-1);
	// Cargo el atlas de sprites
	sprites = cargar_textura("sprites.bmp",-1);
	// Animacion del fuego (implementado como otro sprite
	fuego = cargar_textura("fuego.bmp",-1);
	fuego_dx = 30;
	fuego_dy = 60;

	// Cargo las texturas pp dichas
	DXLoadTextures();

	// tamaño de los tiles
	tile_dx = tdx;
	tile_dy = tdy;

	D3DSURFACE_DESC desc;
	g_pTexture[atlas]->GetLevelDesc(0,&desc);
	atlas_dx = desc.Width;
	atlas_dy = desc.Height;
	tile_cant_col = atlas_dx/tile_dx;
	tile_cant_fil = atlas_dy/tile_dy;

	// Tamaño de los sprites (de momento fijos)
	sprite_dx = 24;
	sprite_dy = 36;
	// Seteo el canal alfa de los atlas 
	SetAlphaChannel(g_pTexture[atlas],0,0,0);
	SetAlphaChannel(g_pTexture[sprites],255,0,255);
	SetAlphaChannelGradient(g_pTexture[fuego],0,0,0,20,150);

	// esqueleto
	cant_vertebras = 0;
	vertebra[cant_vertebras++] = CPoint(7,0);			// Pie Izquierdo
	vertebra[cant_vertebras++] = CPoint(15,0);			// Pie Derecho
	vertebra[cant_vertebras++] = CPoint(11,10);			// tronco
	vertebra[cant_vertebras++] = CPoint(11,15);			// Cuello
	vertebra[cant_vertebras++] = CPoint(11,29);			// Cabeza
	vertebra[cant_vertebras++] = CPoint(6,15);			// Hombro Izquierdo
	vertebra[cant_vertebras++] = CPoint(16,15);			// Hombro Derecho
	vertebra[cant_vertebras++] = CPoint(2,8);			// Mano Izquierda
	vertebra[cant_vertebras++] = CPoint(19,8);			// Mano Derecha


	// inicializo con todo el atlas 
	for(int i=0;i<MAX_TILE_Y;++i)
		for(int j=0;j<MAX_TILE_X;++j)
		{
			C[i][j].flags = 0;
			C[i][j].nro_tile = 0;
		}


	// Creo los vertices de la parte que se ve en pantalla
	int cant_v = cant_fil*cant_col*2*3;
	SAFE_RELEASE(g_pQuad);
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( cant_v*sizeof(QUADVERTEX),
                                                  0, D3DFVF_QUADVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pQuad, NULL ) ) )
    {
        return E_FAIL;
    }

	// Creo los vertices de todo el nivel (para el preview)
	cant_v = MAX_TILE_X*MAX_TILE_Y*2*3;
	SAFE_RELEASE(g_pLevel);
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( cant_v*sizeof(QUADVERTEX),
                                                  0, D3DFVF_QUADVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pLevel, NULL ) ) )
    {
        return E_FAIL;
    }


	LoadPreviewLevel();
    return S_OK;

}

// Devuelve el tile correspondiente a la fila i, col j, teniendo en cuenta si hay animacion
int DXEngine2d::que_tile(int i,int j)
{

	if(i>=MAX_TILE_Y || j>=MAX_TILE_X)
		return 1009;			// vacio

	int n = C[i][j].nro_tile;

	if(C[i][j].tipo==TILE_PISO_INTERMITENTE || C[i][j].tipo==TILE_CADENA)
	{
		tile_intermitente *ti = &tiles_intermitentes[C[i][j].idata];
		if(ti->timer>ti->tp)
			// esta apagado:
			n = 1009;		// negro

		// si esta prendido, dejo el nro de tile como estaba,
		// probablemente sea una animacion
	}

	if(n>=10000)
	{
		// es una animacion
		float x = ftime * 5;
		float r = x - (int)x;			// resto en float [0,1)
		int t = cant_animaciones[n-10000]*r;
		n = tiles_animados[n-10000][t];
	}

	return n;
}



HRESULT DXEngine2d::Render(int x0,int y0,float ex,float ey,BOOL preview)
{
	// Quad para postprocess 2d
	int ox = 2;
	int oy = 2;

	int cant_v = cant_fil*cant_col*2*3;
	int size_vert = sizeof(QUADVERTEX)*cant_v;
    QUADVERTEX vertices[MAX_TILE_X_PAN*2*3];
    // Leer esto en el manual del directX : "Directly Mapping Texels to Pixels"
    float desf = 0.5;
	float du = (float)tile_dx/(float)atlas_dx;
	float dv = (float)tile_dy/(float)atlas_dy;

	// Correccion de escala
	// la pantalla tiene cant_fil x cant_col, y eso ocupa screen_dy x screen_dx pixels, 
	float kx = (float)screen_dx / (float)(cant_col*tile_dx);		// si coincide exacto kx = 1, por ejemplo 40 col x 8 = 320 px
	float ky = (float)screen_dy / (float)(cant_fil*tile_dy);		// si coincide exacto ky = 1, por ejemplo 25 col x 8 = 200 px

	// ajusto la escala
	ex*=kx;
	ey*=ky;

	int j0 = x0/tile_dx;
	float desf_x = ex*(x0%tile_dx) - desf;
	int i0 = y0/tile_dy;
	float desf_y = ey*(y0%tile_dy) - desf;


	int cant_fuegos = 0;
	int fuego_fil[100];
	int fuego_col[100];


	int t = 0;
	for(int i=0;i<cant_fil;++i)
	{
		for(int j=0;j<cant_col;++j)
		{

			if(C[i0+i][j0+j].tipo==TILE_FUEGO)
			{
				fuego_fil[cant_fuegos] = i0+i;
				fuego_col[cant_fuegos] = j0+j;
				++cant_fuegos;
			}


			int n = que_tile(i0+i,j0+j);
			int fil = n / tile_cant_col;
			int col = n % tile_cant_col;

			float u = (float)col*du;
			float v = (float)fil*dv;

			vertices[t].pos = D3DXVECTOR4(ox + ex*j*tile_dx-desf_x, oy + ey*i*tile_dy-desf_y, 0.0f, 1.0f);
			vertices[t].tu = u;
			vertices[t].tv = v;
			++t;

			vertices[t].pos = D3DXVECTOR4(ox + ex*(j+1)*tile_dx-desf_x,oy +  ey*i*tile_dy-desf_y, 0.0f, 1.0f);
			vertices[t].tu = u+du;
			vertices[t].tv = v;
			++t;

			vertices[t].pos = D3DXVECTOR4(ox + ex*(j+1)*tile_dx-desf_x,oy +  ey*(i+1)*tile_dy-desf_y, 0.0f, 1.0f);
			vertices[t].tu = u+du;
			vertices[t].tv = v+dv;
			++t;

			
			vertices[t] = vertices[t-3];
			vertices[t+1] = vertices[t-1];
			vertices[t+2].pos = D3DXVECTOR4(ox + ex*j*tile_dx-desf_x,oy +  ey*(i+1)*tile_dy-desf_y, 0.0f, 1.0f);
			vertices[t+2].tu = u;
			vertices[t+2].tv = v+dv;
			t+=3;
		}
	}



    VOID* pQuad;
    if( FAILED( g_pQuad->Lock( 0, size_vert, (void**)&pQuad, 0 ) ) )
        return E_FAIL;
    memcpy( pQuad, vertices, size_vert );
    g_pQuad->Unlock();

	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0), 1, 0 );
	if( SUCCEEDED(g_pd3dDevice->BeginScene()))
	{

		g_pd3dDevice->SetStreamSource( 0, g_pQuad, 0, sizeof(QUADVERTEX));
		g_pd3dDevice->SetFVF( D3DFVF_QUADVERTEX);
		g_pd3dDevice->SetTexture( 0, g_pTexture[atlas]);


		D3DVIEWPORT9 viewport,viewport_ant;
		g_pd3dDevice->GetViewport(&viewport_ant);
		viewport.MaxZ = 1;
		viewport.MinZ = 0;
		viewport.X = ox;
		viewport.Y = oy;
		viewport.Width = ox + screen_dx*ex;
		viewport.Height = oy + screen_dy*ey;
		g_pd3dDevice->SetViewport(&viewport);

		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, 2*cant_fil*cant_col);
		g_pd3dDevice->SetViewport(&viewport_ant);


		if(preview)
		{
			// Render Preview del Nivel
			g_pd3dDevice->SetStreamSource( 0, g_pLevel, 0, sizeof(QUADVERTEX));
			for(int i=0;i<MAX_TILE_Y;++i)
				g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, i*6*MAX_TILE_X, 2*MAX_TILE_X);
		}

		// dibujo el sprite
		if(sprite_sel!=-1)
		{
			int x = ox + (pos_x - j0*tile_dx)*ex;
			int y = oy + (pos_y - (i0+3)*tile_dy)*ey;

			// Determino que sprite dibujar: 
			int nro_sprite;
			// caso particular: se esta muriendo en el piso:
			if(timer_caida)
				nro_sprite = 4 + (int)(10*timer_caida) % 5;
			else
			if(timer_choco)
				nro_sprite = 4 + (int)(10*timer_choco) % 5;
			else
			// pos. del sprite
			//int pos_j = nearest_x(pos_x);
			//int pos_i = nearest_y(pos_y);
			switch(status)
			{
				case P_EN_ESCALERA:
					sentido = 0;
					nro_sprite = (sprite_sel%2) + (int)(512/sprite_dx);
					break;
				case P_EN_TUBO:
					sentido = 0;
					nro_sprite = 3 + (int)(512/sprite_dx);
					break;
				case P_EN_SOGA:
					sentido = 0;
					nro_sprite = 2 + (sprite_sel%2) + (int)(512/sprite_dx);
					break;
				case P_SALTANDO:
				case P_CAYENDO:
					nro_sprite = 2*(int)(512/sprite_dx);
					break;

				default:
					// secuencia de caminar: 
					nro_sprite = sprite_sel%4;
					break;
			}

			if(timer_quema)
			{
				y-=(2-timer_quema)*6*tile_dy*ey;
				nro_sprite = 4 + (int)(10*timer_quema) % 5;
				XplodeSprite((2-timer_quema)*0.05,x,y,nro_sprite,ex,ey);
				RenderSprite(x,y-sprite_dy/2*ey,(int)(timer_quema*16),ex,ey,fuego_dx,fuego_dy,fuego);
				RenderSprite(x,y+fuego_dy*ey+sprite_dy/2*ey,(int)(timer_quema*16),ex,-ey,fuego_dx,fuego_dy,fuego);
			}
			else
			if(timer_cadena)
			{
				if(sentido==1)
					// Espejo X
					XplodeSprite(1-timer_cadena/2,x+2*tile_dx*ex,y,nro_sprite,-ex,ey);
				else
					XplodeSprite(1-timer_cadena/2,x,y,nro_sprite,ex,ey);
			}
			else
			{
				if(sentido==1)
					// Espejo X
					RenderSprite(x+2*tile_dx*ex,y,nro_sprite,-ex,ey);
				else
					RenderSprite(x,y,nro_sprite,ex,ey);
			}
		}

		// dibujo los enemigos
		for(int i=0;i<cant_enemigos;++i)
		{

			int x = ox + (enemigo[i].pos_x - j0*tile_dx)*ex;
			int y = oy + (enemigo[i].pos_y - (i0+3)*tile_dy)*ey;

			if(timer_choco && i==enemigo_sel)
				XplodeSprite(2-timer_choco, x,y,enemigo[i].nro_sprite[0],ex,ey);
			else
				RenderSprite(x,y,enemigo[i].nro_sprite[enemigo[i].psprite],ex,ey);
		}


		// Dibujo los fuegos
		if(cant_fuegos)
		{
			for(int i=0;i<cant_fuegos;++i)
			{
				int x = ox + (fuego_col[i]-j0)*tile_dx*ex - fuego_dx*0.3*ex;
				int y = oy + (fuego_fil[i]-i0+1)*tile_dy*ey - fuego_dy*0.9*ey;
				RenderSprite(x,y,(frame_fuego + i*11)%33,ex,ey,fuego_dx,fuego_dy,fuego);
			}
		}

		if(cant_items)
		{
			// dibujo los items coleccionables
			int x = ox + 10*ex;
			int y = oy + 2*ey;
			for(i =0;i<cant_items;++i)
			{
				switch(items[i])
				{
					case TILE_LLAVE_BLANCA:
						RenderTile(x,y,tile_dx*ex,tile_dy*ey,967);
						RenderTile(x+tile_dx*ex,y,tile_dx*ex,tile_dy*ey,970);
						RenderTile(x,y+tile_dy*ey,tile_dx*ex,tile_dy*ey,966);
						RenderTile(x+tile_dx*ex,y+tile_dy*ey,tile_dx*ex,tile_dy*ey,969);
						break;
					case TILE_LLAVE_ROJA:
						RenderTile(x,y,tile_dx*ex,tile_dy*ey,888);
						RenderTile(x+tile_dx*ex,y,tile_dx*ex,tile_dy*ey,890);
						RenderTile(x,y+tile_dy*ey,tile_dx*ex,tile_dy*ey,887);
						RenderTile(x+tile_dx*ex,y+tile_dy*ey,tile_dx*ex,tile_dy*ey,889);
						break;
					case TILE_LLAVE_AZUL:
						RenderTile(x,y,tile_dx*ex,tile_dy*ey,1012);
						RenderTile(x+tile_dx*ex,y,tile_dx*ex,tile_dy*ey,1014);
						RenderTile(x,y+tile_dy*ey,tile_dx*ex,tile_dy*ey,1011);
						RenderTile(x+tile_dx*ex,y+tile_dy*ey,tile_dx*ex,tile_dy*ey,1013);
						break;
					case TILE_ESPADA:
						RenderTile(x,y,tile_dx*ex,tile_dy*ey,975);
						RenderTile(x+tile_dx*ex,y,tile_dx*ex,tile_dy*ey,978);
						RenderTile(x,y+tile_dy*ey,tile_dx*ex,tile_dy*ey,974);
						RenderTile(x+tile_dx*ex,y+tile_dy*ey,tile_dx*ex,tile_dy*ey,977);
						break;

				}

				x += (tile_dx+10)*ex;
			}
		}


		{
			// dibujo las vidas
			int x = ox + 360*ex;
			int y = oy + 2*ey;
			for(i =0;i<cant_vidas;++i)
			{
				RenderSprite(x,y,0,ex,ey);
				x += 24*ex;
			}
		}


		g_pd3dDevice->EndScene();
	}



	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

	return S_OK;

}


HRESULT DXEngine2d::RenderTile(int x0,int y0,int dx,int dy,int sel)
{
	LPDIRECT3DVERTEXBUFFER9 g_pQuadAux;
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 4*sizeof(QUADVERTEX),
                                                  0, D3DFVF_QUADVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pQuadAux, NULL ) ) )
    {
        return E_FAIL;
    }


    QUADVERTEX vertices[4];
    float desf = 0.5f;
	float du = (float)tile_dx / (float)atlas_dx;
	float dv = (float)tile_dy / (float)atlas_dy;

	int i = sel / tile_cant_col;
	int j = sel % tile_cant_col;

	float u = (float)j*du;
	float v = (float)i*dv;

	vertices[0].pos = D3DXVECTOR4(x0-desf, y0-desf, 0.0f, 1.0f);
	vertices[0].tu = u;
	vertices[0].tv = v;

	vertices[1].pos = D3DXVECTOR4(x0+dx-desf, y0-desf, 0.0f, 1.0f);
	vertices[1].tu = u+du;
	vertices[1].tv = v;
	
	vertices[3].pos = D3DXVECTOR4(x0+dx-desf, y0+dy-desf, 0.0f, 1.0f);
	vertices[3].tu = u+du;
	vertices[3].tv = v+dv;

	vertices[2].pos = D3DXVECTOR4(x0-desf, y0+dy-desf, 0.0f, 1.0f);
	vertices[2].tu = u;
	vertices[2].tv = v+dv;

    VOID* pQuad;
    if( FAILED( g_pQuadAux->Lock( 0, sizeof(vertices), (void**)&pQuad, 0 ) ) )
        return E_FAIL;
    memcpy( pQuad, vertices, sizeof(vertices));
    g_pQuadAux->Unlock();

	g_pd3dDevice->SetStreamSource( 0, g_pQuadAux, 0, sizeof(QUADVERTEX));
	g_pd3dDevice->SetFVF( D3DFVF_QUADVERTEX);
	g_pd3dDevice->SetTexture( 0, g_pTexture[atlas]);

	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);
	SAFE_RELEASE(g_pQuadAux);
	return S_OK;

}

HRESULT DXEngine2d::RenderTileVP(int x0,int y0,int dx,int dy,int sel)
{
	LPDIRECT3DVERTEXBUFFER9 g_pQuadAux;
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 4*sizeof(QUADVERTEX),
                                                  0, D3DFVF_QUADVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pQuadAux, NULL ) ) )
    {
        return E_FAIL;
    }


    QUADVERTEX vertices[4];
    float desf = 0.5f;
	float du = (float)tile_dx / (float)atlas_dx;
	float dv = (float)tile_dy / (float)atlas_dy;

	int i = sel / tile_cant_col;
	int j = sel % tile_cant_col;

	float u = (float)j*du;
	float v = (float)i*dv;

	vertices[0].pos = D3DXVECTOR4(x0-desf, y0-desf, 0.0f, 1.0f);
	vertices[0].tu = u;
	vertices[0].tv = v;

	vertices[1].pos = D3DXVECTOR4(x0+dx-desf, y0-desf, 0.0f, 1.0f);
	vertices[1].tu = u+du;
	vertices[1].tv = v;
	
	vertices[3].pos = D3DXVECTOR4(x0+dx-desf, y0+dy-desf, 0.0f, 1.0f);
	vertices[3].tu = u+du;
	vertices[3].tv = v+dv;

	vertices[2].pos = D3DXVECTOR4(x0-desf, y0+dy-desf, 0.0f, 1.0f);
	vertices[2].tu = u;
	vertices[2].tv = v+dv;

    VOID* pQuad;
    if( FAILED( g_pQuadAux->Lock( 0, sizeof(vertices), (void**)&pQuad, 0 ) ) )
        return E_FAIL;
    memcpy( pQuad, vertices, sizeof(vertices));
    g_pQuadAux->Unlock();

	if( SUCCEEDED(g_pd3dDevice->BeginScene()))
	{

		D3DVIEWPORT9 viewport,viewport_ant;
		g_pd3dDevice->GetViewport(&viewport_ant);
		viewport.MaxZ = 1;
		viewport.MinZ = 0;
		viewport.X = x0;
		viewport.Y = y0;
		viewport.Width = dx;
		viewport.Height = dy;
		g_pd3dDevice->SetViewport(&viewport);
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(128,128,128), 1, 0 );
		g_pd3dDevice->SetStreamSource( 0, g_pQuadAux, 0, sizeof(QUADVERTEX));
		g_pd3dDevice->SetFVF( D3DFVF_QUADVERTEX);
		g_pd3dDevice->SetTexture( 0, g_pTexture[atlas]);

		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);
		g_pd3dDevice->SetViewport(&viewport_ant);
		g_pd3dDevice->EndScene();
	}

	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
	SAFE_RELEASE(g_pQuadAux);
	return S_OK;
}



HRESULT DXEngine2d::LoadPreviewLevel()
{
	// Quad para postprocess 2d
	int cant_vert = MAX_TILE_X*MAX_TILE_Y*2*3;
	int size_vert = sizeof(QUADVERTEX)*cant_vert;
    QUADVERTEX *vertices = new QUADVERTEX[size_vert];
    float desf = 0.5f;
	float du = (float)tile_dx / (float)atlas_dx;
	float dv = (float)tile_dx / (float)atlas_dx;
	int ox = 500;
	int oy = 2;
	float ex = (float)(1024 - ox)/(float)(MAX_TILE_X*tile_dx);
	float ey = (float)(600 - oy)/(float)(MAX_TILE_Y*tile_dy);

	int t = 0;
	for(int i=0;i<MAX_TILE_Y;++i)
	{
		for(int j=0;j<MAX_TILE_X;++j)
		{

			int n = que_tile(i,j);
			int fil = n / tile_cant_col;
			int col = n % tile_cant_col;

			float u = (float)col * du;
			float v = (float)fil * dv;

			vertices[t].pos = D3DXVECTOR4(ox + ex*j*tile_dx - desf, oy + ey*i*tile_dy- desf, 0.0f, 1.0f);
			vertices[t].tu = u;
			vertices[t].tv = v;
			++t;

			vertices[t].pos = D3DXVECTOR4(ox + ex*(j+1)*tile_dx- desf, oy + ey*i*tile_dy- desf, 0.0f, 1.0f);
			vertices[t].tu = u+du;
			vertices[t].tv = v;
			++t;

			vertices[t].pos = D3DXVECTOR4(ox + ex*(j+1)*tile_dx- desf, oy + ey*(i+1)*tile_dy- desf, 0.0f, 1.0f);
			vertices[t].tu = u+du;
			vertices[t].tv = v+dv;
			++t;

			
			vertices[t] = vertices[t-3];
			vertices[t+1] = vertices[t-1];
			vertices[t+2].pos = D3DXVECTOR4(ox + ex*j*tile_dx- desf, oy + ey*(i+1)*tile_dy- desf, 0.0f, 1.0f);
			vertices[t+2].tu = u;
			vertices[t+2].tv = v+dv;
			t+=3;
		}
	}

    QUADVERTEX * pQuad;
    if( FAILED( g_pLevel->Lock( 0, size_vert, (void**)&pQuad, 0 ) ) )
        return E_FAIL;
    memcpy( pQuad, vertices, size_vert);
    
	g_pLevel->Unlock();
	delete vertices;
	return S_OK;

}


HRESULT DXEngine2d::ReloadPreviewLevel(int i,int j)
{
    QUADVERTEX vertices[6];
    float desf = 0.5f;
	float du = (float)tile_dx / (float)atlas_dx;
	float dv = (float)tile_dx / (float)atlas_dx;
	int ox = 500;
	int oy = 2;
	float ex = (1024 - ox)/(MAX_TILE_X*tile_dx);
	float ey = (600 - oy)/(MAX_TILE_Y*tile_dy);
	/*if(ex<ey)
		ey = ex;
	else
		ex = ey;
	*/


	int n = que_tile(i,j);
	int fil = n / tile_cant_col;
	int col = n % tile_cant_col;
	int t = 0;
	float u = (float)col*du;
	float v = (float)fil*dv;

	vertices[t].pos = D3DXVECTOR4(ox + ex*j*tile_dx - desf, oy + ey*i*tile_dy- desf, 0.0f, 1.0f);
	vertices[t].tu = u;
	vertices[t].tv = v;
	++t;

	vertices[t].pos = D3DXVECTOR4(ox + ex*(j+1)*tile_dx- desf, oy + ey*i*tile_dy- desf, 0.0f, 1.0f);
	vertices[t].tu = u+du;
	vertices[t].tv = v;
	++t;

	vertices[t].pos = D3DXVECTOR4(ox + ex*(j+1)*tile_dx- desf, oy + ey*(i+1)*tile_dy- desf, 0.0f, 1.0f);
	vertices[t].tu = u+du;
	vertices[t].tv = v+dv;
	++t;

	
	vertices[t] = vertices[t-3];
	vertices[t+1] = vertices[t-1];
	vertices[t+2].pos = D3DXVECTOR4(ox + ex*j*tile_dx- desf, oy + ey*(i+1)*tile_dy- desf, 0.0f, 1.0f);
	vertices[t+2].tu = u;
	vertices[t+2].tv = v+dv;
	t+=3;


	int offset = (i*MAX_TILE_X + j)*6*sizeof(QUADVERTEX);
    VOID* pQuad;
    if( FAILED( g_pLevel->Lock( offset, sizeof(vertices), (void**)&pQuad, 0 ) ) )
        return E_FAIL;
    memcpy( pQuad, vertices, sizeof(vertices));
	g_pLevel->Unlock();
	return S_OK;

}



// formato string para iphone
void DXEngine2d::grabar_mapa(char *fname)
{
	FILE *fp = fopen(fname,"wt");

	if(!fp)
		return;

	fprintf(fp,"<MAPA>\n");
	for(int i=0;i<MAX_TILE_Y;++i)
		for(int j=0;j<MAX_TILE_X;++j)
		{
			fprintf(fp,"%d\n",C[i][j].nro_tile);
			fprintf(fp,"%d\n",C[i][j].flags);
			fprintf(fp,"%d\n",C[i][j].tipo);
		}
	fprintf(fp,"</MAPA>\n");
	fclose(fp);
}


void DXEngine2d::cargar_mapa(char *fname)
{
	FILE *fp = fopen(fname,"rT");

	if(!fp)
		return;

	memset(C,0,sizeof(C));
	char buffer[255];
	fgets(buffer,sizeof(buffer),fp);		// flag MAPA

	for(int i=0;i<MAX_TILE_Y;++i)
	{
		for(int j=0;j<MAX_TILE_X;++j)
		{
			fgets(buffer,sizeof(buffer),fp);
			C[i][j].nro_tile = atoi(buffer);
			
			fgets(buffer,sizeof(buffer),fp);
			C[i][j].flags = atoi(buffer);

			fgets(buffer,sizeof(buffer),fp);
			C[i][j].tipo = atoi(buffer);

		}
	}
	fgets(buffer,sizeof(buffer),fp);		// flag /MAPA
	fclose(fp);

	
	// Post proceso los tiles:
	// 1era pasada: determino el flag y el tipo de tiple
	for(i=0;i<MAX_TILE_Y;++i)
	{
		for(int j=0;j<MAX_TILE_X;++j)
		if(C[i][j].tipo!=TILE_FUEGO)	// hardcodeo el fuego
		{

			int ntile = C[i][j].nro_tile;
			int flags = 0;
			char tipo;
			switch(ntile)
			{
				case 1009:
					// bloque negro vacio
					flags = 0;
					tipo = TILE_VACIO;
					break;
				case 1019:
				case 1021:
					// escalera
					flags = 0;
					tipo = TILE_ESCALERA;
					break;
				case 800:
				case 812:
				case 10001:
					// cinta transportadora - extremo izquierdo
					flags = F_PISO;
					tipo = TILE_CINTA;
					C[i][j].nro_tile = 10001;
					break;
				case 810:
				case 814:
				case 10002:
					// cinta transportadora - extremo derecho
					flags = F_PISO;
					tipo = TILE_CINTA;
					C[i][j].nro_tile = 10002;
					break;
				case 858:
				case 809:
				case 10000:
					// cinta transportadora - parte central
					flags = F_PISO;
					tipo = TILE_CINTA;
					// reemplazo el tile 858 x la animacion 0
					C[i][j].nro_tile = 10000;
					break;

				case 811:
				case 10005:
					// cinta transportadora IZQUIERDA - extremo izquierdo
					flags = F_PISO;
					tipo = TILE_CINTA_IZQ;
					C[i][j].nro_tile = 10005;
					break;
				case 813:
				case 10006:
					// cinta transportadora - extremo derecho
					flags = F_PISO;
					tipo = TILE_CINTA_IZQ;
					C[i][j].nro_tile = 10006;
					break;
				case 799:
				case 10004:
					// cinta transportadora - parte central
					flags = F_PISO;
					tipo = TILE_CINTA_IZQ;
					C[i][j].nro_tile = 10004;
					break;

				case 852:
				case 854:
				case 900:
				case 901:
				case 817:
				case 819:
				case 937:
				case 938:
				case 929:
				case 930:
				case 976:
				case 979:
				case 1023:
				case 1024:
				case 1032:
				case 968:
					// escalera c/ piso
					flags = F_PISO;
					tipo = TILE_ESCALERA;
					break;
				default:
					flags = F_PIEDRA;
					tipo = TILE_LADRILLO;
					break;

				case 1031:
				case 112:
				case 154:
				case 196:
				case 238:
					// puerta blanca
					flags = F_PARED_D|F_PARED_I|F_PUERTA;
					tipo = TILE_PTA_BLANCA;
					break;

				case 971:
				case 113:
				case 155:
				case 197:
				case 239:
					// puerta azul
					flags = F_PARED_D|F_PARED_I|F_PUERTA;
					tipo = TILE_PTA_AZUL;
					break;

				case 114:
				case 156:
				case 198:
				case 240:
					// puerta roja
					flags = F_PARED_D|F_PARED_I|F_PUERTA;
					tipo = TILE_PTA_ROJA;
					break;

				case 892:
					// tubo (cae lento)
					flags = F_CENTRO_V;
					tipo = TILE_TUBO;
					break;
				case 942:
					// soga (que permite trepar, y bajar)
					flags = F_CENTRO_V;
					tipo = TILE_SOGA;
					break;
				case 943:
					// parte de arriba de la soga 
					flags = F_CENTRO_V | F_PISO;
					tipo = TILE_SOGA;
					break;

				case 939:
					// portal intermitente
					flags = 0;
					tipo = TILE_VACIO;
					break;

				case 822:
				case 842:
				case 843:
				case 863:
				case 763:
				case 10003:
				case 10008:
					// piso que titila:
					flags = F_PISO;
					tipo = TILE_PISO_INTERMITENTE;
					// Agrego el tile en la lista
					tiles_intermitentes[cant_intermitentes].fil = i;
					tiles_intermitentes[cant_intermitentes].col = j;
					tiles_intermitentes[cant_intermitentes].tp = 2;
					tiles_intermitentes[cant_intermitentes].ta = 1;
					// Para evitar que todos queden sincronizados igual.
					// le aplico un desf. de tiempo segun la fila
					if(ntile==863)
					{
						tiles_intermitentes[cant_intermitentes].timer = ((i+1)%2)*1.0;
						// reemplazo el tile 863 x la animacion 8
						C[i][j].nro_tile = 10008;
					}
					else
					{
						tiles_intermitentes[cant_intermitentes].timer = (i%2)*1.0;
						// reemplazo el tile 822 x la animacion 3
						C[i][j].nro_tile = 10003;
					}

					C[i][j].idata = cant_intermitentes++;
					break;

				case 967:
				case 966:
				case 970:
				case 969:
					flags = F_PICKABLE_ITEM;
					tipo = TILE_LLAVE_BLANCA;
					break;

				case 888:
				case 890:
				case 887:
				case 889:
					flags = F_PICKABLE_ITEM;
					tipo = TILE_LLAVE_ROJA;
					break;

				case 1012:
				case 1014:
				case 1011:
				case 1013:
					flags = F_PICKABLE_ITEM;
					tipo = TILE_LLAVE_AZUL;
					break;

				case 925:
				case 928:
				case 924:
				case 927:
					flags = F_SCORE_ITEM;
					tipo = TILE_DIAMANTE_AZUL;
					break;

				case 948:
				case 885:
				case 946:
				case 883:
					flags = F_SCORE_ITEM;
					tipo = TILE_DIAMANTE_VERDE;
					break;

				case 882:
				case 886:
				case 769:
				case 770:
					flags = F_SCORE_ITEM;
					tipo = TILE_DIAMANTE_AMARILLO;
					break;

				case 975:
				case 978:
				case 974:
				case 977:
					flags = F_PICKABLE_ITEM;
					tipo = TILE_ESPADA;
					break;

				case 947:
				case 884:
				case 760:
				case 761:
					flags = F_SCORE_ITEM;
					tipo = TILE_JARRO;
					break;

				case 859:
				case 860:
				case 902:
				case 904:
					flags = F_PICKABLE_ITEM;
					tipo = TILE_ANTORCHA;
					break;
				
				case 804:
				case 808:
				case 803:
				case 807:
					flags = F_PICKABLE_ITEM;
					tipo = TILE_ANTORCHA_GRANDE;
					break;

				case 774:
				case 936:
				case 944:
				case 10007:
					// cadena que se mueve
					flags = F_PARED_D|F_PARED_I;
					tipo = TILE_CADENA;
					// Agrego el tile en la lista
					tiles_intermitentes[cant_intermitentes].fil = i;
					tiles_intermitentes[cant_intermitentes].col = j;
					tiles_intermitentes[cant_intermitentes].tp = 2;
					tiles_intermitentes[cant_intermitentes].ta = 1;
					tiles_intermitentes[cant_intermitentes].timer = (j%2)*0.5;
					C[i][j].idata = cant_intermitentes++;
					C[i][j].nro_tile = 10007;
					break;



			}
			C[i][j].flags = flags;
			C[i][j].tipo = tipo;

		}
	}


	// 2da pasada: reemplazo grupos de 4 ladrillos por un bloque mas grande
	// y reemplazo
	for(i=0;i<MAX_TILE_Y;++i)
	{
		for(int j=0;j<MAX_TILE_X;++j)
		{

			int ntile = C[i][j].nro_tile;
			switch(ntile)
			{
				case 840:
					if(C[i+1][j].nro_tile==840 && C[i+1][j+1].nro_tile==840 && C[i][j+1].nro_tile==840)
					{
						C[i][j].nro_tile = 25;
						C[i][j+1].nro_tile = 26;
						C[i+1][j].nro_tile = 25 + 42;
						C[i+1][j+1].nro_tile = 26 + 42;
					}
					else
					if(C[i][j+1].nro_tile==840)
					{
						C[i][j].nro_tile = 25;
						C[i][j+1].nro_tile = 26;
					}
					else
					if(C[i+1][j].nro_tile==840)
					{
						C[i][j].nro_tile = 25;
						C[i+1][j].nro_tile = 25 + 42;
					}
					else
						C[i][j].nro_tile = 25;
					break;
			}
		}
	}

}




// Ojo que esta rutina es solo para importar. Una vez que este todo bien hardcodeado
// tiene que haber un mapa.dat con todo los tiles bien cargados.
// Y esta funcion no se va a usar mas.

void DXEngine2d::importar(char *fname)
{


	FILE *fp = fopen(fname,"rt");

	if(!fp)
		return;

	char buffer[255];
	fgets(buffer,255,fp);		// nro de version
	fgets(buffer,255,fp);		// filas x columnas

	int i = 0;
	int j = 0;
	char c;
	char saux[20];
	int t = 0;
	int nro_tile;
	while((c =fgetc(fp))!=EOF)
	{
		if(c==',' || c=='\n')
		{
			saux[t] = '\0';
			t = 0;
			nro_tile = atoi(saux);

			// transformo el nro de tile, de un mapa a otro:
			int fil = 24 - nro_tile / 25;
			int col = nro_tile % 25;
			int ntile = fil*tile_cant_col + col;
			C[250-i-1][j].nro_tile = ntile;

			int flags = 0;
			char tipo = TILE_VACIO;
			C[250-i-1][j].flags = flags;
			C[250-i-1][j].tipo = tipo;

			if(c==',')
				++j;
			else
			{
				j = 0;
				++i;
			}
		}
		else
			saux[t++] = c;
	}

	fclose(fp);



}

void DXEngine2d::cargar_escenario(char *fname)
{

	// Primero cargo las animaciones
	// cinta transportadora
	// parte central
	tiles_animados[0][0] = 858;
	tiles_animados[0][1] = 2;
	tiles_animados[0][2] = 1;
	tiles_animados[0][3] = 0;
	cant_animaciones[0] = 4;
	// izquierda
	tiles_animados[1][0] = 800;
	tiles_animados[1][1] = 5;
	tiles_animados[1][2] = 4;
	tiles_animados[1][3] = 3;
	cant_animaciones[1] = 4;
	// derecha
	tiles_animados[2][0] = 810;
	tiles_animados[2][1] = 6;
	tiles_animados[2][2] = 7;
	tiles_animados[2][3] = 8;
	cant_animaciones[2] = 4;

	// piso intermitente
	tiles_animados[3][0] = 822;
	tiles_animados[3][1] = 9;
	tiles_animados[3][2] = 10;
	tiles_animados[3][3] = 11;
	cant_animaciones[3] = 4;


	// cinta transportadora que gira en sentido anti-horario
	// parte central
	tiles_animados[4][3] = 858;
	tiles_animados[4][2] = 2;
	tiles_animados[4][1] = 1;
	tiles_animados[4][0] = 0;
	cant_animaciones[4] = 4;
	// izquierda
	tiles_animados[5][3] = 800;
	tiles_animados[5][2] = 5;
	tiles_animados[5][1] = 4;
	tiles_animados[5][0] = 3;
	cant_animaciones[5] = 4;
	// derecha
	tiles_animados[6][3] = 810;
	tiles_animados[6][2] = 6;
	tiles_animados[6][1] = 7;
	tiles_animados[6][0] = 8;
	cant_animaciones[6] = 4;

	// cadena que se mueve
	tiles_animados[7][0] = 936;
	tiles_animados[7][1] = 944;
	cant_animaciones[7] = 2;

	// piso intermitente secundario
	tiles_animados[8][0] = 863;
	tiles_animados[8][1] = 9;
	tiles_animados[8][2] = 10;
	tiles_animados[8][3] = 11;
	cant_animaciones[8] = 4;



	// Cargo los enemigos
	cant_enemigos = 0;
	// Calavera nivel cero
	enemigo[0].Create(this,CPoint(372*tile_dx-1,45*tile_dy-1),
			63,64,65,66,67,-1);
	cant_enemigos++;

	// cargo el mapa pp dicho
	cargar_mapa(fname);
}



void DXEngine2d::grabar_tile(int sel[],int cant_f,int cant_c)
{
	HRESULT rta;
	RECT rc_tile = {0, 0, tile_dx, tile_dy};
	// Creo una textura que abarque todos los tiles seleccionados
	LPDIRECT3DTEXTURE9 texture;
	rta = D3DXCreateTexture(g_pd3dDevice, tile_dx*cant_c, tile_dy*cant_f, 1, 
			NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture);
	LPDIRECT3DSURFACE9 pSurfaceOut;
	texture->GetSurfaceLevel(0, &pSurfaceOut);

	int t = 0;
	for(int i=0;i<cant_f;++i)
		for(int j=0;j<cant_c;++j)
		{
			int ii = sel[t] / tile_cant_col;
			int jj = sel[t] % tile_cant_col;

			// Acceso a la memoria del atlas, tomando el rect. que corresponde al tile ii,jj
			RECT rc = {jj*tile_dx,ii*tile_dy,(jj+1)*tile_dx,(ii+1)*tile_dy};
			D3DLOCKED_RECT lockedRect;
			rta = g_pTexture[atlas]->LockRect(0, &lockedRect,&rc, D3DLOCK_DISCARD);
			
			// Y lo copio en la textura, en la posicion i,j
			RECT rc2 = {j*tile_dx, i*tile_dy, (j+1)*tile_dx, (i+1)*tile_dy};
			rta = D3DXLoadSurfaceFromMemory(pSurfaceOut,NULL,&rc2,lockedRect.pBits,D3DFMT_A8R8G8B8,
					lockedRect.Pitch,NULL,&rc_tile,D3DX_FILTER_NONE, 0);
  
			g_pTexture[atlas]->UnlockRect(0);

			// paso al siguiente tile
			++t;
		}

	// Grabo la textura pp dicha:
	rta = D3DXSaveTextureToFile("tile.bmp",D3DXIFF_BMP ,texture,NULL);
	
	// libero la superficie y la textura auxiliar
	SAFE_RELEASE( pSurfaceOut);
	SAFE_RELEASE( texture);
	
}


void DXEngine2d::update_tile(int sel[],int cant_f,int cant_c)
{
	HRESULT rta;
	RECT rc_tile = {0, 0, tile_dx, tile_dy};


	// 
	LPDIRECT3DTEXTURE9 texture;
	rta = D3DXCreateTextureFromFileEx( g_pd3dDevice, "tile.bmp", 
			D3DX_DEFAULT_NONPOW2,    D3DX_DEFAULT_NONPOW2,    
                            1,    // no mip mapping
                            NULL,    // regular usage
                            D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                            D3DPOOL_MANAGED,    // typical memory handling
                            D3DX_DEFAULT,    // no filtering
                            D3DX_DEFAULT,    // no mip filtering
                            0,
                            NULL,    // no image info struct
                            NULL,    // not using 256 colors
							&texture);
				
	D3DSURFACE_DESC desc;
	texture->GetLevelDesc(0,&desc);
	int m_dwWidth = desc.Width;
	int m_dwHeight = desc.Height;

	// Acceso a la superficie del atlas
	LPDIRECT3DSURFACE9 pSurfaceOut;
	g_pTexture[atlas]->GetSurfaceLevel(0, &pSurfaceOut);

	int t = 0;
	for(int i=0;i<cant_f;++i)
		for(int j=0;j<cant_c;++j)
		{
			int ii = sel[t] / tile_cant_col;
			int jj = sel[t] % tile_cant_col;

			// Acceso a la memoria de la textura , tomando el rect. que corresponde al tile i,j
			RECT rc2 = {j*tile_dx, i*tile_dy, (j+1)*tile_dx, (i+1)*tile_dy};
			D3DLOCKED_RECT lockedRect;
			rta = texture->LockRect(0, &lockedRect,&rc2, D3DLOCK_DISCARD);
			
			// Y lo copio en la atlas , en la posicion ii,jj
			RECT rc = {jj*tile_dx,ii*tile_dy,(jj+1)*tile_dx,(ii+1)*tile_dy};
			rta = D3DXLoadSurfaceFromMemory(pSurfaceOut,NULL,&rc,lockedRect.pBits,D3DFMT_A8R8G8B8,
					lockedRect.Pitch,NULL,&rc_tile,D3DX_FILTER_NONE, 0);
  
			texture->UnlockRect(0);

			// paso al siguiente tile
			++t;
		}

	// libero la superficie y la textura auxiliar
	SAFE_RELEASE( pSurfaceOut);
	SAFE_RELEASE( texture);

	// Grabo el atlas de texturas
	char fname_aux[MAX_PATH];
	sprintf(fname_aux,"texturas\\%s",bmp_fname[atlas]);
	rta = D3DXSaveTextureToFile(fname_aux,D3DXIFF_BMP ,g_pTexture[atlas],NULL);

	
}



/////////////////////////////////////////////////////////////////////////////
// sprites
/////////////////////////////////////////////////////////////////////////////

HRESULT DXEngine2d::RenderSprite(int x0,int y0,int nro_sprite,float ex,float ey,
								 			int dx,int dy,int atlas)
{
	LPDIRECT3DVERTEXBUFFER9 g_pQuadAux;
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 4*sizeof(QUADVERTEX),
                                                  0, D3DFVF_QUADVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pQuadAux, NULL ) ) )
    {
        return E_FAIL;
    }


	int atlas_dx = 512;
	int atlas_dy = 512;

	// tomo los parametros x defecto
	if(dx==-1)
		dx = sprite_dx;
	if(dy==-1)
		dy = sprite_dy;
	if(atlas==-1)
		atlas = sprites;
	else
		atlas_dx = 256;

	// de momento el atlas de sprite esta fijo en 512 x 512
    QUADVERTEX vertices[4];
    float desf = 0.5f;
	float du = (float)dx / (float)atlas_dx;
	float dv = (float)dy / (float)atlas_dy;
	int cant_col = atlas_dx / dx;

	int i = nro_sprite / cant_col;
	int j = nro_sprite % cant_col;

	float u = (float)j*du;
	float v = (float)i*dv;

	vertices[0].pos = D3DXVECTOR4(x0-desf, y0-desf, 0.0f, 1.0f);
	vertices[0].tu = u;
	vertices[0].tv = v;

	vertices[1].pos = D3DXVECTOR4(x0+dx*ex-desf, y0-desf, 0.0f, 1.0f);
	vertices[1].tu = u+du;
	vertices[1].tv = v;
	
	vertices[3].pos = D3DXVECTOR4(x0+dx*ex-desf, y0+dy*ey-desf, 0.0f, 1.0f);
	vertices[3].tu = u+du;
	vertices[3].tv = v+dv;

	vertices[2].pos = D3DXVECTOR4(x0-desf, y0+dy*ey-desf, 0.0f, 1.0f);
	vertices[2].tu = u;
	vertices[2].tv = v+dv;

    VOID* pQuad;
    if( FAILED( g_pQuadAux->Lock( 0, sizeof(vertices), (void**)&pQuad, 0 ) ) )
        return E_FAIL;
    memcpy( pQuad, vertices, sizeof(vertices));
    g_pQuadAux->Unlock();

	g_pd3dDevice->SetStreamSource( 0, g_pQuadAux, 0, sizeof(QUADVERTEX));
	g_pd3dDevice->SetFVF( D3DFVF_QUADVERTEX);
	g_pd3dDevice->SetTexture( 0, g_pTexture[atlas]);

	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2);
	SAFE_RELEASE(g_pQuadAux);
	return S_OK;
}


HRESULT DXEngine2d::XplodeSprite(float elapsed_time,int x0,int y0,int nro_sprite,float ex,float ey,
					int dx,int dy,int atlas)
{
	LPDIRECT3DVERTEXBUFFER9 g_pQuadAux;
    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 8192*sizeof(QUADVERTEX),
                                                  0, D3DFVF_QUADVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pQuadAux, NULL ) ) )
    {
        return E_FAIL;
    }

	float an = elapsed_time*2;
	elapsed_time = sin(elapsed_time)*5;

	int atlas_dx = 512;
	int atlas_dy = 512;

	// tomo los parametros x defecto
	if(dx==-1)
		dx = sprite_dx;
	if(dy==-1)
		dy = sprite_dy;
	if(atlas==-1)
		atlas = sprites;
	else
		atlas_dx = 256;

	int cant_part = 24;

	// de momento el atlas de sprite esta fijo en 512 x 512
    QUADVERTEX vertices[8192];
    float desf = 0.5f;
	float du = (float)dx / (float)atlas_dx;
	float dv = (float)dy / (float)atlas_dy;
	int cant_col = atlas_dx / dx;

	int i = nro_sprite / cant_col;
	int j = nro_sprite % cant_col;

	float u = (float)j*du;
	float v = (float)i*dv;


	int t = 0;
	for(int ki = 0; ki<cant_part; ++ki)
		for(int kj = 0; kj<cant_part; ++kj)
		{
			float su = (float)kj/(float)cant_part;
			float sv = (float)ki/(float)cant_part;
			float su1 = 1.0/(float)cant_part;
			float sv1 = 1.0/(float)cant_part;

			TVector2d pos = TVector2d(ki,kj);
			TVector2d Origen = TVector2d(cant_part,cant_part)*0.5;
			TVector2d F = pos - Origen;
			F.normalizar();
			pos = Origen + F*(elapsed_time*50);


			float X0 = x0 + pos.x + su*dx*ex - desf;
			float Y0 = y0 + pos.y + sv*dy*ey - desf;
			TVector2d v0 = TVector2d(X0,Y0);
			TVector2d v1 = v0 + TVector2d(su1*dx*ex,sv1*dy*ey);
			//v1.rotar(v0,an);
			float X1 = v1.x;
			float Y1 = v1.y;


			vertices[t].pos = D3DXVECTOR4(X0, Y0, 0.0f, 1.0f);
			vertices[t].tu = u + su*du;
			vertices[t].tv = v + sv*dv;
			++t;

			vertices[t].pos = D3DXVECTOR4(X1, Y0, 0.0f, 1.0f);
			vertices[t].tu = u + (su+su1)*du;
			vertices[t].tv = v + sv*dv;
			++t;
			
			vertices[t].pos = D3DXVECTOR4(X1, Y1, 0.0f, 1.0f);
			vertices[t].tu = u + (su+su1)*du;
			vertices[t].tv = v + (sv+sv1)*dv;
			++t;

			vertices[t] = vertices[t-3];
			vertices[t+1] = vertices[t-1];

			vertices[t+2].pos = D3DXVECTOR4(X0, Y1, 0.0f, 1.0f);
			vertices[t+2].tu = u + su*du;
			vertices[t+2].tv = v + (sv+sv1)*dv;

			t+=3;
		}

    VOID* pQuad;
    if( FAILED( g_pQuadAux->Lock( 0, sizeof(vertices), (void**)&pQuad, 0 ) ) )
        return E_FAIL;
    memcpy( pQuad, vertices, sizeof(vertices));
    g_pQuadAux->Unlock();

	g_pd3dDevice->SetStreamSource( 0, g_pQuadAux, 0, sizeof(QUADVERTEX));
	g_pd3dDevice->SetFVF( D3DFVF_QUADVERTEX);
	g_pd3dDevice->SetTexture( 0, g_pTexture[atlas]);

	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST , 0, 2*cant_part*cant_part);
	SAFE_RELEASE(g_pQuadAux);
	return S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// Fisica basica
/////////////////////////////////////////////////////////////////////////////

BOOL DXEngine2d::colision(POINT p0,POINT p1)
{

	coli_mask = 0;
	item_collected = -1;
	int dx = p1.x-p0.x;
	int dy = p1.y-p0.y;

	if(!dx && !dy)
		return FALSE;		// no se movio

	BOOL rta = FALSE;

	// guardo el pto actual: 
	int pos_j = coli_j = nearest_x(Ip.x = p0.x);
	int pos_i = coli_i = nearest_y(Ip.y = p0.y);


	// nota:(*) Colision piso - techo
	//
	// si el personaje esta en el piso y salta, puede pasar sobre un tile que no es techo
	// como la cinta transportadora, que es solo piso.
	// En ese caso, llega justo hasta el interior del tile, (no se pasa)
	// y empieza a bajar. Luego detectaria una colision - piso, porque la cinta si es piso.
	// y quedaria trabado en el medio de la cinta

	//         X
	//        XXX
	//         X
	//     ....X......
	//   X  ***X*X****   --> Cinta -> queda trabado ahi, y deberia seguir bajando
	//  XXX   .......... -> Traspasa la cinta
	//   X
	//  XX   --> Salta desde aca
	// X  X
	// ================   -> Piso

	// Para resolver esto, la colision con piso, se tiene que dar solo cuando el personaje
	// viene cayendo desde arriba del piso:

	//     X
	//    XXX
	//     X  ----> la colision se tiene que dar en y_personaje % tile_dy == 0
	//  ============    -> pos_inicial del tile y = K*tile_dy
	//  ...........     -> pos intermedida
	//  ==============  -> pos final del tile y = K*tile_dy  + tile_dy-1

	// en el caso del techo es exactamente al reves

	// Antes yo ponia
	//if(dy>5)
	//	mask |= F_PISO;
	//else
	//if(dy<5)
	//	mask |= F_TECHO;
	// El problema es que ese 5 es medio tirado de los pelos, y depende de los fps

	float inc_x,inc_y;
	float x,y;
	BOOL mov_vert;

	if(abs(dy)>abs(dx))
	{
		// mov. predominantemente vertical
		inc_y = sign(dy);
		y = p0.y + inc_y;
		x = p0.x;
		inc_x = (float)dx/(float)abs(dy);
		mov_vert = TRUE;
	}
	else
	{
		// mov. predominantemente horizontal
		inc_x = sign(dx);
		x = p0.x + inc_x;
		y = p0.y;
		inc_y = (float)dy/(float)abs(dx);
		mov_vert = FALSE;
	}


	BOOL seguir = TRUE;
	while(seguir)
	{

		if(mov_vert)
		{
			if(y==p1.y)
				seguir = FALSE;		// llego al ultimo punto
		}
		else
		{
			if(x==p1.x)
				seguir = FALSE;		// llego al ultimo punto
		}


		if(inc_y>0)
		{
			// Colision con Piso
			int mask = F_PIEDRA | F_PISO;
			int ndx[] = {0,1};		// pie izquierdo / pie derecho
			int cant_v = 2;
			int v = 0;
			while(v<cant_v)
			{
				// pos. de la vertebra en el tile:
				int px = x + vertebra[ndx[v]].x;
				int py = y - vertebra[ndx[v]].y;
				int tj = nearest_x(px);
				int ti = nearest_y(py);


				if(item_collected==-1 && C[ti][tj].flags&(F_PICKABLE_ITEM|F_SCORE_ITEM))
				{
					// paso por un pickable item
					item_collected = C[ti][tj].tipo;
					item_i = ti;
					item_j = tj;
				}


				if(C[ti][tj].flags&mask && py%tile_dy==0)
				{
					rta = TRUE;			// colision
					seguir = FALSE;		// termina el ciclo
					// guardo el tile que hizo colision (el
					coli_j = tj;
					coli_i = ti;
					// guardo el pto de contacto (vertebra)
					coli_v = ndx[v];
					coli_mask = F_PISO;
					// termino el ciclo
					break;
				}
				else
					// paso a la siguiente vertebra
					++v;
			}
		}


		if(inc_x>0)
		{
			// Colision con Pared a Derecha
			int mask = F_PIEDRA | F_PARED_D;
			int ndx[] = {1,8};		// pie derecho , mano derecha
			int cant_v = 2;
			int v = 0;
			while(v<cant_v)
			{
				// pos. de la vertebra en el tile:
				int px = x + vertebra[ndx[v]].x;
				int py = y - vertebra[ndx[v]].y;
				int tj = nearest_x(px);
				int ti = nearest_y(py);

				// primero proceso la pos intermedia: esto permite recoger
				// objetos y abrir puertas y demas en el camino de la colision
				procesar_posicion(ti,tj);

				if(C[ti][tj].flags&mask && px%tile_dx==0)
				{
					rta = TRUE;			// colision
					seguir = FALSE;		// termina el ciclo
					// guardo el tile que hizo colision (el
					coli_j = tj;
					coli_i = ti;
					// guardo el pto de contacto (vertebra)
					coli_v = ndx[v];
					coli_mask = F_PARED_D;
					// termino el ciclo
					break;
				}
				else
					// paso a la siguiente vertebra
					++v;
			}
		}

		if(inc_x<0)
		{
			// Colision con Pared a Izquierda
			int mask = F_PIEDRA | F_PARED_I;
			int ndx[] = {0,7};		// pie izquierdo, mano izquierda
			int cant_v = 2;
			int v = 0;
			while(v<cant_v)
			{
				// pos. de la vertebra en el tile:
				int px = x + vertebra[ndx[v]].x;
				int py = y - vertebra[ndx[v]].y;
				int tj = nearest_x(px);
				int ti = nearest_y(py);

				// primero proceso la pos intermedia: esto permite recoger
				// objetos y abrir puertas y demas en el camino de la colision
				procesar_posicion(ti,tj);

				if(C[ti][tj].flags&mask && px%tile_dx==tile_dx-1)
				{
					rta = TRUE;			// colision
					seguir = FALSE;		// termina el ciclo
					// guardo el tile que hizo colision (el
					coli_j = tj;
					coli_i = ti;
					// guardo el pto de contacto (vertebra)
					coli_v = ndx[v];
					coli_mask = F_PARED_I;
					// termino el ciclo
					break;
				}
				else
					// paso a la siguiente vertebra
					++v;
			}
		}


		// caso particular: tubo
		if(seguir && !flag_tubo && status!=P_EN_TUBO && status!=P_EN_SOGA)
		{
			int tj = nearest_x(x + sprite_dx/2);
			int ti = nearest_y(y);

			if(C[ti][tj].flags&F_CENTRO_V)
			{
				rta = TRUE;			// colision
				seguir = FALSE;		// termina el ciclo
				// guardo el tile que hizo colision (el
				coli_j = tj;
				coli_i = ti;
				// guardo el pto de contacto (vertebra)
				coli_v = 2;		// 2= tronco
				coli_mask = F_CENTRO_V;
			}
		}

		if(seguir)
		{
			// guardo el pto actual: 
			Ip.x = x;
			Ip.y = y;

			// avanzo
			x+=inc_x;
			y+=inc_y;
		}
	}
	return rta;
}

// procesa una pos intermedia
void DXEngine2d::procesar_posicion(int ti,int tj)
{
	if(item_collected==-1 && C[ti][tj].flags&(F_PICKABLE_ITEM|F_SCORE_ITEM))
	{
		// paso por un pickable item
		item_collected = C[ti][tj].tipo;
		item_i = ti;
		item_j = tj;
	}

	if(C[ti][tj].flags&F_PUERTA)
	{
		// quiere pasar a traves de una puerta
		// verifico si tiene la llave
		char llave = -1;
		int desf = 0;
		
		switch(C[ti][tj].tipo)
		{
			case TILE_PTA_BLANCA:
				llave = TILE_LLAVE_BLANCA;
				break;
			case TILE_PTA_AZUL:
				llave = TILE_LLAVE_AZUL;
				desf = 6;
				break;
			case TILE_PTA_ROJA:
				llave = TILE_LLAVE_ROJA;
				desf = 9;
				break;
		}
		char rta = tiene_item(llave);
		if(rta!=-1)
		{
			// si tiene la llave pasa:
			// abro la puerta
			// busco la parte arriba
			int r = 0;
			while(r<4 && C[ti-r][tj].flags&F_PUERTA)
				++r;

			// me posiciono en la parte de arriba de la puerta
			int s = ti-r+1;
			for(r=0;r<4;++r)
			{
				for(int j=0;j<3;++j)
				{
					C[s+r][tj+j].flags = 0;
					C[s+r][tj+j].nro_tile = 25 + j + 42*(2+r) + desf;
				}
			}
			// y uso la llave
			borrar_item(rta);
		}
	}

}

// Descuenta una vida, y lleva al personaje a un lugar seguro
void DXEngine2d::morir()
{
	status = P_STATUS_UNKNOWN;
	cant_vidas--;
	if(cant_vidas==0)
	{
		// game over: ahora vidas infinitas
		cant_vidas = 5;
	}
	
	// llevo al personaje a un lugar seguro
	pos_x = pos_seg_x;
	pos_y = pos_seg_y;

}


void DXEngine2d::Update(float elapsed_time)
{
	ftime += elapsed_time;
	UpdateScene(elapsed_time);

	// timers 
	if(timer_caida>0)
	{
		timer_caida -= elapsed_time;
		if(timer_caida<0)
		{
			timer_caida = 0;
			morir();
		}
		return;
	}

	if(timer_cadena>0)
	{
		timer_cadena-= elapsed_time;
		if(timer_cadena<0)
		{
			timer_cadena = 0;
			morir();
		}
		return;
	}


	if(timer_choco>0)
	{
		timer_choco -= elapsed_time;
		if(timer_choco<0)
		{
			timer_choco = 0;
			morir();
			// Tambien muere el enemigo
			matar_enemigo(enemigo_sel);
		}
		return;
	}

	if(timer_quema>0)
	{
		timer_quema -= elapsed_time;
		if(timer_quema<0)
		{
			timer_quema = 0;
			morir();
		}
		return;
	}


	int ant_pos_x = pos_x;
	int ant_pos_y = pos_y;
	int ant_pos_j = nearest_x(pos_x);
	int ant_pos_i = nearest_y(pos_y);

	if(flag_tubo && !esta_en_tubo() && !esta_en_soga())
		flag_tubo = FALSE;


	if(status==P_EN_TUBO)
		vel_v = 80;		// Cae con velocidad constante (no acelera por la gravedad)
	else
	if(status!=P_SOBRE_PISO && status!=P_SOBRE_CINTA && status!=P_EN_ESCALERA 
		&& status!=P_EN_TUBO && status!=P_EN_SOGA)
		// fuerza de gravedad
		vel_v += 400*elapsed_time;

	if(status!=P_SALTANDO)
	{
		// verifico si esta sobre la cinta
		int pos_i_sig = nearest_y(pos_y+4);

		if(C[pos_i_sig][ant_pos_j].tipo==TILE_CINTA)
		{
			// esta sobre la cinta transportadora: fuerza hacia la derecha
			vel_h = vel_cinta;
			status = P_SOBRE_CINTA;
		}
		else
		// el +1 en ant_pos_j+1 tiene su motivo en que el cero esta del lado izq. del sprite
		if(C[pos_i_sig][ant_pos_j+1].tipo==TILE_CINTA_IZQ)
		{
			vel_h = -vel_cinta;
			status = P_SOBRE_CINTA;
		}
		else
		if(status==P_SOBRE_CINTA)
		{
			status = P_STATUS_UNKNOWN;		// salio de la cinta transportadora:
			// la cinta antihoraria, tiene una diferencia sutil con respecto a la 
			// horaria, y hay que darle un poco de impulso para que salga de la misma
			// la diferencia se basa en que la pos_x es relativa al cero del sprite
			// que esta del lado derecho. (Y no del lado izquierdo)
			if(vel_h<0)
				pos_x -= tile_dx/2;
			vel_h = 0;
		}
	}

	pos_y += elapsed_time*vel_v;

	// Velocidad horizontal
	pos_x += elapsed_time*vel_h;

	if(status==P_EN_ESCALERA)
	{
		// si estaba en la escalera: verifico si sigue estando?
		if(!esta_en_escalera())
			status = P_STATUS_UNKNOWN;		// salio de la escalera
	}
	else
	if(status==P_EN_TUBO)
	{
		// si estaba en la tubo: verifico si sigue estando?
		if(!esta_en_tubo())
			status = P_STATUS_UNKNOWN;		// salto del tubo
	}
	else
	if(status==P_EN_SOGA)
	{
		// si estaba en la soga : verifico si sigue estando?
		if(!esta_en_soga())
			status = P_STATUS_UNKNOWN;		// salto de la soga
	}


	// mov. relativo (al pto anterior)
	int dx = pos_x - ant_pos_x;
	int dy = pos_y - ant_pos_y;


	// Verifico las colisiones en la nueva posicion
	if(colision(CPoint(ant_pos_x,ant_pos_y),CPoint(pos_x,pos_y)))
	{
		// hubo colision: me posiciono en el pto de colision
		pos_y = Ip.y;
		pos_x = Ip.x;

		// analizo la colision
		if(C[coli_i][coli_j].tipo==TILE_TUBO)
		{
			// tiene que quedar pegado contra el tubo:
			vel_h = vel_v = 0;
			status = P_EN_TUBO;
			// lo dejo posicionado agarrandose del tubo 
			pos_x = coli_j*tile_dx + (tile_dx-sprite_dx)/2;
		}
		else
			// si el tile es soga, y no es la parte de arriba (donde cuelga la soga, 
			// que se comporta como piso tambien)
		if(C[coli_i][coli_j].tipo==TILE_SOGA && !(C[coli_i][coli_j].flags & F_PISO))
		{
			// tiene que quedar pegado contra el soga:
			vel_h = vel_v = 0;
			status = P_EN_SOGA;
			// lo dejo posicionado agarrandose del tubo 
			pos_x = coli_j*tile_dx + (tile_dx-sprite_dx)/2;
		}
		else
			// si el tile es una cadena: pierdo una vida
		if(C[coli_i][coli_j].tipo==TILE_CADENA)
		{
			timer_cadena = 2;
			vel_h = vel_v = 0;
			status = P_STATUS_UNKNOWN;

			// dejo la pos. como estaba antes de colisionar
			pos_x = ant_pos_x;
			pos_y = ant_pos_y;
		}
		else
		{
			// termino la velocidad horizontal (si la hubiera)
			vel_h = 0;

			if(status==P_SALTANDO)
				flag_tubo = FALSE;

			// si estaba estaba cayendo y llega al piso: termina la caida
			//if(dy>0 && (coli_v==0 || coli_v==1))
			if(coli_mask&F_PISO)
			{
				if(vel_v>=200)
					// cae con mucha velocidad: se hace mierda contra el piso:
					timer_caida = 2;
				else
				// es una pos. segura?
				if(C[coli_i][coli_j].tipo==TILE_LADRILLO && C[coli_i-1][coli_j].tipo==TILE_VACIO)
				{
					pos_seg_x = pos_x;
					pos_seg_y = pos_y;
				}
				vel_v = 0;
				status = P_SOBRE_PISO;
			}

		}
	}

	// verifico si esta sobre el fuego
	if(C[coli_i-1][coli_j].tipo==TILE_FUEGO || C[coli_i-1][coli_j+1].tipo==TILE_FUEGO)
		timer_quema = 2;

	if(item_collected!=-1 && cant_items<10)
	{
		// paso por arriba de un item pickable
		recoger_item(item_i,item_j);
	}


	// Update de enemigos
	for(int i=0;i<cant_enemigos;++i)
	{
		enemigo[i].Update(elapsed_time);
		// verifico si colisiona contra el enemigo
		if(enemigo[i].colision(pos_x,pos_y))
		{
			timer_choco = 2;
			enemigo_sel = i;		// dejo guardo que enemigo fue el que me estrolo
			break;
		}
	}



}


// Falta: si hay items juntos, uno al lado de otro sin separacion,
// se confunde los tiles. Eso es porque cada item tiene 4 tiles, y deberia guardar
// que esquina es cada tile. Por ejemplo  
//			(1) (2)
//			(3) (4)
// Entonces si toca contra la esquina (3), sabe que tiene borrar arriba y a la derecha

void DXEngine2d::recoger_item(int ti,int tj)
{

	int item_id = C[ti][tj].tipo;

	// si es almacenable: se lo agrego a la lista de items
	if(C[ti][tj].flags & F_PICKABLE_ITEM)
		items[cant_items++] = item_collected;

	// y lo borro del escenario
	for(int i = ti-1;i<=ti+1;++i)
		for(int j = tj-1;j<=tj+1;++j)
			if(C[i][j].flags & (F_PICKABLE_ITEM|F_SCORE_ITEM) && C[i][j].tipo == item_id)
			{
				C[i][j].flags = 0;
				C[i][j].nro_tile = 1009;
				C[i][j].tipo = TILE_VACIO;
			}
}


void DXEngine2d::matar_enemigo(int i)
{
	for(int t=i;t<cant_enemigos-1;++t)
		enemigo[t] = enemigo[t+1];
	cant_enemigos--;
}

void DXEngine2d::UpdateScene(float elapsed_time)
{
	// actualizo los tiles intermitentes
	// por ahora hay menos tiles intermitentes que tiles x pantalla
	// con lo cual conviene actualizar todos los tiles, en lugar de recorrer por pantalla
	// y actualizar solo los que estoy viendo. 
	// (***) Hay que seguir analizando cuando este  terminado el juego, si sigue cierto.

	int pos_j = nearest_x(pos_x);
	int pos_i = nearest_y(pos_y);

	for(int i=0;i<cant_intermitentes;++i)
	{
		int f = tiles_intermitentes[i].fil;
		int c = tiles_intermitentes[i].col;

		tiles_intermitentes[i].timer += elapsed_time;
		if(tiles_intermitentes[i].timer>tiles_intermitentes[i].ta + tiles_intermitentes[i].tp)
		{
			tiles_intermitentes[i].timer = 0;
			// esta prendido
			if(C[f][c].tipo==TILE_PISO_INTERMITENTE)
				C[f][c].flags |= F_PISO;
			else
			{
			
				// cadena intermetiente
				// si se prende la cadena, y justo estoy ahi, me mata:
				C[f][c].flags |= F_PARED_D|F_PARED_I;
				// si esta justo en el tile de la cadena: muere
				// y si no esta justo en el timer cadena (evita que muera 2 veces con la misma cadena)
				if(timer_cadena==0 && f==pos_i && (c==pos_j || c==pos_j+1) )
				{
					timer_cadena = 2;
					// y ubico el personaje, en algun punto donde no toque la cadena:
					if(c==pos_j)
						pos_x += tile_dx;
					else
						pos_x -= tile_dx;
				}
			}
		}
		else
		if(tiles_intermitentes[i].timer>tiles_intermitentes[i].tp)
		{
			// se apago:
			if(C[f][c].tipo==TILE_PISO_INTERMITENTE)
			{
				C[f][c].flags &= ~F_PISO;
				// si se apaga un piso y justo era el tile donde estaba parado: 
				// tiene que caer, probablemente:
				if(f-1==pos_i && c==pos_j)
					status = P_STATUS_UNKNOWN;
			}
			else
				C[f][c].flags &= ~(F_PARED_D|F_PARED_I);


		}
	}

	float total_ani = 3;
	timer_fuego+=elapsed_time;
	if(timer_fuego>=total_ani)
		timer_fuego -= total_ani;
	frame_fuego = timer_fuego/total_ani*33;

}



// x convencion pos_x, pos_y, representa la esquina inferior, izquierda del sprite
int DXEngine2d::nearest_x(int x)
{
	return (float)x/ (float)tile_dx;
}


int DXEngine2d::nearest_y(int y)
{
	return (float)y/(float)tile_dy;
}


BOOL DXEngine2d::esta_sobre_piso()
{
	BOOL rta = FALSE;
	// pos_x,pos_y es la esquina inferior izquierda
	// nearest point: 
	int pos_j = nearest_x(pos_x+vertebra[0].x);
	int pos_i = nearest_y(pos_y+vertebra[0].y+1);
	int mask = F_PISO | F_PIEDRA;
	if(C[pos_i][pos_j].flags&mask)
		rta = TRUE;		// el pie izquierdo apoya
	else
	{
		pos_j = nearest_x(pos_x+vertebra[1].x);
		pos_i = nearest_y(pos_y+vertebra[1].y+1);
		if(C[pos_i][pos_j].flags&mask)
			rta = TRUE;		// el pie derecho
	}

	return rta;
}

BOOL DXEngine2d::esta_en_escalera()
{
	return esta_en_escalera(pos_x,pos_y);
}

BOOL DXEngine2d::esta_en_escalera(int x,int y)
{
	BOOL rta = FALSE;
	int pos_j = nearest_x(x+6);
	int pos_i = nearest_y(y);
	// todo el cuerpo tiene que estar en la escalera
	if(C[pos_i][pos_j].tipo==TILE_ESCALERA)
	{
		pos_j = nearest_x(x+sprite_dx-6);
		if(C[pos_i][pos_j].tipo==TILE_ESCALERA)
			rta = TRUE;
	}
	return rta;
}

BOOL DXEngine2d::esta_en_soga()
{
	return esta_en_soga(pos_x,pos_y);
}

BOOL DXEngine2d::esta_en_soga(int x,int y)
{
	BOOL rta = FALSE;
	int pos_j = nearest_x(x+sprite_dx/2);
	int pos_i = nearest_y(y);
	if(C[pos_i][pos_j].tipo == TILE_SOGA)
		rta = TRUE;
	else
	{
		// todavia puede quedar colgado con las manos de la soga
		pos_i = nearest_y(y-sprite_dy+6);
		if(C[pos_i][pos_j].tipo == TILE_SOGA)
			rta = TRUE;
	}

	return rta;
}

BOOL DXEngine2d::esta_en_tubo()
{
	return esta_en_tubo(pos_x,pos_y);
}

BOOL DXEngine2d::esta_en_tubo(int x,int y)
{
	BOOL rta = FALSE;
	int pos_j = nearest_x(x+sprite_dx/2);
	int pos_i = nearest_y(y);
	if(C[pos_i][pos_j].tipo == TILE_TUBO)
		rta = TRUE;		// el pie izquierdo apoya
	return rta;
}

// Control de mov. horizontal
void DXEngine2d::Move(int dx,int dy)
{

	if(timer_caida>0 || timer_choco>0 || timer_cadena>0 || timer_quema>0)
		return;

	//if(!sobre_piso)
		//dx /= 2;
		//dx = sign(dx);
	if(status==P_EN_TUBO || status==P_EN_SOGA)
		dx = 0;		// en el tubo sole se puede mover para arriba y para abajo

	if(!dy && status==P_EN_ESCALERA)
	{
		// se mueve izq / der en la escalera: solo se puede hacer en el ultimo escalon
		int pos_j = nearest_x(pos_x+12);
		int pos_i = nearest_y(pos_y);
		if(C[pos_i+1][pos_j].tipo==TILE_ESCALERA)
			return;
		else
		{
			status = P_SOBRE_PISO;		// salio de la escalera
			pos_y = (pos_i+1) * tile_dy - 1;  
		}


	}

	// Verifico las colisiones en la nueva posicion
	if(!colision(CPoint(pos_x,pos_y),CPoint(pos_x+dx,pos_y+dy)))
	{
		pos_x+=dx;
		pos_y+=dy;


		// verifico la nueva posicion
		if(status==P_SOBRE_PISO && dx)
		{
			// Si estaba sobre el piso, y se movio para la derecha o para la izquierda
			// se puede caer, (si no hay mas piso en el nuevo lugar)
			if(!esta_sobre_piso())
			{
				// empieza a caer
				status = P_CAYENDO;
				vel_v = 40;
			}
		}


	}
	else
	{
		// contra que choco:
		if(C[coli_i][coli_j].tipo==TILE_CADENA)
		{
			timer_cadena = 2;
		}
	}



	if(dx)
	{
		// Animacion de caminar
		if(dx>0)
			sentido = 0;		// Apunta el sprite para la derecha
		else
			sentido = 1;		// apunta para la izquierda
		++sprite_sel;
		if(sprite_sel>=4)
			sprite_sel = 0;
	}
}



// Baja la escalera 
void DXEngine2d::Bajar(int dy)
{

	// Verifico las colisiones en la nueva posicion
	BOOL pos_ok = TRUE;
	if(colision(CPoint(pos_x,pos_y),CPoint(pos_x,pos_y+dy)))
	{
		// hubo colision, pero la escalera es un caso particular 
		// solo importa colision con el piso solo, (no importa si colisiona con piso + escalera)
		if(C[coli_i][coli_j].tipo == TILE_ESCALERA || C[coli_i][coli_j].tipo == TILE_SOGA)
			// es como si no habria colision
			pos_y += dy;
		else
			// colision pp dicha, me posiciono en el ultimo pto que no colisiono
			pos_y = Ip.y;
	}
	else
		pos_y += dy;
	
	status = P_SOBRE_PISO;
}



char *DXEngine2d::que_status()
{
	char *rta;
	switch(status)
	{
		case P_STATUS_UNKNOWN:
		default:
			rta = "Desconocido";
			break;
		case P_SOBRE_PISO:
			rta = "Sobre el Piso";
			break;
		case P_SOBRE_CINTA:
			rta = "Sobre la Cinta Transportadora";
			break;
		case P_SALTANDO:
			rta = "Saltando";
			break;
		case P_EN_ESCALERA:
			rta = "Subiendo Escalera";
			break;
		case P_CAYENDO:
			rta = "Cayendo";
			break;
		case P_EN_TUBO:
			rta = "En el Tubo";
			break;
		case P_EN_SOGA:
			rta = "En la Soga";
			break;
	}

	return rta;
}
			

			


void CEnemigo::Create(DXEngine2d *m,POINT pt,int sp1, ... )
{
	motor = m;
	pos_ini_x = pos_x = pt.x;
	pos_ini_y = pos_y = pt.y;
	pos_fin_x = pos_ini_x + 200;
	pos_fin_y = pos_ini_y;

	int i = sp1;
	cant_sprites = 0;
	va_list	marker;
	va_start( marker, sp1);
	while( i != -1 )
	{
		nro_sprite[cant_sprites++] = i;
		i = va_arg( marker, int);
	}
	va_end( marker);
	psprite = 0;

	vel_v = 50;
	vel_h = 0;
	sprite_frame = 0.1;
	timer_ani = 0;

}


void CEnemigo::Update(float elapsed_time)
{
	pos_x += vel_v*elapsed_time;
	if(vel_v>0 && pos_x>pos_fin_x)
		vel_v *= -1;
	else
	if(vel_v<0 && pos_x<pos_ini_x)
		vel_v *= -1;

	pos_y += vel_h*elapsed_time;

	timer_ani += elapsed_time;
	if(timer_ani>=sprite_frame)
	{
		if(vel_v>0)
		{
			if(++psprite>=cant_sprites)
				psprite = 0;
		}
		else
		{
			if(--psprite<0)
				psprite = cant_sprites-1;
		}

		timer_ani-= sprite_frame;
	}

}

BOOL CEnemigo::colision(int x,int y)
{
	BOOL rta = FALSE;
	if(abs(x-pos_x)<motor->sprite_dx && abs(y-pos_y)<motor->sprite_dy)
		rta = TRUE;

	return rta;
}

char DXEngine2d::tiene_item(char item)
{
	char rta = -1;
	int i=0;
	while(i<cant_items && rta==-1)
		if(items[i] == item)
			rta = i;
		else
			++i;

	return rta;
}
	

// usa el item i (entonces lo saca de la lista)
void DXEngine2d::borrar_item(int i)
{
	for(int t=i;t<cant_items;++t)
		items[t] = items[t+1];
	items[t] = 0;
	cant_items--;
}


