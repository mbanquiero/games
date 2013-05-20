//
//  TileViewViewController.h
//  TileView
//
//  Created by user on 19/07/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>



typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned long COLORREF;

typedef struct tagBITMAPINFOHEADER{ // bmih 
    DWORD  biSize; 
    LONG   biWidth; 
    LONG   biHeight; 
    WORD   biPlanes; 
    WORD   biBitCount; 
    DWORD  biCompression; 
    DWORD  biSizeImage; 
    LONG   biXPelsPerMeter; 
    LONG   biYPelsPerMeter; 
    DWORD  biClrUsed; 
    DWORD  biClrImportant; 
} BITMAPINFOHEADER; 

// no puedo usar pack(2) para que anden bien los struct align porque se cuelga en el device
#define sizeof_BITMAPINFOHEADER	40

typedef struct tagBITMAPFILEHEADER { // bmfh 
    WORD    bfType; 
    DWORD   bfSize; 
    WORD    bfReserved1; 
    WORD    bfReserved2; 
    DWORD   bfOffBits; 
} BITMAPFILEHEADER; 

#define sizeof_BITMAPFILEHEADER	14


// Vertex format para Quads
typedef struct 
{
	GLfloat x,y,z;		// Position
    GLfloat tu,tv;		// Texture coords
} QUADVERTEX;


#define MAX_TEXTURAS	50
#define MAX_TILE_X		800
#define MAX_TILE_Y		250
#define MAX_TILE_X_PAN	4000

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

typedef struct 
{
	int nro_tile;
	int flags;
	char tipo;
	int idata;
} cell;

#define WINDOW_WIDTH    320
#define WINDOW_HEIGHT   480

typedef struct 
{
	int x;
	int y;
} POINT;

typedef struct 
{
	int cx;
	int cy;
} SIZE;


#define RGB(r, g ,b)  ((DWORD) (((BYTE) (r) |    ((WORD) (g) << 8)) | (((DWORD) (BYTE) (b)) << 16))) 
#define GetRValue(rgb)   ((BYTE) (rgb)) 
#define GetGValue(rgb)   ((BYTE) (((WORD) (rgb)) >> 8)) 
#define GetBValue(rgb)   ((BYTE) ((rgb) >> 16)) 



@class TileViewViewController;

@interface CEnemigo  : NSObject
{
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
	
	TileViewViewController *motor;
};	

- (void) Create: (TileViewViewController *)m withPos:(POINT )pt withCantSprites:(int)cant withSprites:(int*)sp;
- (BOOL) colision: (POINT) pt;
- (void) Update: (float) elapsed_time;

@end


typedef struct 
{
	int fil,col;		// puntero al tile que es intermitente
	float timer;		// elapsed time para controlar la intermitencia
	float tp;			// tiempo predido 
	float ta;			// tiempo apagado
} tile_intermitente;


// user interface
struct ui_item
{
	int x;
	int y;
	int dx;
	int dy;
	int nro_sprite;
	char key;
};

#define MAX_UI_ITEMS		50


@interface CFile : NSObject
{
	NSString *file;
	int cant_bytes;
	int fp;
	
}
- (BOOL) open: (NSString *)fileLocation;
- (void) readln: (char *)buffer;
- (void) close;
- (BOOL) eof;

@end

@interface TileViewViewController : UIViewController
{
    EAGLContext *context;
	BOOL animating;

	GLfloat glW,glH;
	int step;
	
	NSTimer *timer;
	long cur_time;
	
	float elapsed_time;
	int cant_frames;
	
	BOOL init;
	
	NSMutableString *bmp_fname[MAX_TEXTURAS];	// nombre del archivo bmp 
	GLuint tx_id[MAX_TEXTURAS];				// id de la textura
	COLORREF bmp_mask[MAX_TEXTURAS];		
	BYTE bmp_ds[MAX_TEXTURAS];
	SIZE bmp_size[MAX_TEXTURAS];			// tamaÒo del bmp en pixels
	int cant_texturas;
	int cant_bmp;
	
	// Titles
	// --------------------------
	cell C[MAX_TILE_Y][MAX_TILE_X];
	int atlas;
	// tamaÒo del atlas
	int atlas_dx;
	int atlas_dy;
	// cantidad total de filas x columnas
	int tile_cant_fil;
	int tile_cant_col;
	int map_dx;
	int map_dy;
	char tile_dx,tile_dy;
	// TamaÒo de pantalla
	// en filas x col
	char cant_fil;
	char cant_col;
	// en pixeles de salida
	int screen_dx,screen_dy;
	// tiles animados
	int tiles_animados[20][8];
	int cant_animaciones[20];
	// tiles intermitentes
	tile_intermitente tiles_intermitentes[500];
	int cant_intermitentes;
	// items recogidos (maximo 10 items)
	char cant_items;
	char items[10];
	
