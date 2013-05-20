/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <mmsystem.h>
#include "\Archivos de programa\Microsoft DirectX SDK (April 2006)\include\d3dx9.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include "\msdev\games\clases\vectores.h"
#include "\msdev\games\clases\motor3d.h"
#include "\msdev\games\clases\mouse.h"
#include "dxgui.h"
#include "mysocket.h"
#include "\msdev\games\sound\sndmng.h"

// modos de juego
#define GAME_TEST_RUTA		0		
#define GAME_GRID_BATTLE		1		
#define GAME_GRID_SURVIVAL	2		
// Tests
#define GAME_TEST_LABERINTO	10		

// animacion_gui
#define GUI_ESCENARIO		0
#define GUI_PREVIEW_RUTA	1
#define GUI_RECORRER_RUTA	2
#define GUI_PRESENTACION	3
#define GUI_PREVIEW_MOTO	4
#define GUI_GAME_PAUSED		5


#define MAX_MSG_DBG		30
#define MAX_TRAMOS		100
struct cell
{
	int cant_tramos;		// cantidad de tramos del ligth paths
	int p[MAX_TRAMOS];				// puntero a path[]
	char player[MAX_TRAMOS];		// de que jugador es
};




class player
{
	public:
	BOOL init;
	int nro_player;							// numero de jugador
	static double max_an_lat;		// maximo angulo lateral al girar
	static double max_w0;			// maxima velocidad angular
	static TVector3d size_cycle;	// tamaño de la moto
	double vel_salto;				// aceleracion vertical para saltar
	char cant_vidas;
	char score;
	char explosion_barata;


	TVector3d Pos_Inicial;	// posicion de salida del juego, cada vez que pierde vuelve a esta posicion
	TVector3d Pos;			// posicion
	TVector3d Dir;			// Direccion del motor (no incluye gravedad, saltos etc)
	TVector3d Vmoto;		// Velocidad vectorial de la moto |Vmoto| = dir moto
	TVector3d antVel;		// Vmoto anterior
	TVector3d Dir_Original;	// Direccion Original (para hacer un cambio suave de direccion)
	float Hpiso;			// Altura del piso en la Pos
	float Hpiso_s;			// Altura del piso en la Pos delantera

	// angulos de direccion del motor
	double alfa,alfa_d;
	double beta;
	
	double vel_ini;
	double vel;				// modulo de la velocidad de la moto
	double vel_v;			// Velocidad Vertical
	double an_lat;			// angulo lateral al girar

	// Test
	double ant_alfa_1,ant_alfa_2;



	// Espacio de la moto, (Dir,Up,N)
	TVector3d Cycle_Dir;	// Direccion de la trayectoria de la moto
	TVector3d Cycle_Up;		// Direccion Arriba de la moto
	TVector3d Cycle_N;		// Direccion normal a la trayectoria
	TVector3d Cycle_Pos;	// Posicion del centro del mesh

	// status
	int pos_en_ruta;
	float timer_cayendo;		// cayendo de la ruta
	float timer_rebotando;		// rebotando del toolpath
	float timer_rearmar_moto;	// la moto se esta re-armando
	float timer_light_path;		// el ligth path se prende de nuevo: delay 
	float timer_ia;				// movimiento al azar
	BOOL implotando;	// 
	BOOL derrapando;

	// Recorrido
	int cant_ptos;
	TVector3d path[MAX_PUNTOS];
	double path_an[MAX_PUNTOS];
	int pos_i,pos_j;		// posicion en la grilla
	int pos_vertex_der;
	int pos_vertex_izq;
	int cant_ptos_delay;	// valor de cant_ptos justo antes de hacer un delay

	// colisiones
	TVector2d Ip;		// ultimo punto de colision
	int path_choque;	// choco en path[path_choque]

	// colores
	D3DCOLOR c_lightpath;
	D3DCOLOR c_lightpath_espejo;
	D3DCOLOR c_linepath;

	// varios
	char nro_color;
	char nombre[255];
	char slogan[255];
	char nro_avatar;			// nro de textura del avatar


	// Particle System
	CParticleSystem PS,PSaux;

	// sounido
	CSoundManager sound;
	double timer_sound;			// sound events
	BOOL mute;					// esta con el sonido apagado

	// comportamiento automatico
	TVector3d Dir_d;			// Direccion deseada
	static double treaccion;			// tiempo de reaccion
	double timer_girando;			// esta girando
	double timer_evitando;			// evita un obstaculo
	double timer_reculando;			// esta por irse del domo...tiene que recular
	BOOL giro_horario;
	static double dist_radar;
	BOOL juego_justo;

	// multiplayer
	int cant_paquetes;
	int paq_desde;
	grid_packet paquete[MAX_PAQ];
	BOOL ack;

	class CClientSocket socket;
	virtual void Send();
	void llenar_paquete(grid_packet *packet,char cmd=1);


	class CGameEngine *model;		// modelo 3d
	class DXMesh *cycle_mesh;		// mesh de la moto 
	LPD3DXMESH g_pMeshCycle;		// auxiliara para explotar la moto

	// DIRECTX
	LPDIRECT3DDEVICE9       g_pd3dDevice;
	LPDIRECT3DVERTEXBUFFER9 g_pVBPath;			// Buffer para vertices del ligth path


	player();
	virtual void Create(CGameEngine *model,TVector3d pos,int n);
	virtual void Reset();
	virtual void SetCamara();
	virtual void poner_en_ruta();
	virtual void calc_angulos();	// recalcula alfa y beta
	virtual void SetColor(int n);
	virtual void PerderVida();

	// light path
	virtual void initPath();
	virtual void actualizarPath();
	virtual void crearLaberintoLuz();

	HRESULT DXCreatePathVertex();
	void DXCleanup();

