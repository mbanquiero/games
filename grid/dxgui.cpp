// dxgui.cpp
//

#include "stdafx.h"
#include "grid.h"
#include "dxgui.h"
#include "GameEngine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


LPD3DXSPRITE gui_item::pSprite = NULL;
LPD3DXLINE gui_item::ppLine = NULL;
class CGameEngine *gui_item::model = NULL;
class DXGui *gui_item::gui = NULL;
float _vel_char_menu = 240;		// caracters x segundo



/////////////////////////////////////////////////////////////////////////////
BOOL gui_item::pt_inside(POINT p)
{
	return rc.PtInRect(p);
}

void gui_item::Render(BOOL resplandor)
{

	BOOL sel = gui->sel==nro_item?TRUE:FALSE;
	float delay_sel = gui->delay_sel;

	if(nro_textura != -1)
	{
		D3DXVECTOR3 pos = D3DXVECTOR3(rc.left-64,rc.top-8,0);
		pSprite->Draw(model->g_pTexture[nro_textura],NULL,NULL,&pos,0xFFFFFFFF);
	}

	D3DCOLOR color = sel?D3DCOLOR_XRGB( 255,220,220):
		(tipo>=GUI_SEL?D3DCOLOR_XRGB(130,255,130):D3DCOLOR_XRGB(0, 255,255));


	if(sel)
	{
		gui->RoundRect(rc.left-1,rc.top-3,rc.right+1,rc.bottom+3,6,2,D3DCOLOR_XRGB(128,192,255));
		double tr = 4*(0.25-delay_sel);
		int dy = 15;

		ppLine->SetWidth(1);
		ppLine->Begin();
		BYTE r0,g0,b0,r1,g1,b1;

		r1 = 255 , g1 = 235 ,b1 = 182;
		r0 = 255 , g0 = 254 ,b0 = 237;
		// Gradiente de abajo
		for(int i=0;i<dy;++i)
		{

			D3DXVECTOR2 pt[5];
			pt[0].x = rc.left - 3;
			pt[1].x = rc.right + 3;
			pt[1].y = pt[0].y = (rc.top+rc.bottom)/2 - i;
			gui->Transform(pt,2);
			float t = (float)i/(float)dy;
			BYTE r = r0*t+r1*(1-t);
			BYTE g = g0*t+g1*(1-t);
			BYTE b = b0*t+b1*(1-t);
			ppLine->Draw(pt,2,D3DCOLOR_ARGB((BYTE)(255*tr),r,g,b));
		}
		
		// Gradiente de arriba
		r0 = 255 , g0 = 231 , b0 = 162;
		r1 = 255 , g1 = 217,  b1 = 120;
		for( i=0;i<dy;++i)
		{

			D3DXVECTOR2 pt[5];
			pt[0].x = rc.left - 3;
			pt[1].x = rc.right + 3;
			pt[1].y = pt[0].y = (rc.top+rc.bottom)/2 + i;
			gui->Transform(pt,2);
			float t = (float)i/(float)dy;
			BYTE r = r0*t+r1*(1-t);
			BYTE g = g0*t+g1*(1-t);
			BYTE b = b0*t+b1*(1-t);
			ppLine->Draw(pt,2,D3DCOLOR_ARGB((BYTE)(255*tr),r,g,b));
		}

		
		/*
		ppLine->SetWidth(dy);
		D3DXVECTOR2 pt[5];
		pt[0].x = rc.left - 3;
		pt[1].x = rc.right + 3;
		pt[1].y = pt[0].y = (rc.top+rc.bottom-dy)/2;
		gui->Transform(pt,2);
		ppLine->Begin();
		ppLine->Draw(pt,2,D3DCOLOR_ARGB((BYTE)(255*tr),80,220,20));

		pt[1].y = pt[0].y = (rc.top+rc.bottom+dy)/2;
		gui->Transform(pt,2);
		ppLine->Draw(pt,2,D3DCOLOR_ARGB((BYTE)(255*tr),40,110,10));
		*/



		ppLine->End();


	}


	// dibujo el texto pp dicho
	char buffer[255];
	if(pc!=-1)
	{
		pc=ftime*vel_char;
		if(pc>=len)
			pc = -1;
	}

	if(flags&GUI_CURSOR)
	{
		if(pc==-1)
		{
			if(sel)
				sprintf(buffer,"%s >>>",text);
			else
				strcpy(buffer,text);
		}
		else
		{
			strncpy(buffer,text,pc);
			buffer[pc] = '>'; 
			buffer[pc+1] = '\0'; 
		}
	}
	else
		strcpy(buffer,text);


	if(!resplandor)
		g_pFont->DrawText( pSprite, buffer, -1, &rc, DT_NOCLIP|DT_TOP, sel?D3DCOLOR_XRGB(0,32,128):
					c_font!=-1?c_font:color);

	if(pc!=-1 && flags&GUI_CURSOR)
	{
		buffer[pc] = '\0'; 
		CRect rc2 = rc;
		g_pFont->DrawText( pSprite, buffer, -1, &rc2, DT_NOCLIP|DT_TOP|DT_CALCRECT, color);
		
		ppLine->SetWidth(2);
		D3DXVECTOR2 pt[2];
		pt[0].x = rc2.right-50;
		pt[1].x = rc2.right;
		pt[1].y = pt[0].y = (rc2.top+rc2.bottom)/2;
		gui->Transform(pt,2);
		ppLine->Begin();
		ppLine->Draw(pt,2,D3DCOLOR_XRGB(64,255,64));
		ppLine->End();

		rc2.left = rc2.right;
		rc2.right += 20;
		g_pFont->DrawText( pSprite, ">", -1, &rc2, DT_NOCLIP|DT_TOP, color);


	}
	
	if(sel && !resplandor && flags&GUI_CURSOR)
	{
		CRect rc2 = rc;
		g_pFont->DrawText( pSprite, text, -1, &rc2, DT_NOCLIP|DT_TOP|DT_CALCRECT, color);
		rc2.left = rc2.right;
		rc2.right += 12;

		// dibujo el cursor titilando
		int cursor = (int)(gui->time*5);
		if(cursor%2)
		{
			ppLine->SetWidth(8);
			D3DXVECTOR2 pt[5];
			pt[0].x = rc2.left;
			pt[1].x = rc2.right;
			pt[1].y = pt[0].y = rc2.bottom;

			gui->Transform(pt,2);
			ppLine->Begin();
			ppLine->Draw(pt,2,D3DCOLOR_XRGB(0,64,0));
			ppLine->End();
		}
	}		
}



