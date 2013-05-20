// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "TileEditor.h"
#include "ChildView.h"
#include "motor2d.h"
#include "SelNroTile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// globales
// ---------------------------------------------------------------------------
DXEngine2d motor;

// ---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
	eventoInterno = 0;
	origen = CPoint(0,0);
	tile_sel2 = tile_sel = 0;
	tile_sel4 = tile_sel3 = -1;
	ex = ey = 1;
	ox = oy = 2;

	m_font.CreateFont(16, 0, 0, 0, FW_NORMAL,0, 0, 0, 0, 0, 0, 0, 0, "Tahoma");
	hay_preview = TRUE;
	grilla = TRUE;
	sel_i = sel_j = -1;
	running = FALSE;

	tool_sel = TOOL_NADA;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONDOWN()
	ON_WM_CHAR()
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_SAVE_TILE, OnSaveTile)
	ON_COMMAND(ID_UPDATE_TILE, OnUpdateTile)
	ON_WM_TIMER()
	ON_COMMAND(ID_ESCAPE, OnEscape)
	ON_COMMAND(ID_TOOL_PONER_TILE, OnToolPonerTile)
	ON_COMMAND(ID_PONER_FUEGO, OnPonerFuego)
	ON_COMMAND(ID_SEL_NRO_TILE, OnSelNroTile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CDC *pDC = &dc;
	
	if(!motor.init)
	{
		motor.Create();
		motor.DXInit(m_hWnd);
		motor.LoadLevel(12,12);
		//motor.importar("levels.dat");
		motor.cargar_escenario("mapa.dat");
		motor.LoadPreviewLevel();

		motor.init = TRUE;
		motor.pos_y = 31 * 12;
		motor.pos_x = 380 * 12;
		// inicio el timer a aprox 20 fps (cada 50 ms)
		SetTimer(999,50,NULL);
	}

	hay_preview = ex>1?FALSE:TRUE;


	int tdx = motor.tile_dx;
	int tdy = motor.tile_dy;
	int f,c;

	if(running)
	{
		// calcula el origen desde la posicion del sprite
		int pos_j = motor.nearest_x(motor.pos_x);
		int pos_i = motor.nearest_y(motor.pos_y);
		int pant_fil = 	pos_i/motor.cant_fil;
		int pant_col = 	pos_j/motor.cant_col;
		f = pant_fil * motor.cant_fil;
		c = pant_col * motor.cant_col;
	}
	else
	{
		// Calcula el origen desde el preview
		c = max(-origen.x/tdx,0);
		f = max(-origen.y/tdy,0);
	}

	motor.Render(c*tdx,f*tdy,ex,ey,hay_preview);
	motor.RenderTileVP(2,520,tdx*4,tdy*4,tile_sel);
	if(tile_sel2!=-1 && tile_sel2!=tile_sel)
		motor.RenderTileVP(2+tdx*4,520,tdx*4,tdy*4,tile_sel2);
	if(tile_sel4!=-1 && tile_sel3!=-1)
	{
		motor.RenderTileVP(2,520+tdy*4,tdx*4,tdy*4,tile_sel3);
		motor.RenderTileVP(2+tdx*4,520+tdy*4,tdx*4,tdy*4,tile_sel4);
	}

	

	CPen hpen,hpen2,hpen3,hpen4,*hpenOld;
	hpen.CreatePen(PS_SOLID,1,RGB(240,240,255));
	hpen2.CreatePen(PS_SOLID,2,RGB(255,255,255));
	hpen4.CreatePen(PS_SOLID,2,RGB(0,0,0));
	hpen3.CreatePen(PS_SOLID,1,RGB(255,128,128));
	hpenOld = pDC->SelectObject(&hpen);
	int i;

	// Esqueleto
	if(grilla)
	{
		int pos_x = motor.pos_x-c*tdx;
		int pos_y = motor.pos_y-f*tdy;
		if(pos_x>=0 && pos_x<480 && pos_y>=0 && pos_y<320)
		{
			for(i=0;i<9;++i)
				pDC->FillSolidRect(	2 + (pos_x+motor.vertebra[i].x)*ex,
									2 + (pos_y-motor.vertebra[i].y)*ey,3,3,RGB(255,255,255));
		}
	}



	// grilla del nivel en pantalla
	if(grilla)
	{
		// Correccion de escala
		// la pantalla tiene cant_fil x cant_col, y eso ocupa screen_dy x screen_dx pixels, 
		float kx = (float)motor.screen_dx / (float)(motor.cant_col*tdx);		// si coincide exacto kx = 1, por ejemplo 40 col x 8 = 320 px
		float ky = (float)motor.screen_dy / (float)(motor.cant_fil*tdy);		// si coincide exacto ky = 1, por ejemplo 25 col x 8 = 200 px
		for(i=0;i<=motor.cant_col;++i)
		{
			pDC->MoveTo(ox + i*tdx*ex*kx,oy);
			pDC->LineTo(ox + i*tdx*ex*kx,oy + motor.screen_dy*ey);
		}
		for(i=0;i<=motor.cant_fil;++i)
		{
			pDC->MoveTo(ox,oy + i*tdy*ey*ky);
			pDC->LineTo(ox + motor.screen_dx*ex,oy + i*tdy*ey*ky);
		}
	}
	

	if(hay_preview)
	{
		
		// rectangulo del preview (representa la pantalla que estoy viendo)
		{
			CBrush *hbrushOld = (CBrush *)pDC->SelectStockObject(NULL_BRUSH);
			pox = 500;
			poy = 2;
			pex = (float)(1024 - pox)/(float)(MAX_TILE_X*tdx);
			pey = (float)(600 - poy)/(float)(MAX_TILE_Y*tdy);

			int px = pox+c*tdx*pex;
			int py = poy+f*tdy*pey;
			pDC->SelectObject(&hpen2);
			pDC->Rectangle(px,py,px+motor.screen_dx*pex,py+motor.screen_dy*pey);
			pDC->SelectObject(&hpen4);
			pDC->Rectangle(px+2,py+2,px+motor.screen_dx*pex-2,py+motor.screen_dy*pey-2);
			pDC->SelectObject(hbrushOld);
		}
		
	}

	pDC->SelectObject(hpenOld);
	CFont *hfontOld = pDC->SelectObject(&m_font);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(240,240,255));

	char buffer[255];
	sprintf(buffer,"Ln %d, Col %d || Tile = %d , %d || Tile Sel=%d || Spt %d,%d  x=%d, y=%d "
		"|| idata=%d Nro=%d",
			f,c,sel_i,sel_j,tile_sel,
			motor.nearest_y(motor.pos_y)-f,motor.nearest_x(motor.pos_x)-c,
			motor.pos_x,motor.pos_y,
			motor.C[sel_i][sel_j].idata,motor.C[sel_i][sel_j].nro_tile);
	pDC->TextOut(400,630,buffer);

	if(running)
		pDC->TextOut(10,630,motor.que_status());

	pDC->SelectObject(hfontOld);

}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point) 
{


	switch(tool_sel)
	{
		case TOOL_PONER_TILE:
			{
				// verifico si toca contra algun tile
				int c = max(-origen.x/motor.tile_dx,0);
				int f = max(-origen.y/motor.tile_dy,0);
				float kx = (float)motor.screen_dx / (float)(motor.cant_col*motor.tile_dx);		
				float ky = (float)motor.screen_dy / (float)(motor.cant_fil*motor.tile_dy);		
				int tdx = motor.tile_dx*ex*kx;
				int tdy = motor.tile_dy*ey*ky;
				int j = (point.x - ox)/tdx + c;
				int i = (point.y - oy)/tdy + f;
				if(i>=0 && i<MAX_TILE_Y && j>=0 && j<MAX_TILE_X)
				{
					motor.C[i][j].nro_tile = tile_sel;
					if(tile_sel == 42)
					{
						if(motor.C[i][j].tipo == TILE_FUEGO)
							motor.C[i][j].tipo = TILE_VACIO;
						else
							motor.C[i][j].tipo = TILE_FUEGO;
					}

					// Recargo el level
					motor.ReloadPreviewLevel(i,j);
					RedrawWindow();
				}
			}
			break;

		default:
			if(hay_preview)
			{
				// verifico si toca contra el preview
				if(point.x>=pox)
				{
					// area de preview
					pt_ant = point;	
					eventoInterno = 2;
					SetCapture();
				}
			}
			else
			{
				// si no hay preview, con shift scrolea: 
				if(nFlags&MK_SHIFT)
				{
					pt_ant = point;	
					eventoInterno = 1;
					SetCapture();
				}
			}
			break;
	}


	/*
	if(zoom)
	{
		if(nFlags&MK_SHIFT)
		{
			pt_ant = point;	
			eventoInterno = 1;
			SetCapture();
		}
		else
		{
			// verifico si toca contra algun tile
			int c = max(-origen.x/20,0);
			int f = max(-origen.y/20,0);

			int i = (point.y - oy)/40 + f;
			int j = (point.x - ox)/40 + c;
			if(i>=0 && i<MAX_TILE_Y && j>=0 && j<MAX_TILE_X)
			{
				motor.C[i][j].fil = tile_sel/25;
				motor.C[i][j].col = tile_sel%25;
				// Recargo el level
				motor.ReloadPreviewLevel(i,j);
				RedrawWindow();
			}
		}
	}
	else
	{
		if(point.y>=410)
		{
			// verifico si toca contra algun tile
			int fil = (point.y - 410)/20;
			int col = point.x/20;
			int nro_tile = fil*45 + col;
			if(nro_tile>=0 && nro_tile<450)
			{
				tile_sel = nro_tile;
				RedrawWindow();
			}
		}
		else
		if(point.x>=500)
		{
			// area de preview
			pt_ant = point;	
			eventoInterno = 2;
			SetCapture();
		}
		else
		{
			// verifico si toca contra algun tile
			int c = max(-origen.x/20,0);
			int f = max(-origen.y/20,0);

			int i = (point.y - oy)/20 + f;
			int j = (point.x - ox)/20 + c;
			if(i>=0 && i<MAX_TILE_Y && j>=0 && j<MAX_TILE_X)
			{
				motor.C[i][j].fil = tile_sel/25;
				motor.C[i][j].col = tile_sel%25;
				// Recargo el level
				motor.ReloadPreviewLevel(i,j);
				RedrawWindow();
			}
		}
	}
	*/
	CWnd ::OnLButtonDown(nFlags, point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	eventoInterno = 0;
	ReleaseCapture();
	CWnd ::OnLButtonUp(nFlags, point);
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(eventoInterno==1)
	{
		origen.x += (point.x-pt_ant.x)/ex;
		origen.y += (point.y-pt_ant.y)/ey;
		pt_ant = point;
		RedrawWindow();
	}
	else
	if(eventoInterno==2)
	{
		origen.x = -(point.x - pox)/pex;
		origen.y = -(point.y - poy)/pey;
		RedrawWindow();
	}

	SetCursor(AfxGetApp()->LoadStandardCursor(tool_sel==TOOL_PONER_TILE?IDC_CROSS:IDC_ARROW));

	CWnd ::OnMouseMove(nFlags, point);
}

BOOL CChildView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
	//return CWnd ::OnEraseBkgnd(pDC);
}

