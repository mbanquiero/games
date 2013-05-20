
#pragma once

#include <mmsystem.h>
#include "\Archivos de programa\Microsoft DirectX SDK (April 2006)\include\d3dx9.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include "\msdev\games\clases\vectores.h"

// -----------------------------------------------------
#define MAX_TEXTURAS	50
#define MAX_MESH		100

// -----------------------------------------------------


// Vertex format para Quads
struct QUADVERTEX
{
    D3DXVECTOR4 pos;	// Posicion
    FLOAT tu,tv;		// Texture coords
};

#define D3DFVF_QUADVERTEX  (D3DFVF_XYZRHW | D3DFVF_TEX1)


#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }



// helpers
// -----------------------------------------------------
HRESULT SetAlphaChannel(LPDIRECT3DTEXTURE9  g_pTexture,BYTE r0,BYTE g0,BYTE b0,BYTE ds=4);
HRESULT SetAlphaChannel(LPDIRECT3DTEXTURE9  g_pTexture,BYTE alpha);
HRESULT SetAlphaChannelGradient(LPDIRECT3DTEXTURE9  g_pTexture,BYTE r0,BYTE g0,BYTE b0,BYTE ds0,BYTE ds1);


#define MAX_TILE_X		800
#define MAX_TILE_Y		250
#define MAX_TILE_X_PAN	8000

// flags de los tiles
#define F_PIEDRA			1
#define F_PISO				2
#define F_TECHO				4
#define F_PARED_I			8
#define F_PARED_D			16
#define F_CENTRO_V			32
#define F_PICKABLE_ITEM		64
#define F_PUERTA			128
#define F_SCORE_ITEM		256

// tipo de tile:
#define TILE_VACIO				0
#define TILE_LADRILLO			1
#define TILE_ESCALERA			2
#define TILE_CINTA				3
#define TILE_TUBO				4
#define TILE_SOGA				5
#define TILE_PISO_INTERMITENTE	6
#define TILE_FUEGO				7
#define TILE_LLAVE_BLANCA		8
#define TILE_LLAVE_ROJA			9
#define TILE_LLAVE_AZUL			10
#define TILE_PTA_BLANCA			11
#define TILE_PTA_ROJA			12
#define TILE_PTA_AZUL			13
#define TILE_CINTA_IZQ			14
#define TILE_CADENA				15
#define TILE_DIAMANTE_AZUL		16
#define TILE_DIAMANTE_VERDE		17
#define TILE_DIAMANTE_AMARILLO	18
#define TILE_ESPADA				19
#define TILE_ANTORCHA			20
#define TILE_ANTORCHA_GRANDE	21
#define	TILE_JARRO				22


// Status del personaje
#define P_STATUS_UNKNOWN	0
#define P_SOBRE_PISO		1
#define P_SALTANDO			2
#define P_EN_ESCALERA		3
#define P_CAYENDO			4
#define P_SOBRE_CINTA		5
#define P_EN_TUBO			6
#define P_EN_SOGA			7


struct cell
{
	int nro_tile;
	int flags;
	char tipo;
	int idata;
};

class CEnemigo
{
public:
	int pos_ini_x;
	int pos_ini_y;
	int pos_fin_x;
	int pos_fin_y;
	int pos_x;
	int pos_y;
	int nro_sprite[20];
	int cant_sprites;
	int psprite;
	float vel_v;
	float vel_h;
	float sprite_frame;
	float timer_ani;

	class DXEngine2d *motor;

public:
	virtual void Create(DXEngine2d *m,POINT pt,int sp1, ... );
	virtual void Update(float elapsed_time);
	virtual BOOL colision(int x,int y);

};


struct tile_intermitente
{
	int fil,col;		// puntero al tile que es intermitente
	float timer;		// elapsed time para controlar la intermitencia
	float tp;			// tiempo predido 
	float ta;			// tiempo apagado
};


	
class DXEngine2d
{
	public:
		BOOL init;
		HWND m_hWnd;				// windows asociado

		// Direct X 
		//-------------------------------
		LPDIRECT3D9             g_pD3D;			// Used to create the D3DDevice
		LPDIRECT3DDEVICE9       g_pd3dDevice;	// Our rendering device
		LPDIRECT3DVERTEXBUFFER9 g_pVB;			// Buffer para vertices
		LPDIRECT3DVERTEXBUFFER9 g_pQuad;		// Buffer para el quad de pantalla
		ID3DXFont				*g_pFont,*g_pFontb,*g_pFont2;
		LPD3DXSPRITE			pSprite;		// sprite para las cotas
		D3DPRESENT_PARAMETERS	d3dpp;			// Presentation parameters 
		LPD3DXLINE ppLine;						// lines varios
		LPDIRECT3DVERTEXBUFFER9 g_pLevel;		// Buffer para todo el nivel

		// Memoria para los Quads
		QUADVERTEX *screen_quad;

		// Soporte de texturas Directx
		LPDIRECT3DTEXTURE9      g_pTexture[MAX_TEXTURAS];
		
		char bmp_fname[MAX_TEXTURAS][MAX_PATH];	// nombre del archivo bmp 
		int bmp_k[MAX_TEXTURAS];				// mm del mundo real que representa el bmp
		int cant_texturas;
		int cant_bmp;

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