void gui_button::Render(BOOL resplandor)
{
	BOOL sel = gui->sel==nro_item?TRUE:FALSE;
	float delay_sel = gui->delay_sel;
	if(nro_textura != -1)
	{
		D3DXVECTOR3 pos = D3DXVECTOR3(rc.left-64,rc.top-8,0);
		pSprite->Draw(model->g_pTexture[nro_textura],NULL,NULL,&pos,0xFFFFFFFF);
	}

	
	// recuadro del boton
	gui->RoundRect(rc.left,rc.top+5,rc.right,rc.bottom-5,15,3,D3DCOLOR_XRGB(80,220,20));

	
	if(sel)
	{
		// boton seleccionado: lleno el interior
		double tr = 4*(0.25-delay_sel);
		gui->RoundRect(rc.left,rc.top+5,rc.right,rc.bottom-5,10,1,
				D3DCOLOR_ARGB((BYTE)(255*tr),184,224,248),TRUE);
	}

	// Texto del boton
	if(!resplandor)
	{
		D3DCOLOR color = sel?D3DCOLOR_XRGB( 255,220,220):D3DCOLOR_XRGB(130,255,130);
		g_pFont->DrawText( pSprite, text, -1, &rc, DT_NOCLIP|DT_TOP|DT_CENTER, color);
	}


}


void gui_tb_button::Render(BOOL resplandor)
{
	BOOL sel = gui->sel==nro_item?TRUE:FALSE;
	float delay_sel = gui->delay_sel;

	// nro de textura indica el nro de boton dentro de la textura del atlas
	int n = model->textura_toolbar;
	int col = nro_textura%8;
	int fil = nro_textura/8;
	CRect src = CRect(col*32,fil*32,(col+1)*32,(fil+1)*32);

	D3DXVECTOR3 pos = D3DXVECTOR3(rc.left,rc.top,0);
	pSprite->Draw(model->g_pTexture[n],&src,NULL,&pos,
		state&GUI_CHECKED?0xFFFFFFFF:0xFFB0B0B0);

	// recuadro del boton
	if(sel)
		gui->RoundRect(rc.left-2,rc.top-2,rc.right+2,rc.bottom+2,1,2,D3DCOLOR_XRGB(245,235,200));
	else
		gui->RoundRect(rc.left-1,rc.top-1,rc.right+1,rc.bottom+1,1,1,
		state&GUI_CHECKED?D3DCOLOR_XRGB(245,128,128):D3DCOLOR_XRGB(128,128,128));
	
	// mensaje de ayuda asociado al boton del toolbar
	if(!resplandor && sel)
	{
		CRect rc2 = rc;
		rc2.top-=30;
		g_pFont->DrawText( pSprite, text, -1, &rc2, 
				DT_NOCLIP|DT_TOP|DT_LEFT, D3DCOLOR_XRGB( 245,235,200));
	}


}