void CChildView::OnRButtonDown(UINT nFlags, CPoint point) 
{

	// verifico si toca contra algun tile
	int f,c;
	if(running)
	{
		// calcula el origen desde la posicion del sprite
		int pos_j = motor.nearest_x(motor.pos_x);
		int pos_i = motor.nearest_y(motor.pos_y);
		int pant_fil = 	pos_i/motor.cant_fil;
		int pant_col = 	pos_j/motor.cant_col;
		f = pant_fil * motor.cant_fil;
		c = pant_col * motor.cant_col;
	}
	else
	{
		// Calcula el origen desde el preview
		c = max(-origen.x/motor.tile_dx,0);
		f = max(-origen.y/motor.tile_dy,0);
	}



	float kx = (float)motor.screen_dx / (float)(motor.cant_col*motor.tile_dx);		
	float ky = (float)motor.screen_dy / (float)(motor.cant_fil*motor.tile_dy);		
	int tdx = motor.tile_dx*ex*kx;
	int tdy = motor.tile_dy*ey*ky;
	int j = (point.x - ox)/tdx + c;
	int i = (point.y - oy)/tdy + f;
	if(i>=0 && i<MAX_TILE_Y && j>=0 && j<MAX_TILE_X)
	{
		int sel = motor.que_tile(i,j);
	
		if(nFlags&MK_SHIFT)
		{
			// selecciona el tile 2
			if(i==sel_i || j==sel_j)
				// selecciono el tile de la derecha o el tile de abajo
				tile_sel2 = sel;
			else
			{
				// selecciono 4 tiles
				tile_sel2 = motor.que_tile(sel_i,sel_j+1);
				tile_sel3 = motor.que_tile(sel_i+1,sel_j);
				// supuestamente tile_sel4 = sel (pero por las dudas ajusto a 2 x 2 tiles
				tile_sel4 = motor.que_tile(sel_i+1,sel_j+1);
			}

		}
		else
		if(nFlags&MK_CONTROL)
		{
			// llevo el sprite al tile seleccionado
			motor.pos_x = j*motor.tile_dx;
			motor.pos_y = (i+1)*motor.tile_dy-1;
		}
		else
		{
			tile_sel = sel;
			tile_sel2 = tile_sel3 = tile_sel4 = -1;
		}

		sel_i = i;
		sel_j = j;

		RedrawWindow();
	}

	CWnd ::OnRButtonDown(nFlags, point);
}

