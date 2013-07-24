
#pragma once

#include <mmsystem.h>
#include "\Program Files (x86)\Microsoft DirectX SDK (April 2006)\include\d3dx9.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include "\msdev\games\clases\vectores.h"

// -----------------------------------------------------
#define MAX_TEXTURAS	50
#define MAX_MESH		100

// -----------------------------------------------------

// vertice en FVF 
struct CUSTOMVERTEX
{
    FLOAT x,y,z;			// Posicion
	D3DXVECTOR3 N;			// Normal
    D3DCOLOR    color;		// Color
	FLOAT       tu, tv;		// Coordenada u,v de la textura
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// Vertex format para Quads
struct QUADVERTEX
{
    D3DXVECTOR4 pos;	// Posicion
    FLOAT tu,tv;		// Texture coords
};

#define D3DFVF_QUADVERTEX  (D3DFVF_XYZRHW | D3DFVF_TEX1)

// Vertex format para mesh 2
struct VERTEX2
{
    D3DXVECTOR3 position;
    D3DXVECTOR3 normal;
    D3DXVECTOR2 texcoord0;
    D3DXVECTOR3 texcoord1;
    D3DXVECTOR3 texcoord2;
};

struct VERTEX_POS_COLOR
{
    FLOAT x,y,z;		// Posicion
    D3DCOLOR    color;		// Color
};

#define D3DFVF_POS_COLOR (D3DFVF_XYZ|D3DFVF_DIFFUSE)

struct VERTEX_POS_TEX1
{
    FLOAT x,y,z;		// Posicion
    FLOAT tu,tv;		// Texture coords
};

#define D3DFVF_POS_TEX1 (D3DFVF_XYZ|D3DFVF_TEX1)
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }



#define MAX_PUNTOS		10000
#define MAX_PART		1000
#define MAX_PAQ			5000


// Particle System
struct particle
{
	TVector3d Pos;
	TVector3d Dir;
	double delay;
	double rtime;
	double vel;
	double vel_v;
	double acel;
	double dq;
};


class CParticleSystem
{
	public:
		int cant_part;
		particle particle[MAX_PART];
		double g_time;
		TVector3d P0;
		int textura;
		BYTE r0,g0,b0;		// modula con el color
		double t0,t1;
		float gravedad;
		int cant_pasos;
		int max_pasos;
		BOOL rastro;
		char tipo;
	
		// DIRECTX
		class DXEngine *model;		// modelo 3d
		LPDIRECT3DDEVICE9       g_pd3dDevice;
		LPDIRECT3DVERTEXBUFFER9 g_pVB;			// Buffer para vertices
		size_t vertex_size;

		CParticleSystem();
		~CParticleSystem();
		
		// simulacion stage
		virtual HRESULT Create(DXEngine *model,TVector3d Pos,int cant,int nro_textura,char tipo_explosion=0);
		virtual HRESULT Update(float time,TVector3d pos_emisor);
		// rendering stage
		virtual void Render();
		void Release();

};

// vertice para la particula
struct VERTEX_PARTICLE
{
    D3DXVECTOR3 position;
    D3DXVECTOR3 normal;
    D3DCOLOR    color;		// Color
    D3DXVECTOR2 texcoord0;
    D3DXVECTOR3 texcoord1;
};


#define D3DFVF_VERTEX_PARTICLE (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX2|D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE3(1))


// helpers
// -----------------------------------------------------
HRESULT SetAlphaChannel(LPDIRECT3DTEXTURE9  g_pTexture,BYTE r0,BYTE g0,BYTE b0);
HRESULT SetAlphaChannel(LPDIRECT3DTEXTURE9  g_pTexture,BYTE alpha);




// -----------------------------------------------------

class DXMesh
{
	public:
		char fname[MAX_PATH];
		LPD3DXMESH              g_pMesh;
		D3DMATERIAL9*           g_pMeshMaterials;
		LPDIRECT3DTEXTURE9*     g_pMeshTextures;
		DWORD                   g_dwNumMaterials; 
		TVector3d P0;			// Posicion
		TVector3d size;			// Tamaño
		TVector3d rot;			// orientacion general
		class DXEngine *modelo;

		DXMesh();
		virtual void limpiar();