void gui_color::Render(BOOL resplandor)
{
	BOOL sel = gui->sel==nro_item?TRUE:FALSE;
	float delay_sel = gui->delay_sel;


	TVector2d Q[10];
	double rx = rc.Width()/2;
	double ry = rc.Height()/2;
	double x0 = rc.left + rx;
	double y0 = rc.top + ry;

	if(sel || state&GUI_CHECKED)
	{
		rx+=12;
		ry+=12;
		x0-=6;
		y0-=6;
	}

	for(int i=0;i<6;++i)
	{
		Q[i].x = x0 + rx*cos(2*M_PI/6*i);
		Q[i].y = y0 + ry*sin(2*M_PI/6*i);
	}
	Q[6] = Q[0];

	gui->DrawSolidPoly(Q,7,c_fondo,FALSE);

	if(sel)
		gui->DrawPoly(Q,7,4,D3DCOLOR_XRGB(255,255,0));
	else
	if(state&GUI_CHECKED)
		gui->DrawPoly(Q,7,3,D3DCOLOR_XRGB(240,245,245));
	else
		gui->DrawPoly(Q,7,1,D3DCOLOR_XRGB(120,120,64));


}

void gui_face::Render(BOOL resplandor)
{

	BOOL sel = gui->sel==nro_item?TRUE:FALSE;

	TVector2d Q[10];
	double rx = rc.Width()/2;
	double ry = rc.Height()/2;
	double x0 = rc.left + rx;
	double y0 = rc.top + ry;


	if(nro_textura != -1)
	{
		D3DXVECTOR3 pos = D3DXVECTOR3(x0-rx+5,y0-ry+7,0);
		pSprite->Draw(model->g_pTexture[nro_textura],NULL,NULL,&pos,0xFFFFFFFF);
	}

	if(sel)
	{
		rx+=4;
		ry+=4;
		x0-=2;
		y0-=2;
	}

	for(int i=0;i<6;++i)
	{
		Q[i].x = x0 + rx*cos(2*M_PI/6*i);
		Q[i].y = y0 + ry*sin(2*M_PI/6*i);
	}
	Q[6] = Q[0];

	gui->DrawSolidPoly(Q,7,D3DCOLOR_XRGB(238,239,244));

	if(sel)
		gui->DrawPoly(Q,7,4,D3DCOLOR_XRGB(255,255,0));
	else
	if(state&GUI_CHECKED)
		gui->DrawPoly(Q,7,4,D3DCOLOR_XRGB(240,64,64));
	else
		gui->DrawPoly(Q,7,2,D3DCOLOR_XRGB(64,128,128));

	if(state&GUI_CHECKED && !gui->closing)
	{

		D3DXMATRIX mat;
		D3DXMATRIX matAnt;
		D3DXVECTOR2 escale;
		D3DXVECTOR2 offset;
		float K = 1.5;
		offset.x = gui->ox/K;
		offset.y = gui->oy/K;
		escale.x = gui->ex*K;
		escale.y = gui->ey*K;

		// Falta Configurar esta posicion. Representa la seleccion actual
		int xi = 80;
		int yi = 40;
		Q[0].x = xi-15;
		Q[0].y = yi-15;
		Q[1].x = xi+80;
		Q[1].y = yi-15;
		Q[2].x = xi+80;
		Q[2].y = yi+80;
		Q[3].x = xi-15;
		Q[3].y = yi+80;
		Q[4] = Q[0];
		gui->DrawSolidPoly(Q,5,D3DCOLOR_XRGB(255,176,138));
		gui->DrawPoly(Q,5,2,D3DCOLOR_XRGB(240,235,245));

		D3DXVECTOR3 pos = D3DXVECTOR3(xi/K,yi/K,0);
		pSprite->GetTransform(&matAnt);

		D3DXMatrixTransformation2D(&mat,NULL,0.0,&escale,NULL,NULL,&offset);
		pSprite->SetTransform(&mat);

		pSprite->Draw(model->g_pTexture[nro_textura],NULL,NULL,&pos,0xFFFFFFFF);
		pSprite->SetTransform(&matAnt);

	}



}


void gui_edit::Render(BOOL resplandor)
{
	BOOL sel = gui->sel==nro_item?TRUE:FALSE;
	BOOL foco = gui->foco==nro_item?TRUE:FALSE;
	float delay_sel = gui->delay_sel;

	// recuadro del edit
	gui->RoundRect(rc.left,rc.top,rc.right,rc.bottom,11,2,D3DCOLOR_XRGB(80,220,20));

	
	if(foco)
	{
		// tiene foco
		gui->RoundRect(rc.left,rc.top,rc.right,rc.bottom,8,1,D3DCOLOR_ARGB(255,255,255,255),TRUE);
	}

	// Texto del edit
	D3DCOLOR color = foco?D3DCOLOR_XRGB( 0,0,0):D3DCOLOR_XRGB(130,255,130);
	if(!resplandor)
		g_pFont->DrawText( pSprite, text, -1, &rc, DT_NOCLIP|DT_TOP|DT_LEFT, color);

	if(foco && !resplandor)
	{
		// si esta vacio, le agrego una I para que cuente bien el alto del caracter
		char *p = !strlen(text)?"I":text;
		CRect rc2 = rc;
		g_pFont->DrawText( pSprite, p, -1, &rc2, DT_NOCLIP|DT_TOP|DT_CALCRECT,color);
		rc2.left = rc2.right;
		rc2.right += 12;

		// dibujo el cursor titilando
		int cursor = (int)(gui->time*5);
		if(cursor%2)
		{
			ppLine->SetWidth(8);
			D3DXVECTOR2 pt[5];
			pt[0].x = rc2.left;
			pt[1].x = rc2.right;
			pt[1].y = pt[0].y = rc2.bottom;

			gui->Transform(pt,2);
			ppLine->Begin();
			ppLine->Draw(pt,2,D3DCOLOR_XRGB(0,64,0));
			ppLine->End();
		}
	}		

}