void CChildView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{

	switch(nChar)
	{
		case '+':
			// Agrandar escala
			ex+=0.5;
			if(ex>3)
				ex = 3;
			ey = ex;
			RedrawWindow();
			break;
		case '-':
			// Achicar escala
			ex-=0.5;
			if(ex<1)
				ex = 1;
			ey = ex;
			RedrawWindow();
			break;
		case 'G':
		case 'g':
			// Activa / desactiva grila
			grilla = !grilla;
			RedrawWindow();
			break;

		case '4':
			motor.flag_tubo = motor.esta_en_tubo()?TRUE:FALSE;
			motor.Move(-4);
			RedrawWindow();
			break;
		case '6':
			motor.flag_tubo = motor.esta_en_tubo()?TRUE:FALSE;
			motor.Move(4);
			RedrawWindow();
			break;
		case '8':
			{
				if(motor.esta_en_escalera())
				{
					// sube la escalera
					if(motor.status!=P_EN_ESCALERA)
					{
						// recien entra a la escalera
						motor.status = P_EN_ESCALERA;
						motor.sprite_sel = 0;
						motor.sentido = 0;
						motor.pos_y -= 10;
						motor.vel_h = 0;

					}
					else
						motor.sprite_sel++;
					motor.pos_y -= 4;

					// verifico si sigue en la escalera: 
					if(!motor.esta_en_escalera())
					{
						motor.status = P_SOBRE_PISO;		// salio de la escalera
						// me posiciono justo sobre el piso. (Es el tile de arriba de la escalera)
						int pos_i = motor.nearest_y(motor.pos_y);
						motor.pos_y = (pos_i+1) * motor.tile_dy - 1;  
					}

					RedrawWindow();
				}
				else
				if(motor.status==P_EN_SOGA)
				{
					motor.sprite_sel++;
					motor.pos_y -= 4;
					// verifico si sigue en la soga (tomo la parte de arriba del sprite)
					if(!motor.esta_en_soga())
					{
						motor.status = P_SOBRE_PISO;		// salio de la escalera
						// me posiciono justo sobre el piso. (Es el tile de arriba de la soga)
						int pos_i = motor.nearest_y(motor.pos_y);
						motor.pos_y = (pos_i+1) * motor.tile_dy - 1;  
					}
					RedrawWindow();
				}
				else
				if(motor.status==P_SOBRE_PISO || motor.status==P_SOBRE_CINTA)
				{
					// salta
					motor.vel_v = -150;
					motor.status = P_SALTANDO;
					RedrawWindow();
				}
			}
			break;

		case '2':
			{

				int pos_j = motor.nearest_x(motor.pos_x+motor.sprite_dx/2);
				int pos_i = motor.nearest_y(motor.pos_y);
				if(motor.esta_en_escalera(motor.pos_x,motor.pos_y + 4))
				{
					// si el tile de abajo, es una escalera, puede empezar a bajar
					// o continuar bajando la escalera: 
					motor.Bajar();
					motor.sprite_sel++;
					motor.status = P_EN_ESCALERA;
					RedrawWindow();
				}
				else
				if(motor.esta_en_soga(motor.pos_x,motor.pos_y + 4))
				{
					// si el tile de abajo, es una soga , puede empezar a bajar
					// o continuar bajando la soga: 
					motor.Bajar();
					motor.sprite_sel++;
					motor.status = P_EN_SOGA;
					RedrawWindow();
				}
				else
				if(motor.status==P_EN_SOGA)
				{
					// si esta en la soga
					motor.Bajar();
					motor.sprite_sel++;
					motor.status = P_EN_SOGA;
					// verifico si sigue en la soga: 
					pos_i = motor.nearest_y(motor.pos_y);
					//if(motor.C[pos_i+1][pos_j].tipo!=TILE_SOGA)
					if(!motor.esta_en_soga())
					{
						motor.status = P_STATUS_UNKNOWN;		// salio de la soga
						motor.pos_y = (pos_i+1) * motor.tile_dy - 1;  
					}

					RedrawWindow();
				}
				else
				if(motor.status == P_EN_ESCALERA)
				{
					// si el tile de abajo no es una escalera, el personaje
					// esta en la en la escalera, lo hago salir de la misma
					// (es el ultimo escalon)
					motor.status = P_SOBRE_PISO;		// salio de la escalera
					motor.pos_y = (pos_i+1) * motor.tile_dy - 1;  
					RedrawWindow();
				}

			}
			break;


		case '9':
			if(motor.status==P_SOBRE_PISO || motor.status==P_SOBRE_CINTA 
				|| motor.status==P_EN_TUBO || motor.status==P_EN_SOGA)
			{
				// salto derecha
				motor.flag_tubo = motor.status==P_EN_TUBO || motor.status==P_EN_SOGA?TRUE:FALSE;
				motor.vel_v = -150;
				motor.vel_h = 100;
				if(motor.status==P_SOBRE_CINTA)
					motor.vel_h += motor.vel_cinta;
				motor.status = P_SALTANDO;
				motor.sentido = 0;
				RedrawWindow();
			}
			break;

		case '7':
			if(motor.status==P_SOBRE_PISO || motor.status==P_SOBRE_CINTA 
				|| motor.status==P_EN_TUBO || motor.status==P_EN_SOGA)
			{
				// salto izquierda
				motor.flag_tubo = motor.status==P_EN_TUBO || motor.status==P_EN_SOGA?TRUE:FALSE;
				motor.vel_v = -150;
				motor.vel_h = -100;
				if(motor.status==P_SOBRE_CINTA)
					motor.vel_h += motor.vel_cinta;
				motor.status = P_SALTANDO;
				motor.sentido = 1;
				RedrawWindow();
			}
			break;

		case ' ':
			running = !running;
			RedrawWindow();
			break;


	}

	CWnd ::OnChar(nChar, nRepCnt, nFlags);
}