	// Memoria para los Quads
	QUADVERTEX *screen_quad;
	
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
	float vel_v_seg;
	float vel_h_seg;
	
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
	CEnemigo *enemigo[256];
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

	// user interface
	ui_item ui_items[MAX_UI_ITEMS];
	int cant_ui_items;

	
}


- (void) startAnimation;
- (void) stopAnimation;
- (void) refreshScreen;
- (void) updateFrame:(CFTimeInterval) timeElapsed;
- (void) drawFrame;
- (BYTE *) LoadBitmap: (NSString *)filename header:(BITMAPINFOHEADER *)header;
- (BYTE *) LoadBitmap: (NSString *)filename header:(BITMAPINFOHEADER *)header
			mask: (COLORREF)mask ds: (BYTE)ds;

- (void) Create;
- (void) Init: (SIZE)s;
- (void) CleanUp;

// Textures
- (int) cargar_textura: (NSString *)filename mask: (COLORREF) mask ds: (BYTE)ds;
- (int) cargar_textura: (NSString *)filename mask: (COLORREF) mask;
- (int) cargar_textura: (NSString *)filename;

- (void) CleanTextures;
- (void) LoadTextures;

// tiles:
- (BOOL) LoadLevel: (char)tdx tdy: (char)tdy;
- (int) nearest_x: (int) x;
- (int) nearest_y: (int) y;


- (BOOL) Render: (int)x0 y:(int)y0 ex:(float)ex ey:(float)ey;
- (BOOL) RenderTile: (int)x0 y:(int) y0 dx:(int)dx dy:(int)dy sel: (int)sel;
- (int) que_tile: (int) i j:(int)j;

// Escenario
- (void) cargar_escenario: (NSString *) fname;
- (void) cargar_mapa: (NSString *) fname;

// Sprites
- (BOOL) RenderSprite: (int) x0 
					y:(int) y0 nro_sprite:(int)nro_sprite
					ex: (float)ex ey:(float) ey
					dx: (int)dx dy:(int) dy atlas:(int) atlas;

- (BOOL) RenderSprite: (int) x0 
					y:(int) y0 nro_sprite:(int)nro_sprite
					ex: (float)ex ey:(float) ey;

- (BOOL) XplodeSprite: (float)elapsed_time
					x: (int) x0 y:(int) y0 
					nro_sprite:(int)nro_sprite
					ex: (float)ex ey:(float) ey
					dx: (int)dx dy:(int) dy atlas:(int) atlas;

- (BOOL) XplodeSprite: (float)elapsed_time
					x: (int) x0 y:(int) y0 
					nro_sprite:(int)nro_sprite
				   ex: (float)ex ey:(float) ey;


// Fisica basica
- (void) Update: (float) elapsed_time;
- (void) UpdateScene: (float) elapsed_time;
- (void) Move: (int) dx dy:(int)dy;
- (void) Bajar: (int) dy;
- (BOOL) colision: (POINT) p0 p1:(POINT) p1;
- (void) procesar_posicion: (int) ti tj:(int) tj;		// procesa una pos intermedia
- (void) recoger_item: (int) ti j:(int) tj;
- (char) tiene_item: (char) item;
- (void) borrar_item: (int) i;		// usa el item i (entonces lo saca de la lista)
- (void) matar_enemigo: (int) i;
- (void) morir;					// Descuenta una vida, y lleva al personaje a un lugar seguro

// Helpers relacion con el escenario
- (BOOL) esta_sobre_piso;
- (BOOL) esta_en_escalera: (int) x y:(int) y;
- (BOOL) esta_en_escalera;
- (BOOL) esta_en_soga: (int) x y:(int) y;
- (BOOL) esta_en_soga;
- (BOOL) esta_en_tubo: (int) x y:(int) y;
- (BOOL) esta_en_tubo;
- (char *)que_status;

// user interface
- (int) toca_sobre:int x y:(int) y;

// Input Process
- (BOOL) OnChar: (int)nChar nRepCnt:(int)nRepCnt nFlags:(int)nFlags;
- (BOOL) ProcessTouch: (int) x y:(int) y;


@end