int gui_edit::ProcessMsg(MSG *Msg)
{
	int rta = 0;
	int c = Msg->wParam;

	// proceso el mensaje
	switch(Msg->message)
	{

		case WM_KEYUP:
			if(c==VK_BACK && len)
			{
				if(GetAsyncKeyState(VK_CONTROL))
					text[len = 0] = '\0';
				else
					text[--len] = '\0';
				rta = 1;		// proceso el msg
			}
			break;
		case WM_CHAR:
			if(c>=32 && c<=0x7F && len<sizeof(text)-5)
			{
				text[len++] = c;
				text[len] = '\0';
				rta = 1;		// proceso el msg
			}
			break;
	}

	return rta;		
}

void gui_frame::Render(BOOL resplandor)
{

	BOOL sel = gui->sel==nro_item?TRUE:FALSE;
	float delay_sel = gui->delay_sel;

	TVector2d pt[255];
	float da = M_PI/8;
	float alfa;
	
	float x0 = rc.left;
	float x1 = rc.right;
	float y0 = rc.top;
	float y1 = rc.bottom;
	float r = 10;
	int t =0;
	float x = x0;
	float y = y0;
	for(alfa =0;alfa<M_PI/2;alfa+=da)
	{
		pt[t].x = x - r*cos(alfa);
		pt[t].y = y - r*sin(alfa);
		++t;
	}
	pt[t].x = x;
	pt[t].y = y-r;
	++t;

	pt[t].x = (x1+x0)/2;
	pt[t].y = y-r;
	++t;
	pt[t].x = (x1+x0)/2+50;
	pt[t].y = y+20-r;
	++t;

	x = x1;
	y = y0+20;
	for(alfa =M_PI/2;alfa<M_PI;alfa+=da)
	{
		pt[t].x = x - r*cos(alfa);
		pt[t].y = y - r*sin(alfa);
		++t;
	}
	pt[t].x = x+r;
	pt[t].y = y;
	++t;


	x = x1;
	y = y1;
	for(alfa =0;alfa<M_PI/2;alfa+=da)
	{
		pt[t].x = x + r*cos(alfa);
		pt[t].y = y + r*sin(alfa);
		++t;
	}
	pt[t].x = x;
	pt[t].y = y+r;
	++t;

	
	pt[t].x = x0+150;
	pt[t].y = y+r;
	
	++t;
	pt[t].x = x0+100;
	pt[t].y = y-20+r;
	++t;


	x = x0;
	y = y-20;
	for(alfa =M_PI/2;alfa<M_PI;alfa+=da)
	{
		pt[t].x = x + r*cos(alfa);
		pt[t].y = y + r*sin(alfa);
		++t;
	}
	pt[t++] = pt[0];


	// interior
	if(!resplandor)
		gui->DrawSolidPoly(pt,t,c_fondo);

	// contorno
	gui->DrawPoly(pt,t,resplandor?1:6,D3DCOLOR_XRGB(130,255,130));

	if(!resplandor)
	{
		// Texto del frame
		RECT rc2 = rc;
		rc2.top += 15;
		rc2.left += 30;
		D3DCOLOR color = sel?D3DCOLOR_XRGB( 255,220,220):D3DCOLOR_XRGB(130,255,130);
		g_pFont->DrawText( pSprite, text, -1, &rc2, DT_NOCLIP|DT_TOP, color);
	}


}

void gui_rect::Render(BOOL resplandor)
{
	BOOL sel = gui->sel==nro_item?TRUE:FALSE;
	D3DCOLOR color = sel?D3DCOLOR_XRGB( 255,220,220):D3DCOLOR_XRGB(130,255,130);
	gui->RoundRect(rc.left,rc.top,rc.right,rc.bottom,radio,resplandor?5:2,color,FALSE);
}


void gui_ruta::Create(int truta)
{
	tipo_ruta = truta;
	if(tipo_ruta==-1)
	{
		cant_ptos = 0;
		return;
	}

	cant_ptos = model->load_pt_ruta(tipo_ruta,pt_ruta);

	// en principio la ruta ocupa el diametro del domo, pero 
	// en la altura, no se, tengo que averiguar la altura maxima
	double dr = 2*model->radio;
	for(int i=0;i<cant_ptos;++i)
		if(2*pt_ruta[i].y > dr)
			dr = 2*pt_ruta[i].y;


	double dx = rc.Width();
	double dy = rc.Height()-30;
	double ex = dx / dr;
	double ey = dy / dr;
	if(ex<ey)
		ey = ex;
	else
		ex = ey;

	double k = 0.707;

	// los paso a 2d
	for(i=0;i<cant_ptos;++i)
	{
		pt[i].x = pt_ruta[i].x*ex - pt_ruta[i].z*k*ex;
		pt[i].y = pt_ruta[i].y*ey + pt_ruta[i].z*k*ey;
	}
}