		HRESULT LoadMesh(class DXEngine *m,char *file);
		void Release();
		void CalcularMatriz(TVector3d Origen,TVector3d S,TVector3d Dir,D3DXMATRIXA16 *matWorld,
			TVector3d VUP=TVector3d(0,1,0));
		virtual void DXRender();
		virtual void DXRender(TVector3d Origen,TVector3d S,TVector3d Dir,TVector3d VUP=TVector3d(0,1,0));
		virtual void SetColor(D3DCOLOR color);

};

class DXEngine
{
	public:
		BOOL init;
		HWND m_hWnd;				// windows asociado
		float far_plane;
		float near_plane;
		double aspect_ratio;
		TVector3d LA;		// Look at
		TVector3d LF;		// Look from
		TVector3d ant_LA;		// Look at
		TVector3d ant_LF;		// Look from

		// Direct X 
		//-------------------------------
		LPDIRECT3D9             g_pD3D;			// Used to create the D3DDevice
		LPDIRECT3DDEVICE9       g_pd3dDevice;	// Our rendering device
		LPDIRECT3DVERTEXBUFFER9 g_pVB;			// Buffer para vertices
		LPDIRECT3DVERTEXBUFFER9 g_pVBFocos;			// Buffer para vertices
		LPDIRECT3DVERTEXBUFFER9 g_pQuad;		// Buffer para el quad de pantalla
		ID3DXFont				*g_pFont,*g_pFontb,*g_pFont2;
		LPD3DXSPRITE			pSprite;		// sprite para las cotas
		D3DPRESENT_PARAMETERS	d3dpp;			// Presentation parameters 
		LPD3DXLINE ppLine;						// lines varios

		// Transformaciones propias
		D3DXMATRIXA16 matWorld;
		D3DXMATRIXA16 matView;
		D3DXMATRIXA16 matProj;
		D3DVIEWPORT9 viewport;

		// Soporte de texturas Directx
		LPDIRECT3DTEXTURE9      g_pTexture[MAX_TEXTURAS];
		LPDIRECT3DTEXTURE9      g_pTxLigthPath;	// textura para generar el ligthpath (input)
		LPDIRECT3DTEXTURE9      g_pTxLigthPath2;	// idem, para la 2da pasada
		LPDIRECT3DTEXTURE9      g_pRenderTarget;	// textura para el RenderTarget 
		LPDIRECT3DTEXTURE9      g_pRenderTarget2,g_pRenderTarget3;

		// downsampling
		BOOL dxcaps_downsampling;
		BOOL downsampling;
		LPDIRECT3DSURFACE9		g_pTargetZ;
		LPDIRECT3DSURFACE9		g_pTargetB;

		//BYTE textura_trans[MAX_TEXTURAS];		// indica que la textura tiene transparencia
		char bmp_fname[MAX_TEXTURAS][MAX_PATH];	// nombre del archivo bmp 
		int bmp_k[MAX_TEXTURAS];				// mm del mundo real que representa el bmp
		int cant_texturas;
		int cant_bmp;

		// Luces
		D3DLIGHT9 light[8];

		// materiales standard
		D3DMATERIAL9 mtrl_std;		// material opaco blanco
		D3DMATERIAL9 mtrl_tx;		// material transparente
		D3DMATERIAL9 mtrl_negro;	// material opaco negro

		// render states
		DWORD ant_zenable,ant_cullmode,ant_dmsource,ant_alpha;

		// -----------------
		// pixel shader
		ID3DXEffect* g_pEffect;			// D3DX effect interface
		BOOL hay_shader;
		float sdx,sdy;					// Tamaño de la pantalla Auxiliar

		// Mallas del direct X
		DXMesh Mesh[MAX_MESH];
		int cant_mesh;
		LPD3DXMESH              g_pMeshCycle;

		virtual void Create();
		virtual HRESULT DXInit(HWND hWnd);
		virtual HRESULT DXSetupRender();
		virtual void DXCleanup();
		virtual HRESULT DXRender(){return S_OK;};

		int cargar_textura(char *filename,int K=0);
		void DXCleanTextures();
		HRESULT DXLoadTextures();
		void DXSetTrans(double alpha);		// no se usa mas
		void SaveRenderStates();
		void RestoreRenderStates();
		void DXSetModoNegro();			// pone el modo negro, para borrar cosas de la escane
			
		HRESULT LoadFx(char *fx_file);


};

	