	void RenderLightPath(BOOL resplandor=FALSE,BOOL espejo=FALSE);
	void RenderCycle(BOOL resplandor=FALSE);


	// input
	void ProcessMsg(MSG *Msg);
	void ProcessInput(CDirectInputMouse *mouse);

	// logica del jugador
	void Update();
	void UpdateTimers();
	void IA();
	void UpdateSound();

	// multiplayer
	void ProcessPacket(grid_packet *packet);
	void UpdateRemoto();
	void Wait();




};


class CGameEngine : public DXEngine
{


public:
	CGameEngine();
	virtual ~CGameEngine();

	TVector3d Dir_D;			// Direccion Deseaada
	float dtime;	
	int cant_cell;
	float dgrid;
	TVector3d centro;
	float radio;
	int cant_focos;
	int pos_vertex_focos;
	int pos_vertex_focos2;
	int pos_vertex_skydome;
	int cant_pri_skydome;

	int cant_ptos_ruta;
	int pos_vertex_ruta;
	TVector3d pt_ruta[500];
	float aux_tu;
	float aux_tv;
	int aux_tramo;
	BOOL aux_en_ruta;
	double max_tramo_ruta;
	double ancho_ruta;
	double ancho_guarray;
	float fps;
	int nro_frame;
	int tipo_ruta;
	BOOL techo_ruta;

	int animacion_gui;			// animacion del escenario en modo gui
	double pr_pos;
	TVector3d pr_LA;
	TVector3d pr_LF;

	TVector3d LA_d;		// Look at deseado
	TVector3d LF_d;		// Look from deseado
	float vel_camara;	// Velocidad de la camara (para el LF)
	float vel_rot_camara;	// Velocidad de la camara (para el LA)
	BOOL smooth_cam;


	BOOL modo_camara;
	BOOL modo_gui;
	BOOL fpc;			// camara en primera persona


	float timer_pausa;
	gui_frame *frame_pausa;
	gui_item *msg_pausa;

	float timer_warming;
	float timer_presentacion;
	float timer_game_over;
	float timer_lag;
	float timer_modalless;
	BOOL lagging;

	// celdas
	cell celdas[64][64];


	// mutiplayer
	BOOL multiplayer;
	BOOL socket_init;
	BOOL es_servidor;
	class CServerSocket server;


	int textura_piso;
	int textura_pared;
	int textura_path;
	int textura_skydome;
	int textura_ruta;
	int textura_explo;
	int textura_gui;
	int textura_face;
	int textura_face_enemigo;
	int textura_cursor;
	int textura_menu;
	int textura_roundrect;
	int textura_toolbar;

	// auxiliar para guardar el ligth path esquematico en 2D 
	D3DXVECTOR2 lightpath[10][MAX_PUNTOS];
	int ligthpath_cant[10];

	// mov. del mouse
	int mouse_dx;
	int mouse_dy;

	// tiempo
	double time,elapsed_time;
	double remote_time;
	BOOL paused;
	
	// jugadores
	player P[10];
	int cant_players;
	player *player_one;		// short cut = P[0]
	int cant_enemigos;
	int nro_player_perdio;

	// modo de juego
	char modo_juego;
	char ronda;
	char max_rondas;
	char juega_sola;
	char nivel;
	float survival_time;
	
	TVector3d VUP;			// Velocidad vectorial de la moto |Vmoto| = dir moto

	// gui
	DXGui gui;

	// direct input mouse
	CDirectInputMouse mouse;

	// sonido
	//char fname_midi[255];
	BOOL hay_radio;
	UINT midi_DeviceID;
	// play list
	char midi_files[50][MAX_PATH];
	int cant_midis;
	int midi_actual;


	// mensajes debug
	char debug_msg[MAX_MSG_DBG][255];
	int cant_msg;
	int primer_msg;
	BOOL debug_ia;
	void agregar_msg(char *buffer);


	virtual HRESULT DXInit(HWND hWnd);
	virtual HRESULT DXSetupRender();
	virtual HRESULT DXRender();
	virtual HRESULT DXCreateVertex();
	virtual HRESULT DXdibujar_planoVP(char plano=1,BOOL clear=FALSE);

	
	void scoreboard();
	void brd_player(int player);

	void MainLoop();
	void InitGame();
	void Presentacion();
	void FinRonda(int player);

	void RenderScene(BOOL resplandor=FALSE,BOOL mirror=FALSE);
	void RenderBolas();
	void RenderMirror();
	void RenderRecognizers(BOOL resplandor=FALSE);
	void RenderFocos();

	// comportamiento del juego
	int entra_en_ruta(TVector3d Pos,TVector3d Dir);
	BOOL choca_ligthpath(TVector3d desde,TVector3d hasta,int *path_choque,TVector2d *Ip,int player);
	BOOL choca(TVector3d desde,TVector3d hasta,int player,TVector2d *Ip,BOOL prediccion=FALSE);

	double que_altura(double x,double z,char *tipo_celda=NULL,int *tramo=NULL);
	double que_altura(int i,TVector2d p);

	// rutas
	int load_pt_ruta(int tipo,TVector3d *pt);
	void load_ruta();

	// gui
	void GUI_loop();
	void GUI_mainmenu();
	int GUI_messagebox(char *msg,int tipo=0);
	int GUI_options_ruta();
	int GUI_options_battle();
	int GUI_options_survival();
	int GUI_options_profile();
	int GUI_esperar_conexion();
	int GUI_input_data(char *buffer,char *titulo,char *msg);
	int GUI_pausa();
	int GUI_elegir_midi();

	// playlist
	void cargarPlayList();
	void playMidi();
	void loopMidi();

};


// motor3d 
extern CGameEngine escena;