void gui_ruta::Render(BOOL resplandor)
{

	BOOL sel = gui->sel==nro_item?TRUE:FALSE;
	float delay_sel = gui->delay_sel;
	if(resplandor && !sel)
		return;

	if(cant_ptos)
	{
		double ant_ox = gui->ox;
		double ant_oy = gui->oy;
		// cambio el origen:
		gui->ox = (rc.left + rc.right)/2;
		gui->oy = (rc.top + rc.bottom)/2 - 15;
		char checked = state&GUI_CHECKED;
		gui->DrawPoly(pt,cant_ptos,sel&&!resplandor?5:checked?4:2,
			sel?D3DCOLOR_XRGB(40,245,210):
			checked?D3DCOLOR_XRGB(230,245,240):D3DCOLOR_XRGB(230,245,40));

		// restauro el origen:
		gui->ox = ant_ox;
		gui->oy = ant_oy;
	}

	// Texto del frame
	RECT rc2 = rc;
	rc2.top = rc.bottom-20;
	rc2.bottom = rc.bottom;
	D3DCOLOR color = sel?D3DCOLOR_XRGB( 255,220,220):D3DCOLOR_XRGB(130,255,130);
	g_pFont->DrawText( pSprite, text, -1, &rc2, DT_NOCLIP|DT_TOP, color);



}


DXGui::DXGui()
{
	cant_items = 0;
	cant_dialog = 0;
	resplandor_enabled = FALSE;
}


DXGui::~DXGui()
{
	Reset();
}

void DXGui::Reset()
{
	for(int i=0;i<cant_items;++i)
		delete items[i];
	cant_items = 0;
	sel = -1;
	time = 0;
	total_delay = 0;
	item_0 = 0;
	sin_delay = FALSE;

	ey = ex = 1;
	ox = oy = 0;
	closing = FALSE;

}


void DXGui::Create(CGameEngine *m)
{
	Reset();
	model = m;
	gui_item::pSprite= m->pSprite;
	gui_item::ppLine= m->ppLine;
	gui_item::model = m;
	gui_item::gui = this;

}

void DXGui::InitDialog(BOOL delay)
{
	sin_delay = !delay;
	// guardo el valor de item_0 en la pila
	pila_item_0[cant_dialog++] = item_0;
	// y el primer item del nuevo dialog es cant items
	item_0 = cant_items;
	foco = -1;
	rbt = -1;
	sel = -1;
	closing = FALSE;

	SetCursorPos(model->d3dpp.BackBufferWidth/2,model->d3dpp.BackBufferHeight/2);

}


void DXGui::EndDialog()
{
	// primero libero la memoria de los items asociados al dialogo actual
	for(int i=item_0;i<cant_items;++i)
	{
		delete items[i];
		items[i] = NULL;
	}
	// actualizo la cantidad de items
	cant_items = item_0;

	// recupero el valor de item_0
	item_0 = pila_item_0[--cant_dialog];

	foco = -1;

	// valores x defecto
	sin_delay = FALSE;
	ey = ex = 1;
	ox = oy = 0;

}



void DXGui::Update()
{
	// timers 
	double elapsed_time = model->elapsed_time;
	time+=elapsed_time;
	if(delay_sel>0)
	{
		delay_sel-=elapsed_time;
		if(delay_sel<0)
			delay_sel = 0;
	}

	for(int i=item_0;i<cant_items;++i)
	{
		if(items[i]->delay>0)
		{
			items[i]->delay-=elapsed_time;
			if(items[i]->delay<0)
			{
				items[i]->delay = 0;
				items[i]->ftime = 0;		// start time
			}

		}
		else
			items[i]->ftime += model->elapsed_time;
	}
}