void CChildView::OnFileSave() 
{
	// Grabo el nivel	
	motor.grabar_mapa("mapa.dat");
	AfxMessageBox("Mapa.DAT grabado");
}

void CChildView::OnFileOpen() 
{
	// Cargo el nivel	
	motor.cargar_mapa("mapa.dat");
	motor.LoadPreviewLevel();
	RedrawWindow();
	
}

void CChildView::OnSaveTile() 
{
	if(tile_sel4!=-1)
	{
		int tiles[] = {tile_sel,tile_sel2,tile_sel3,tile_sel4};
		motor.grabar_tile(tiles,2,2);
	}
	else
	{
		int tiles[] = {tile_sel};
		motor.grabar_tile(tiles,1,1);
	}
	AfxMessageBox("tile.bmp grabado");
}

void CChildView::OnUpdateTile() 
{
	if(tile_sel4!=-1)
	{
		int tiles[] = {tile_sel,tile_sel2,tile_sel3,tile_sel4};
		motor.update_tile(tiles,2,2);
	}
	else
	{
		int tiles[] = {tile_sel};
		motor.update_tile(tiles,1,1);
	}
	AfxMessageBox("Atlas Actualizado");
	RedrawWindow();
	
}

void CChildView::OnTimer(UINT nIDEvent) 
{

	if(nIDEvent==999 && running)
	{
		float elapsed_time = 50.0/1000.0;
		if(GetAsyncKeyState(VK_CONTROL))
			elapsed_time*=0.1;
		motor.Update(elapsed_time);
		RedrawWindow();
	}
	CWnd ::OnTimer(nIDEvent);
}

void CChildView::OnEscape() 
{
	tool_sel = TOOL_NADA;
	RedrawWindow();
	
}

void CChildView::OnToolPonerTile() 
{
	tool_sel = TOOL_PONER_TILE;	
	tile_sel2 = 0;
	tile_sel4 = tile_sel3 = -1;
	RedrawWindow();
}

void CChildView::OnPonerFuego() 
{
	tool_sel = TOOL_PONER_TILE;	
	tile_sel = 42;
	tile_sel2 = 0;
	tile_sel4 = tile_sel3 = -1;
	RedrawWindow();
}

void CChildView::OnSelNroTile() 
{
	CSelNroTile	auxDialog;
	if(auxDialog.DoModal()==IDOK)
	{
		tile_sel = auxDialog.m_nro_tile;
		tile_sel2 = tile_sel3 = tile_sel4 = -1;
		RedrawWindow();
	}

}
