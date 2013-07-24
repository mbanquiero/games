// DXGui.h : 
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <mmsystem.h>
#include "\Program Files (x86)\Microsoft DirectX SDK (April 2006)\include\d3dx9.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include "\msdev\games\clases\vectores.h"

#define MAX_GUI_ITEMS		100

// tipos de items
// staticos
#define GUI_STATIC_TEXT			0
#define GUI_SPRITE				1
#define GUI_FRAME				2
#define GUI_RECT				3

// seleccionables
#define GUI_SEL					10		// primer gui sel
#define GUI_MENU_ITEM			10
#define GUI_BUTTON				11
#define GUI_RUTA				12
#define GUI_EDIT				13
#define GUI_COLOR				14
#define GUI_FACE				15
#define GUI_TOOLBAR				16


// status
#define GUI_CHECKED				1

// flags
#define GUI_CURSOR				1


// Vertex format para dibujar en 2d 
struct VERTEX2D
{
    FLOAT x,y,z,rhw;		// Posicion
    D3DCOLOR color;		// Color
};

#define D3DFVF_VERTEX2D  (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)


class gui_item
{
	public:
		int nro_item;
		int flags;
		CRect rc;
		CRect rc_original;
		char text[256];
		char tipo;
		float delay0;		// delay original
		float delay;
		float ftime;
		size_t pc;
		size_t len;
		int nro_textura;
		int state;
		D3DCOLOR c_fondo;		// color del fondo
		D3DCOLOR c_font;		// color de los textos
		float vel_char;

		ID3DXFont	*g_pFont;
		static LPD3DXSPRITE	pSprite;
		static LPD3DXLINE	ppLine;
		static class CGameEngine *model;		// modelo 3d
		static class DXGui *gui;

		gui_item();
	
		// interface:
		virtual BOOL pt_inside(POINT p);
		virtual void Render(BOOL resplandor=FALSE);
		virtual int ProcessMsg(MSG *Msg){return FALSE;};


};


class gui_button : public gui_item
{
	public:
		virtual void Render(BOOL resplandor=FALSE);

};

class gui_color : public gui_item
{
	public:
		virtual void Render(BOOL resplandor=FALSE);

};

class gui_face : public gui_item
{
	public:
		virtual void Render(BOOL resplandor=FALSE);

};

class gui_edit : public gui_item
{
	public:
		virtual void Render(BOOL resplandor=FALSE);
		virtual int ProcessMsg(MSG *Msg);

};


class gui_frame : public gui_item
{
	public:
		virtual void Render(BOOL resplandor=FALSE);

};

class gui_rect : public gui_item
{
	public:
		int radio;
		virtual void Render(BOOL resplandor=FALSE);

};

class gui_tb_button : public gui_item
{
	public:
		virtual void Render(BOOL resplandor=FALSE);
};


class gui_ruta : public gui_item
{
	public:
		int tipo_ruta;
		int cant_ptos;
		TVector3d pt_ruta[500];
		TVector2d pt[500];

		virtual void Create(int truta);

		virtual void Render(BOOL resplandor=FALSE);

};


class DXGui
{
	public:
		class CGameEngine *model;		// modelo 3d
		class gui_item *items[MAX_GUI_ITEMS];
		int cant_items;
		int item_0;
		POINT pt;		// pos. del cursor
		char sel;		// item seleccionado
		char foco;		// item con foco
		float delay_sel;
		float time;
		float total_delay;
		BOOL resplandor_enabled;
		BOOL closing;

		// Cableados
		char rbt;				// radio button transfer
		D3DCOLOR sel_color;		// color seleccionado
		char face_sel;			// Cara seleccionada


		BOOL sin_delay;

		double ex,ey,ox,oy;

		// pila para dialogos
		int pila_item_0[20];		// pila para guardar el primer item
		int cant_dialog;


		DXGui();
		~DXGui();

		// interface
		virtual void Create(class CGameEngine *m);
		virtual void Reset();
		virtual void RestaurarPos();


		// dialog support
		void InitDialog(BOOL delay=TRUE);
		void EndDialog();
		

		// input
		int ProcessMsg(MSG *Msg);
		void Update();
		void Render(BOOL resplandor=FALSE);

		// 
		gui_item *InsertItem(char tipo,char *s,int x,int y,int dx=300,int dy=20);
		gui_ruta *InsertItemRuta(int tipo_ruta,char *s,int x,int y,int dx=300,int dy=20);
		gui_rect *InsertItemRect(char *s,int x,int y,int dx=300,int dy=20,int r=0);
		gui_color *InsertItemColor(int x,int y,D3DCOLOR color);

		// line support
		void Transform(VERTEX2D pt[],int cant_ptos);
		void Transform(D3DXVECTOR2 pt[],int cant_ptos);

		void DrawPoly(TVector2d P[],int cant_ptos,int dw,D3DCOLOR color);
		void DrawSolidPoly(TVector2d P[],int cant_ptos,D3DCOLOR color,BOOL gradiente=TRUE);
		void RoundRect(int x0,int y0,int x1,int y1,int r,int dw,D3DCOLOR color,BOOL solid=FALSE);
	
};