int DXGui::ProcessMsg(MSG *Msg)
{
	int rta = 0;
	int ant_sel = sel;
	GetCursorPos(&pt);
	ScreenToClient(model->m_hWnd , &pt);


	if(foco!=-1 && foco>=item_0 && foco<cant_items)
	{
		// hago que el item con foco procese el msg
		if(items[foco]->ProcessMsg(Msg))
			return rta;		// si el item proceso el msg retorno sin mas procesamiento
	}


	// proceso el mensaje
	switch(Msg->message)
	{

		case WM_MOUSEWHEEL:
			{
				short zDelta = HIWORD(Msg->wParam);    // wheel rotation
			}
			break;

		case WM_LBUTTONDOWN:
			if(sel!=-1)
			{
				switch(items[sel]->tipo)
				{
					default:
						if(items[sel]->state &GUI_CHECKED)
							items[sel]->state &= 0xFFFF - GUI_CHECKED;
						else
							items[sel]->state |= GUI_CHECKED;
						break;
					case GUI_COLOR:
						// selecciono el color
						items[sel]->state |= GUI_CHECKED;
						rbt = sel;
						// y deselecciono cualquier otro item color
						{
							for(int t=item_0;t<cant_items;++t)
								if(t!=sel && items[t]->tipo==GUI_COLOR)
									items[t]->state &= 0xFFFF - GUI_CHECKED;
						}
						sel_color = items[sel]->c_fondo;
						break;
					case GUI_FACE:
						items[sel]->state |= GUI_CHECKED;
						// y deselecciono cualquier otro item FACE
						{
							for(int t=item_0;t<cant_items;++t)
								if(t!=sel && items[t]->tipo==GUI_FACE)
									items[t]->state &= 0xFFFF - GUI_CHECKED;
						}
						face_sel = sel;
						break;

					case GUI_RUTA:
						// 
						{
							// desmarco todos los anteriores
							for(int i=item_0;i<cant_items;++i)
								if(items[i]->tipo==GUI_RUTA)
									items[i]->state &= 0xFFFF - GUI_CHECKED;
							// marco el seleccionado
							items[sel]->state |= GUI_CHECKED;
							model->tipo_ruta = ((gui_ruta*)items[sel])->tipo_ruta;
							model->load_ruta();
							model->DXCreateVertex();

						}
						break;

					case GUI_TOOLBAR:
						// termino el dialogo (sin delay)
						rta = 2;		// idok
						break;

					case GUI_MENU_ITEM:
					case GUI_BUTTON:
						// termino el dialogo
						rta = 1;		// idok
						break;

					case GUI_EDIT:
						// le doy foco al edit
						foco = sel;
						break;
				}
			}
			break;

		case WM_LBUTTONUP:
			break;

		case WM_MOUSEMOVE:
			{
				// verifico si el cusor pasa por arriba de un item
				sel = -1;
				int t = item_0;
				while(t<cant_items && sel==-1)
					if(items[t]->tipo>=GUI_SEL && items[t]->pt_inside(pt))
						sel = t;
					else
						++t;
			}

			break;

		case WM_KEYDOWN:
			switch((int) Msg->wParam)	    // virtual-key code 
			{
				case VK_F3:
					rta = -1;
					break;
				case VK_UP:
					--sel;
					if(sel<item_0)
						sel = cant_items-1;
					break;
				case VK_DOWN:
					++sel;
					if(sel==cant_items)
						sel = item_0;
					break;

			}
			break;
	}


	if(ant_sel!=sel)
		// cambio de seleccion
		delay_sel = 0.25;
	return rta;		// 0->cotinua, 1->termina el dialogo con IDOK, -1 cancela el dialogo
}


void DXGui::Render(BOOL resplandor)
{

	if(resplandor && !resplandor_enabled)
		return;

	model->g_pd3dDevice->BeginScene();
	model->g_pd3dDevice->SetTexture( 0, NULL);		// elimino cualquier textura que me cague el modulate del vertex color


	model->pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	
	D3DXMATRIX mat;
	D3DXMATRIX matAnt;
	D3DXVECTOR2 escale;
	D3DXVECTOR2 offset;
	model->pSprite->GetTransform(&matAnt);

	double ant_ex = ex;
	double ant_ey = ey;
	if(resplandor)
	{
		// correccion escala resplandor: 
		ex *= 0.35;
		ey *= 0.35;
	}

	escale.x = ex;
	escale.y = ey;
	offset.x = ox;
	offset.y = oy;

	D3DXMatrixTransformation2D(&mat,NULL,0.0,&escale,NULL,NULL,&offset);
	mat._12 *= 5;
	mat._14 *= 5;
	model->pSprite->SetTransform(&mat);

	for(int i=item_0;i<cant_items;++i)
		if(items[i]->delay<=0 || sin_delay)
			items[i]->Render(resplandor);

	// dibujo el cusor (solo si esta en modo gui)
	if(model->modo_gui && !resplandor)
	{
		D3DXVECTOR3 pos = D3DXVECTOR3(pt.x-8,pt.y,0);
		model->pSprite->Draw(model->g_pTexture[model->textura_cursor],NULL,NULL,&pos,0xFFFFFFFF);
	}
	
	
	model->pSprite->SetTransform(&matAnt);
	model->pSprite->End();

	model->g_pd3dDevice->EndScene();

	
	if(resplandor)
	{
		// restauro la escala anterior
		ex = ant_ex;
		ey = ant_ey;
	}

}

gui_item::gui_item()
{
	nro_item = -1;		// no pertenece a ninguna gui 
	tipo = 0;
	ftime = 0;
	state = 0;
	pc = 0;
	strcpy(text, "");
	rc.SetRect(0,0,0,0);
	rc_original = rc;
	nro_textura = -1;
	delay0 = delay = 0;
	len = 0;
	c_fondo = D3DCOLOR_ARGB(80,30,155,110);
	c_font = -1;		// indica que no tiene color definido, usa uno x defecto

	vel_char = 120000;		// caracters x segundo

}	