		// Titles
		// --------------------------
		cell C[MAX_TILE_Y][MAX_TILE_X];
		int atlas;
		// tamaño del atlas
		int atlas_dx;
		int atlas_dy;
		// cantidad total de filas x columnas
		int tile_cant_fil;
		int tile_cant_col;
		int map_dx;
		int map_dy;
		char tile_dx,tile_dy;
		// Tamaño de pantalla
		// en filas x col
		char cant_fil;
		char cant_col;
		// en pixeles de salida
		int screen_dx,screen_dy;
		int nearest_x(int x);
		int nearest_y(int y);
		// tiles animados
		int tiles_animados[20][8];
		int cant_animaciones[20];
		// tiles intermitentes
		tile_intermitente tiles_intermitentes[500];
		int cant_intermitentes;


		// items recogidos (maximo 10 items)
		char cant_items;
		char items[10];


		// Sprites
		// --------------------------
		int sprites;
		int sprite_dx;
		int sprite_dy;
		// fuego 
		int fuego;
		int fuego_dx;
		int fuego_dy;
		int frame_fuego;
		float timer_fuego;

		// Personaje
		int pos_x,pos_y;
		float vel_v;
		float vel_h;
		int pos_seg_x,pos_seg_y;		// ultima posicion segura
		char status_seg;				// status de la ultima pos seg
		// ptos de contacto, simulman un esqueleto
		POINT vertebra[32];
		int cant_vertebras;
		char status;
		int sprite_sel;
		char sentido;			// 0 -> derecha, 1->izquierda
		int cant_vidas;
		float timer_caida;		// se cayo al piso
		float timer_choco;		// choco contra un enemigo
		float timer_cadena;		// lo agarro la cadena
		float timer_quema;		// se quema con el fuego

		// pantalla actual
		int screen_i;
		int screen_j;


		// Enemigos
		CEnemigo enemigo[256];
		int cant_enemigos;
		int enemigo_sel;

		char flag_tubo;			// hack para poder saltar del tubo
		// -> permite anular temporariamente las colisiones con tubo, para permitir
		// saltar del tubo. De lo contrario, durante la primer parte del salto, 
		// detectaria una colsion con el tubo, y no podria "despegar".


		// Colisiones
		// datos de la ultima colision
		POINT Ip;			// Pto de colision x,y
		int coli_i,coli_j;	// i,j del tile donde colisiono (no es x,y, es el tile siguiente!)
		char coli_v;		// Vertebra que hizo colision
		char coli_mask;		// F_PISO si colisiono con un piso, F_PARED_D, F_PARED_I, etc
		char item_collected;	// si paso por arriba de un item 
		int item_i,item_j;	// i,j del tile donde recogio al item

		// tiempo
		float ftime;

		int vel_cinta;


		virtual void Create();
		virtual HRESULT DXInit(HWND hWnd);
		virtual void DXCleanup();
		virtual HRESULT DXRender(){return S_OK;};

		int cargar_textura(char *filename,int K=0);
		void DXCleanTextures();
		HRESULT DXLoadTextures();
		void SaveRenderStates();
		void RestoreRenderStates();
			
		HRESULT LoadFx(char *fx_file);

		// Sprites
		HRESULT RenderSprite(int x0,int y0,int nro_sprite,float ex=1,float ey=1,
			int dx=-1,int dy=-1,int atlas=-1);
		HRESULT XplodeSprite(float elapsed_time,int x0,int y0,int nro_sprite,float ex=1,float ey=1,
			int dx=-1,int dy=-1,int atlas=-1);


		// tiles:
		HRESULT LoadLevel(char tdx=8,char tdy=8);
		HRESULT LoadPreviewLevel();
		HRESULT ReloadPreviewLevel(int i,int j);
		HRESULT Render(int x0,int y0,float ex=1,float ey=1,BOOL preview=FALSE);
		HRESULT RenderTile(int x0,int y0,int dx,int dy,int sel);
		HRESULT RenderTileVP(int x0,int y0,int dx,int dy,int sel);
		int que_tile(int i,int j);
		void grabar_tile(int sel[],int cant_f,int cant_c);
		void update_tile(int sel[],int cant_f,int cant_c);
		
		// Escenario
		void grabar_mapa(char *fname);
		void cargar_mapa(char *fname);
		void importar(char *fname);
		void cargar_escenario(char *fname);


		// Fisica basica
		void Update(float elapsed_time);
		void UpdateScene(float elapsed_time);
		void Move(int dx=4,int dy=0);
		void Bajar(int dy=4);
		BOOL colision(POINT p0,POINT p1);
		void procesar_posicion(int ti,int tj);		// procesa una pos intermedia
		void recoger_item(int i,int j);
		char tiene_item(char item);
		void borrar_item(int i);		// usa el item i (entonces lo saca de la lista)
		void matar_enemigo(int i);
		void morir();					// Descuenta una vida, y lleva al personaje a un lugar seguro

		// Helpers relacion con el escenario
		BOOL esta_sobre_piso();
		BOOL esta_en_escalera(int x,int y);
		BOOL esta_en_escalera();
		BOOL esta_en_soga(int x,int y);
		BOOL esta_en_soga();
		BOOL esta_en_tubo(int x,int y);
		BOOL esta_en_tubo();
		char *que_status();


		// Input Process
		BOOL OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

		

};

	