gui_item *DXGui::InsertItem(char tipo,char *s,int x,int y,int dx,int dy)
{

	switch(tipo)
	{
		default:
			items[cant_items] = new gui_item;
			break;
		case GUI_BUTTON:
			items[cant_items] = new gui_button;
			break;
		case GUI_FRAME:
			items[cant_items] = new gui_frame;
			break;
		case GUI_RECT:
			items[cant_items] = new gui_rect;
			break;
		case GUI_RUTA:
			items[cant_items] = new gui_ruta;
			break;
		case GUI_EDIT:
			items[cant_items] = new gui_edit;
			break;
		case GUI_COLOR:
			items[cant_items] = new gui_color;
			break;
		case GUI_FACE:
			items[cant_items] = new gui_face;
			break;
		case GUI_TOOLBAR:
			items[cant_items] = new gui_tb_button;
			break;
	}
	gui_item *pitem = items[cant_items];
	pitem->nro_item = cant_items;
	pitem->tipo = tipo;
	pitem->flags = tipo==GUI_MENU_ITEM || tipo==GUI_EDIT?GUI_CURSOR:0;
	pitem->g_pFont = model->g_pFontb;
	pitem->ftime = 0;
	pitem->state = 0;
	pitem->pc = 0;
	strcpy(pitem->text, s);
	pitem->rc.SetRect(x,y,x+dx,y+dy);
	pitem->rc_original = pitem->rc;
	pitem->nro_textura = -1;
	pitem->delay0 = pitem->delay = total_delay;
	pitem->len = strlen(s);
	if(tipo==GUI_MENU_ITEM)
		pitem->vel_char = _vel_char_menu;		// caracters x segundo
	total_delay+=(float)pitem->len/pitem->vel_char;
	++cant_items;


	return pitem;
}

gui_ruta *DXGui::InsertItemRuta(int tipo_ruta,char *s,int x,int y,int dx,int dy)
{
	gui_ruta *pitem = (gui_ruta*)InsertItem(GUI_RUTA,s,x,y,dx,dy);
	pitem->Create(tipo_ruta);
	return pitem;
}

gui_rect *DXGui::InsertItemRect(char *s,int x,int y,int dx,int dy,int r)
{
	gui_rect *pitem = (gui_rect*)InsertItem(GUI_RECT,s,x,y,dx,dy);
	pitem->radio = r;
	return pitem;
}

gui_color *DXGui::InsertItemColor(int x,int y,D3DCOLOR color)
{
	gui_color *pitem = (gui_color*)InsertItem(GUI_COLOR,"",x,y,50,50);
	pitem->c_fondo = color;
	return pitem;
}

// restuara las posiciones originales	
// usualemnte se llama despues de alguna animacion de los items en pantalal
void DXGui::RestaurarPos()
{
	for(int i=0;i<cant_items;++i)
		items[i]->rc = items[i]->rc_original;
}

void DXGui::Transform(VERTEX2D pt[],int cant_ptos)
{
	for(int i=0;i<cant_ptos;++i)
	{
		pt[i].x = ox + pt[i].x*ex;
		pt[i].y = oy + pt[i].y*ey;
	}
}


void DXGui::Transform(D3DXVECTOR2 pt[],int cant_ptos)
{
	for(int i=0;i<cant_ptos;++i)
	{
		pt[i].x = ox + pt[i].x*ex;
		pt[i].y = oy + pt[i].y*ey;
	}
}


void DXGui::DrawPoly(TVector2d V[],int cant_ptos,int dw,D3DCOLOR color)
{
	if(dw<1)
		dw = 1;
	// Elimino ptos repetidos
	TVector2d P[1000];
	int cant = 1;
	P[0] = V[0];
	for(int i=1;i<cant_ptos;++i)
		if((V[i]-V[i-1]).mod()>0.01)
			P[cant++] = V[i];

	cant_ptos = cant;
	BOOL closed  = (P[0]-P[cant_ptos-1]).mod()<0.1?TRUE:FALSE;

	
	// calculo el offset
	TVector2d Q[1000];
	TVector2d N[1000];
	for(i=0;i<cant_ptos-1;++i)
	{
		TVector2d p0 = P[i];
		TVector2d p1 = P[i+1];
		TVector2d v = p1-p0;
		v.normalizar();
		N[i] = v.normal();
	}

	// ptos intermedios
	int i0 = closed?0:1;
	for(i=i0;i<cant_ptos;++i)
	{
		int ia = i?i-1:cant_ptos-2;
		TVector2d n = N[ia]+N[i];
		n.normalizar();
		double r = (N[ia]>>n);
		if(r!=0)
			Q[i] = P[i] + n*((double)dw/r);
		else
			Q[i] = P[i];

	}

	if(!closed)
	{
		// poligono abierto: primer y ultimo punto: 
		Q[0] = P[0] + N[0]*dw;
		Q[cant_ptos-1] = P[cant_ptos-1] + N[cant_ptos-2]*dw;
	}
	else
		Q[cant_ptos-1] = Q[0];
	

	VERTEX2D pt[4000];
	int t = 0;

	for(i=0;i<cant_ptos-1;++i)
	{
		// 1er triangulo
		pt[t].x = P[i].x;
		pt[t].y = P[i].y;
		pt[t+1].x = Q[i].x;
		pt[t+1].y = Q[i].y;
		pt[t+2].x = P[i+1].x;
		pt[t+2].y = P[i+1].y;


		// segundo triangulo
		pt[t+3].x = Q[i].x;
		pt[t+3].y = Q[i].y;
		pt[t+4].x = P[i+1].x;
		pt[t+4].y = P[i+1].y;
		pt[t+5].x = Q[i+1].x;
		pt[t+5].y = Q[i+1].y;

		for(int j=0;j<6;++j)
		{
			pt[t].z = 0.5;
			pt[t].rhw = 1;
			pt[t].color = color;
			++t;
		}
	}

	Transform(pt,t);

	// dibujo como lista de triangulos
	model->g_pd3dDevice->SetFVF(D3DFVF_VERTEX2D);
	model->g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST,
		2*(cant_ptos-1),&pt,sizeof(VERTEX2D));

}
	

void DXGui::DrawSolidPoly(TVector2d P[],int cant_ptos,D3DCOLOR color,BOOL gradiente)
{
	// calculo el centro de gravedad
	double xc = 0;
	double yc = 0;
	double ymin = 100000;
	double ymax = -100000;

	for(int i=0;i<cant_ptos-1;++i)
	{
		xc+=P[i].x;
		yc+=P[i].y;

		if(P[i].y>ymax)
			ymax = P[i].y;
		if(P[i].y<ymin)
			ymin = P[i].y;

	}

	xc/=(double)(cant_ptos-1);
	yc/=(double)(cant_ptos-1);

	double dy = max(1,ymax - ymin);

	BYTE a =  (BYTE) ((color>>24) & 0xFF);
	BYTE r =  (BYTE) ((color>>16) & 0xFF);
	BYTE g =  (BYTE) ((color>>8) & 0xFF);
	BYTE b =  (BYTE) (color&0xFF);


	VERTEX2D pt[4000];
	pt[0].x = xc;
	pt[0].y = yc;
	for(i=0;i<cant_ptos;++i)
	{
		pt[i+1].x = P[i].x;
		pt[i+1].y = P[i].y;
	}

	for(i=0;i<cant_ptos+1;++i)
	{
		pt[i].z = 0.5;
		pt[i].rhw = 1;
		if(gradiente)
		{
			double k = 1 - (pt[i].y - ymin) / dy * 0.5;
			pt[i].color = D3DCOLOR_ARGB(a,
				min(255,(BYTE)(r*k)),min(255,(BYTE)(g*k)),min(255,(BYTE)(b*k)));
		}
		else
			pt[i].color = color;
	}

	Transform(pt,cant_ptos+1);
	model->g_pd3dDevice->SetFVF(D3DFVF_VERTEX2D);
	model->g_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,cant_ptos-1,&pt,sizeof(VERTEX2D));
}

void DXGui::RoundRect(int x0,int y0,int x1,int y1,int r,int dw,D3DCOLOR color,BOOL solid)
{
	if(dw<1)
		dw = 1;
	TVector2d pt[1000];
	float da = M_PI/8;
	float alfa;
	
	int t =0;
	float x = x0;
	float y = y0;
	for(alfa =0;alfa<M_PI/2;alfa+=da)
	{
		pt[t].x = x - r*cos(alfa);
		pt[t].y = y - r*sin(alfa);
		++t;
	}
	pt[t].x = x;
	pt[t].y = y-r;
	++t;

	x = x1;
	y = y0;
	for(alfa =M_PI/2;alfa<M_PI;alfa+=da)
	{
		pt[t].x = x - r*cos(alfa);
		pt[t].y = y - r*sin(alfa);
		++t;
	}
	pt[t].x = x+r;
	pt[t].y = y;
	++t;


	x = x1;
	y = y1;
	for(alfa =0;alfa<M_PI/2;alfa+=da)
	{
		pt[t].x = x + r*cos(alfa);
		pt[t].y = y + r*sin(alfa);
		++t;
	}
	pt[t].x = x;
	pt[t].y = y+r;
	++t;

	x = x0;
	y = y1;
	for(alfa =M_PI/2;alfa<M_PI;alfa+=da)
	{
		pt[t].x = x + r*cos(alfa);
		pt[t].y = y + r*sin(alfa);
		++t;
	}
	pt[t++] = pt[0];

	if(solid)
		DrawSolidPoly(pt,t,color);
	else
		DrawPoly(pt,t,dw,color);
}