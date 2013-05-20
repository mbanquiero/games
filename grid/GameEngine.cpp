#include "stdafx.h"
#include "grid.h"
#include "GameEngine.h"

// motor3d 
CGameEngine escena;


/////////////////////////////////////////////////////////////////////////////
// Jugador
/////////////////////////////////////////////////////////////////////////////
double player::max_an_lat = M_PI/4;	// maxima inclinacion lateral 
double player::max_w0 = 3*M_PI/3;		// maxima velocidad de giro, en grados por segundo.
TVector3d player::size_cycle = TVector3d(0.5,0.75,2)*1.2;	
double player::treaccion = 0.1;			// tiempo de reaccion
double player::dist_radar = 9;

float _g = 10;

BOOL vidas_infinitas = FALSE;
BOOL _generar_bmps = FALSE;

D3DCOLOR _lst_colores[] = {
							D3DCOLOR_XRGB(100,220,255),
							D3DCOLOR_XRGB(100,255,220),
							D3DCOLOR_XRGB(220,100,255),
							D3DCOLOR_XRGB(220,255,100),
							D3DCOLOR_XRGB(255,100,220),
							D3DCOLOR_XRGB(255,220,100),
							D3DCOLOR_XRGB(128,128,128),
							D3DCOLOR_XRGB(64,255,64),
							D3DCOLOR_XRGB(64,64,255),
							D3DCOLOR_XRGB(255,0,255),
							D3DCOLOR_XRGB(255,255,0),
							D3DCOLOR_XRGB(0,255,255)};
int _cant_colores = sizeof(_lst_colores)/sizeof(D3DCOLOR);



player::player()
{
	init = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void player::Create(CGameEngine *m,TVector3d pos,int n)
{

	model = m;
	nro_player = n;
	g_pd3dDevice = m->g_pd3dDevice;
	g_pMeshCycle = m->g_pMeshCycle;
	
	// muevo: las 2 mallas son practicamente iguales...
	cycle_mesh = &m->Mesh[nro_player==0?0:6];

	Pos_Inicial = Pos = pos;
	pos_i = model->cant_cell/2 + Pos.x / model->dgrid;
	pos_j = model->cant_cell/2 + Pos.z / model->dgrid;

	Vmoto = Dir = pos*(-1);
	Dir.normalizar();
	calc_angulos();
	alfa_d = alfa;

	an_lat = 0;
	// modulo de la vel. en metros x segundos
	switch(m->nivel)
	{
		case 0:
		default:
			_g = 10;
			vel_ini = vel = nro_player?12:10;
			vel_salto = sqrt(2*_g*1.2);
			dist_radar = 9;
			break;
		case 1:
			_g = 12;
			vel_ini = vel = nro_player?20:18;
			vel_salto = sqrt(2*_g*1.1);
			dist_radar = 12;
			break;
		case 2:
			_g = 14;
			vel_ini = vel = 35;
			vel_salto = sqrt(2*_g*1);
			dist_radar = 25;
			break;
		case 3:
			_g = 20;
			vel_ini = vel = 50;
			vel_salto = sqrt(2*_g*1);
			dist_radar = 30;
			break;
	}


	// los enemigos x defecto tienen la explosion barata, total mucho no los veo
	explosion_barata = nro_player?TRUE:FALSE;

	/*
	if(nro_player)
	{
		vel_ini *= 2;
		vel *= 2;
	}*/

	vel_v = 0;		// velocidad vertical

	pos_en_ruta = -1;
	timer_light_path = timer_rearmar_moto = timer_cayendo = timer_rebotando = 0;
	timer_reculando = 0;
	implotando = FALSE;
	derrapando = 0;
	timer_girando = 0;
	timer_evitando = 0;
	giro_horario = TRUE;
	juego_justo = FALSE;

	g_pVBPath = NULL;

	if(!init)
	{
		// personalizaciond el jugador (los valores x defecto solo la primera vez)
		switch(nro_player)
		{
			case 0:
				c_lightpath_espejo = D3DCOLOR_XRGB(43,97,113);
				c_lightpath = D3DCOLOR_XRGB(100,220,255);
				c_linepath = D3DCOLOR_XRGB(100,100,255);
				break;
			default:
			case 1:
				c_lightpath_espejo = D3DCOLOR_XRGB(113,97,43);
				c_lightpath = D3DCOLOR_XRGB(255,220,100);
				c_linepath = D3DCOLOR_XRGB(255,100,100);
				break;
			case 2:
				c_lightpath_espejo = D3DCOLOR_XRGB(97,113,43);
				c_lightpath = D3DCOLOR_XRGB(220,255,100);
				c_linepath = D3DCOLOR_XRGB(100,255,100);
				break;
			case 3:
				c_lightpath_espejo = D3DCOLOR_XRGB(100,100,43);
				c_lightpath = D3DCOLOR_XRGB(155,155,100);
				c_linepath = D3DCOLOR_XRGB(90,155,90);
				break;
		}

		sprintf(nombre,"player %d",nro_player+1);
		strcpy(slogan,"");
		nro_color = nro_player;
		nro_avatar = nro_player%6;

		// solo la primera vez: levanto la interface de sonido
		switch(nro_player)
		{
			case 0:
				sound.Create(m->m_hWnd,"sounds\\engine1.wav");
				sound.k_start = 0.1;
				sound.k_decay = 0.6;
				sound.k_end = 0.8;
				break;
			case 1:
				sound.Create(m->m_hWnd,"sounds\\Spaceship.wav");
				break;
		}
		mute = FALSE;
		init = TRUE;
	}


	cant_vidas = 1;
	score = 0;

	// multiplayer
	cant_paquetes = 0;
	paq_desde = 0;
	ack = TRUE;


}

void player::Reset()
{
	cant_vidas = model->modo_juego==GAME_GRID_BATTLE?1:model->multiplayer?6:3;
	Pos = Pos_Inicial;
	pos_i = model->cant_cell/2 + Pos.x / model->dgrid;
	pos_j = model->cant_cell/2 + Pos.z / model->dgrid;

	Vmoto = Dir = Pos*(-1);
	Dir.normalizar();
	calc_angulos();
	alfa_d = alfa;
	an_lat = 0;
	vel = vel_ini;
	vel_v = 0;		// velocidad vertical
	pos_en_ruta = -1;
	timer_light_path = timer_rearmar_moto = timer_cayendo = timer_rebotando = 0;
	timer_reculando = 0;
	implotando = FALSE;
	derrapando = 0;
	timer_girando = 0;
	timer_evitando = 0;
	giro_horario = TRUE;
	juego_justo = FALSE;
	initPath();
	actualizarPath();
}


// sugiere una pos. de la camara para ver la moto 
void player::SetCamara()
{
	if(model->fpc)
	{
		TVector3d dir_p = Dir;
		model->LA = Pos + dir_p*50;
		model->LF = Pos + dir_p*2+ TVector3d(0,1.5,0);
		model->smooth_cam = FALSE;
		return;
	}
	

	if(derrapando==3)
	{
		// se estrolo contra la pared. Dejo la camara quieta, y que siga a la moto, mientras
		// da vueltas:
		model->LA = Pos;
		model->smooth_cam = FALSE;
		return;
	}


	if(pos_en_ruta>=0)
	{
		// esta en la ruta
		model->LA = Pos + Dir*25;
		model->LF = Pos - Dir*10 + TVector3d(0,2,1);
		model->smooth_cam = FALSE;

	}
	else
	{
		// esta en el suelo
		TVector3d dir_p = Dir;
		dir_p.y = 0;
		dir_p.normalizar();
		TVector3d n = TVector3d(0,1,0)*dir_p;
		double d = Pos.mod();
		if(model->cant_ptos_ruta)
		{
			// caso general
			model->LA = Pos + dir_p*30;
			model->LF = Pos - dir_p*12 + TVector3d(0,3,0) + n*3.5;
			model->smooth_cam = FALSE;
		}
		else
		{
			if(d>=model->radio-8)
			{
				// caso particular: esta muy cerca de la pared del domo:
				model->LF_d = Pos - dir_p*4 + TVector3d(0,2,0);
				model->LA_d = Pos + dir_p*30;
				model->smooth_cam = TRUE;
			}
			else
			if(d>=model->radio-15)
			{
				// caso particular: esta relativamente cerca de la pared del domo:
				model->LF_d = Pos - dir_p*12 + TVector3d(0,9,0) + n*3.5;
				model->LA_d = Pos + dir_p*30;
				model->smooth_cam = TRUE;
			}
			else
			if(d>=model->radio-20 && model->smooth_cam)
			{
				// caso particular: estaba saliendo del smooth cam
				model->LA_d = Pos + dir_p*30;
				model->LF_d = Pos - dir_p*12 + TVector3d(0,3,0) + n*3.5;
			}
			else
			{
				// caso general
				model->LA = Pos + dir_p*30;
				model->LF = Pos - dir_p*12 + TVector3d(0,3,0) + n*3.5;
				model->smooth_cam = FALSE;

			}
		}

	}
}


void player::SetColor(int n)
{
	nro_color = n;
	D3DCOLOR c = c_lightpath = _lst_colores[n];
	BYTE r =  (BYTE) ((c>>16) & 0xFF);
	BYTE g =  (BYTE) ((c>>8) & 0xFF);
	BYTE b =  (BYTE) (c&0xFF);
	c_lightpath_espejo = D3DCOLOR_XRGB(r/2,g/2,b/2);
	cycle_mesh->SetColor(c);
}


void player::poner_en_ruta()
{
	pos_en_ruta = 0;
	TVector3d aux_dir = model->pt_ruta[1] - model->pt_ruta[0];
	Pos = model->pt_ruta[0] + TVector3d(0,1,0);
	aux_dir.normalizar();
	Dir = aux_dir;
	calc_angulos();
}


void player::calc_angulos()
{
	// recupero los alfa, beta
	beta = asin(Dir.y);
	double cosb = cos(beta);
	if(cosb!=0)
	{
		alfa = acos(Dir.x/cosb);
		// pero cos alfa = cos - alfa, entonces pueden ser 2 valores, alfa o -alfa
		double z = cosb*sin(alfa);
		if(fabs(z-Dir.z)>0.01)
			alfa = -alfa;
	}
	else
		alfa = 0;
}

void player::initPath()
{
	if(!cant_vidas)
		return;

	// inicializo el light path
	cant_ptos_delay = -1;
	cant_ptos = 2;
	path[1] = path[0] = Pos;		// posicion inicial
	path_an[1] = path_an[0] = an_lat;
	pos_i = model->cant_cell/2 + Pos.x / model->dgrid;
	pos_j = model->cant_cell/2 + Pos.z / model->dgrid;
	// agrego el tramo asociado a la celda actual
	int t = model->celdas[pos_i][pos_j].cant_tramos++;
	model->celdas[pos_i][pos_j].p[t] = cant_ptos;
	model->celdas[pos_i][pos_j].player[t] = nro_player;


}

void player::crearLaberintoLuz()
{

	char L[64][65];
	memset(L,0,sizeof(L));
	char buffer[255];
	int i =0;
	int i0=-1,j0=-1;
	FILE *fp = fopen("laberinto.dat","rt");
	while(fgets(buffer,sizeof(buffer),fp)!=NULL)
	{
		char *p;
		if((p=strchr(buffer,'+'))!=NULL)
		{
			i0 = i;
			j0 = p-buffer;
		}

		strncpy(L[i],buffer,62);
		L[i][64] = '\0';
		++i;
	}
	fclose(fp);


	if(i0==-1 || j0==-1)
		return;		// error

	BOOL primera_vez = TRUE;
	BOOL hay_mas = TRUE;
	while(hay_mas)
	{

		// simulo que la moto se esta moviendo
		Pos = TVector3d((i0-32)*model->dgrid*0.5,0,(j0-32)*model->dgrid*0.5);
		Vmoto = TVector3d(1,0,0);
		antVel = TVector3d(0,0,0);
		if(primera_vez)
		{
			initPath();
			primera_vez = FALSE;
		}
		else
			actualizarPath();
			

		// borro el actual
		L[i0][j0] = '.';
		// busco el siguiente caracter
		hay_mas = FALSE;
		if(L[i0+1][j0]=='*')
		{
			i0++;
			hay_mas = TRUE;
		}
		else
		if(L[i0-1][j0]=='*')
		{
			i0--;
			hay_mas = TRUE;
		}
		else
		if(L[i0][j0+1]=='*')
		{
			j0++;
			hay_mas = TRUE;
		}
		else
		if(L[i0][j0-1]=='*')
		{
			j0--;
			hay_mas = TRUE;
		}

	}
					
	DXCreatePathVertex();

}


void player::actualizarPath()
{
	if(!cant_vidas || timer_rebotando)
		return;

	// posicion en la grilla
	int ant_pos_i = pos_i;
	int ant_pos_j = pos_j;
	pos_i = model->cant_cell/2 + Pos.x / model->dgrid;
	pos_j = model->cant_cell/2 + Pos.z / model->dgrid;

	// si cambio de direccion, agrego puntos al path
	//if((Vmoto-antVel).mod()> 0.01 && cant_ptos<MAX_PUNTOS )
	//	&& (Pos-path[cant_ptos-1]).mod()>0.01)
	if((Vmoto-antVel).mod()> 0.01 && (Pos-path[cant_ptos-1]).mod()>0.01
		&& cant_ptos<MAX_PUNTOS )
	{

		if(pos_i>=0 && pos_i<model->cant_cell && pos_j>=0 && pos_j<model->cant_cell && 
			model->celdas[pos_i][pos_j].cant_tramos<MAX_TRAMOS)
		{
			// agrego el tramo asociado a la celda actual
			int t = model->celdas[pos_i][pos_j].cant_tramos++;
			model->celdas[pos_i][pos_j].p[t] = cant_ptos;
			model->celdas[pos_i][pos_j].player[t] = nro_player;
		}
		++cant_ptos;
		antVel = Vmoto;
	}
	else
	if(pos_i!=ant_pos_i || pos_j!=ant_pos_j)
	{
		// paso a otra celda, agrego este punto en la celda que esta dejando
		if(ant_pos_i>=0 && ant_pos_i<model->cant_cell && ant_pos_j>=0 && ant_pos_j<model->cant_cell && 
			model->celdas[ant_pos_i][ant_pos_j].cant_tramos<MAX_TRAMOS)
		{
			// agrego el tramo asociado a la celda actual
			int t = model->celdas[ant_pos_i][ant_pos_j].cant_tramos++;
			model->celdas[ant_pos_i][ant_pos_j].p[t] = cant_ptos-1;
			model->celdas[ant_pos_i][ant_pos_j].player[t] = nro_player;
		}
	}

	// actualizo el ultimo punto
	path[cant_ptos-1] = Pos;
	path_an[cant_ptos-1] = an_lat;
}



HRESULT player::DXCreatePathVertex()
{
	if(cant_ptos<2 || !cant_vidas)
		return S_OK;

	// borro el buffer de vertices
	SAFE_RELEASE(g_pVBPath);

	// Alloco memoria y creo un buffer para todos los vertices
	CUSTOMVERTEX* pVertices;
	size_t size = sizeof(CUSTOMVERTEX)*cant_ptos*2*6;
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( size,
				0 , D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVBPath, NULL ) ) )
		return E_FAIL;
	if( FAILED( g_pVBPath->Lock( 0, size, (void**)&pVertices, 0 ) ) )
		return E_FAIL;


	double len_0 = model->time*20;
	double len = len_0;
	double H = 0.7;
	int t = 0;

	int cant = cant_ptos_delay!=-1?cant_ptos_delay:cant_ptos;

	//D3DCOLOR color = D3DCOLOR_XRGB(227,194,87);
	// "pared de luz" vertical 
	D3DCOLOR color = D3DCOLOR_XRGB(255,255,255);
	for(int i=0;i<cant;++i)
	{
		TVector3d v = i==0?path[i+1]-path[i]:path[i]-path[i-1];
		v.normalizar();
		TVector3d vup = TVector3d(0,1,0);
		vup.rotar(TVector3d(0,0,0),v,path_an[i]);
		TVector3d n = vup*v;
		TVector3d Up = v*n;
		TVector3d q0 = path[i] ;
		TVector3d q1 = path[i] +Up*(H*0.8);

		pVertices[t].x = q0.x;
		pVertices[t].y = q0.y;
		pVertices[t].z = q0.z;
		pVertices[t].color = color;
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 0;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 0;
		pVertices[t].tu = len;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = q1.x;
		pVertices[t].y = q1.y;
		pVertices[t].z = q1.z;
		pVertices[t].tv = 1;
		++t;

		if(i<cant)
			len += (path[i+1]-path[i]).mod();
	}


	//  path superior
	for(i=0;i<cant;++i)
	{
		float dn = (float)i/(float)cant*0.06;
		TVector3d v = i==0?path[i+1]-path[i]:path[i]-path[i-1];
		v.normalizar();
		TVector3d vup = TVector3d(0,1,0);
		vup.rotar(TVector3d(0,0,0),v,path_an[i]);
		TVector3d n = vup*v;
		TVector3d Up = v*n;
		TVector3d q0 = path[i] +Up*(H*1.1) + n*dn;
		TVector3d q1 = path[i] +Up*(H*1.1) - n*dn;

		pVertices[t].x = q0.x;
		pVertices[t].y = q0.y;
		pVertices[t].z = q0.z;
		pVertices[t].color = c_lightpath;
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 0;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 0;
		pVertices[t].tu = len;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = q1.x;
		pVertices[t].y = q1.y;
		pVertices[t].z = q1.z;
		pVertices[t].tv = 1;
		++t;

		if(i<cant)
			len += (path[i+1]-path[i]).mod();
	}


	// line - path superior central
	color = D3DCOLOR_XRGB(255,255,0);
	int i0 = max(0,cant-41);
	for(i=cant-1;i>=0;--i)
	{
		TVector3d v = i==0?path[i+1]-path[i]:path[i]-path[i-1];
		v.normalizar();
		TVector3d vup = TVector3d(0,1,0);
		vup.rotar(TVector3d(0,0,0),v,path_an[i]);
		TVector3d n = vup*v;
		TVector3d Up = v*n;
		TVector3d q0 = path[i]+ Up*H;
		
		pVertices[t].x = q0.x;
		pVertices[t].y = q0.y;
		pVertices[t].z = q0.z;
		pVertices[t].color = color;
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 1;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 0;
		pVertices[t].tu = 0;

		++t;
	}

	if(timer_light_path<=0)
	{
		// lateral derecho
		pos_vertex_der = t;
		len = 0;
		for(i=cant-1;i>=i0 && len<10;--i)
		{
			TVector3d v = i==0?path[i+1]-path[i]:path[i]-path[i-1];

			v.normalizar();
			TVector3d vup = TVector3d(0,1,0);
			vup.rotar(TVector3d(0,0,0),v,path_an[i]);
			TVector3d n = vup*v;
			TVector3d Up = v*n;
			TVector3d q0 = path[i]+ Up*H + n*(len*0.05);
			
			pVertices[t].x = q0.x;
			pVertices[t].y = q0.y;
			pVertices[t].z = q0.z;
			pVertices[t].color = c_lightpath;
			pVertices[t].N.x = 0;
			pVertices[t].N.y = 1;
			pVertices[t].N.z = 0;
			pVertices[t].tv = 0;
			pVertices[t].tu = 0;

			++t;

			if(i<cant-1)
				len += (path[i+1]-path[i]).mod();


		}

		pos_vertex_izq = t;
		len = 0;
		for(i=cant-1;i>=i0 && len<10;--i)
		{
			TVector3d v = i==0?path[i+1]-path[i]:path[i]-path[i-1];

			v.normalizar();
			TVector3d vup = TVector3d(0,1,0);
			vup.rotar(TVector3d(0,0,0),v,path_an[i]);
			TVector3d n = vup*v;
			TVector3d Up = v*n;
			TVector3d q0 = path[i]+ Up*H - n*(len*0.05);
			
			pVertices[t].x = q0.x;
			pVertices[t].y = q0.y;
			pVertices[t].z = q0.z;
			pVertices[t].color = c_lightpath;
			pVertices[t].N.x = 0;
			pVertices[t].N.y = 1;
			pVertices[t].N.z = 0;
			pVertices[t].tv = 0;
			pVertices[t].tu = 0;

			++t;

			if(i<cant-1)
				len += (path[i+1]-path[i]).mod();

		}
	}



	g_pVBPath->Unlock();

	return S_OK;
}

void player::DXCleanup()
{
	SAFE_RELEASE(g_pVBPath);
}


void player::RenderLightPath(BOOL resplandor,BOOL espejo)
{
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
	g_pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, espejo?c_lightpath_espejo:c_lightpath);

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE);
	g_pd3dDevice->SetStreamSource( 0, g_pVBPath, 0, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->SetTexture( 0, model->g_pTexture[model->textura_path]);
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,0,(cant_ptos-1)*2);
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

	// line path blanco asi lo agranda

	if(resplandor)
	{
		g_pd3dDevice->SetTexture( 0, NULL);
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);		// uso el color blanco del vertex
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,cant_ptos*2,(cant_ptos-1)*2);
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP,cant_ptos*4,min(40,cant_ptos-1));
		if(timer_light_path<=0)
		{
			int cant_lat = pos_vertex_izq - pos_vertex_der-1;
			g_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP,pos_vertex_der,cant_lat);
			g_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP,pos_vertex_izq,cant_lat);
		}
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	}

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE);

}



void player::ProcessInput(CDirectInputMouse *mouse)
{
	int xm = mouse->dims2.lX;
	int ym = mouse->dims2.lY;
	int zm = mouse->dims2.lZ;

	if(zm)
		// Mouse Wheele
		vel += (float)zm/120.0;

	if(xm)
	{
		// xm, ym tienen el mov. relativo del mouse
		// an = angulo total que quiere girar 
		double an = 0.8*(double)xm/(double)model->d3dpp.BackBufferWidth*2*M_PI;
		double w0 = an/model->elapsed_time;		// velocidad angular
		if(fabs(w0)>max_w0)
			w0 = max_w0 * sign(w0);
		an = w0*model->elapsed_time;
		alfa-=an;
		an_lat-=an;
		if(fabs(an_lat)>max_an_lat)
			an_lat = max_an_lat*sign(an_lat);
	}

	if(mouse->dims2.rgbButtons[0] & 0x80)
		// salto	(en la ruta no me deja saltar)
		if(pos_en_ruta==-1 && Pos.y<=Hpiso + 0.001)
			vel_v = vel_salto;
}


void player::ProcessMsg(MSG *Msg)
{
	switch(Msg->message)
	{
		case WM_MOUSEWHEEL:
			if(!model->modo_camara)
			{
				short zDelta = HIWORD(Msg->wParam);    // wheel rotation
				vel += zDelta/120;
			}
			break;

		case WM_LBUTTONDOWN:
			if(!model->modo_camara)
				// salto	(en la ruta no me deja saltar)
				if(pos_en_ruta==-1 && Pos.y<=Hpiso + 0.001)
					vel_v = vel_salto;
			break;


		case WM_MOUSEMOVE:
			{
				// mouse_dx, mouse_dy tienen el mov. relativo del mouse
				if(!model->modo_camara)
				{
					// an = angulo total que quiere girar 
					double an = 0.8*(double)model->mouse_dx/(double)model->d3dpp.BackBufferWidth*2*M_PI;
					double w0 = an/model->elapsed_time;		// velocidad angular
					if(fabs(w0)>max_w0)
						w0 = max_w0 * sign(w0);
					an = w0*model->elapsed_time;
					alfa-=an;
					an_lat-=an;
					if(fabs(an_lat)>max_an_lat)
						an_lat = max_an_lat*sign(an_lat);
				}

			}
			break;

		case WM_KEYDOWN:
			switch((int) Msg->wParam)	    // virtual-key code 
			{
				case VK_LEFT:
					alfa += M_PI/32;
					break;
				case VK_RIGHT:
					alfa -= M_PI/32;
					break;
				case VK_UP:
					if(!model->modo_camara)
						// salto	(en la ruta no me deja saltar)
						if(pos_en_ruta==-1 && Pos.y<=Hpiso + 0.001)
							vel_v = 8;
					break;
				case VK_ADD:
					vel++;
					break;
				case VK_SUBTRACT:
					vel--;
					break;
			}
			break;
	}

}



void player::PerderVida()
{
	cant_vidas--;

	if(model->multiplayer)
	{
		// le aviso al otro jugador que me estrole
		// se supone que this es el player one, ya que los demas jugadores
		// no llaman a Update, si no a UpdateRemote
		grid_packet packet;
		llenar_paquete(&packet,3);
		if(model->es_servidor)
		{
			// le aviso que choque a todo el mundo
			model->P[1].socket.SendData((char *)&packet,sizeof(packet));
			model->P[1].socket.Flush();
		}
		else
		{
			// le mando mi informacion al servidor
			socket.SendData((char *)&packet,sizeof(packet));
			socket.Flush();
		}
	}
}

void player::UpdateSound()
{
		
	if(nro_player>1)
		return;

	double elapsed_time = model->elapsed_time;
	timer_sound+=elapsed_time;
	if(timer_sound>=sound.lap)
	{


		// llego al tick 
		// le doy un ciclo de procesamiento al sonido
		if(mute)
			sound.volumen = 0;
		else
		if(nro_player)
		{
			// enemigo:
			double dist = (Pos-model->player_one->Pos).distancia();
			sound.volumen = max(0,1.5-dist/60.);

			// efecto dopplert
			TVector3d pos_s = Pos + Dir*vel;
			TVector3d pos_s_p1 = model->player_one->Pos + model->player_one->Dir*model->player_one->vel;
			double dist2 = (pos_s-pos_s_p1).distancia();
			double d = dist - dist2;
			// d>0 se acerca, d<0 se aleja
			if(sound.tipo_mezcla!=2)
			{
				sound.k_freq = max(0.5,3 + d/20); 
				sound.volumen = 0.2;
			}
			else
				sound.k_freq = max(0.5,3 + d/3); 
		}
		else
		{
			// Jugador
			sound.volumen = 1;
			if(sound.tipo_mezcla!=2)
			{
				// Test 440 hz
				sound.k_freq = 1+ (vel-10)/2;
			}
			else
			{
				if(Pos.y>0.1)
				{
					sound.volumen = 1 + Pos.y;
					sound.k_freq = 3 + Pos.y/16;
				}
				else
				{
					sound.volumen = 1;
					sound.k_freq = 2;
					if(fabs(an_lat))
						sound.k_freq +=4 + fabs(an_lat)/4;
				}
				sound.k_freq += (vel-10)/2;
			}

			// le aplico el volumen gral (x el momento en un 25%)
			sound.volumen *= 0.25;


		}

		// 1- que genere mas info de ondas
		sound.WaveOut();
		// 2- que libere buffers anteriores
		sound.Streaming();

		timer_sound=0;
	}
}

void player::UpdateTimers()
{
	double elapsed_time = model->elapsed_time;
	if(timer_light_path>0)
	{
		timer_light_path -= elapsed_time;
		if(timer_light_path<=0)
		{
			timer_light_path = 0;
			cant_ptos_delay = -1;
		}
	}
	if(timer_rearmar_moto>0)
	{
		timer_rearmar_moto -= elapsed_time;
		if(timer_rearmar_moto<=0)
			timer_rearmar_moto = 0;
	}
	if(timer_cayendo>0)
	{
		timer_cayendo -= elapsed_time;
		if(timer_cayendo<=0)
		{
			timer_cayendo = 0;
			pos_en_ruta = -1;
		}
	}
	if(timer_rebotando>0)
	{
		timer_rebotando -= elapsed_time;
		if(timer_rebotando<=0)
		{
			timer_rebotando = 0;
			if(derrapando)
				derrapando = FALSE;
			else
				// habia chocado contra el lightpath o contra algo que la hizo explotar
				// tiene que re-armar la moto: (le doy 1 segundo para salir del bardo)
				timer_rearmar_moto = 1;

			switch(model->modo_juego)
			{
				case GAME_TEST_RUTA:
					// si esta en modo ruta: lo vuelvo a poner al principio de la ruta: 
					// borro todo el light path y empieza de nuevo:
					poner_en_ruta();
					initPath();
					break;
				case GAME_GRID_BATTLE:
					// termina la ronda, re-inicio todo
					model->FinRonda(nro_player);
					break;
			}
		}
	}
}



void player::Update()
{


	if(!cant_vidas && !timer_rebotando)
		return;

	// sonido
	UpdateSound();

	// timers 
	UpdateTimers();

	double elapsed_time = model->elapsed_time;
	ant_alfa_2 = ant_alfa_1;
	ant_alfa_1 = alfa;
	
	// verifico la altura del piso en el lugar de la moto
	TVector3d antPos = Pos;
	TVector3d pos_delantera = Pos+Dir*size_cycle.z;
	char tipo;
	int tramo;
	double H = Hpiso = model->que_altura(Pos.x,Pos.z);
	double Hs = Hpiso_s = model->que_altura(pos_delantera.x,pos_delantera.z,&tipo,&tramo);
	double dH = pos_delantera.y-Hs;
	if(tipo==2 && !timer_rebotando && model->modo_juego!=GAME_TEST_RUTA)
	{
		// esta en el guarray, determino la pendiente lateral
		// para ello uso la direccion n normal a la direccion de la moto
		// y aprovechando que ya tengo el tramo de la ruta donde esta ubicada
		// uso la altura relativa del tramo, en 2 puntos a izquierda y derecha de 
		// la rueda de atras. uso una distancia de un metro para ahorrarme la division 
		// por x, en atan(y/x) (x=1)

		TVector2d n = Dir.pxz().normal();
		n.normalizar();
		TVector2d pizq = Pos.pxz() - n*0.5;
		TVector2d pder = Pos.pxz() + n*0.5;
		double Hizq = model->que_altura(tramo,pizq);
		double Hder = model->que_altura(tramo,pder);
		an_lat = atan(Hizq-Hder);
		// de esta forma la moto se inclina de acuerdo al gradiente de entrada al guarray
		if(fabs(an_lat)>max_an_lat)
		{
			// se cayo porque esta muy inclinada....
			int t = model->aux_tramo; 
			TVector3d N = model->pt_ruta[t+1] - model->pt_ruta[t];
			N.y = 0;
			N.normalizar();
			N.rotar_xz(-M_PI/8);		
			Dir = N;		// que salga por la TG
			// recupero los alfa, beta
			calc_angulos();
			timer_rebotando = 2;
			derrapando = sign(an_lat);
			
			PerderVida();
			return;
		}
	}


	// Secuencicia principal
	
	// 1- Actualizo la direccion
	Dir = TVector3d(cos(beta)*cos(alfa),sin(beta),cos(beta)*sin(alfa));

	// 2- Actualizo la velocidad
	// hay una aceleracion producto del motor de la moto en direccion Dir  (cte) = Dir*vel
	// Y esta la velocidad vertical, cuando aplica un salto, que se ve afectada por la fuerza de gravedad
	if(pos_en_ruta==-1 && Pos.y>H+0.1)
		vel_v -= _g*elapsed_time;		// cae


	// 3- actualizo el angulo lateral
	if(an_lat!=0)
	{
		char signo = sign(an_lat);
		an_lat -= M_PI/2*elapsed_time*signo;
		if(sign(an_lat)!=signo)
			an_lat = 0;
	}

	// 4- la velocidad de la moto es la resultante 
	Cycle_Dir = Vmoto = Dir*vel + TVector3d(0,1,0)*vel_v;
	Cycle_Dir.normalizar();
	Cycle_Up = TVector3d(0,1,0);
	Cycle_Up.rotar(TVector3d(0,0,0),Cycle_Dir,an_lat);
	Cycle_N = Cycle_Dir*Cycle_Up;		// Direccion normal a la trayectoria
	
	//5- Actualizo la posicion pp dicha
	Pos = Pos + Vmoto*elapsed_time;
	Cycle_Pos = Pos+Cycle_Dir*(size_cycle.z/2) + Cycle_Up*(size_cycle.y*0.5);
	pos_delantera = Pos+Cycle_Dir*size_cycle.z;


	// Secuencia de Validaciones
	if(pos_en_ruta==-1 && fabs(vel_v)<0.001)
	{
		// si no tiene vel vertical: se esta moviendo sobre el suelo
		// (ni salta, ni se esta cayendo) 
		if(fabs(dH)<0.001)
			// la diferencia de altura es minima, esta sobre un terreno plano
			// y evito errores de redondeo
			Pos.y = H;
		else
		if(fabs(dH)<0.5)
		{
			// la diferencia de altura no supera cierta medida que la moto puedo 
			// sortear : se adapta al territorio: sube o baja, y cambia el angulo beta
			// para adaptarse a la pendiente
			Pos.y = H;
			// Calculo en angulo vertical
			beta = atan2(Hs-H,size_cycle.z);
			Dir = TVector3d(cos(beta)*cos(alfa),sin(beta),cos(beta)*sin(alfa));

		}
		else
		if(pos_delantera.y<Hs-0.5 && !timer_rebotando)
		{
			// se estrolo contra la pared de la ruta
			int t = model->aux_tramo; 
			TVector3d N = model->pt_ruta[t+1] - model->pt_ruta[t];
			N.y = 0;
			N.normalizar();
			N.rotar_xz(M_PI/2);		
			// calculo el rebote
			// v = i - 2 * dot(i, n) * n
			Dir = Dir - N*(2*(Dir>>N));

			Pos = antPos;		// + Dir*5;

			// recupero los alfa, beta
			calc_angulos();
			
			timer_rebotando = 2;
			derrapando = 3;

			PerderVida();

			return;
		}
	}
	
	// verifico si cae sobre la ruta ?
	// no estaba en la ruta y luego de actualizar la posicion entro en un tramo de la misma
	if(pos_en_ruta==-1 && tipo==1 && !timer_rebotando)		
	{
		pos_en_ruta = tramo;
		beta = 0;
		vel_v = 0;		// se tiene que alinear con la direccion de la ruta
		vel = 20;		// tiene que ir rapido
		timer_cayendo = 0;		// si estaba cayendo detengo la caida
		return;		// entro al caminito
	}

	// Estaba en la ruta: validaciones especiales
	if(pos_en_ruta>=0)
	{
		TVector3d dir = model->pt_ruta[pos_en_ruta+1]-model->pt_ruta[pos_en_ruta];
		double len = dir.mod();
		dir.normalizar();
		TVector3d n = dir*TVector3d(0,1,0);
		double tu = (Pos-model->pt_ruta[pos_en_ruta])>>n;
		if(fabs(tu)>model->ancho_ruta/2)		// se cayo de la ruta
		{
			// si esta jugando a GAME_TEST_RUTA y se cae de la ruta: 
			//  pierda la vida y empieza de nuevo:
			if(model->modo_juego==GAME_TEST_RUTA)
			{
				timer_rebotando = 2;
				pos_en_ruta = -2;		
				timer_light_path = 4;	// de aca a 4 seg no genere mas light path
				cant_ptos_delay = cant_ptos;
				PerderVida();
				// inicio el sistema de particulas
				PS.Create(model,Cycle_Pos,20,model->textura_explo+nro_player%4);
				PlaySound("sounds\\Explosion.wav",NULL,SND_ASYNC);
			}
			else
			{
				pos_en_ruta = -2;		// el -2 indica que no puede volver a entrar a la ruta hasta que termine el timer
				timer_cayendo = 0.5;		// 3secs  para que salga de la ruta
				vel = 15;		// tiene que ir mas despacio
			}
		}
		else
		{
			double tv = ((Pos-model->pt_ruta[pos_en_ruta])>>dir)/len;		// tv==0..1
			if(tv>=1)
			{
				// paso al siguiente tramo de la ruta
				pos_en_ruta++;
				if(pos_en_ruta>=model->cant_ptos_ruta)
					// se supone que la ruta es un camino cerrado
					pos_en_ruta = 0;
				tv = ((Pos-model->pt_ruta[pos_en_ruta])>>dir)/len;		// tv==0..1
			}
			// Actualizo la altura interpolando ambos valores
			Pos.y = model->pt_ruta[pos_en_ruta].y*(1-tv) + model->pt_ruta[pos_en_ruta+1].y*tv;
			tv += size_cycle.z/len;
			double y_del = model->pt_ruta[pos_en_ruta].y*(1-tv) + model->pt_ruta[pos_en_ruta+1].y*tv;
			beta = atan2(y_del-Pos.y,size_cycle.z);
			
		}
	}
	else
	if(vel_v<0.01)
	{
		// si no esta en la ruta, verifico que no se pase del suelo, una vez 
		// que llega al suelo termina la caida y se anula la velocidad vertical
		if(Pos.y<=Hpiso + 0.01)
		{

			Pos.y = Hpiso;		// no se puede pasar del nivel del piso
			vel_v = 0;		// termina la caida
			beta = 0;
		}
		else
		if(pos_delantera.y<Hpiso_s)
		{
			// la parte de adelante de la moto choca contra el suelo:
			// siendo que 
			// pos_delantera = pos + Cycle_Dir*size_cycle.z, 
			// y que quiero pos_delantera.y = Hpiso_s, lo unico que puedo cambiar el la direccion de la moto, con lo cual
			Cycle_Dir.y = (Hpiso_s-Pos.y)/size_cycle.z;
			// hace que Pos.y = 0
			// pero ademas necesito que |Cycle_Dir| = 1, con lo cual las otras 2 direcciones se ven afectadas por un cte
			// resolviendo esas ecuaciones queda :
			double landa = sqrt((1-Cycle_Dir.y*Cycle_Dir.y)/(Cycle_Dir.x*Cycle_Dir.x + Cycle_Dir.z*Cycle_Dir.z));
			Cycle_Dir.x*=landa;
			Cycle_Dir.z*=landa;
			// Recalculo los demas parametros: 
			Cycle_Up = TVector3d(0,1,0);
			Cycle_Up.rotar(TVector3d(0,0,0),Cycle_Dir,an_lat);
			Cycle_N = Cycle_Dir*Cycle_Up;		// Direccion normal a la trayectoria
			// y finalmente la nueva posicon de la moto, que hace que pos quede igual, pero pos delantera quede sobre el suelo
			// eso gracias a haber cambiado la direccion de la moto
			Cycle_Pos = Pos+Cycle_Dir*(size_cycle.z/2) + Cycle_Up*(size_cycle.y*0.5);
		}

	}


	// falta contemplar si choca conta el limite del escenario, y justo esta cayendo
	// verifico si choca contra la pared del domo
	if(model->modo_juego!=GAME_TEST_RUTA && 
			(Pos-model->centro).mod()>=model->radio && pos_en_ruta<0)
	{
		// choco contra la esfera, lo hago rebotar
		Dir_Original = Dir;
		TVector3d Ip = (Pos+antPos)*0.5;
		TVector3d N = model->centro-Ip;
		N.y = 0;
		N.normalizar();
		// rebota reflejando la direccion
		Dir = Dir - N*(2*(Dir>>N));
		calc_angulos();
		Pos = antPos;
		if(!timer_rebotando)
		{
			timer_rebotando = 2;		// 1segundos de explosion + 1 de implosion
			PerderVida();
		}

	}
	else
	if(timer_rebotando<=0 && timer_rearmar_moto<=0)	// si esta en pleno proceso de haber chocado, no quiero que siga chocando
	{
		Dir_Original = Dir;
		TVector3d N;
		BOOL choque = FALSE;
		if(model->choca_ligthpath(antPos,pos_delantera,&path_choque,&Ip,nro_player) 
				&& (nro_player || !vidas_infinitas))
		{
			// choco contra el ligth path
			N = path[path_choque+1] - path[path_choque];
			N.y = 0;
			N.normalizar();
			N.rotar_xz(M_PI/2);		

			// calculo el rebote
			// v = i - 2 * dot(i, n) * n
			Dir = Dir - N*(2*(Dir>>N));

			// recupero los alfa, beta
			calc_angulos();
			
			// actualizo la pos para que sea el pto de colision
			Pos.x = Ip.x;
			Pos.z = Ip.y;

			timer_rebotando = 2;
			timer_light_path = 4;	// de aca a 4 seg no genere mas light path
			cant_ptos_delay = cant_ptos;
			PerderVida();


			// inicio el sistema de particulas
			if(model->modo_juego == GAME_GRID_BATTLE)
			{
				// explocion con rastro de particulas y con gravedad
				PS.rastro = TRUE;
				PS.Create(model,Cycle_Pos,100,model->textura_explo+3,1);
			}
			else
			{
				PS.rastro = FALSE;
				if(nro_player)
				{
					// explosion barata:
					PS.Create(model,Cycle_Pos,5,model->textura_explo+nro_player%4);
				}
				else
				{
					PS.Create(model,Cycle_Pos,20,model->textura_explo+nro_player%4);
					// inicio el sistema de particulas auxiliar
					PSaux.Create(model,Cycle_Pos,5,model->textura_explo+2,1);
				}
			}

			PlaySound("sounds\\Explosion.wav",NULL,SND_ASYNC);

		}
	}


}

void player::UpdateRemoto()
{
	// timers 
	UpdateTimers();

	// verifico si hay paquetes para procesar
	if(!cant_paquetes)
	{
		// no hay paquetes:
		// lag
		// prediccion del movimiento

		double elapsed_time = model->elapsed_time;

		char tipo;
		int tramo;
		double H = Hpiso = model->que_altura(Pos.x,Pos.z,&tipo,&tramo);

		// 2- Actualizo la velocidad
		// hay una aceleracion producto del motor de la moto en direccion Dir  (cte) = Dir*vel
		// Y esta la velocidad vertical, cuando aplica un salto, que se ve afectada por la fuerza de gravedad
		if(pos_en_ruta==-1 && Pos.y>H+0.1)
			vel_v -= 10*elapsed_time;		// cae


		// 4- la velocidad de la moto es la resultante 
		Cycle_Dir = Vmoto = Dir*vel + TVector3d(0,1,0)*vel_v;
		Cycle_Dir.normalizar();
		Cycle_Up = TVector3d(0,1,0);
		Cycle_Up.rotar(TVector3d(0,0,0),Cycle_Dir,an_lat);
		Cycle_N = Cycle_Dir*Cycle_Up;		// Direccion normal a la trayectoria
		
		//5- Actualizo la posicion pp dicha
		Pos = Pos + Vmoto*elapsed_time;
		Cycle_Pos = Pos+Cycle_Dir*(size_cycle.z/2) + Cycle_Up*(size_cycle.y*0.5);
		

	}
	else
	{
		// reseteo el laggin
		model->timer_lag = 0;

		// proceso todos los paquetes
		while(paq_desde<cant_paquetes)
			ProcessPacket(&paquete[paq_desde++]);

		if(paq_desde==cant_paquetes)
			// se vacio el buffer
			paq_desde = cant_paquetes = 0;

	}


}

void player::Wait()
{
	// espera a recibir un ack del usuario remoto
		// verifico si hay paquetes para procesar
	ack = FALSE;
	while(!ack)
	{
		while(paq_desde<cant_paquetes)
		{
			ProcessPacket(&paquete[paq_desde++]);
			actualizarPath();
		}

		if(paq_desde==cant_paquetes)
		{
			// se vacio el buffer
			paq_desde = cant_paquetes = 0;

			if(!ack)
			// lagging: espero hasta que reciba mas paquetes
			while(!cant_paquetes)
			{
				MSG Msg;
				ZeroMemory( &Msg, sizeof(Msg) );
				if(PeekMessage( &Msg, NULL, 0U, 0U, PM_REMOVE ) )
				{
					if(Msg.message==WM_QUIT)
						return;
					else
					if(Msg.message==WM_KEYDOWN && (int) Msg.wParam==VK_ESCAPE)
						return;
					else
					{
						// dejo que windows procese el mensaje
						TranslateMessage( &Msg );
						DispatchMessage( &Msg );
					}
				}

				model->DXRender();
			}
		}
	}
}


void player::ProcessPacket(grid_packet *packet)
{
	model->elapsed_time = packet->elapsed_time;
	model->remote_time = packet->time;
	Pos.x = packet->pos_x;
	Pos.y = packet->pos_y;
	Pos.z = packet->pos_z;
	Dir.x = packet->dir_x;
	Dir.y = packet->dir_y;
	Dir.z = packet->dir_z;
	vel = packet->vel;
	vel_v = packet->vel_v;
	alfa = packet->alfa;
	beta = packet->beta;

	// 4- la velocidad de la moto es la resultante 
	Cycle_Dir = Vmoto = Dir*vel + TVector3d(0,1,0)*vel_v;
	Cycle_Dir.normalizar();
	Cycle_Up = TVector3d(0,1,0);
	Cycle_Up.rotar(TVector3d(0,0,0),Cycle_Dir,an_lat);
	Cycle_N = Cycle_Dir*Cycle_Up;		// Direccion normal a la trayectoria
	
	Cycle_Pos = Pos+Cycle_Dir*(size_cycle.z/2) + Cycle_Up*(size_cycle.y*0.5);

	// proceso el comando
	switch(packet->cmd)
	{
		default:
			break;
		case 99:
			// ack
			// el servidor primero mando un msg ack al remoto, el remoto le contesta
			// con un msg ack
			if(model->es_servidor)
				ack = TRUE;		// recibio el ack del remoto
			else
			{
				// el servidor le pide que conteste con un ack
				grid_packet packet_ack;
				model->player_one->llenar_paquete(&packet_ack,99);
				model->player_one->socket.SendData((char *)&packet_ack,sizeof(packet_ack));
				model->player_one->socket.Flush();
				ack = TRUE;		// ack OK
			}
			break;
		case 3:
			// el jugador remoto exploto
			timer_rebotando = 2;
			cant_vidas--;
			// inicio el sistema de particulas
			PS.Create(model,Cycle_Pos,20,model->textura_explo+nro_player%4);
			PlaySound("sounds\\Explosion.wav",NULL,SND_ASYNC);

			break;
	}


}


void player::llenar_paquete(grid_packet *packet,char cmd)
{

	packet->cmd = cmd;
	packet->dir_x = Dir.x;
	packet->dir_y = Dir.y;
	packet->dir_z = Dir.z;
	packet->pos_x = Pos.x;
	packet->pos_y = Pos.y;
	packet->pos_z = Pos.z;
	packet->vel = vel;
	packet->vel_v = vel_v;
	packet->elapsed_time = model->elapsed_time;
	packet->time = model->time;
	packet->alfa = alfa;
	packet->beta = beta;
}


// multiplayer: le mando la info al servidor
void player::Send()
{
	grid_packet packet;
	llenar_paquete(&packet);
	socket.SendData((char *)&packet,sizeof(packet));
	socket.Flush();
}





// dibujo la moto de luz
void player::RenderCycle(BOOL resplandor)
{
	if(!cant_vidas && !timer_rebotando)
		return;

	D3DXMATRIXA16 mat;
	if(timer_rebotando)
	{
		// perturo la orientacion de la moto de luz
		double an = M_PI*timer_rebotando*5;
		if(derrapando==3)
		{
			// se estrolo contra la pared, 
			// rota en torno a la normal (ambos ejes, el de direccion y el up)
			TVector3d N = Cycle_Up*Cycle_Dir;
			TVector3d AuxUp = Cycle_Up;
			TVector3d AuxDir = Cycle_Dir;
			TVector3d AuxPos = Cycle_Pos;
			AuxUp.rotar(TVector3d(0,0,0),N,an*0.5);
			AuxDir.rotar(TVector3d(0,0,0),N,an*0.5);
			AuxPos.y += size_cycle.y*2;
			cycle_mesh->CalcularMatriz(AuxPos,size_cycle,AuxDir,&mat,AuxUp);
		}
		else
		if(derrapando)
		{
			// se de costado, rota en torno al eje de la direccion, 
			// para unlado o para el otro depende para que lado se cayo.
			TVector3d AuxUp = Cycle_Up;
			AuxUp.rotar(TVector3d(0,0,0),Dir,-an*derrapando);
			cycle_mesh->CalcularMatriz(Cycle_Pos,size_cycle,Dir,&mat,AuxUp);
		}
		else
		{
			// choco contra el light path, gira sobre el eje Up
			TVector3d AuxDir = Cycle_Dir;
			//AuxDir.rotar(TVector3d(0,0,0),Cycle_Up,an*0.5);
			cycle_mesh->CalcularMatriz(Cycle_Pos,size_cycle,AuxDir,&mat,Cycle_Up);
		}
	}
	else
		cycle_mesh->CalcularMatriz(Cycle_Pos,size_cycle,Cycle_Dir,&mat,Cycle_Up);
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &mat);
	g_pd3dDevice->SetTexture( 0, NULL);


	if((timer_rebotando && !derrapando) || timer_rearmar_moto)
	{

		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetTransform( D3DTS_WORLD, &model->matWorld );

		if(!timer_rearmar_moto)
		{
			// explosion de plasma: 
			// halos de particulas que dejan rastro
			double s = 2-timer_rebotando;
			PS.Update(s,Cycle_Pos);
			PS.Render();

			PSaux.Update(s,Cycle_Pos);
			PSaux.Render();

		}

		// piezas de la moto explotando, o implotando para re-armar la moto
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		model->hay_shader = TRUE;
		model->g_pEffect->SetTechnique( "Explotar" );
		D3DXMATRIXA16 mViewProj = model->matView * model->matProj;
		model->g_pEffect->SetMatrix( "g_mViewProj", &mViewProj);


		if(explosion_barata)
		{
			// Explosion barata
			float t = timer_rearmar_moto?timer_rearmar_moto:2-timer_rebotando;
			model->g_pEffect->SetFloat("g_fTime", t);
			model->Mesh[7].CalcularMatriz(Cycle_Pos,size_cycle,Cycle_Dir,&mat,Cycle_Up);
			model->g_pEffect->SetMatrix( "g_mWorld", &mat);
			model->Mesh[7].DXRender();

		}
		else
		{
			// Explosion cara:
			float t = timer_rearmar_moto?timer_rearmar_moto:2-timer_rebotando;
			model->g_pEffect->SetFloat("g_fTime", t);
			TVector3d auxDir = Cycle_Dir;
			
			for(int K=1;K<4;++K)
			{
				model->Mesh[7].CalcularMatriz(Cycle_Pos,size_cycle*(K),auxDir,&mat,Cycle_Up);
				model->g_pEffect->SetMatrix( "g_mWorld", &mat);
				model->Mesh[7].DXRender();
				auxDir.rotar_xz(0.1);
			}

			// esqueleto de la moto (va como adentro,y desde aca sale el humo de la explosion
			model->g_pEffect->SetFloat("g_fTime", t*0.05);
			model->Mesh[7].CalcularMatriz(Cycle_Pos,size_cycle,Cycle_Dir,&mat,Cycle_Up);
			model->g_pEffect->SetMatrix( "g_mWorld", &mat);
			model->Mesh[7].DXRender();
		}

		model->hay_shader = FALSE;
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);

	}
	else
		cycle_mesh->DXRender();

	// Restauro el matworld
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &model->matWorld );


}

void player::IA()
{
	if(!cant_vidas)
		return;

	double tg = treaccion;
	
	if(fabs(alfa-alfa_d)>0.001)
	{
		// esta girando:
		double an = alfa-alfa_d;			// giro deseado
		double w0 = an/model->elapsed_time;		// velocidad angular
		if(fabs(w0)>max_w0)
			w0 = max_w0 * sign(w0);
		if(model->nivel<=2)
			// la maquina esta limitada en el giro maximo, en nivel experto o superior no hay limite!
			an = w0*model->elapsed_time;
		alfa-=an;
		an_lat-=an;
		if(fabs(an_lat)>max_an_lat)
			an_lat = max_an_lat*sign(an_lat);
		return;
	}

	Dir_d = Dir;

	if(pos_en_ruta<0)
	{
		TVector3d ant_Dir = Dir;
		TVector2d Ip;
		int tipo_choque;
		if((tipo_choque=model->choca(Pos,Pos+Dir_d*dist_radar,nro_player,&Ip,TRUE)))
		{
			if(model->debug_ia)
				model->agregar_msg("Detecto Choque");
			int t =0;
			if(tipo_choque==2)
			{
				// choca contra el domo
				Dir_d.rotar_xz(M_PI/4);
				if(!model->choca(Pos,Pos+Dir_d*dist_radar,nro_player,&Ip,TRUE))
					t = 100;
				else
				{
					Dir_d.rotar_xz(-M_PI/2);
					if(!model->choca(Pos,Pos+Dir_d*dist_radar,nro_player,&Ip,TRUE))
						t = 100;
					else
						// restuaro la direccion Deseada
						Dir_d = Dir;
				}

			}


			if(t!=100)
			{
				if(Pos.y<=0.001 && (Ip-Pos.pxz()).mod()<dist_radar/3 )
				{
					// Esta demasiado cerca para esquivar: salta (un poco mas fuerte que el jugador)
					vel_v = vel_salto*1.1;
					if(model->debug_ia)
						model->agregar_msg("Intento Salto");
				}
				else
				{
					if(model->debug_ia)
						model->agregar_msg("Busca Direccion de salida");
					// busco alguna direccion donde pueda avanzar sin chocar
					t = 0;
					int step = 32;
					double an = M_2PI/step;
					double alfa = an;
					TVector3d Dir_m;
					double min_dist = model->radio;
					while(t<step)
					{
						Dir_d = ant_Dir;
						if(t%2==0)
							Dir_d.rotar_xz(alfa);
						else
						{
							Dir_d.rotar_xz(-alfa);
							alfa+=an;
						}

						if(model->choca(Pos,Pos+Dir_d*dist_radar,nro_player,&Ip,TRUE))
						{
							// si choca,
							// pondero la solucion segun la distancia de choque.
							double dist = (Ip-Pos.pxz()).mod();
							if(dist<min_dist)
							{
								min_dist = dist;
								Dir_m = Dir_d;
							}
							// y sigo evaluando, por ahi encuentro alguna salida mejor
							++t;
						}
						else
							// si no choca: encontro una salida, termina la busqueda
							t = 100;
					}

					if(t!=100)
					{
						// no hubo una salida libre, de todas las que choca, me 
						// quedo con la que lo hace mas lejos
						Dir_d = Dir_m;
					}
				}

				// no hay forma de salir: mejor salto: 
				if(t!=100 && Pos.y<=0.001 && model->modo_juego!=GAME_TEST_LABERINTO)
				{
					vel_v = vel_salto;
					if(model->debug_ia)
						model->agregar_msg("No encontro salida: salta");
				}
				else
				{
					// y para que no caiga en un loop choto, por un tiempo dejo de perseguir
					timer_evitando = 1;
					if(model->debug_ia)
						model->agregar_msg("Ejecuta direccion de salida");
				}
			}
		}
		else
		if((timer_ia-=model->elapsed_time)<=0 && 0)
		{
			// tiempo de moverse aleatoreamente
			timer_ia = 3;
			float k = (float)rand()/(float)RAND_MAX - 0.5;
			Dir_d.rotar_xz(M_PI/4*k);
		}
		else
		if(nro_player && timer_evitando<=0 && 0)
		{
			// persigo al player one
			// Establezco la direccion deseada hacia la pos del player one
			double dist = (model->player_one->Pos-Pos).mod();
			double k = dist<=10?10:20;
			TVector3d N = model->player_one->Pos + model->player_one->Dir*k-Pos;
			N.y = 0;
			N.normalizar();

			// modo perseguir
			Dir_d = N;
			// modo huir
			//Dir = N*(-1);

		
			// Verifico esta nueva direccion a donde me va a llevar :
			if(model->choca(Pos,Pos+Dir_d*10,nro_player,&Ip))
			{
				// choca : mejor me quedo como estaba que no chocaba
				Dir_d = Dir;
				// y para que no caiga en un loop choto, por un tiempo dejo de perseguir
				timer_evitando = 1;
			}
			
		}
		else
		if(!nro_player && timer_evitando<=0 && model->cant_players>=2 && 0)
		{
			// Establezco la direccion deseada hacia la pos del player one
			double dist = (model->P[1].Pos-Pos).mod();
			double k = dist<=10?10:20;
			TVector3d N = model->P[1].Pos + model->P[1].Dir*k-Pos;
			N.y = 0;
			N.normalizar();

			// modo huir
			Dir = N*(-1);

		
			// Verifico esta nueva direccion a donde me va a llevar :
			if(model->choca(Pos,Pos+Dir_d*10,nro_player,&Ip))
			{
				// choca : mejor me quedo como estaba que no chocaba
				Dir_d = Dir;
				// y para que no caiga en un loop choto, por un tiempo dejo de perseguir
				timer_evitando = 1;
			}
			
		}

	}


	if(timer_evitando)
	{
		timer_evitando-=model->elapsed_time;
		if(timer_evitando<0)
			timer_evitando = 0;
	}

	if((Dir_d-Dir).mod()>0.001)
	{
		// voy a girar hacia la direccion deseada Dir_d
		// durante los proximos treaccion segundos: seteo el timer de reaccion
		timer_girando = 0.01;
		// determino el angulo deseado 
		TVector3d ant_Dir = Dir;
		double ant_alfa = alfa;
		double ant_beta = beta;
		Dir = Dir_d;
		calc_angulos();

		// me gustaria que los angulos fuesen alfa y beta, 
		// pero solo puedo girar un cierto angulo por unidad de tiempo
		alfa_d = alfa;		// angulo deseado
		// Restauro la direccion
		alfa = ant_alfa;
		beta = ant_beta;
		Dir = ant_Dir;

		// normalizo alfa y beta: quiero que esten entre -pi y pi
		if(alfa>M_PI)
			alfa-=2*M_PI;
		else
		if(alfa<-M_PI)
			alfa+=2*M_PI;

		// normalizo el angulo deseado, alfa_d para que el angulo an que representa la dif.
		// queden entre -pi y pi tambien. 
		// OJO: Si los angulos representa objetos reales, recordar 
		// que en la "vida real" no existen angulos mayores a pi
		double an = alfa-alfa_d;			// giro deseado
		if(an>M_PI)
			alfa_d+=2*M_PI;
		else
		if(an<-M_PI)
			alfa_d-=2*M_PI;

	}


}


/////////////////////////////////////////////////////////////////////////////
// Juego
/////////////////////////////////////////////////////////////////////////////

BOOL camara_lenta = FALSE;
BOOL modo_draft = FALSE;

CGameEngine::CGameEngine()
{
	fpc = FALSE;
	modo_camara = FALSE;
	modo_gui = FALSE;
	animacion_gui = 0;
	pr_pos = 0;
	timer_pausa = 0;
	timer_warming = 0;
	timer_game_over = 0;
	timer_presentacion = 0;
	timer_modalless = 0;
	nro_player_perdio = -1;
	remote_time = 0;
	cant_cell = 32;			// cant de celdas 
	dgrid = 5;				// cada celda mide dgrid metros
	//cant_cell = 20;			// cant de celdas 
	//dgrid = 3;				// cada celda mide dgrid metros

	lagging = FALSE;

	radio = cant_cell/2*dgrid;
	centro = TVector3d(0,0,0);
	
	fps = 0;
	nro_frame = 0;

	cant_focos = 15;
	paused = FALSE;
	memset(celdas,0,sizeof(celdas));
	memset(ligthpath_cant,0,sizeof(ligthpath_cant));

	modo_juego = GAME_GRID_BATTLE;
	ronda = 0;
	cant_enemigos = 1;
	juega_sola = FALSE;
	tipo_ruta = -1;
	nivel = 0;
	max_rondas = 7;
	techo_ruta = FALSE;

	server.model = this;
	for(int i=0;i<10;++i)
	{
		P[i].socket.model = this;
		P[i].socket.player = &P[i];
	}

	multiplayer = FALSE;

	// inicializo el motor del directX
	Create();

	cant_msg = 0;
	primer_msg = 0;
	debug_ia = FALSE;

	// cargo el play list de la radio
	cargarPlayList();
	// y paso al primer tema
	midi_actual = 0;
	playMidi();

}

void CGameEngine::agregar_msg(char *buffer)
{
	strcpy(debug_msg[cant_msg%MAX_MSG_DBG],buffer);
	if(++cant_msg>=MAX_MSG_DBG)
		++primer_msg;
}



CGameEngine::~CGameEngine()
{
	// limpio el modelo pp dicho
	DXCleanup();
}


HRESULT CGameEngine::DXInit(HWND hWnd)
{

	if(init)
		return S_OK;		// Ya esta inicializado

	// Primero inicializo el device
	HRESULT rta = DXEngine::DXInit(m_hWnd);
	if(FAILED(rta))
		return rta;


	init = TRUE;
	vel_camara = 1.1;		// 110% de la velocidad de la moto
	vel_rot_camara = 2;	// 200% de la velocidad de la moto
	smooth_cam = FALSE;

	// Cargo la moto de luz
	Mesh[cant_mesh++].LoadMesh(this,"cycle.x");		//0
	Mesh[cant_mesh++].LoadMesh(this,"torre1.x");	//1
	Mesh[cant_mesh++].LoadMesh(this,"torre2.x");	//2
	Mesh[cant_mesh++].LoadMesh(this,"tribuna.x");	//3
	Mesh[cant_mesh++].LoadMesh(this,"edificios.x");	//4
	Mesh[cant_mesh++].LoadMesh(this,"recognizer.x");	//5
	Mesh[cant_mesh++].LoadMesh(this,"cycle2.x");			// 6 = moto de los enemigos
	Mesh[cant_mesh++].LoadMesh(this,"cycle_explo.x");			// 7 = explosion

	{
		Mesh[7].g_pMesh->CloneMeshFVF(D3DXMESH_SYSTEMMEM|D3DXMESH_32BIT,
			D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX3|D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE3(1)| D3DFVF_TEXCOORDSIZE3(2)
			,g_pd3dDevice,&g_pMeshCycle);


		srand( (unsigned)::time( NULL ) );
		
		VERTEX2* pVertices;
		g_pMeshCycle->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pVertices); 
		DWORD* pIndices=NULL;
		g_pMeshCycle->LockIndexBuffer(D3DLOCK_READONLY, (LPVOID*)&pIndices); 
		int cant_v = g_pMeshCycle->GetNumVertices();
		int cant_f = g_pMeshCycle->GetNumFaces();
		double pert = 0;
		for(int i=0;i<cant_f;++i)
		{
			int i0 = (unsigned)pIndices[3*i];
			int i1 = (unsigned)pIndices[3*i+1];
			int i2 = (unsigned)pIndices[3*i+2];
			TVector3d p0 = TVector3d(pVertices[i0].position.x,pVertices[i0].position.y,pVertices[i0].position.z);
			TVector3d p1 = TVector3d(pVertices[i1].position.x,pVertices[i1].position.y,pVertices[i1].position.z);
			TVector3d p2 = TVector3d(pVertices[i2].position.x,pVertices[i2].position.y,pVertices[i2].position.z);

			TVector3d N = (p1-p0)*(p2-p0);
			N.normalizar();
			//TVector3d cg = (p0 + p1 + p2)*(1.0/3.0);
			// yeite:
			// el cg lo calculo como el primer punto o el ultimo de acuerdo a si la cara
			// es par o no, para mantener juntos los pares de triangulos que forman un quads
			// que x convencion vienen juntos en la malla (x diseño)
			// de esta forma en el shader, al explotar, los 6 vertices que componen
			// el quads se alejan del mismo punto que representa un centro de gravedad
			// (no interesa que sea el centro geomtrico, es un pto a partir del cual se alejan
			// los vertices, simulando una explosion)
			// De esta manera produce el efecto de expansion. 
			TVector3d cg = i%2==0?p0:p2;

			// eje de rotacion, sobre el que va...
			TVector3d eje = N*TVector3d(1,1,0);

			// lo perturbo
			if(i%2==0)
				pert = (double)rand()/(double)RAND_MAX/5 - 0.1;

			eje.x += pert;
			eje.y += pert;
			eje.z += pert;
			eje.normalizar();

			for(int j=0;j<3;++j)
			{
				int index = (unsigned)pIndices[3*i+j];
				pVertices[index].texcoord1.x = cg.x;
				pVertices[index].texcoord1.y = cg.y;
				pVertices[index].texcoord1.z = cg.z;
				
				// direccion de la explosion
				pVertices[index].normal.x = N.x;
				pVertices[index].normal.y = N.y;
				pVertices[index].normal.z = N.z;

				// eje de rotacion 
				pVertices[index].texcoord2.x = eje.x;
				pVertices[index].texcoord2.y = eje.y;
				pVertices[index].texcoord2.z = eje.z;

			}


		}
		g_pMeshCycle->UnlockVertexBuffer(); 
		g_pMeshCycle->UnlockIndexBuffer(); 

		SAFE_RELEASE(Mesh[7].g_pMesh);
		Mesh[7].g_pMesh = g_pMeshCycle;


	}

	
	// Cargo las texturas
	textura_piso = cargar_textura("-piso.bmp");		// el - es necesario para
	// cargar textura genere un alpha - chanel para la textura
	textura_pared = cargar_textura("pared5.bmp");
	textura_path = cargar_textura("+path2.jpg");
	textura_skydome = cargar_textura("skydome.bmp");
	textura_ruta = cargar_textura("-ruta.bmp");
	cargar_textura("-guarray.bmp");
	cargar_textura("-techo.bmp");
	cargar_textura("-ruta2.bmp");
	cargar_textura("guarray_mask.bmp");
	textura_explo = cargar_textura("-explo1.bmp");
	cargar_textura("-explo2.bmp");
	cargar_textura("-explo3.bmp");
	cargar_textura("-explo4.bmp");
	textura_gui = cargar_textura("*scbrd.bmp");		// el * indica sin mipmaping
	cargar_textura("*triangulito_r2.bmp");
	cargar_textura("*triangulito.bmp");
	cargar_textura("*triangulito_d.bmp");
	cargar_textura("*triangulito_r.bmp");
	cargar_textura("*hexa.bmp");
	// caras de los jugadores
	textura_face = cargar_textura("*face1.bmp");
	cargar_textura("*face2.bmp");
	cargar_textura("*face3.bmp");
	cargar_textura("*face4.bmp");
	cargar_textura("*face5.bmp");
	cargar_textura("*face6.bmp");

	// cara del enemigo
	textura_face_enemigo = cargar_textura("*face_enemigo.bmp");
	textura_cursor = cargar_textura("*cursor.bmp");
	textura_menu = cargar_textura("*mainmenu.bmp");
	textura_roundrect = cargar_textura("*roundrect.bmp");
	// Atlas de texturas del toolbar
	textura_toolbar = cargar_textura("*toolbar.bmp");


	DXLoadTextures();

	SetAlphaChannel(g_pTexture[textura_piso],0,0,0);
	SetAlphaChannel(g_pTexture[textura_ruta],0,0,0);
	SetAlphaChannel(g_pTexture[textura_ruta+1],0,0,0);
	SetAlphaChannel(g_pTexture[textura_ruta+2],0,0,0);	
	SetAlphaChannel(g_pTexture[textura_ruta+3],0,0,0);
	
	// scoreboard, vidas, energia, y caras
	for(int t=0;t<=13;++t)
		SetAlphaChannel(g_pTexture[textura_gui+t],255,0,255);
	//SetAlphaChannel(g_pTexture[textura_toolbar],255,0,255);

	// menu items (fondo negro)
	SetAlphaChannel(g_pTexture[textura_menu],0,0,0);
	SetAlphaChannel(g_pTexture[textura_roundrect],0,0,0);

	// explosiones 
	SetAlphaChannel(g_pTexture[textura_explo],128);		// factor cte
	SetAlphaChannel(g_pTexture[textura_explo+1],128);		// factor cte
	SetAlphaChannel(g_pTexture[textura_explo+2],128);		// factor cte
	SetAlphaChannel(g_pTexture[textura_explo+3],128);		// factor cte

	InitGame();

	// levanto el GUI
	gui.Create(this);
	gui.Reset();


	// inicializo el mouse
	mouse.CreateDevice(m_hWnd);
	
	// archivo de midi de la radio de musica de fondo
	hay_radio = TRUE;
	//strcpy(fname_midi,"sounds\\Shook_Me_All_Night.mid");
	//strcpy(fname_midi,"sounds\\drum_sample.mid");

	ShowCursor(FALSE);

	// inicio rapido
	if(1)
	{
		modo_juego = GAME_GRID_BATTLE;
		//modo_juego = GAME_GRID_SURVIVAL;
		InitGame();
		modo_gui = FALSE;

		/*
		{
			// Inicio un dialogo modalless
			gui.InitDialog();
			int x0 = 100;
			int y0 = 70;
			int dy = 20;
			int dx = 300;
			gui.total_delay = 0;
			gui.InsertItem(GUI_FRAME,"Grid Battle",x0-50,y0-50,200,100);
			gui.InsertItem(GUI_STATIC_TEXT,"Ready!",x0,y0);
			timer_modalless = 3;		// le doy 3 segundos de vida
		}
		*/

		MainLoop();
	}
		
	//Presentacion();
	GUI_mainmenu();

	return S_OK;
}


HRESULT CGameEngine::DXCreateVertex()
{

	// borro el buffer de vertices
	SAFE_RELEASE(g_pVB);
	SAFE_RELEASE(g_pVBFocos);
	SAFE_RELEASE(g_pQuad);
	int pasos = 64;		// precision del muro circular

	// Alloco memoria y creo un buffer para todos los vertices
	CUSTOMVERTEX* pVertices;
	size_t size = sizeof(CUSTOMVERTEX)*(4+(pasos+1)*2*2 + (4+10)*2*cant_focos + (cant_ptos_ruta+1)*16);
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( size,
				0 , D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
		return E_FAIL;
	if( FAILED( g_pVB->Lock( 0, size, (void**)&pVertices, 0 ) ) )
		return E_FAIL;

	size = sizeof(CUSTOMVERTEX)*((4+10)*2*cant_focos);
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( size,
				0 , D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVBFocos, NULL ) ) )
		return E_FAIL;


	// Piso
	pVertices[0].x = -radio;
	pVertices[0].y = 0;
	pVertices[0].z = -radio;
	pVertices[0].color = D3DCOLOR_XRGB(255,255,255);
	pVertices[0].N.x = 0;
	pVertices[0].N.y = 1;
	pVertices[0].N.z = 0;
	pVertices[0].tu = 0;
	pVertices[0].tv = 0;

	pVertices[1].x = radio;
	pVertices[1].y = 0;
	pVertices[1].z = -radio;
	pVertices[1].color = D3DCOLOR_XRGB(255,255,255);
	pVertices[1].N.x = 0;
	pVertices[1].N.y = 1;
	pVertices[1].N.z = 0;
	pVertices[1].tu = cant_cell;
	pVertices[1].tv = 0;

	pVertices[2].x = radio;
	pVertices[2].y = 0;
	pVertices[2].z = radio;
	pVertices[2].color = D3DCOLOR_XRGB(255,255,255);
	pVertices[2].N.x = 0;
	pVertices[2].N.y = 1;
	pVertices[2].N.z = 0;
	pVertices[2].tu = cant_cell;
	pVertices[2].tv = cant_cell;

	pVertices[3].x = -radio;
	pVertices[3].y = 0;
	pVertices[3].z = radio;
	pVertices[3].color = D3DCOLOR_XRGB(255,255,255);
	pVertices[3].N.x = 0;
	pVertices[3].N.y = 1;
	pVertices[3].N.z = 0;
	pVertices[3].tu = 0;
	pVertices[3].tv = cant_cell;


	double dw = 2*radio;
	int t = 4;
	// Paredes
	double Kt = 50;		// tamaño de la textura de la pared
	double da = 2*M_PI/pasos;
	double alfa = 0;
	for(int i=0;i<pasos;++i)
	{
		pVertices[t].x = radio*cos(alfa);
		pVertices[t].y = 0;
		pVertices[t].z = radio*sin(alfa);
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = -cos(alfa);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa);
		pVertices[t].tv = 1;
		pVertices[t].tu = -alfa*dw/Kt;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].y = 7;
		pVertices[t].tv = 0;


		++t;
	
		alfa+=da;

	}
	pVertices[t] = pVertices[4];
	pVertices[t].tu = -2*M_PI*dw/Kt;
	++t;
	pVertices[t] = pVertices[5];
	pVertices[t].tu = -2*M_PI*dw/Kt;
	++t;

	// focos de luz
	D3DCOLOR color_luz = D3DCOLOR_XRGB(255,255,255);//D3DCOLOR_XRGB(141,232,240);
	pos_vertex_focos = t;
	da = 2*M_PI/cant_focos*0.16;
	alfa = 0.13;
	double dt = 0.02;
	double r = radio+5;
	double hf = 2;
	// los primeros no se usan mas ....lo estoy estudiando que conviene mas
	for(i=0;i<cant_focos;++i)
	{
		// ------------------------------------------------
		// focos de abajo
		double H = 12;
		pVertices[t].x = r*cos(alfa+dt);
		pVertices[t].y = H-hf;
		pVertices[t].z = r*sin(alfa+dt);
		pVertices[t].color = color_luz;
		pVertices[t].N.x = -cos(alfa);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa);
		pVertices[t].tv = 0;
		pVertices[t].tu = 0;

		++t;
		pVertices[t].x = r*cos(alfa-dt);
		pVertices[t].y = H+hf;
		pVertices[t].z = r*sin(alfa-dt);
		pVertices[t].color = color_luz;
		pVertices[t].N.x = -cos(alfa-dt);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa-dt);
		pVertices[t].tv = 1;
		pVertices[t].tu = -1;

		++t;
		pVertices[t].x = r*cos(alfa-dt);
		pVertices[t].y = H-hf;
		pVertices[t].z = r*sin(alfa-dt);
		pVertices[t].color = color_luz;
		pVertices[t].N.x = -cos(alfa-dt);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa-dt);
		pVertices[t].tv = 1;
		pVertices[t].tu = -1;


		++t;
		pVertices[t].x = r*cos(alfa+dt);
		pVertices[t].y = H+hf;
		pVertices[t].z = r*sin(alfa+dt);
		pVertices[t].color = color_luz;
		pVertices[t].N.x = -cos(alfa+dt);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa+dt);
		pVertices[t].tv = 1;
		pVertices[t].tu = 1;

		++t;

		// ------------------------------------------------
		// focos de arriba
		H = 27;
		pVertices[t].x = r*cos(alfa+dt);
		pVertices[t].y = H-hf;
		pVertices[t].z = r*sin(alfa+dt);
		pVertices[t].color = color_luz;
		pVertices[t].N.x = -cos(alfa);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa);
		pVertices[t].tv = 0;
		pVertices[t].tu = 0;

		++t;
		pVertices[t].x = r*cos(alfa-dt);
		pVertices[t].y = H+hf;
		pVertices[t].z = r*sin(alfa-dt);
		pVertices[t].color = color_luz;
		pVertices[t].N.x = -cos(alfa-dt);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa-dt);
		pVertices[t].tv = 1;
		pVertices[t].tu = -1;

		++t;
		pVertices[t].x = r*cos(alfa-dt);
		pVertices[t].y = H-hf;
		pVertices[t].z = r*sin(alfa-dt);
		pVertices[t].color = color_luz;
		pVertices[t].N.x = -cos(alfa-dt);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa-dt);
		pVertices[t].tv = 1;
		pVertices[t].tu = -1;


		++t;
		pVertices[t].x = r*cos(alfa+dt);
		pVertices[t].y = H+hf;
		pVertices[t].z = r*sin(alfa+dt);
		pVertices[t].color = color_luz;
		pVertices[t].N.x = -cos(alfa+dt);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa+dt);
		pVertices[t].tv = 1;
		pVertices[t].tu = 1;

		++t;
	
		alfa+=da;

	}

	// foco de luz: triangle fan
	alfa = 0.13;
	pos_vertex_focos2 = t;
	for(i=0;i<cant_focos;++i)
	{
		// ------------------------------------------------
		// focos de abajo
		double H = 12;
		double an_y_altura[] = {
				alfa,H,
				alfa,H-hf*0.15,
				alfa-dt,H-hf,
				alfa-dt/10,H,
				alfa-dt,H+hf,
				alfa,H+hf*0.15,
				alfa+dt,H+hf,
				alfa+dt/10,H,
				alfa+dt,H-hf,
				alfa,H-hf*0.15};

		for(int j=0;j<10;++j)
		{
			double an = an_y_altura[2*j];

			pVertices[t].x = r*cos(an);
			pVertices[t].y = an_y_altura[2*j+1];
			pVertices[t].z = r*sin(an);
			pVertices[t].color = D3DCOLOR_XRGB(60,80,100);//D3DCOLOR_XRGB(141,232,240);
			pVertices[t].N.x = -cos(an);
			pVertices[t].N.y = 0;
			pVertices[t].N.z = -sin(an);
			pVertices[t].tv = 0;
			pVertices[t].tu = 0;
			++t;
		}


		// focos de arriba
		for(j=0;j<10;++j)
		{
			double an = an_y_altura[2*j];

			pVertices[t].x = r*cos(an);
			pVertices[t].y = an_y_altura[2*j+1] + 15;
			pVertices[t].z = r*sin(an);
			pVertices[t].color = D3DCOLOR_XRGB(60,80,100);
			pVertices[t].N.x = -cos(an);
			pVertices[t].N.y = 0;
			pVertices[t].N.z = -sin(an);
			pVertices[t].tv = 0;
			pVertices[t].tu = 0;
			++t;
		}


		alfa+=da;
	}



	// Skydome
	pasos = 8;
	pos_vertex_skydome = t;
	cant_pri_skydome = pasos*2;
	dw = 4*radio;
	Kt = 600;		// tamaño de la textura de la pared
	da = 2*M_PI/pasos;
	alfa = 0;
	for(i=0;i<pasos;++i)
	{
		pVertices[t].x = dw*cos(alfa);
		pVertices[t].y = 0;
		pVertices[t].z = dw*sin(alfa);
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = -cos(alfa);
		pVertices[t].N.y = 0;
		pVertices[t].N.z = -sin(alfa);
		pVertices[t].tv = 1;
		pVertices[t].tu = -alfa*dw/Kt;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].y = 150;
		pVertices[t].tv = 0;

		++t;
	
		alfa+=da;

	}
	pVertices[t] = pVertices[pos_vertex_skydome];
	pVertices[t].tu = -2*M_PI*dw/Kt;
	++t;
	pVertices[t] = pVertices[pos_vertex_skydome+1];
	pVertices[t].tu = -2*M_PI*dw/Kt;
	++t;




	// Ruta
	// ----------------------------------------------
	ancho_ruta = 5;
	ancho_guarray = 3;
	pos_vertex_ruta = t;
	double Kr = 0.5;
	double dr = ancho_ruta/2;
	double dh = 3;		// alto de la pared
	// piso
	for(i=0;i<cant_ptos_ruta;++i)
	{
		TVector3d dir = pt_ruta[i+1]-pt_ruta[i];
		dir.normalizar();
		TVector3d n = dir*TVector3d(0,1,0);
		TVector3d p0 = pt_ruta[i] - n*dr;
		TVector3d p1 = pt_ruta[i] + n*dr;


		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 1;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 1;
		pVertices[t].tu = i*Kr;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p1.x;
		pVertices[t].y = p1.y;
		pVertices[t].z = p1.z;
		pVertices[t].tv = 0;

		++t;

	}
	// pared izquierda
	for(i=0;i<cant_ptos_ruta;++i)
	{
		TVector3d dir = pt_ruta[i+1]-pt_ruta[i];
		dir.normalizar();
		TVector3d n = dir*TVector3d(0,1,0);
		TVector3d u = n*dir;
		TVector3d p0 = pt_ruta[i] - n*(dr+ancho_guarray);
		TVector3d p1 = pt_ruta[i] - n*dr;	// + u*dh;
		p0.y = 0;


		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 1;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 1;
		pVertices[t].tu = i*Kr;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p1.x;
		pVertices[t].y = p1.y;
		pVertices[t].z = p1.z;
		pVertices[t].tv = 0;

		++t;

	}
	// pared derecha
	for(i=0;i<cant_ptos_ruta;++i)
	{
		TVector3d dir = pt_ruta[i+1]-pt_ruta[i];
		dir.normalizar();
		TVector3d n = dir*TVector3d(0,1,0);
		TVector3d u = n*dir;
		TVector3d p0 = pt_ruta[i] + n*(dr+ancho_guarray);
		TVector3d p1 = pt_ruta[i] + n*dr;	// + u*dh;
		p0.y = 0;


		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 1;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 1;
		pVertices[t].tu = i*Kr;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p1.x;
		pVertices[t].y = p1.y;
		pVertices[t].z = p1.z;
		pVertices[t].tv = 0;

		++t;
	}

	// techo, tiene 5 partes
	// 1-pared izquierda abajo 
	for(i=0;i<cant_ptos_ruta;++i)
	{
		TVector3d dir = pt_ruta[i+1]-pt_ruta[i];
		dir.normalizar();
		TVector3d n = dir*TVector3d(0,1,0);
		TVector3d u = n*dir;
		TVector3d p0 = pt_ruta[i] - n*dr;
		TVector3d p1 = pt_ruta[i] - n*(2*dr) + u*dh;


		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 1;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 1;
		pVertices[t].tu = i*Kr;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p1.x;
		pVertices[t].y = p1.y;
		pVertices[t].z = p1.z;
		pVertices[t].tv = 0;

		++t;

	}

	// 2-pared izquierda arriba
	for(i=0;i<cant_ptos_ruta;++i)
	{
		TVector3d dir = pt_ruta[i+1]-pt_ruta[i];
		dir.normalizar();
		TVector3d n = dir*TVector3d(0,1,0);
		TVector3d u = n*dir;
		TVector3d p0 = pt_ruta[i] - n*(2*dr) + u*dh;
		TVector3d p1 = pt_ruta[i] - n*dr + u*(2*dh);


		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 1;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 1;
		pVertices[t].tu = i*Kr;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p1.x;
		pVertices[t].y = p1.y;
		pVertices[t].z = p1.z;
		pVertices[t].tv = 0;

		++t;

	}

	// 3- techo pp dicho
	for(i=0;i<cant_ptos_ruta;++i)
	{
		TVector3d dir = pt_ruta[i+1]-pt_ruta[i];
		dir.normalizar();
		TVector3d n = dir*TVector3d(0,1,0);
		TVector3d u = n*dir;
		TVector3d p0 = pt_ruta[i] - n*dr + u*(2*dh);
		TVector3d p1 = pt_ruta[i] + n*dr + u*(2*dh);

		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 1;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 1;
		pVertices[t].tu = i*Kr;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p1.x;
		pVertices[t].y = p1.y;
		pVertices[t].z = p1.z;
		pVertices[t].tv = 0;

		++t;

	}
	
	// 4-pared derecha abajo 
	for(i=0;i<cant_ptos_ruta;++i)
	{
		TVector3d dir = pt_ruta[i+1]-pt_ruta[i];
		dir.normalizar();
		TVector3d n = dir*TVector3d(0,1,0);
		TVector3d u = n*dir;
		TVector3d p0 = pt_ruta[i] + n*dr;
		TVector3d p1 = pt_ruta[i] + n*(2*dr) + u*dh;


		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 1;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 1;
		pVertices[t].tu = i*Kr;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p1.x;
		pVertices[t].y = p1.y;
		pVertices[t].z = p1.z;
		pVertices[t].tv = 0;

		++t;

	}

	// 5-pared derecha arriba
	for(i=0;i<cant_ptos_ruta;++i)
	{
		TVector3d dir = pt_ruta[i+1]-pt_ruta[i];
		dir.normalizar();
		TVector3d n = dir*TVector3d(0,1,0);
		TVector3d u = n*dir;
		TVector3d p0 = pt_ruta[i] + n*(2*dr) + u*dh;
		TVector3d p1 = pt_ruta[i] + n*dr + u*(2*dh);


		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;
		pVertices[t].color = D3DCOLOR_XRGB(255,255,255);
		pVertices[t].N.x = 0;
		pVertices[t].N.y = 1;
		pVertices[t].N.z = 0;
		pVertices[t].tv = 1;
		pVertices[t].tu = i*Kr;

		++t;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p1.x;
		pVertices[t].y = p1.y;
		pVertices[t].z = p1.z;
		pVertices[t].tv = 0;

		++t;

	}

	g_pVB->Unlock();


	// Quad para postprocess 2d
    QUADVERTEX vertices[8];
    // Leer esto en el manual del directX : "Directly Mapping Texels to Pixels"
    FLOAT desf = 0.1f;

    vertices[0].pos = D3DXVECTOR4(sdx-desf, -desf, 0.0f, 1.0f);
    vertices[0].tu = 1;
	vertices[0].tv = 0;

    vertices[1].pos = D3DXVECTOR4(sdx-desf, sdy-desf, 0.0f, 1.0f);
    vertices[1].tu = 1;
	vertices[1].tv = 1;

    vertices[2].pos = D3DXVECTOR4(-desf, -desf, 0.0f, 1.0f);
    vertices[2].tu = 0;
	vertices[2].tv = 0;

    vertices[3].pos = D3DXVECTOR4(-desf, sdy-desf, 0.0f, 1.0f);
    vertices[3].tu = 0;
	vertices[3].tv = 1;

	// estos 4 son para la pantalla de BackBufferWidth x BackBufferHeight
    FLOAT fWidth5 = (FLOAT)d3dpp.BackBufferWidth- desf;
    FLOAT fHeight5 = (FLOAT)d3dpp.BackBufferHeight- desf;
    vertices[4].pos = D3DXVECTOR4(fWidth5, -desf, 0.0f, 1.0f);
    vertices[4].tu = 1;
	vertices[4].tv = 0;

    vertices[5].pos = D3DXVECTOR4(fWidth5, fHeight5, 0.0f, 1.0f);
    vertices[5].tu = 1;
	vertices[5].tv = 1;

    vertices[6].pos = D3DXVECTOR4(-desf, -desf, 0.0f, 1.0f);
    vertices[6].tu = 0;
	vertices[6].tv = 0;

    vertices[7].pos = D3DXVECTOR4(-desf, fHeight5, 0.0f, 1.0f);
    vertices[7].tu = 0;
	vertices[7].tv = 1;


    if( FAILED( g_pd3dDevice->CreateVertexBuffer( 8*sizeof(QUADVERTEX),
                                                  0, D3DFVF_QUADVERTEX,
                                                  D3DPOOL_DEFAULT, &g_pQuad, NULL ) ) )
    {
        return E_FAIL;
    }

    VOID* pQuad;
    if( FAILED( g_pQuad->Lock( 0, sizeof(vertices), (void**)&pQuad, 0 ) ) )
        return E_FAIL;
    memcpy( pQuad, vertices, sizeof(vertices) );
    g_pQuad->Unlock();


	return S_OK;
}




HRESULT CGameEngine::DXSetupRender()
{
	// Especifico del Juego: Calculo el LF,LA, etc 
	if(!modo_gui && !modo_camara)
	{
		if(timer_warming>0 )
		{
			float t = min(timer_warming,3);
			// entrada en calor: muevo la camara en posicion a la moto
			double alfa = P[0].alfa;
			TVector3d dir = TVector3d(cos(alfa),0,sin(alfa));
			TVector3d n = TVector3d(0,1,0)*dir;
			LA = P[0].Pos + dir*30;
			LF = P[0].Pos - dir*(12+t*10) + TVector3d(0,3+t*4,0) + n*3.5;
		}
		else
		{
			if(player_one->cant_vidas || player_one->timer_rebotando)
				player_one->SetCamara();
			else
			{
				int i=0;
				while(i<cant_players && !P[i].cant_vidas)
					++i;

				if(i<cant_players)
					P[i].SetCamara();
				else
					// no hay mas jugadores: dejo puesto el modo camara
					modo_camara = TRUE;
			}



			if(smooth_cam)
			{
				{
					TVector3d v = LA_d - LA;
					float d = v.mod();
					double s = elapsed_time*vel_rot_camara*player_one->vel;
					if(s>d)
						s = d;
					v.normalizar();
					LA = LA + v*s;
					if(smooth_cam==2)
					{
						LA.x = LA_d.x;
						LA.z = LA_d.z;
					}
				}
			
				{
					TVector3d v = LF_d - LF;
					float d = v.mod();
					double s = elapsed_time*vel_camara*player_one->vel;
					if(s>d)
						s = d;
					v.normalizar();
					LF = LF + v*s;
					if(smooth_cam==2)
					{
						LF.x = LF_d.x;
						LF.z = LF_d.z;
					}
				}
			}

		}
	}

	return DXEngine::DXSetupRender();
}



HRESULT CGameEngine::DXRender()
{
	// Guardo el render target anterior antes de empezar con todo el proceso
    LPDIRECT3DSURFACE9 pRTOld = NULL;
	LPDIRECT3DSURFACE9 pDSOld = NULL;
    g_pd3dDevice->GetRenderTarget( 0, &pRTOld );
	g_pd3dDevice->GetDepthStencilSurface( &pDSOld);

	downsampling = FALSE;

	// inicializo las matrices 
	DXSetupRender();

	//1- Inicializo el Efecto
	//-------------------------------------------------
	if(!g_pEffect)
		LoadFx("grid.fx");

	// Resolucion de pantalla
	g_pEffect->SetFloat("screen_dx", sdx);      
	g_pEffect->SetFloat("screen_dy", sdy);      


	if(modo_draft)
	{
		// MODO DRAFT
		//-------------------------------------------------
		// limpio la pantalla 
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0), 1, 0 );
		// dibujo la escena pp dicha
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
		if( SUCCEEDED( g_pd3dDevice->BeginScene()))
		{
			RenderScene();
			g_pd3dDevice->EndScene();
		}
	}
	else
	{
		LPDIRECT3DSURFACE9 pRenderSurface,pRenderSurface2,pRenderSurface3,pRenderSurface4;
		

		//2- dibujo todo lo que tiene halos de luz para luego borronear
		//-------------------------------------------------
		g_pTxLigthPath->GetSurfaceLevel(0,&pRenderSurface);
		if(!SUCCEEDED(g_pd3dDevice->SetRenderTarget(0,pRenderSurface)))
			return E_FAIL;
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0), 1, 0 );
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);

		if( SUCCEEDED( g_pd3dDevice->BeginScene()))
		{
			RenderMirror();
			RenderScene(TRUE);

			g_pd3dDevice->EndScene();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );

		//if(_generar_bmps)
		//	D3DXSaveTextureToFile("test1.bmp",D3DXIFF_BMP,g_pTxLigthPath,NULL);
		
		// llamo a una PS para "borronear" el ligthPath y dar el efecto de luz 
		// 1era pasada: borroneo horizontal
		g_pTxLigthPath2->GetSurfaceLevel(0,&pRenderSurface2);
		if(!SUCCEEDED(g_pd3dDevice->SetRenderTarget(0,pRenderSurface2)))
			return E_FAIL;
		// limpio la pantalla 
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0), 1, 0 );
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
		g_pEffect->SetTexture( "g_Bloom", g_pTxLigthPath);

		if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
		{
			g_pd3dDevice->SetStreamSource( 0, g_pQuad, 0, sizeof(QUADVERTEX));
			g_pd3dDevice->SetFVF( D3DFVF_QUADVERTEX);
        
			UINT cPasses;
			g_pEffect->SetTechnique( "AgrandarBlancos" );
			g_pEffect->Begin(&cPasses, 0);
			g_pEffect->BeginPass(0);
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
			g_pEffect->EndPass();
			g_pEffect->End();
			
			g_pd3dDevice->EndScene();
			g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		}

		//if(_generar_bmps)
		//	D3DXSaveTextureToFile("test2.bmp",D3DXIFF_BMP,g_pTxLigthPath2,NULL);


		// segunda pasada : borroneo horizontal
		if(!SUCCEEDED(g_pd3dDevice->SetRenderTarget(0,pRenderSurface)))
			return E_FAIL;
		if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
		{
			g_pd3dDevice->SetStreamSource( 0, g_pQuad, 0, sizeof(QUADVERTEX));
			g_pd3dDevice->SetFVF( D3DFVF_QUADVERTEX);
			g_pEffect->SetTexture( "g_LigthPath", g_pTxLigthPath2);
        
			UINT cPasses;
			g_pEffect->SetTechnique( "LigthPathH" );
			g_pEffect->Begin(&cPasses, 0);
			g_pEffect->BeginPass(0);
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
			g_pEffect->EndPass();
			g_pEffect->End();
			
			g_pd3dDevice->EndScene();
			g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		}
		
		//if(_generar_bmps)
		//	D3DXSaveTextureToFile("test3.bmp",D3DXIFF_BMP,g_pTxLigthPath,NULL);


		// tercer pasada : borroneo Vertical
		if(!SUCCEEDED(g_pd3dDevice->SetRenderTarget(0,pRenderSurface2)))
			return E_FAIL;
		if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
		{
			g_pd3dDevice->SetStreamSource( 0, g_pQuad, 0, sizeof(QUADVERTEX));
			g_pd3dDevice->SetFVF( D3DFVF_QUADVERTEX);
			g_pEffect->SetTexture( "g_LigthPath", g_pTxLigthPath);
        
			UINT cPasses;
			g_pEffect->SetTechnique( "LigthPathV" );
			g_pEffect->Begin(&cPasses, 0);
			g_pEffect->BeginPass(0);
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
			g_pEffect->EndPass();
			g_pEffect->End();

			g_pd3dDevice->EndScene();
			g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		}

		//if(_generar_bmps)
		//	D3DXSaveTextureToFile("test4.bmp",D3DXIFF_BMP,g_pTxLigthPath2,NULL);


		// creo la textura para dibujar la escena pp dicha
		g_pRenderTarget->GetSurfaceLevel(0,&pRenderSurface3);
		if(!SUCCEEDED(g_pd3dDevice->SetRenderTarget(0,pRenderSurface3)))
			return E_FAIL;
		// limpio la pantalla 
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0), 1, 0 );
		// dibujo la escena pp dicha
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
		//if(dxcaps_downsampling)
			//downsampling = TRUE;		// activo el downsampling
		if( SUCCEEDED( g_pd3dDevice->BeginScene()))
		{
			RenderScene();
			g_pd3dDevice->EndScene();
		}
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		//if(_generar_bmps)
		//	D3DXSaveTextureToFile("test5.bmp",D3DXIFF_BMP,g_pRenderTarget,NULL);

		//downsampling = FALSE;

		// Combino todos los maps en una imagen para presentar
		//-------------------------------------------------
		g_pRenderTarget3->GetSurfaceLevel(0,&pRenderSurface4);
		if(!SUCCEEDED(g_pd3dDevice->SetRenderTarget(0,pRenderSurface4)))
			return E_FAIL;
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(255,0,0), 0.0f, 0);
		g_pEffect->SetTexture( "g_Mirror", g_pTxLigthPath);			// Escena Reflejada
		g_pEffect->SetTexture( "g_LigthPath", g_pTxLigthPath2);			// Mapa de Reflexiones
		g_pEffect->SetTexture( "g_RenderTarget", g_pRenderTarget);		// Escena pp dicha
		g_pEffect->SetTexture( "g_RenderTarget2", g_pRenderTarget2);	// Escena anterior
		
		// efecto de tembleque (no me gusto como queda, asi que se lo saque del fx)
		//g_pEffect->SetFloat("desf_luz",(float)(((int)(time*1000))%2));

		// motion blur en la ruta ? 
		if(modo_juego==GAME_TEST_RUTA)
			g_pEffect->SetBool("g_bMotionBlur",player_one->vel>15?TRUE:FALSE);
		
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);

		if( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
		{
			g_pd3dDevice->SetStreamSource( 0, g_pQuad, 0, sizeof(QUADVERTEX));
			g_pd3dDevice->SetFVF( D3DFVF_QUADVERTEX);
        
			UINT cPasses;
			g_pEffect->SetTechnique( "TexCombine" );
			g_pEffect->Begin(&cPasses, 0);
			g_pEffect->BeginPass(0);

			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 4, 2 );

			g_pEffect->EndPass();
			g_pEffect->End();



			g_pd3dDevice->EndScene();
			g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		}

		// Restauro el RenderTarget anterior
		g_pd3dDevice->SetRenderTarget( 0, pRTOld  );
		g_pd3dDevice->SetDepthStencilSurface(pDSOld);

		// copio a la pantalla:
		g_pd3dDevice->StretchRect(pRenderSurface4,NULL,pRTOld,NULL,D3DTEXF_NONE);
		//if(_generar_bmps)
		//	D3DXSaveSurfaceToFile("test6.bmp",D3DXIFF_BMP,pRenderSurface4,NULL,NULL);


		SAFE_RELEASE( pRenderSurface);
		SAFE_RELEASE( pRenderSurface2);
		SAFE_RELEASE( pRenderSurface3);
		SAFE_RELEASE( pRenderSurface4);


	}

	
	if(modo_gui || timer_pausa)
	{
		gui.Update();
		gui.Render();
		if(animacion_gui==GUI_PREVIEW_RUTA || animacion_gui==GUI_PREVIEW_MOTO)
		{

			D3DVIEWPORT9 viewport,viewport_ant;
			g_pd3dDevice->GetViewport(&viewport_ant);
			viewport.MaxZ = 1;
			viewport.MinZ = 0;

			if(animacion_gui==GUI_PREVIEW_RUTA)
			{
				viewport.X = 300;
				viewport.Y = 240;
				viewport.Width = 350;
				viewport.Height = 200;
			}
			else
			{
				viewport.X = 400;
				viewport.Y = 200;
				viewport.Width = 400;
				viewport.Height = 300;
			}
 
			g_pd3dDevice->SetViewport(&viewport);
			// inicializo las matrices 
			TVector3d LA_ant = LA;
			TVector3d LF_ant = LF;
			LA = pr_LA;
			LF = pr_LF;
			DXSetupRender();

			if( SUCCEEDED( g_pd3dDevice->BeginScene()))
			{
				g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
				if(animacion_gui==GUI_PREVIEW_RUTA)
				{
					g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
					g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
					g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(128,128,128), 1, 0 );


					// dibujo la ruta
					g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);		
					g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
					// piso de la ruta
					g_pd3dDevice->SetTexture( 0, g_pTexture[textura_ruta+3]);
					g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta,2*(cant_ptos_ruta-1));
			
					// paredes de la ruta:
					g_pd3dDevice->SetTexture( 0, g_pTexture[textura_ruta+1]);
					g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta+2*cant_ptos_ruta,2*(cant_ptos_ruta-1));
					g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta+4*cant_ptos_ruta,2*(cant_ptos_ruta-1));
				}
				else
				{
					// borro solo el zbuffer
					g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(0,0,0), 1, 0 );
					// dibujo la moto, pintada con el color que esta seleccionado en el gui
					player_one->cycle_mesh->SetColor(gui.sel_color);
					player_one->cycle_mesh->DXRender(TVector3d(0,0,0),
						player::size_cycle*15,TVector3d(1,0,0));
					// Restauro el color de la moto
					player_one->cycle_mesh->SetColor(player_one->c_lightpath);
				}
				g_pd3dDevice->EndScene();
			}
			g_pd3dDevice->SetViewport(&viewport_ant);
			LA = LA_ant;
			LF = LF_ant;
		}

	}
	else
	{
		scoreboard();

		// dialogos no modales
		if(timer_modalless>0)
		{
			// --- Dialogos no modales ------
			timer_modalless -= elapsed_time;
			if(timer_modalless<0)
			{
				timer_modalless = 0;
				// finalizo el dialogo no modal
				gui.EndDialog();
			}
			else
			{
				// actualizo y dibujo el dialogo
				gui.Update();
				gui.Render();
			}
		}
	}
	
	
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );


	//-------------------------------------------------
	// dejo todo como estaba antes
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
	
	SAFE_RELEASE( pRTOld );
	SAFE_RELEASE( pDSOld );
	
	static double ftime = 0;		// frame time
	ftime += elapsed_time;
	//if(ftime>0.05)
	{
		ftime = 0;
		LPDIRECT3DTEXTURE9 aux = g_pRenderTarget2;
		g_pRenderTarget2 = g_pRenderTarget3;
		g_pRenderTarget3 = aux;
	}
	

	return S_OK;
}




void CGameEngine::InitGame()
{

	// Reseteo algunas variables y timers
	ronda = 0;
	timer_pausa = 0;
	timer_warming = 0;
	timer_presentacion = 0;
	timer_game_over = 0;
	timer_modalless = 0;

	// limpio celdas y lightpath 
	memset(celdas,0,sizeof(celdas));
	memset(ligthpath_cant,0,sizeof(ligthpath_cant));


	// Genero el path de la ruta 
	load_ruta();

	// Creo el/ los jugadores
	cant_players = 0;
	if(multiplayer)
	{

		if(es_servidor)
		{
			// jugador local
			P[0].Create(this,TVector3d(-35,0,40),0);
			// jugador remoto
			P[1].Create(this,TVector3d(-25,0,40),1);
		}
		else
		{
			// jugador remoto
			P[1].Create(this,TVector3d(-35,0,40),1);
			// jugador local
			P[0].Create(this,TVector3d(-25,0,40),0);
		}

	}
	else
	{
		// jugador principal
		float an = M_PI/4;
		P[0].Create(this,TVector3d(cos(an)*radio/2,0,sin(an)*radio/2),0);
		// enemigo
		an+=M_PI/2;
		P[1].Create(this,TVector3d(cos(an)*radio/2,0,sin(an)*radio/2),1);
	}
	// creo el resto de los enemigos en lugares especificos 
	P[2].Create(this,TVector3d(-20,0,20),2);
	P[3].Create(this,TVector3d(20,0,-20),3);
	P[4].Create(this,TVector3d(-20,0,0),4);
	P[5].Create(this,TVector3d(20,0,0),5);
	P[6].Create(this,TVector3d(0,0,-20),6);
	P[7].Create(this,TVector3d(0,0,20),7);

	// Reseteo la cantidad de vidas y vuelvo a la pos inicial
	for(int i=0;i<8;++i)
	{
		P[i].Reset();
		//P[i].Update();
		//P[i].actualizarPath();
	}

	switch(modo_juego)
	{
		case GAME_TEST_RUTA:
			cant_players = 1;
			P[0].vel_ini = P[0].vel = 10;
			P[0].cant_vidas = 3;
			break;
		case GAME_GRID_BATTLE:
			// batalla a muerte, en rondas uno contra uno
			cant_players = 2;
			break;
		case GAME_GRID_SURVIVAL:
			// todos contra todos, el que mas dure
			cant_players = 1 + cant_enemigos;
			break;
		case GAME_TEST_LABERINTO:
			P[0].Create(this,TVector3d((17-32)*dgrid*0.5,0,(31-32)*dgrid*0.5),0);
			P[1].Create(this,TVector3d(20,0,20),1);
			// le genero un ligthpath
			P[1].crearLaberintoLuz();
			P[1].cant_vidas = 0;
			cant_players = 2;
			break;
	}
	player_one = &P[0];

	// Creo los vertices del escenario fijo (eso incluye la ruta, que puede variar de juego a juego)
	DXCreateVertex();

	//midi_DeviceID = playMIDIFile(m_hWnd,fname_midi);

}


BOOL _semaforo = FALSE;

struct param_play_list
{
	HWND m_hWnd;
	char fname[255];
	UINT *midi_DeviceID;

};


DWORD WINAPI _avanzarPlayList(long lParam)
{ 
	_semaforo = TRUE;
	param_play_list	*p = (param_play_list*)lParam;
	mciSendCommand(*p->midi_DeviceID, MCI_CLOSE, 0, NULL);

	*p->midi_DeviceID = playMIDIFile(p->m_hWnd,p->fname);
	delete p;
	_semaforo = FALSE;
	return 0;

}


// paso al siguiente tmea
void CGameEngine::playMidi()
{
	//if(_semaforo)
	//	return;

	// corto el anterior
	mciSendCommand(midi_DeviceID, MCI_CLOSE, 0, NULL);

	char buffer[255];
	sprintf(buffer,"sounds\\%s",midi_files[midi_actual]);
	midi_DeviceID = playMIDIFile(m_hWnd,buffer);

	/*
	struct param_play_list *p = new param_play_list;
	strcpy(p->fname,buffer);
	p->m_hWnd = m_hWnd;
	p->midi_DeviceID = &midi_DeviceID;
	DWORD ID;
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)_avanzarPlayList,p,0,&ID);
	*/

}





void CGameEngine::loopMidi()
{

}


// Carga los ptos de la ruta
int CGameEngine::load_pt_ruta(int tipo,TVector3d *pt)
{
	// Genero el path de la ruta 
	double dw = radio-10;
	double dt = M_PI/64;
	double t = 0;
	double hasta = tipo?2*M_PI:4*M_PI;
	int cant = 0;
	while(t<hasta+0.1)
	{

		switch(tipo)
		{
			case 0:
				// espiral
				pt[cant].x = dw*cos(t);
				pt[cant].z = dw*sin(t);
				pt[cant].y = t*50;
				break;
			case 1:
				// espiral modificada
				pt[cant].x = dw*cos(t);
				pt[cant].z = dw*sin(t);
				pt[cant].y = 15+((2+sin(t) + cos(t)))*10;
				break;
			case 2:
				// estrella
				pt[cant].x = 8*(8*cos(t)+cos(5*t)*cos(t));
				pt[cant].z = 8*(8*sin(t)+cos(5*t)*sin(t));
				pt[cant].y = 50+(8*sin(3*t)) + 3*(2-sin(8*t));
				break;
			case 3:
				// ruta "casi" plana
				pt[cant].x = 8*(8*cos(t)+cos(5*t)*cos(t));
				pt[cant].z = 8*(8*sin(t)+cos(4*t)*sin(t));
				pt[cant].y = max(0,3 + 2*cos(3+t*5));
				break;
		}
		

		t+=dt;
		++cant;
	}

	--cant;		// me aseguro que siempre exista el i+1 

	return cant;

}


void CGameEngine::load_ruta()
{

	if(tipo_ruta==-1)
	{
		// sin ruta
		cant_ptos_ruta = 0;
		max_tramo_ruta = 0;
	}
	else
	{
		cant_ptos_ruta = load_pt_ruta(tipo_ruta,pt_ruta);
		max_tramo_ruta = 0;
		for(int i =0;i<cant_ptos_ruta;++i)
		{
			double dist = (pt_ruta[i].pxz()-pt_ruta[i+1].pxz()).mod();
			if(dist>max_tramo_ruta)
				max_tramo_ruta	= dist;
		}
	}
}







// dibuja toda la escena
void CGameEngine::RenderScene(BOOL resplandor,BOOL mirror)
{
	
	g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
	int i;

	if(!resplandor)	
	{
		// dibujo el skydome :
		g_pd3dDevice->SetTexture( 0, g_pTexture[textura_skydome]);
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_skydome,cant_pri_skydome);

		if(!mirror)
		{
			// dibujo el piso:
			g_pd3dDevice->SetTexture( 0, g_pTexture[textura_piso]);
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,0,2);


		}
	}

	if(modo_gui && animacion_gui!=GUI_PRESENTACION && animacion_gui!=GUI_GAME_PAUSED)
	{
		// dibujo la pared circular
		g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		g_pd3dDevice->SetTexture( 0, g_pTexture[textura_pared]);
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,4,64*2);


		// controles del gui pp dicho
		if(resplandor)
			gui.Render(TRUE);

		//y me rajo
		return;
	}



	for(i=0;i<cant_players;++i)
	{
		P[i].RenderLightPath(resplandor,mirror);
		if(P[i].timer_rebotando<=0)			//|| !resplandor
			P[i].RenderCycle();
	}

	RenderBolas();
	RenderRecognizers(resplandor);


	// dibujo la pared circular
	g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->SetTexture( 0, g_pTexture[textura_pared]);
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,4,64*2);



	if(resplandor)
	{
		// dibujo los focos de luz
		g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE);
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
		if(mirror)
		{
			// quiero que el foco reflejado sea mas claro que lo normal
			// ademas, como no es 255,255,255 no lo va a agrandar el PS Agrandar
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
			g_pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_XRGB(128,128,128));
		}

		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);
		
		// estrella con triangulos
		for(int t=0;t<2*cant_focos;++t)
			g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,pos_vertex_focos2+10*t,8);
		// estrella con 2 lineas, que rota s/ el pto de vista
		RenderFocos();
		//g_pd3dDevice->DrawPrimitive( D3DPT_LINELIST,pos_vertex_focos,4*cant_focos);

		// Restauro todo el entorno
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE);
		if(mirror)
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);

	}

	// dibujo la ruta
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);		
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	if(!mirror )
	{
		// piso de la ruta
		g_pd3dDevice->SetTexture( 0, g_pTexture[resplandor?textura_ruta:textura_ruta+3]);
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta,2*(cant_ptos_ruta-1));
	}

	// paredes de la ruta:
	g_pd3dDevice->SetTexture( 0, g_pTexture[resplandor?textura_ruta+4:textura_ruta+1]);
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta+2*cant_ptos_ruta,2*(cant_ptos_ruta-1));
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta+4*cant_ptos_ruta,2*(cant_ptos_ruta-1));
	if(player_one->pos_en_ruta>=0 && techo_ruta)
	{
		// dibujo el techo 
		g_pd3dDevice->SetTexture( 0, g_pTexture[textura_ruta+2]);		// techo
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta+6*cant_ptos_ruta,2*(cant_ptos_ruta-1));
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta+8*cant_ptos_ruta,2*(cant_ptos_ruta-1));
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta+10*cant_ptos_ruta,2*(cant_ptos_ruta-1));
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta+12*cant_ptos_ruta,2*(cant_ptos_ruta-1));
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,pos_vertex_ruta+14*cant_ptos_ruta,2*(cant_ptos_ruta-1));
	}
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);		
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	
	for(i=0;i<cant_players;++i)
	if(P[i].timer_rebotando)
		// dibujo la moto explotando, lo hago al final de todo sin zbuffer activo,
		// para que no se vea oculto por el resto de las cosas, siendo que es 
		// basicamente un efecto 2d. 
		P[i].RenderCycle(resplandor);


}



void CGameEngine::MainLoop()
{

	BOOL seguir = TRUE;
	CPoint Move,Last;
	GetCursorPos(&Last);
	SetCapture(m_hWnd);
	BOOL moving = FALSE;

	int nFlags = 0;
	int cant_frames = 0;
	time = 0;				// tiempo total
	double ftime = 0;		// frame time
	elapsed_time = 0;		// tiempo parcial (laps) 

	LARGE_INTEGER F,T0,T1;   // address of current frequency
	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);

	// update inicial: 
	for(int i=0;i<cant_players;++i)
		P[i].Update();

	SetCursorPos(d3dpp.BackBufferWidth/2,d3dpp.BackBufferHeight/2);

	timer_warming = 3;		// seg de "entrada en calor"

	grid_packet paq;
	memset(&paq,0,sizeof(paq));
	// multiplayer: le mando un paquete al usuario remoto para avisarle que empieza el juego
	if(multiplayer && es_servidor)
	{
		// el packete se usa para mandarle info del nombre del usuario y otros datos
		strncpy(paq.data , player_one->nombre,31);
		paq.data[32] = '\0'; 
		paq.cmd = 0;		// Hello packet
		paq.alfa = player_one->nro_avatar;
		paq.beta = player_one->nro_color;
		P[1].socket.SendData((char *)&paq,sizeof(paq));
		P[1].socket.Flush();
	}

	float timer_sync = 0;
	lagging = FALSE;
	timer_lag = 0;

	while(seguir)
	{
		// Avanzo el tiempo
		QueryPerformanceCounter(&T1);
		elapsed_time = (double)(T1.LowPart - T0.LowPart) / (double)F.LowPart;
		if(GetAsyncKeyState(VK_CONTROL))
			elapsed_time*=0.1;

		if(camara_lenta)
			elapsed_time*=0.25;

		T0 = T1;
		

		// debugg paso a paso
		//elapsed_time = 0.05;

		// control de lagging
		if(multiplayer)
		{
			timer_lag += elapsed_time;
			if(timer_lag>=5)
			{
				timer_lag = 0;
				if(es_servidor)
				{
					// envio un msg de sincronizacion al remoto
					grid_packet packet;
					player_one->llenar_paquete(&packet,99);
					P[1].socket.SendData((char *)&packet,sizeof(packet));
					P[1].socket.Flush();
				}

				// y espero que me conteste con un ack
				lagging = TRUE;
				P[1].Wait();
				lagging = FALSE;
			}
		}


		if(timer_pausa)
		{
			timer_pausa-=elapsed_time;
			if(timer_pausa<0)
			{
				timer_pausa = 0;
				gui.EndDialog();
			}
			else
			{
				sprintf(msg_pausa->text,"Get Ready - %5.1f",timer_pausa);
				frame_pausa->c_fondo = D3DCOLOR_ARGB((BYTE)(timer_pausa*0.1*255),0,0,0);
				if(timer_pausa<1)
				{
					float t = timer_pausa;
					gui.ex = t;
					gui.ey = t;
				}
			}
		}

		// logica del juego
		if(!paused && timer_pausa<=0)
		{
			time += elapsed_time;		// avanzo el tiempo
			
			if(timer_game_over>0)
			{
				// --- Game over------
				timer_game_over -= elapsed_time;
				survival_time += elapsed_time;
				if(timer_game_over<0)
				{
					timer_game_over = 0;
					seguir = FALSE;		// termina el mainloop

					if(multiplayer)
					{
						// antes de salir, espero a que los demas jugadores reciban
						// el msg de que termino el juego
						if(es_servidor)
						{
							// envio un msg de sincronizacion al remoto
							grid_packet packet;
							player_one->llenar_paquete(&packet,99);
							P[1].socket.SendData((char *)&packet,sizeof(packet));
							P[1].socket.Flush();
						}

						// y espero que me conteste con un ack
						lagging = TRUE;
						P[1].Wait();
						lagging = FALSE;
					}
				}
			}
			else
			if(timer_warming>0)
			{
				// --- Warming up ------
				timer_warming -= elapsed_time;
				if(timer_warming<0)
				{
					timer_warming = 0;
					survival_time = time;
					// comienza el juego pp dicho: inicializao el lightpath de cada jugador:
					for(int i=0;i<cant_players;++i)
						P[i].initPath();
				}
			}
			else
			{				

				if(multiplayer)
				{
					// el jugador uno es el usuario local
					// -------------------------------------

					//if(!es_servidor)
						// si no es servidor, hago que la maquina juege sola: 
						// para poder probarlo sin manejar los 2 al mismo tiempo. 
					//player_one->IA();


					player_one->Update();
					player_one->actualizarPath();


					if(camara_lenta)
						timer_sync += elapsed_time/0.25;
					else
						timer_sync += elapsed_time;
					if(timer_sync>=0.01)
					{
						timer_sync = 0;
						if(es_servidor)
						{
							// le mando la informacion del player one a cada jugador remoto
							grid_packet packet;
							player_one->llenar_paquete(&packet);
							P[1].socket.SendData((char *)&packet,sizeof(packet));
							P[1].socket.Flush();
						}
						else
							// le mando mi informacion al servidor
							player_one->Send();
					}

					// el jugador 2 es el usuario remoto, que es una maquina ahora
					// -------------------------------------
					P[1].UpdateRemoto();
					P[1].actualizarPath();


				}
				else
				{
					// logica de c/jugador
					for(int i=cant_players-1;i>=0;--i)
					{
						if(i || juega_sola)
							P[i].IA();
						P[i].Update();
						// actualizo el ligth path
						P[i].actualizarPath();
					}
				}

				if(modo_juego==GAME_GRID_SURVIVAL)
				{ 
					if(player_one->cant_vidas<=0)
						// termino el juego
						timer_game_over = 5;
					else
					if(multiplayer && P[1].cant_vidas<=0)
						// game: termina el juego tambien
						timer_game_over = 5;
				}

			}
			
		}


		// calculo los fps
		// --------------------
		++cant_frames;
		ftime += elapsed_time;
		if(ftime>1)
		{
			fps = cant_frames/ftime;
			ftime = 0;
			cant_frames = 0;

		}

		// --------------------
		// Renderizo
		for(int i=0;i<cant_players;++i)
			P[i].DXCreatePathVertex();
		DXRender();

		// --------------------
		// Input del mouse (con DirectInput)
		if(mouse.ReadImmediateData()==S_OK)
		{
			
			// proceso el mensaje
			mouse_dx = mouse.dims2.lX;
			mouse_dy = mouse.dims2.lY;

			if(mouse.dims2.lZ)
			{
				//	WM_MOUSEWHEEL:
				short zDelta = mouse.dims2.lZ;    // wheel rotation
				TVector3d dir_cam = LA-LF;
				double dist = dir_cam.mod();
				dir_cam.normalizar();
				LF = LF + dir_cam*(zDelta/30);
				LA = LF + dir_cam*dist;
			}
			else
			if((mouse_dx || mouse_dy) && modo_camara && mouse.dims2.rgbButtons[0]&0x80)
			{
				// Mouse move en modo camara (con el boton apretado)
				double an = (double)mouse_dx/(double)d3dpp.BackBufferWidth*2*M_PI*2;
				if(GetAsyncKeyState(VK_CONTROL))
				{
					LF.y -= mouse_dy;
					LF.rotar_xz(LA,-an);
				}
				else
				{
					// uso el desplazamiento vertical, par elevar o bajar el punto de vista
					LA.y -= mouse_dy;
					// uso el desplazamiento en x para rotar el punto de vista  en el plano xy
					LA.rotar_xz(LF,-an);
				}
			}

			// dejo que el jugador procese el msg
			if(!modo_camara && timer_warming<=0 /*&& !(multiplayer && !es_servidor)*/)
				//	player_one->ProcessMsg(&Msg);
				player_one->ProcessInput(&mouse);
		}


		// --------------------
		// Proceso los msg de windows
		MSG Msg;
        ZeroMemory( &Msg, sizeof(Msg) );
		if(PeekMessage( &Msg, NULL, 0U, 0U, PM_REMOVE ) )
		{

			if(Msg.message == WM_QUIT)
			{
				seguir = FALSE;
				break;
			}

			// dejo que windows procese el mensaje
			TranslateMessage( &Msg );
            DispatchMessage( &Msg );

			// Proceso msg de teclas usando el windows (no el direct input)
			if(Msg.message==WM_KEYDOWN)
			{
				switch((int) Msg.wParam)	    // virtual-key code 
				{
					case VK_F1:
						modo_camara = !modo_camara;
						break;
					case VK_UP:
						if(modo_camara)
						{
							LF.y+=2;
							LA.y+=2;
						}
						break;
					case VK_DOWN:
						if(modo_camara)
						{
							LF.y-=2;
							LA.y-=2;
						}
						break;
					case VK_SPACE:
						paused = !paused;
						if(paused)
						{
							GUI_pausa();
							// engaño al timer: 
							QueryPerformanceCounter(&T0);
						}
						break;

					case VK_ESCAPE:
						seguir = FALSE;
						break;

					
					case 'D':
						// debug 
						_generar_bmps = TRUE;
						break;

					case 'F':
						// first person camara
						fpc = !fpc;
						break;

					case 'C':
						// modo camara:
						modo_camara = !modo_camara;
						paused = TRUE;
						break;

					case 'R':
						// Reset 
						modo_camara = FALSE;
						paused = FALSE;
						fpc = FALSE;
						break;

					case 'T':
						// Techo ruta 
						techo_ruta = !techo_ruta;
						break;

					case VK_F2:
						//player_one->vel = 2;
						player_one->explosion_barata = FALSE;
						player_one->PS.rastro = FALSE;
						// inicio el sistema de particulas
						player_one->PS.Create(this,player_one->Cycle_Pos,50,textura_explo);
						// inicio el sistema de particulas auxiliar
						player_one->PSaux.Create(this,player_one->Cycle_Pos,5,textura_explo+2,1);

						// inicio la explocion
						player_one->timer_rebotando = 2;
						player_one->timer_light_path = 4;

						break;

					case VK_F3:
						player_one->explosion_barata = FALSE;
						player_one->PS.rastro = TRUE;
						player_one->PS.Create(this,player_one->Cycle_Pos,100,
								textura_explo+3,1);

						// inicio la explocion
						player_one->timer_rebotando = 2;
						player_one->timer_light_path = 4;

						break;

					case VK_F4:
						// mini explosion barata
						player_one->explosion_barata = TRUE;
						player_one->PS.rastro = FALSE;
						player_one->PS.Create(this,player_one->Cycle_Pos,5,textura_explo+3,1);
						// inicio la explocion
						player_one->timer_rebotando = 2;
						player_one->timer_light_path = 4;
						break;

				}
			}
		}

	}
	ReleaseCapture();
}




void CGameEngine::RenderBolas()
{

	int pasos;
	double da,alfa;
	int i;

	
	pasos = 4;
	da = M_PI/pasos;
	alfa = -0.3;
	for(i=0;i<pasos;++i)
	{
		{
			// torres con forma de control de aeropuerto, que se meten un poco 
			// adentro del domo
			double cosa = cos(alfa);
			double sina = sin(alfa);
			double x = (radio)*cosa;
			double z = (radio)*sina;
			TVector3d size = TVector3d(50,25,20);
			TVector3d pos = TVector3d(x,5+size.y/2,z);
			Mesh[1].DXRender(pos,size,TVector3d(sina,0,-cosa));
		}

		{
			// torres con forma de edificio 
			double cosa = cos(alfa+0.25);
			double sina = sin(alfa+0.25);
			double x = (radio+5)*cosa;
			double z = (radio+5)*sina;
			TVector3d size = TVector3d(10,30*(1+fabs(cosa)*0.5),15*(1+fabs(sina)*0.3));
			TVector3d pos = TVector3d(x,size.y/2,z);
			Mesh[2].DXRender(pos,size,TVector3d(sina,0,-cosa));
		}

		alfa-=da;
	}



	// Recognizer
	/*
	{

		D3DXMATRIXA16 mat;
		TVector3d size = TVector3d(5,30,20);
		TVector3d pos = TVector3d(0,size.y/2,0);
		Mesh[5].CalcularMatriz(pos,size,TVector3d(0,0,1),&mat);
		g_pd3dDevice->SetTransform( D3DTS_WORLD, &mat);
		g_pd3dDevice->SetTexture( 0, NULL);
		Mesh[5].DXRender();
	}*/
	


	// tribunas
	pasos = 6;
	da = 2*M_PI/pasos*0.15;
	alfa = 0.2;
	for(i=0;i<pasos;++i)
	{
		double cosa = cos(alfa);
		double sina = sin(alfa);
		double x = (radio+5)*cosa;
		double z = (radio+5)*sina;
		TVector3d size = TVector3d(10,22.5,20);
		TVector3d pos = TVector3d(x,5+size.y/2,z);
		Mesh[3].DXRender(pos,size,TVector3d(sina,0,-cosa));
		alfa+=da;
	}

	// edificios
	pasos = 3;
	da = 1.2*M_PI/2/pasos;
	alfa = 1.5;
	for(i=0;i<pasos;++i)
	{
		double cosa = cos(alfa);
		double sina = sin(alfa);
		double x = (radio+20)*cosa;
		double z = (radio+20)*sina;
		TVector3d size = TVector3d(20,40,60);
		TVector3d pos = TVector3d(x,size.y/2,z);
		Mesh[4].DXRender(pos,size,TVector3d(sina,0,-cosa));
		alfa+=da;
	}


	// Restauro el matworld
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
}


void CGameEngine::RenderRecognizers(BOOL resplandor)
{

	return;

	D3DXMATRIXA16 mat;
	TVector3d size = TVector3d(5,30,20);
	TVector3d pos = TVector3d(0,size.y/2,0);
	Mesh[5].CalcularMatriz(pos,size,TVector3d(0,0,1),&mat);
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &mat);
	g_pd3dDevice->SetTexture( 0, NULL);
	
	if(resplandor)
	{

		// 1- borro el layer opaco, quiero que salga en negro y que borre 
		// todo lo que estaba de antes
		DWORD ant_zenable,ant_cullmode,ant_dmsource;
		g_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &ant_zenable);
		g_pd3dDevice->GetRenderState( D3DRS_CULLMODE, &ant_cullmode);
		g_pd3dDevice->GetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, &ant_dmsource);

		// z-enable, cull none
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
		g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
		// x defecto uso el color asociado al material
		g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
		// blend disable
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);

		
		// Material negro
		D3DMATERIAL9 mtrl_negro;
		ZeroMemory( &mtrl_negro, sizeof(mtrl_negro) );
		mtrl_negro.Diffuse.r = mtrl_negro.Ambient.r = 0;
		mtrl_negro.Diffuse.g = mtrl_negro.Ambient.g = 0;
		mtrl_negro.Diffuse.b = mtrl_negro.Ambient.b = 0;
		mtrl_negro.Diffuse.a = mtrl_negro.Ambient.a = 1;
		g_pd3dDevice->SetMaterial( &mtrl_negro);


		Mesh[5].g_pMesh->DrawSubset(0);		// layer opaco


		// resto de los layers
		for( int i=1; i<(int)Mesh[5].g_dwNumMaterials; i++ )
		{
			g_pd3dDevice->SetMaterial(&Mesh[5].g_pMeshMaterials[i] );
			Mesh[5].g_pMesh->DrawSubset( i );

		}


		// Resturo todo como estaba antes
		g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, ant_zenable);
		g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, ant_cullmode);
		g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, ant_dmsource);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
		g_pd3dDevice->SetMaterial( &mtrl_std);
		
	}
	else
	{
		// dibujo normal
		Mesh[5].DXRender();
	}



	// Restauro el matworld
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
}



void CGameEngine::RenderMirror()
{

	// Preparo la matriz de reflexion
	// ------------------------------
	TVector3d ant_LF = LF;
	D3DXMATRIX ant_matView = matView;
	D3DXPLANE espejo;
	espejo.a = 0;
	espejo.b = 1;
	espejo.c = 0;
	espejo.d = 0;
	D3DXMATRIX mReflectView;
	D3DXMatrixReflect(&mReflectView,&espejo);
	D3DXMatrixMultiply(&matView, &mReflectView, &matView);
	// actualizo el pipeline
	// solo cambio la matriz de view
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
	// inicializo un clipping plane del lado del espejo que esta el punto de vista 
	// ojo!!!: When the fixed function pipeline is used the plane equations are assumed 
	// to be in world space. 
	//float plano[] = {0,1,0,0};
	//g_pd3dDevice->SetClipPlane(0,plano);
	//	g_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,TRUE);
	
	// dibujo lo que se puede reflejar
	RenderScene(TRUE,TRUE);
	//g_pd3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE,FALSE);
	
	
	// Resturo las matrices
	LF = ant_LF;
	matView = ant_matView;
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );


	// Ahora dibujo el piso, con la textura transparente, y sin zbuffer
	// la textura tiene solo un contorno tipo perimetro de baldosa, y sirve
	// para que reemplace el reflejo por dicho contorno, que es donde no se refleja nada
	// es mucho mas barato que definir un stencil buffer, con esa forma. 
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
	g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->SetTexture( 0, g_pTexture[textura_piso]);
	g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLEFAN,0,2);
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);

}


double CGameEngine::que_altura(double x,double z,char *tipo_celda,int *tramo)
{
	// Verifico si el pto x,z esta cerca de la ruta
	// busco todos los puntos de la ruta cercanos a Pos
	double mdist = ancho_ruta/2 + ancho_guarray;
	aux_tramo = -1;
	aux_en_ruta = FALSE;
	if(tipo_celda)
		*tipo_celda = 0;	// piso
	int cant_p = 0;
	int ndx[700];
	for(int i=0; i<cant_ptos_ruta-1;++i)
		if(fabs(x-pt_ruta[i].x)<mdist+5 && fabs(z-pt_ruta[i].z)<mdist+5)
			// es un pto posible
			ndx[cant_p++] = i;
	
	if(!cant_p)
		return 0;			// nivel del suelo

	double dr = ancho_ruta/2;
	double H = 0;
	TVector2d p = TVector2d(x,z);
	for(int t = 0;t<cant_p;++t)
	{
		i = ndx[t];

		TVector2d r0 = pt_ruta[i].pxz();
		TVector2d r1 = pt_ruta[i+1].pxz();
		TVector2d r = r1-r0;
		double rm = r.mod();
		r.normalizar();
		double d = (p-r0)>>r;
		// d ==0 , rm
		
		if(d>=-0.5 && d<=rm+0.5)
		{
			TVector2d rc = r0 + r*d;
			double dist = (rc-p).mod();
			if(dist<mdist)
			{
				aux_tramo = i;
				mdist = dist;
				// interpolo la altura de la ruta
				double k = d/rm;
				if(k<0)
					k = 0;
				else
				if(k>1)
					k = 1;
				double Hruta = pt_ruta[i].y*(1-k) + pt_ruta[i+1].y*k;
				if(dist<=dr)
				{
					// esta en la ruta
					aux_en_ruta = TRUE;
					H = Hruta;
					if(tipo_celda)
						*tipo_celda = 1;		// ruta
				}
				else
				{
					// esta en el guarray
					aux_en_ruta = FALSE;
					H = (1-(dist-dr)/ancho_guarray)*Hruta;
					if(tipo_celda)
						*tipo_celda = 2;		// guarray
				}
			}
		}
	}

	if(tramo!=NULL)
		*tramo = aux_tramo;
	return H;
}


// devuelve la altura de la ruta o guarray en el tramo i
double CGameEngine::que_altura(int i,TVector2d p)
{
	double dr = ancho_ruta/2;
	TVector2d r0 = pt_ruta[i].pxz();
	TVector2d r1 = pt_ruta[i+1].pxz();
	TVector2d r = r1-r0;
	double rm = r.mod();
	r.normalizar();
	double d = (p-r0)>>r;
	// d ==0 , rm
	TVector2d rc = r0 + r*d;
	double dist = (rc-p).mod();
	// interpolo la altura de la ruta
	double k = d/rm;
	if(k<0)
		k = 0;
	else
	if(k>1)
		k = 1;
	double H = pt_ruta[i].y*(1-k) + pt_ruta[i+1].y*k;
	if(dist>=dr)
		// esta en el guarray
		H = (1-(dist-dr)/ancho_guarray)*H;

	return H;
}




int CGameEngine::entra_en_ruta(TVector3d Pos,TVector3d Dir)
{	
	int rta = -1;
	int i = 0;	
	while(i<cant_ptos_ruta-1 && rta==-1)
	{
		if( Pos.y>=pt_ruta[i].y && Pos.y<=pt_ruta[i+1].y+1 && 
				(Pos-pt_ruta[i]).mod()<ancho_ruta/2+2)
		{
			// Verifico el sentido (no puede entrar a contramano a la ruta)
			TVector3d dir_ruta = pt_ruta[i+1] - pt_ruta[i];
			dir_ruta.normalizar();
			//if(dir_ruta>>Dir>0.3)
				rta = i;
		}
		++i;
	}
	return rta;
}


// la moto se quiere mover desde - hasta
BOOL CGameEngine::choca_ligthpath(TVector3d desde,TVector3d hasta,
				int *path_choque,TVector2d *Ip,int player)
{
	// verifico si choca contra el ligth path
	
	// la recta actual va desde p0 a p1
	TVector2d p0 = TVector2d(desde.x,desde.z);
	TVector2d p1 = TVector2d(hasta.x,hasta.z);
	TVector2d pos_moto = P[player].Pos.pxz();		// pos. de la moto en planta


	// celda actual
	float fi0 = cant_cell/2 + p0.x / dgrid;
	float fj0 = cant_cell/2 + p0.y / dgrid;
	// celda a la que quiero ir
	float fi1 = cant_cell/2 + p1.x / dgrid;
	float fj1 = cant_cell/2 + p1.y / dgrid;

	// normalizo y redondeo con "prudencia"
	int i0,i1,j0,j1;
	if(fi0<fi1)
	{
		i0 = fi0-0.5;
		i1 = fi1+0.5;
	}
	else
	{
		i0 = fi1-0.5;
		i1 = fi0+0.5;
	}
	
	if(fj0<fj1)
	{
		j0 = fj0-0.5;
		j1 = fj1+0.5;
	}
	else
	{
		j0 = fj1-0.5;
		j1 = fj0+0.5;
	}
	
	double Hligthpath = 0.7;

	BOOL rta = FALSE;
	for(int pos_i=i0;pos_i<=i1 && !rta;++pos_i)
	for(int pos_j=j0;pos_j<=j1 && !rta;++pos_j)
	{
		if(pos_i>=0 && pos_i<cant_cell && pos_j>=0 && pos_j<cant_cell)
		{
			// recorro el toolpath solo de esta celda
			int i =0;
			while(i<celdas[pos_i][pos_j].cant_tramos && !rta)
			{
				int t = celdas[pos_i][pos_j].p[i];
				int n = celdas[pos_i][pos_j].player[i];
				int ult_pto = P[n].cant_ptos;
				if(n==player)
					ult_pto -=2;
				TVector2d q1 = TVector2d(P[n].path[t].x,P[n].path[t].z);
				if(t<ult_pto)
				{

					if(t<ult_pto-1)
					{
						TVector2d q0 = TVector2d(P[n].path[t+1].x,P[n].path[t+1].z);
						// verifico la interseccion entre p0-p1, y q0-q1
						double s = interseccion_2segmentos(p0,p1,q0,q1);
						if(s!=-1)
						{
							double Hpath = P[n].path[t+1].y + (P[n].path[t].y-P[n].path[t+1].y)*s;
							double Hmoto = desde.y + (hasta.y-desde.y)*s;
							// verifico la altura 
							if(fabs(Hpath-Hmoto)<Hligthpath)
							{
								// quiero evitar que choque con el lightpath que apenas
								// sale de la moto. El pto de interseccion tiene 
								// que estar al menos un metro alejado de la moto,
								// si no es como chocar consigo mismo. 
								TVector2d ip = p0+(p1-p0)*s;
								//if(n!=player || (ip-pos_moto).mod()>5)
								{

									*Ip = ip;			// guado el pto de colision
									*path_choque = t;	// choco en path[path_choque]
									rta = TRUE;			// se hizo mierda contra el ligthpath
								}
							}
						}
					}

					if(!rta && t>0)
					{
						TVector2d q2 = TVector2d(P[n].path[t-1].x,P[n].path[t-1].z);
						// verifico la interseccion entre p0-p1, y q1-q2
						double s = interseccion_2segmentos(p0,p1,q1,q2);
						if(s!=-1)
						{
							double Hpath = P[n].path[t].y + (P[n].path[t-1].y-P[n].path[t].y)*s;
							double Hmoto = desde.y + (hasta.y-desde.y)*s;
							// verifico la altura
							if(fabs(Hpath-Hmoto)<Hligthpath)
							{
								TVector2d ip = p0+(p1-p0)*s;
								//if(n!=player || (ip-pos_moto).mod()>5)
								{
									*Ip = ip;			// guado el pto de colision
									*path_choque = t-1;	// choco en path[path_choque]
									rta = TRUE;			// se hizo mierda contra el ligthpath
								}
							}
						}
					}
				}

				if(!rta)
					++i;		// sigo buscando
			}
		}
	}


	// TEST :
	/*
	if(!rta)
	{
		for(int n=0;n<cant_players;++n)
		for(int t =0;t<P[n].cant_ptos-1 && !rta;++t)
		{
			TVector2d q0 = TVector2d(P[n].path[t+1].x,P[n].path[t+1].z);
			TVector2d q1 = TVector2d(P[n].path[t].x,P[n].path[t].z);
			// verifico la interseccion entre p0-p1, y q0-q1
			double s = interseccion_2segmentos(p0,p1,q0,q1);
			if(s!=-1)
			{
				double Hpath = P[n].path[t+1].y + (P[n].path[t].y-P[n].path[t+1].y)*s;
				double Hmoto = desde.y + (hasta.y-desde.y)*s;
				// verifico la altura
				if(fabs(Hpath-Hmoto)<Hligthpath)
				{

					TVector2d ip = p0+(p1-p0)*s;
					if(n!=player || (ip-pos_moto).mod()>1)
					{

						*Ip = ip;			// guado el pto de colision
						*path_choque = t;	// choco en path[path_choque]

						int ri = round(cant_cell/2 + Ip->x / dgrid);
						int rj = round(cant_cell/2 + Ip->y / dgrid);


						rta = TRUE;			// se hizo mierda contra el ligthpath
					}

				}
			}
		}
	}
	*/
	return rta;
}


// devuelve true si choca contra el light path o contra la moto
BOOL CGameEngine::choca(TVector3d desde,TVector3d hasta,int player,TVector2d *Ip,BOOL prediccion)
{


	TVector3d Dir = hasta-desde;
	Dir.normalizar();

	// verifico si choca contra el limite del domo, pero para eso me muevo un poco
	// mas en la direccion Dir
	float R = radio;
	if(tipo_ruta!=-1)
		R -= 15;		// la ruta perimetral achica al domo
	if((hasta + Dir*10).mod()>=R)
		return 2;			// va a chocar contra la pared del domo 

	// verifico si choca contra la moto de los demas jugadores
	BOOL choca_moto = FALSE;
	// la recta actual va desde p0 a p1
	TVector2d p0 = TVector2d(desde.x,desde.z);			// pos trasera de la moto
	TVector2d p1 = TVector2d(hasta.x,hasta.z);			// pos a la cual quiero ir usualmente 10 o 5 metros hacia adelante
	TVector2d pa = p0 + Dir.pxz()*player::size_cycle.z;	// pos delantera de la moto
	int i = 0;
	while(i<cant_players && !choca_moto)
	{
		if(player!=i)
		{
			// falta comtemplar que coincida la altura! 
			TVector2d q0 = P[i].Pos.pxz();
			double dz;
			if(prediccion)
			{
				// si tiene puesto el flag de prediccion, es porque quiero ver que pasaria 
				// si la moto sigue avanzando un cierto tiempo mas, dpt
				// usualmente se llama con esta opcion en el contexto de IA()
				double dpt = 0.2;		// de momento 10 ms de prediccion 
				// como la moto se sigue moviendo en la misma velocidad, 
				// es como si se agrandara en dicha direccion,
				// lo simulo con el tamaño de la moto: 
				dz = player::size_cycle.z + P[i].vel*dpt;
			}
			else
				dz = player::size_cycle.z;


			TVector2d q1 = (P[i].Pos + P[i].Dir * dz).pxz();
			// verifico la interseccion entre p0-p1, y q0-q1
			double s = interseccion_2segmentos(p0,p1,q0,q1);
			if(s!=-1)
				choca_moto = TRUE;
			else
			// verifico 3 ptos de la moto con 3 ptos del recorrido
			if((p0-q0).mod()<player::size_cycle.z/2)
				choca_moto = TRUE;
			else
			if((p0-q1).mod()<player::size_cycle.z/2)
				choca_moto = TRUE;
			else
			if((pa-q0).mod()<player::size_cycle.z/2)
				choca_moto = TRUE;
			else
			if((pa-q1).mod()<player::size_cycle.z/2)
				choca_moto = TRUE;


		}

		++i;
	}

	if(choca_moto)
		return TRUE;


	// verifico si choca contra el ligth path
	int path_choque;
	return choca_ligthpath(desde,hasta,&path_choque,Ip,player);
}
		



void CGameEngine::RenderFocos()
{
	// focos de luz

	// Alloco memoria y creo un buffer para todos los vertices
	size_t size = sizeof(CUSTOMVERTEX)*((4+10)*2*cant_focos);
	CUSTOMVERTEX* pVertices;
	if( FAILED( g_pVBFocos->Lock( 0, size, (void**)&pVertices, 0 ) ) )
		return ;

	TVector2d ViewDir = (LA-LF).pxz();
	ViewDir.normalizar();
	double w0 = 2 + 3*ViewDir.angulo2();

	D3DCOLOR color_luz = D3DCOLOR_XRGB(255,255,255);//D3DCOLOR_XRGB(141,232,240);
	double da = 2*M_PI/cant_focos*0.16;
	double alfa = 0.13;
	double dt = 0.02;
	double r = radio+5;
	double hf = 2.5;
	int t = 0;
	for(int i=0;i<cant_focos;++i)
	{
		// ------------------------------------------------
		// focos de abajo
		double H = 12;

		TVector3d pos_foco = TVector3d(r*cos(alfa),H,r*sin(alfa));
		TVector3d N = TVector3d(-cos(alfa),0,-sin(alfa));
		TVector3d Tg = TVector3d(sin(alfa),0,-cos(alfa));
		TVector3d Up = N*Tg;

		TVector3d d1 = Up;
		d1.rotar(TVector3d(0,0,0),N,w0);
		TVector3d d2 = d1*N;


		TVector3d p0 = pos_foco + d1*(hf*1.5);
		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;
		pVertices[t].color = color_luz;
		pVertices[t].N.x = N.x;
		pVertices[t].N.y = N.y;
		pVertices[t].N.z = N.z;
		pVertices[t].tv = 0;
		pVertices[t].tu = 0;

		++t;
		p0 = pos_foco - d1*(hf*1.5);
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;

		++t;
		p0 = pos_foco + d2*hf;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;


		++t;
		p0 = pos_foco - d2*hf;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;

		++t;

		// ------------------------------------------------
		// focos de arriba
		H = 27;
		pos_foco = TVector3d(r*cos(alfa),H,r*sin(alfa));
		
		p0 = pos_foco + d1*hf;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;

		++t;
		p0 = pos_foco - d1*hf;
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;

		++t;
		p0 = pos_foco + d2*(hf*1.5);
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;


		++t;
		p0 = pos_foco - d2*(hf*1.5);
		pVertices[t] = pVertices[t-1];
		pVertices[t].x = p0.x;
		pVertices[t].y = p0.y;
		pVertices[t].z = p0.z;

		++t;
	
		alfa+=da;

	}
	g_pVBFocos->Unlock();

	g_pd3dDevice->SetStreamSource( 0, g_pVBFocos, 0, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->DrawPrimitive( D3DPT_LINELIST,0,4*cant_focos);
	g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );

}




// dibuja el plano ajustando la escala al viewport 
HRESULT CGameEngine::DXdibujar_planoVP(char plano,BOOL clear)
{
	// matriz de Workd = matrix identidad
	D3DXMatrixIdentity( &matWorld);
	// matrix de proyeccion = identidad 
	D3DXMatrixIdentity( &matProj);
	double ds = 2*radio;
	double offset = ds;
	// aspect ratio real: 
	double k = (double)d3dpp.BackBufferWidth/(double)d3dpp.BackBufferHeight*1;


	switch(plano)
	{
		default:
		case 0:
		// DX_PLANO_XY:
			matView = D3DXMATRIX(	2/ds,	0,		0,		0,
									0,		-2/ds,	0,		0,
									0,		0,		0,		0,
									-1+offset/ds,		1-offset/ds,		0,		1	);
			break;
		case 1:
			// DX_PLANO_YZ:
			matView = D3DXMATRIX(	2/ds,	0,		0,		0,
									0,		0,		0,		0,
									0,		2/ds,	0,		0,
									-1+offset/ds,		-1+offset/ds,		0,		1	);
			break;
		case 2:
			// DX_PLANO_XZ:
			matView = D3DXMATRIX(	0,		0,		0,		0,
									2/ds,	0,		0,		0,
									0,		2/ds,	0,		0,
									-1+offset/ds,		-1+offset/ds,		0,		1	);
			break;
	}		

	// con las matrices generados, seteo el pipeline de transformaciones del DX
    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );


	if(clear)	 // Clear ?
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB(255,0,0), 1, 0 );

	// dibujo la pared circular
	g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE);
	g_pd3dDevice->SetTexture( 0, NULL);
	g_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP,4,64*2);

	for(int i=0;i<cant_players;++i)
	{
		g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
		g_pd3dDevice->SetRenderState(D3DRS_TEXTUREFACTOR, P[i].c_lightpath);

		g_pd3dDevice->SetStreamSource( 0, P[i].g_pVBPath, 0, sizeof(CUSTOMVERTEX) );
		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		g_pd3dDevice->DrawPrimitive( D3DPT_LINESTRIP,0,3*(P[i].cant_ptos-1)*2);
	}
	g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );


    return S_OK;
}



void CGameEngine::scoreboard()
{

	D3DCOLOR color = D3DCOLOR_XRGB(255,255,255);

	g_pd3dDevice->BeginScene();
	char buffer[255];
	sprintf(buffer,"Fps = %5.1f Cant.Paquetes =%d ",fps,player_one->sound.cant_bloques);
	//sprintf(buffer,"Fps = %5.1f Path Lenght = %5d  LA= (%5.1f,%5.1f,%5.1f) LF= (%5.1f,%5.1f,%5.1f) Y = %5.1f  Frame# %d",
	//		fps,player_one->cant_ptos,LA.x,LA.y,LA.z,LF.x,LF.y,LF.z,player_one->Pos.y,nro_frame++);
	//sprintf(buffer,"Fps = %5.1f Path Lenght = %5d  tu= %5.1f tv =%5.1f pos_ruta=%d",
		//fps,cant_ptos,aux_tu,aux_tv,pos_en_ruta);
	//TVector3d pos_delantera = Pos+Cycle_Dir*size_cycle.z;
	//sprintf(buffer,"i=%d, j=%d",pos_i,pos_j);
	//sprintf(buffer,"Y = %5.1f ",player_one->Pos.y);
	//sprintf(buffer,"Pos= (%5.1f,%5.1f) H = %5.1f Tramo=%2d ruta=%2d",
	//	player_one->Pos.x,player_one->Pos.z,que_altura(player_one->Pos.x,player_one->Pos.z),
	//	aux_tramo,aux_en_ruta);
	//sprintf(buffer,"Dir Y = %5.1f  Dir Y= %5.1f ",player_one->Cycle_Dir.y,player_one->Dir.y);
	//sprintf(buffer,"Angulo lateral %d %s",(int)(player_one->an_lat * 180/M_PI),
		//player_one->derrapando?"derrapando":"");
	//sprintf(buffer,"# %d",nro_frame++);
	//if(es_servidor)
		//sprintf(buffer,"SERVIDOR time = %10.2f   tiempo remoto =%10.2f",time,remote_time);
	//else
	//	sprintf(buffer,"remoto (time=%10.2f)  Vidas Servidor=%d",time,P[1].cant_vidas);


	CRect rc(5,5,0,0);
	pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	g_pFont->DrawText( pSprite, buffer, -1, &rc, DT_NOCLIP, color);

	// debug msg
	if(cant_msg && debug_ia)
	{
		int pos_y = 5;
		char buffer[255];
		for(int t=primer_msg;t<cant_msg;++t)
		{
			CRect rc(600,pos_y,0,0);
			sprintf(buffer,"%3d :%s",t,debug_msg[t%MAX_MSG_DBG]);
			g_pFont->DrawText( pSprite, buffer,-1, &rc, DT_NOCLIP, color);
			pos_y+=15;
		}
	}
	

	if(hay_radio && midi_actual>=0)
	{
		CRect rc(5,520,0,0);
		g_pFontb->DrawText( pSprite, midi_files[midi_actual], -1, &rc, DT_NOCLIP,color);
	}

	ppLine->SetAntialias(TRUE);
	ppLine->SetWidth(1);

	D3DXVECTOR2 pt[40];
	D3DXVECTOR2 pt2[40];
	float r = 75;
	float an = 0;
	int cant = 32;
	int x0 = 20+r;
	int y0 = d3dpp.BackBufferHeight/2;

	for( int i = 0; i < cant; i++)
	{
		an = (2.0f * D3DX_PI) * ((float)i / (float)cant); 
		pt[i].x = x0+r*cos(an);
		pt[i].y = y0+r*sin(an);
		pt2[i].x = x0+(r-1)*cos(an);
		pt2[i].y = y0+(r-1)*sin(an);
	}

	pt[cant] = pt[0];
	pt2[cant] = pt2[0];
	ppLine->Begin();
	ppLine->Draw(pt,cant+1,D3DCOLOR_XRGB(255,255,255));
	ppLine->Draw(pt2,cant+1,D3DCOLOR_XRGB(128,128,255));


	if(modo_juego==GAME_GRID_SURVIVAL && timer_warming<=0)
	{
		sprintf(buffer, "Tiempo %10.2f s",time-survival_time);
		CRect rc(350,5,0,0);
		g_pFontb->DrawText( pSprite, buffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 255, 128,128, 255));
	}


	D3DVIEWPORT9 viewport;
	g_pd3dDevice->GetViewport(&viewport);
	float E = r/radio;
	for(int j=0;j<cant_players;++j)
	{
		if(P[j].cant_vidas)
		{
			TVector2d p0 = TVector2d(P[j].Pos.x,P[j].Pos.z);
			TVector2d dir = TVector2d(P[j].Dir.x,P[j].Dir.z);
			dir.normalizar();
			TVector2d n = dir.normal();
			TVector2d a = p0 + dir*10;
			TVector2d b = p0 - n*3;
			TVector2d c = p0 + n*3;
			pt[0].x = x0 + a.y*E;
			pt[0].y = y0 + a.x*E;
			pt[1].x = x0 + b.y*E;
			pt[1].y = y0 + b.x*E;
			pt[2].x = x0 + c.y*E;
			pt[2].y = y0 + c.x*E;
			pt[3] = pt[0];
			ppLine->Draw(pt,4,P[j].c_lightpath);

			if(j && timer_warming<=0)
			{
				TVector3d Pos = P[j].Pos+P[j].Dir*(P[j].size_cycle.z*0.5) + TVector3d(0,1.25,0);
				D3DXVECTOR3 pV(Pos.x,Pos.y,Pos.z);
				D3DXVECTOR3 pm;
				D3DXVec3Project(&pm,&pV,&viewport,&matProj,&matView,&matWorld);
				if(pm.z>=0 && pm.z<=1)
				{
					sprintf(buffer, "%d",j+1);
					if(multiplayer)
					{
						// le muestro el nombre del jugador
						strcat(buffer,"-");
						strcat(buffer,P[j].nombre);
						rtrim(buffer);
					}

					CRect rc(pm.x+10,pm.y-3,0,0);
					g_pFont->DrawText( pSprite, buffer, -1, &rc, DT_NOCLIP, D3DXCOLOR( 255, 128,128, 255));
					D3DXVECTOR3 pos(pm.x,pm.y,0);
					D3DXVECTOR3 center(8,8,0);
					pSprite->Draw(g_pTexture[textura_gui+5],NULL,&center,&pos,0xFFFFFFFF);

				}
			}
		}

		// dibujo el lightpath
		// calculo y actualizo los ptos faltnates
		for(int t=ligthpath_cant[j];t<P[j].cant_ptos;++t)
		{
			lightpath[j][t].x = x0 + P[j].path[t].z*E;
			lightpath[j][t].y = y0 + P[j].path[t].x*E;
		}
		// el ultimo punto es la posicion de la moto
		lightpath[j][t].x = x0 + P[j].Pos.z*E;
		lightpath[j][t].y = y0 + P[j].Pos.x*E;

		ppLine->Draw(lightpath[j],t+1,D3DCOLOR_XRGB(128,128,128));
		ligthpath_cant[j] = t;		// no vuelvo a calcular estos puntos

	}
	ppLine->End();
	pSprite->End();

	// maximo 4 jugadores informa el scoreboard
	int cant_sb = 	min(4,cant_players);
	for(i=0;i<cant_sb;++i)
		brd_player(i);


	// timers del scoreboard
	if(timer_game_over>0)
	{
		// --- Game Over------
		pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		D3DXMATRIX mat,matAnt;
		pSprite->GetTransform(&matAnt);
		D3DCOLOR color = D3DCOLOR_ARGB(240,255,255,255);

		double K = 3;
		int aux_y = 100;
		char buffer[256];
		strcpy(buffer,"Game Over");
		BOOL ganaste;
		if(modo_juego==GAME_GRID_BATTLE)
			ganaste = player_one->score>P[1].score;
		else
			ganaste = player_one->cant_vidas?TRUE:FALSE;

		strcat(buffer, ganaste?" - Ganaste":" - Perdiste");
		CRect rc2(0,0,0,0);
		g_pFontb->DrawText( pSprite, buffer, -1, &rc2, DT_NOCLIP|DT_CALCRECT, D3DXCOLOR( 255, 128,128, 1));

		D3DXVECTOR2 escale = D3DXVECTOR2(K,K);
		D3DXVECTOR2 tras = D3DXVECTOR2(	d3dpp.BackBufferWidth/2-rc2.right*K*0.5,d3dpp.BackBufferHeight/2-rc2.bottom*K*0.5);
		D3DXMatrixTransformation2D(&mat,NULL,0.0,&escale,NULL,NULL,&tras);
		pSprite->SetTransform(&mat);
		CRect rc(0,0,0,0);
		g_pFontb->DrawText( pSprite, buffer, -1, &rc, DT_NOCLIP, color);
		// dejo la matrix como estaba
		pSprite->SetTransform(&matAnt);
		pSprite->End();
	}
	else
	if(timer_warming>0)
	{
		pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		D3DXMATRIX mat,matAnt;
		pSprite->GetTransform(&matAnt);
		D3DCOLOR color = D3DCOLOR_ARGB(255,255,255,255);

		double K = 1.5;
		int aux_y = 100;
		char buffer[256];
		if(timer_warming>5 && nro_player_perdio!=-1 && modo_juego==GAME_GRID_BATTLE)
		{
			sprintf(buffer,"Pierde %s",P[nro_player_perdio].nombre);
		}
		else
		if(timer_warming>3.5 && modo_juego==GAME_GRID_BATTLE)
			sprintf(buffer,"Ronda %d",ronda+1);
		else
		if(timer_warming>0.5)
		{
			sprintf(buffer,"%d",(int)(timer_warming+0.5));
			K = 8;
		}
		else
			strcpy(buffer,"Listo!");

		CRect rc2(0,0,0,0);
		g_pFontb->DrawText( pSprite, buffer, -1, &rc2, DT_NOCLIP|DT_CALCRECT, color);

		D3DXVECTOR2 escale = D3DXVECTOR2(K,K);
		D3DXVECTOR2 tras = D3DXVECTOR2(	d3dpp.BackBufferWidth/2-rc2.right*K*0.5 ,
										d3dpp.BackBufferHeight/2-rc2.bottom*K*0.5);
		if(timer_warming>3.5)
		{
			tras.y= aux_y;
//			color = D3DCOLOR_ARGB(255,255,128,128);
		}


		D3DXMatrixTransformation2D(&mat,NULL,0.0,&escale,NULL,NULL,&tras);
		pSprite->SetTransform(&mat);

		CRect rc(0,0,0,0);
		g_pFontb->DrawText( pSprite, buffer, -1, &rc, DT_NOCLIP, color);
		// dejo la matrix como estaba
		pSprite->SetTransform(&matAnt);
		pSprite->End();

	}
	else
	if(lagging)
	{
		// --- laggin up ------
		pSprite->Begin(D3DXSPRITE_ALPHABLEND);
		D3DXMATRIX mat,matAnt;
		pSprite->GetTransform(&matAnt);
		D3DCOLOR color = D3DCOLOR_ARGB(240,255,255,255);

		double K = 3;
		int aux_y = 100;
		char buffer[256];
		strcpy(buffer,"LAGGING ...");
		CRect rc2(0,0,0,0);
		g_pFontb->DrawText( pSprite, buffer, -1, &rc2, DT_NOCLIP|DT_CALCRECT, D3DXCOLOR( 255, 128,128, 1));

		D3DXVECTOR2 escale = D3DXVECTOR2(K,K);
		D3DXVECTOR2 tras = D3DXVECTOR2(	d3dpp.BackBufferWidth/2-rc2.right*K*0.5 ,
										d3dpp.BackBufferHeight/2-rc2.bottom*K*0.5);
		
		D3DXMatrixTransformation2D(&mat,NULL,0.0,&escale,NULL,NULL,&tras);
		pSprite->SetTransform(&mat);
		g_pFontb->DrawText( pSprite, buffer, -1, &rc, DT_NOCLIP, color);
		// dejo la matrix como estaba
		pSprite->SetTransform(&matAnt);
		pSprite->End();

	}




	g_pd3dDevice->EndScene();

}




void CGameEngine::brd_player(int player)
{
	int cant_vidas = modo_juego==GAME_GRID_SURVIVAL?P[player].cant_vidas:P[player].score;

	pSprite->Begin(D3DXSPRITE_ALPHABLEND);
	D3DXVECTOR3 center = D3DXVECTOR3(0,0,0);
	D3DXVECTOR3 pos,ant_pos,pos_cara;
	D3DXVECTOR2 escale;

	switch(player)
	{
		case 0:
			// jugador principal arriba a la izquierda
			pos = D3DXVECTOR3(-170,80,0);
			escale = D3DXVECTOR2(-1,1);
			break;
			// el resto de los jugadores (enemigos)
		case 1:
		default:
			pos = D3DXVECTOR3(d3dpp.BackBufferWidth-170,d3dpp.BackBufferHeight-120,0);
			escale = D3DXVECTOR2(1,1);
			break;
		case 2:
			pos = D3DXVECTOR3(d3dpp.BackBufferWidth-170,-144,0);
			escale = D3DXVECTOR2(1,-1);
			break;
		case 3:
			pos = D3DXVECTOR3(-170,-((int)d3dpp.BackBufferHeight-120+70),0);
			escale = D3DXVECTOR2(-1,-1);
			break;

	}
	ant_pos = pos;
	D3DXMATRIX mat;
	D3DXMATRIX matIdent;
	D3DXMatrixIdentity(&matIdent);
	D3DXMatrixTransformation2D(&mat,NULL,0.0,&escale,NULL,NULL,NULL);
	pSprite->SetTransform(&mat);
	
	pSprite->Draw(g_pTexture[textura_gui],NULL,&center,&pos,0xFFFFFFFF);
	// cara del jugador
	pSprite->SetTransform(&matIdent);
	pos_cara = pos;
	pos_cara.x += 103;
	pos_cara.y += 12;
	if(escale.x<0)
	{
		pos_cara.x*=escale.x;
		pos_cara.x-=52;
	}
	if(escale.y<0)
	{
		pos_cara.y*=escale.y;
		pos_cara.y-=43;
	}

	int nro_textura;
	if(!player)
		nro_textura = textura_face + player_one->nro_avatar;
	else
		// los enemigos tienen todos la misma cara y el jugador puede elegir entre 4 avatares
		// si esta en mutiplayer, usa el avatar remoto: 
		nro_textura = multiplayer?textura_face + P[player].nro_avatar:textura_face_enemigo;		
	pSprite->Draw(g_pTexture[nro_textura],NULL,&center,&pos_cara,0xFFFFFFFF);

	// vidas
	CRect rc = CRect((pos.x+78)*escale.x,(pos.y+3)*escale.y,(pos.x+94)*escale.x,(pos.y+27)*escale.y);
	char buffer[255];
	sprintf(buffer,"%d",cant_vidas);
	g_pFontb->DrawText( pSprite, buffer, -1, &rc, DT_NOCLIP|DT_CENTER|DT_VCENTER|DT_SINGLELINE,D3DXCOLOR( 128, 128,240, 1));
	// un poco mas abajo el nombre del jugador
	rc.top+=60;
	rc.bottom+=60;
	rc.left-=80;
	g_pFontb->DrawText( pSprite, P[player].nombre , -1, &rc, DT_NOCLIP, D3DCOLOR_XRGB(255,255,255));


	pos.x += 13;
	pos.y += 36;
	pSprite->SetTransform(&mat);

	if(escale.x>0)
	{
		// triangulo izquierdo
		pSprite->Draw(g_pTexture[cant_vidas<3?textura_gui+4:textura_gui+2],NULL,&center,&pos,0xFFFFFFFF);

		if(cant_vidas==7)
		{
			// triangulo derecho
			pos.x += 68;
			pSprite->Draw(g_pTexture[textura_gui+3],NULL,&center,&pos,0xFFFFFFFF);
		}
	}
	else
	{
		// triangulo izquierdo
		if(cant_vidas==7)
			pSprite->Draw(g_pTexture[cant_vidas<3?textura_gui+4:textura_gui+2],NULL,&center,&pos,0xFFFFFFFF);

		// triangulo derecho
		pos.x += 68;
		pSprite->Draw(g_pTexture[textura_gui+3],NULL,&center,&pos,0xFFFFFFFF);
	}


	pSprite->SetTransform(&matIdent);
	pSprite->End();

	pos = ant_pos;
	pos.x+=20;
	pos.y+=42;
	double ptje_lifes = cant_vidas / 7.0;

	D3DXVECTOR2 pt[2];
	if(escale.x>0)
	{
		pt[0]  = D3DXVECTOR2(pos.x,pos.y*escale.y);
		pt[1]  = D3DXVECTOR2(pos.x+70*ptje_lifes,pos.y*escale.y);
	}
	else
	{
		pt[0]  = D3DXVECTOR2(-pos.x-70,pos.y*escale.y);
		pt[1]  = D3DXVECTOR2(-pos.x-70*(1-ptje_lifes),pos.y*escale.y);
	}

	ppLine->SetWidth(9);
	ppLine->SetAntialias(FALSE);
	ppLine->Begin();
	ppLine->Draw(pt,2,cant_vidas<3?D3DCOLOR_XRGB(255,0,0):D3DCOLOR_XRGB(0,192,0));
	ppLine->End();



}



void CGameEngine::GUI_loop()
{

	// el modo gui se trabaja con el input de windows standard
	// asi que libero el direct input
	mouse.FreeDirectInput();
	// ----------------------
//	SetCursorPos(d3dpp.BackBufferWidth/2,d3dpp.BackBufferHeight/2);

	BOOL seguir = TRUE;
	int nFlags = 0;
	int cant_frames = 0;
	time = 0;				// tiempo total
	double ftime = 0;		// frame time
	elapsed_time = 0;		// tiempo parcial (laps) 
	double timer_closing = 0;

	LARGE_INTEGER F,T0,T1;   // address of current frequency
	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);
	TVector3d LA_ant = LA;
	pr_LA = TVector3d(0,0,0);
	pr_LF = TVector3d(radio*1.5,50,radio*1.5);
	while(seguir)
	{
		// Avanzo el tiempo
		QueryPerformanceCounter(&T1);
		elapsed_time = (double)(T1.LowPart - T0.LowPart) / (double)F.LowPart;
		T0 = T1;
		time+=elapsed_time;
		// animacion del escenario modo gui

		switch(animacion_gui)
		{
			case GUI_GAME_PAUSED:
				break;

			case GUI_ESCENARIO:
			default:
				LF = TVector3d(-radio*0.5,10,-radio*0.5);
				LA = TVector3d(0,5,0);
				LF.rotar_xz(LA,time*0.05);
				break;

			case GUI_PREVIEW_MOTO:
				{
					pr_LF = TVector3d(-radio*0.5,10,-radio*0.5);
					pr_LA = TVector3d(0,5,0);
					pr_LF.rotar_xz(pr_LA,time*0.5);
				}
				break;

			case GUI_PRESENTACION:
				{
					double H = 100-time*10;
					if(H>15)
					{
						LF = TVector3d(-radio*0.5,H,-radio*0.5);
						LA = TVector3d(0,5,0);
					}
					else
					{
						LF = TVector3d(0,15,0);
						LA = TVector3d(radio*sin(time*0.25),8,radio*cos(time*0.25));
					}
				}
				break;

			case GUI_PREVIEW_RUTA:
				{
					double an = M_PI*elapsed_time*0.2;
					pr_LF.rotar_xz(pr_LA,-an);
				}
				break;
				
			case GUI_RECORRER_RUTA:
				{

					double vel_preview = 5;
					pr_pos += vel_preview*elapsed_time;
					if(pr_pos>=cant_ptos_ruta)
						pr_pos = 0;
					int i = pr_pos;
					double resto = pr_pos-i;

					int i0 = i?i-1:cant_ptos_ruta-1;
					pr_LA = pt_ruta[i]*(1-resto) + pt_ruta[i+1]*resto;

					TVector3d dir_r = pr_LA-LA_ant;
					dir_r.normalizar();
					pr_LF = pr_LA - dir_r*20 + TVector3d(0,5,0);
					LA_ant = pr_LA;
				}
				break;
		}

		// Renderizo
		DXRender();
		

		if(timer_closing>0)
		{
			// breve instatne que no proceso ningun msg mientras cierro el dialogo
			timer_closing-=elapsed_time;
			double dp = elapsed_time*4000;
			for(int i=gui.item_0;i<gui.cant_items;++i)
			{
				gui.items[i]->rc.left-=dp;
				gui.items[i]->rc.right-=dp;
			}

			if(timer_closing<0)
			{
				seguir = FALSE;		// termino pp dicho
				// restauro las pos. originales
				gui.RestaurarPos();
				gui.closing = FALSE;
			}
		}
		else
		{
			// --------------------
			// Proceso los msg de windows
			MSG Msg;
			ZeroMemory( &Msg, sizeof(Msg) );
			if(PeekMessage( &Msg, NULL, 0U, 0U, PM_REMOVE ) )
			{

				if(Msg.message == WM_QUIT)
				{
					seguir = FALSE;
					break;
				}

				// dejo que windows procese el mensaje
				TranslateMessage( &Msg );
				DispatchMessage( &Msg );
				// dejo que el gui procese el msg
				int rta = gui.ProcessMsg(&Msg);
				if(rta)
				{
					// rta = 0->cotinua, 1->termina el dialogo con IDOK, -1 cancela el dialogo
					// rta = 2->termina sin delay
					if(rta==2)
					{
						// cierre sin delay
						seguir = FALSE;
					}
					else
					{
						// cierre con delay
						timer_closing = 0.5;
						gui.closing = TRUE;
					}
				}
			}
		}

	}

	// ----------------------
	// termina el modo gui, vuelvo a usar el directinput como interface del mouse:
	mouse.CreateDevice(m_hWnd);

}

void CGameEngine::Presentacion()
{

	modo_gui = TRUE;
	animacion_gui = GUI_ESCENARIO;
	gui.Reset();
	gui.InitDialog();
	int x0 = 150;
	int y0 = 120;
	int dx=300;
	int dy=20;

	gui_item *frame = gui.InsertItem(GUI_FRAME,"TRON INVOLUTION",x0-30,y0-30,600,250);
	frame->c_fondo = D3DCOLOR_ARGB(128,0,0,0);
	x0+=50;
	gui.InsertItem(GUI_STATIC_TEXT,"=== Tributo a la pelicula Tron ===",x0,y0+=dy);
	y0+=dy;
	gui.InsertItem(GUI_STATIC_TEXT,"MOUSE: Gira la moto.",x0,y0+=dy);
	gui.InsertItem(GUI_STATIC_TEXT,"CLICK: permite saltar (se puede girar en el aire).",x0,y0+=dy);
	gui.InsertItem(GUI_STATIC_TEXT,"RUEDITA:  Usa la ruedita para acelerar o frenar.",x0,y0+=dy);
	gui.InsertItem(GUI_STATIC_TEXT,"F3: Menu / Salir / Opciones",x0,y0+=dy);
	gui.InsertItem(GUI_STATIC_TEXT,"F2: cámara lenta",x0,y0+=dy);
	gui.InsertItem(GUI_STATIC_TEXT,"ESPACIO: pausa",x0,y0+=dy);
	gui.InsertItem(GUI_STATIC_TEXT,"F1: módo camara (pone pausa primero)",x0,y0+=dy);
	gui.InsertItem(GUI_BUTTON,"Comenzar",x0-50,y0+120,200,dy);
	
	BOOL seguir = TRUE;

	while(seguir)
	{
		int btn_ok = gui.sel = gui.cant_items-1;

		// entro en loop
		GUI_loop();

		if(gui.sel==btn_ok)
			// comenzar
			seguir = FALSE;
	}

	modo_gui = FALSE;
	animacion_gui = GUI_ESCENARIO;
	gui.EndDialog();

}


void CGameEngine::FinRonda(int nro_player)
{
	// --- perdio, empieza la siguiente ronda------
	nro_player_perdio = nro_player;
	if(nro_player)
		player_one->score++;
	else
		P[1].score++;
	
	// empieza todo de nuevo
	// limpio celdas y lightpath 
	memset(celdas,0,sizeof(celdas));
	memset(ligthpath_cant,0,sizeof(ligthpath_cant));
	// Reseteo la cantidad de vidas y vuelvo a la pos inicial
	for(int i=0;i<cant_players;++i)
	{
		P[i].Reset();
		P[i].Update();
		P[i].actualizarPath();
	}

	primer_msg = cant_msg = 0;

	// paso a la siguiente ronda
	if(++ronda>=max_rondas)
		timer_game_over = 5;
	else
	{
		if(modo_juego==GAME_GRID_SURVIVAL)
			// paso al warming up
			timer_warming = 4;
		else
			timer_warming = 7;

			/*
			// Inicio un dialogo modalless
			gui.InitDialog();
			int x0 = 300;
			int y0 = 70;
			int dy = 20;
			int dx = 300;
			gui.total_delay = 0;
			gui.InsertItem(GUI_FRAME,"Grid Battle",x0-50,y0-50,300,200);
			char buffer[255];
			sprintf(buffer,"Pierde Player %s",P[nro_player_perdio].nombre);
			gui_item *item = gui.InsertItem(GUI_STATIC_TEXT,buffer,x0,y0);
			item->nro_textura = textura_menu;

			gui_item *item_face = gui.InsertItem(GUI_FACE,"",x0+30,y0+100,60,60);
			item_face->nro_textura = textura_face + P[0].nro_avatar;
			
			item_face = gui.InsertItem(GUI_FACE,"",x0+170,y0+100,60,60);
			item_face->nro_textura = textura_face + P[1].nro_avatar;

			timer_modalless = 6;		// le doy 3 segundos de vida
			*/
	}


}


void CGameEngine::GUI_mainmenu()
{

	gui.Reset();

	int x0 = 80;
	int y0 = 40;
	int dx = 500;
	int dy = 20;
	gui_item *mainmenu = gui.InsertItem(GUI_STATIC_TEXT,"Main Menu",x0,y0);
	mainmenu->nro_textura = textura_menu;

	gui.InsertItem(GUI_MENU_ITEM,"GRID BATTLE - vos contra la maquina a 7 rondas",x0,y0+=50,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"SURVIVAL - vos contra todos hasta que te estroles",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"RACE - recorrer la ruta",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"PROFILE - configurar personaje",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"AYUDA",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"EXIT",x0,y0+=30,dx,dy);

	// Tests
	y0+=50;
	gui.InsertItem(GUI_MENU_ITEM,"Survival Test - todos contra todos",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"Survival Test - maquina sola",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"Battle Grid Test - maquina vs maquina",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"Training - vos solo con la ruta",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"Combo Rata- maquina vs laberinto de luz",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"MULTIPLAYER",x0,y0+=30,dx,dy);

	
	BOOL seguir = TRUE;

	while(seguir)
	{
		modo_gui = TRUE;
		gui.sel = 1;

		// entro en loop
		GUI_loop();

		switch(gui.sel)
		{
			case 1:
				// grid battle
				if(GUI_options_battle())
				{
					modo_juego = GAME_GRID_BATTLE;
					InitGame();
					modo_gui = FALSE;
					MainLoop();
				}
				break;
			case 2:
				// survival
				if(GUI_options_survival())
				{
					modo_juego = GAME_GRID_SURVIVAL;
					InitGame();
					modo_gui = FALSE;
					MainLoop();
				}
				break;
			case 3:
				// Driving Test
				if(GUI_options_ruta())
				{
					modo_juego = GAME_TEST_RUTA;
					InitGame();
					player_one->poner_en_ruta();
					player_one->initPath();
					player_one->Update();
					player_one->Update();			// ojo que el segundo update esta a proposito, necesito 2 llamadas
					modo_gui = FALSE;
					MainLoop();
				}
				break;

			case 4:
				// configurar profile
				GUI_options_profile();
				break;

			case 5:
				// Ayuda
				break;

			case 6:
				if(GUI_messagebox("Desea salir del juego?")==1)
					exit(0);
				break;

			case 7:
				// Test de todos contra todos juega sola
				juega_sola = TRUE;
				cant_enemigos = 7;
				nivel = 1;
				modo_juego = GAME_GRID_SURVIVAL;
				InitGame();
				modo_gui = FALSE;
				MainLoop();
				juega_sola = FALSE;
				cant_enemigos = 1;
				nivel = 0;
				break;

			case 8:
				// suvival test, un solo jugador que juega solo
				juega_sola = TRUE;
				cant_enemigos = 0;
				nivel = 3;
				modo_juego = GAME_GRID_SURVIVAL;
				InitGame();
				modo_gui = FALSE;
				MainLoop();
				juega_sola = FALSE;
				cant_enemigos = 1;
				nivel = 0;
				break;

			case 9:
				// Battle Grid Test
				max_rondas = 2;
				juega_sola = TRUE;
				nivel = 3;
				modo_juego = GAME_GRID_BATTLE;
				InitGame();
				modo_gui = FALSE;
				MainLoop();
				juega_sola = FALSE;
				nivel = 0;
				max_rondas = 7;
				break;


			case 10:
				// training test
				cant_enemigos = 0;
				nivel = 0;
				modo_juego = GAME_GRID_SURVIVAL;
				tipo_ruta = 3;
				InitGame();
				modo_gui = FALSE;
				MainLoop();
				cant_enemigos = 1;
				tipo_ruta = -1;
				break;

			case 11:
				// laberinto de luz
				juega_sola = TRUE;
				cant_enemigos = 0;
				nivel = 0;
				modo_juego = GAME_TEST_LABERINTO;
				tipo_ruta = 3;
				InitGame();
				modo_gui = FALSE;
				MainLoop();
				cant_enemigos = 1;
				tipo_ruta = -1;
				juega_sola = FALSE;

				break;

			case 12:
				// multiplayer:
				if((socket_init = AfxSocketInit()))
				{
					multiplayer = TRUE;
					if(GUI_messagebox("Poner como Servidor")==1)
					{
						es_servidor = TRUE;
						server.Create(80);
						server.Listen();


						
					}
					else
					{
						// intento conectar el player one con un servidor ya abierto
						es_servidor = FALSE;
						player_one->socket.Create();

						// pregunto el ip 
						// tomo el ip x defecto: 
						char saddress[255];
						FILE *fp = fopen("ip.dat","rt");
						fgets(saddress,sizeof(saddress),fp);
						fclose(fp);
						if(GUI_input_data(saddress,"Conectarse a Servidor","Ip = "))
						{
							// Grabo el ultimo Ip
							FILE *fp = fopen("ip.dat","wt");
							fprintf(fp,"%s",saddress);
							fclose(fp);

							// y me conecto con el servidor
							player_one->socket.Connect(saddress,80);
						}
					}

					cant_players = 1;
					if(GUI_esperar_conexion())
					{
						// ahora deberia haber 2 jugadores 
						cant_enemigos = 1;
						// inicio el juego
						modo_juego = GAME_GRID_SURVIVAL;
						InitGame();
						modo_gui = FALSE;
						MainLoop();
					}
				}
				break;

		}

		if(timer_modalless)
		{
			// si habia un dialogo no modall: lo corto
			timer_modalless = 0;
			// finalizo el dialogo no modal
			gui.EndDialog();
		}

		if(seguir)
			InitGame();



	}

	modo_gui = FALSE;

}

int CGameEngine::GUI_esperar_conexion()
{
	BOOL rta = TRUE;
	gui.InitDialog();
	int x0 = 100;
	int y0 = 120;
	int dx=300;
	int dy=20;
	gui.total_delay = 0;
	char msg[255];
	if(es_servidor)
	{
		// Determino el IP del servidor.
		strcpy(msg,"Esperando por conexiones entrantes.");

		BYTE ip[4];
		if(get_ip(ip))
		{
			char saddress[255];
			sprintf(saddress,"IP = %d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
			strcat(msg, saddress);
		}
		else
			strcat(msg, "Error IP");
	}
	else
	{
		strcpy(msg,"Espere mientras se establece la conexion");
	}

	gui.InsertItem(GUI_FRAME,"Tron Involution",x0-50,y0-50,600,350);
	gui.InsertItem(GUI_STATIC_TEXT,msg,x0,y0+=20);
	gui.InsertItem(GUI_BUTTON,"Cancelar",x0,y0+=50,60,dy);
	gui.sel = gui.item_0+2;

	// el modo gui se trabaja con el input de windows standard
	// asi que libero el direct input
	mouse.FreeDirectInput();
	// ----------------------
	
	// entro en loop
	BOOL seguir = TRUE;
	CPoint Move,Last;
	GetCursorPos(&Last);
	BOOL moving = FALSE;
	int nFlags = 0;
	int cant_frames = 0;
	time = 0;				// tiempo total (local)
	remote_time = 0;		// tiempo remoto
	double ftime = 0;		// frame time
	elapsed_time = 0;		// tiempo parcial (laps) 
	double timer_closing = 0;

	LARGE_INTEGER F,T0,T1;   // address of current frequency
	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);
	SetCursorPos(d3dpp.BackBufferWidth/2,d3dpp.BackBufferHeight/2);
	TVector3d LA_ant = LA;
	pr_LA = TVector3d(0,0,0);
	pr_LF = TVector3d(radio*1.5,50,radio*1.5);
	while(seguir)
	{
		// Avanzo el tiempo
		QueryPerformanceCounter(&T1);
		elapsed_time = (double)(T1.LowPart - T0.LowPart) / (double)F.LowPart;
		T0 = T1;
		time+=elapsed_time;
		// animacion del escenario modo gui
		LF = TVector3d(-radio*0.5,10,-radio*0.5);
		LA = TVector3d(0,5,0);
		LF.rotar_xz(LA,time*0.05);

		// Renderizo
		DXRender();
		

		if(timer_closing>0)
		{
			// breve instatne que no proceso ningun msg mientras cierro el dialogo
			timer_closing-=elapsed_time;
			double dp = elapsed_time*4000;
			for(int i=gui.item_0;i<gui.cant_items;++i)
			{
				gui.items[i]->rc.left-=dp;
				gui.items[i]->rc.right-=dp;
			}

			if(timer_closing<0)
			{
				seguir = FALSE;		// termino pp dicho
				// restauro las pos. originales
				gui.RestaurarPos();
			}
		}
		else
		{
			// --------------------
			// Proceso los msg de windows
			MSG Msg;
			ZeroMemory( &Msg, sizeof(Msg) );
			if(PeekMessage( &Msg, NULL, 0U, 0U, PM_REMOVE ) )
			{

				if(Msg.message == WM_QUIT)
				{
					seguir = FALSE;
					break;
				}

				// dejo que windows procese el mensaje
				TranslateMessage( &Msg );
				DispatchMessage( &Msg );
				// dejo que el gui procese el msg
				if(gui.ProcessMsg(&Msg))
					// rta = 0->cotinua, 1->termina el dialogo con IDOK, -1 cancela el dialogo
					timer_closing = 0.5;
			}
		}


		// verifico si se completo la operacion de conexion,
		// cuando se completa la conexion, se agrego un jugador mas,
		// en esta version soporta 2 jugadores, cuando llegue a 2 jugadores,
		// empieza el juego
		if(cant_players>=2)
			seguir = FALSE;
	}


	gui.EndDialog();


	// ----------------------
	// termina el modo gui, vuelvo a usar el directinput como interface del mouse:
	mouse.CreateDevice(m_hWnd);

	return rta;
}

int CGameEngine::GUI_messagebox(char *msg,int tipo)
{
	BOOL rta = 0;
	gui.InitDialog();
	int dx = 350;
	int dy = 200;

	int x0 = (d3dpp.BackBufferWidth - dx)/2;
	int y0 = (d3dpp.BackBufferHeight - dy)/2 - 50;

	gui.total_delay = 0;
	gui.InsertItem(GUI_FRAME,"Tron Involution",x0,y0,dx,dy);
	x0+=50,y0+=50;
	gui.InsertItem(GUI_STATIC_TEXT,msg,x0,y0+=20);
	gui.InsertItem(GUI_BUTTON,"SI",x0,y0+=50,60,20);
	gui.InsertItem(GUI_BUTTON,"NO",x0+150,y0,60,20);
	gui.sel = gui.item_0+2;
	// entro en loop
	GUI_loop();
	if(gui.sel == gui.item_0+2)
		rta = 1;

	gui.EndDialog();


	return rta;

}



int CGameEngine::GUI_options_ruta()
{
	BOOL rta = 0;
	// activo el modo preview ruta
	animacion_gui = GUI_PREVIEW_RUTA;
	pr_pos = 0;
	gui.InitDialog();
	int x0 = 100;
	int y0 = 70;
	int dy = 20;
	gui.total_delay = 0;
	gui.InsertItem(GUI_FRAME,"Opciones ",x0-50,y0-50,600,180);
	gui.InsertItem(GUI_BUTTON,"Seguir",x0-50,y0+200,200,dy);
	gui.InsertItemRect("Preview",300-3,240-3,350+6,200+6,8);
	gui.InsertItem(GUI_STATIC_TEXT,"Seleccione el tipo de ruta",x0,y0);
	gui.InsertItemRuta(0,"Espiral",x0,y0,100,100);
	gui.InsertItemRuta(1,"Espiral *",x0+120,y0,100,100);
	gui.InsertItemRuta(2,"Estrella",x0+240,y0,100,100);
	gui.InsertItemRuta(3,"Plana",x0+360,y0,100,100);
	gui.InsertItemRuta(-1,"nada",x0+480,y0,100,100);

	int sel;
	if(tipo_ruta==-1)
		sel = gui.sel = gui.item_0+8;
	else
		sel = gui.sel = gui.item_0+4+tipo_ruta;
	gui.items[sel]->state |= GUI_CHECKED;
	
	// entro en loop
	GUI_loop();

	// tomo la respuesta
	int i0 = gui.item_0+4;
	tipo_ruta = -1;
	int t = 0;
	while(t<5 && tipo_ruta==-1)
		if(gui.items[i0+t]->state&GUI_CHECKED)
			tipo_ruta = t;
		else
			++t;

	if(tipo_ruta==4)
		tipo_ruta = -1;		// nada

	gui.EndDialog();
	animacion_gui = GUI_ESCENARIO;
	return tipo_ruta!=-1?TRUE:FALSE;

}



int CGameEngine::GUI_options_battle()
{
	gui.InitDialog();
	int x0 = 100;
	int y0 = 70;
	int dy = 20;
	int dx = 300;
	gui.total_delay = 0;
	gui.InsertItem(GUI_FRAME,"Opciones - Battle Grid",x0-50,y0-50,600,400);
	gui.InsertItem(GUI_STATIC_TEXT,"Vos contra la maquina a 7 rounds",x0,y0);
	gui.InsertItem(GUI_MENU_ITEM,"Principiante",x0,y0+=50,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"Intermedio",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"Experto",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"A full",x0,y0+=30,dx,dy);

	gui.InsertItem(GUI_STATIC_TEXT,"Domo con ruta",x0,y0+=50);
	gui.InsertItem(GUI_MENU_ITEM,"Principiante",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"Intermedio",x0,y0+=30,dx,dy);

	int sel = gui.sel = gui.item_0+2;
	
	// entro en loop
	GUI_loop();

	// tomo la respuesta
	int t = gui.sel-gui.item_0-2;
	if(t>=0 && t<=3)
	{
		nivel = t;
		tipo_ruta = -1;
	}
	else
	if(t>=5 && t<=6)
	{
		// niveles lentos, con ruta 
		// (los niveles rapidos con ruta no se puede, porque no hay forma de subirse la ruta)
		nivel = t-5;
		tipo_ruta = 3;
	}

	gui.EndDialog();
	return TRUE;
}



int CGameEngine::GUI_options_survival()
{
	gui.InitDialog();
	int x0 = 100;
	int y0 = 70;
	int dy = 20;
	int dx = 300;
	gui.total_delay = 0;
	gui.InsertItem(GUI_FRAME,"Opciones - Survival",x0-50,y0-50,600,400);
	gui.InsertItem(GUI_STATIC_TEXT,"Todos contra todos, dura lo que dura",x0,y0);
	gui.InsertItem(GUI_MENU_ITEM,"Principiante",x0,y0+=50,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"Intermedio",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"Experto",x0,y0+=30,dx,dy);
	gui.InsertItem(GUI_MENU_ITEM,"A full",x0,y0+=30,dx,dy);

	// cantidad de enemigos: 
	char saux[40];
	strcpy(saux, "1");
	gui.InsertItem(GUI_STATIC_TEXT,"Número de Enemigos:",x0,y0+=40);
	gui_item *item_cant = gui.InsertItem(GUI_EDIT,saux,x0,y0+=40);

	int sel = gui.sel = gui.item_0+2;
	
	// entro en loop
	GUI_loop();

	// este va siempre sin ruta
	tipo_ruta = -1;

	// tomo la respuesta
	int t = gui.sel-gui.item_0-2;
	if(t>=0 && t<=3)
	{
		nivel = t;
	}

	// tomo la cant. de enemigos (1..7)
	cant_enemigos = atoi(item_cant->text);
	if(cant_enemigos>7)
		cant_enemigos = 7;
	else
	if(cant_enemigos<1)
		cant_enemigos = 1;



	gui.EndDialog();
	return TRUE;
}



int CGameEngine::GUI_options_profile()
{
	BOOL rta = 0;
	// activo el modo preview ruta
	animacion_gui = GUI_PREVIEW_MOTO;
	pr_pos = 0;
	gui.InitDialog();
	int x0 = 100;
	int y0 = 40;
	int dy = 20;
	gui.total_delay = 0;
	gui.InsertItem(GUI_FRAME,"          Profile Identity",x0-50,y0-20,600,440);
	gui_item *item_aux = gui.InsertItem(GUI_STATIC_TEXT,"Configuración de datos Personales",x0+80,y0+25);
	item_aux->g_pFont = g_pFont2;		// le pongo una letra mas chica
	item_aux->c_font = D3DCOLOR_XRGB(240,240,130);
	item_aux = gui.InsertItem(GUI_STATIC_TEXT,"Avatar, colores, etc del personaje.",x0+80,y0+50);
	item_aux->g_pFont = g_pFont2;
	item_aux->c_font = D3DCOLOR_XRGB(240,240,130);

	gui.InsertItem(GUI_BUTTON,"seguir",x0-50,d3dpp.BackBufferHeight-dy-20,200,dy);
	gui.InsertItem(GUI_STATIC_TEXT,"Ingresa tu nombre:",x0,y0+=90);
	gui_item *item_nombre = gui.InsertItem(GUI_EDIT,player_one->nombre,x0,y0+=40);
	gui.InsertItem(GUI_STATIC_TEXT,"Tu mensaje:",x0,y0+=40);
	gui_item *item_slogan = gui.InsertItem(GUI_EDIT,player_one->slogan,x0,y0+=40);
	gui.InsertItem(GUI_STATIC_TEXT,"Color de la moto:",x0,y0+=40);


	int dx = 50;
	int pos_x = x0;
	int pos_y = y0+40;
	int s = 0;
	for(int i=0;i<_cant_colores;++i)
	{
		gui_item *item = gui.InsertItemColor(pos_x,pos_y,_lst_colores[i]);
		if((i+1)%4==0)
		{
			if(s%2==1)
				pos_x = x0;
			else
				pos_x = x0-38;
			pos_y += dx/2;
			s++;
		}
		else
			pos_x += dx + dx/2;

		if(i==player_one->nro_color)
			item->state |= GUI_CHECKED;

		// uso el texto para meter el dato del nro de color
		sprintf(item->text,"%d",i);
	}

	x0 = 440;
	y0 = 60;
	gui.InsertItem(GUI_STATIC_TEXT,"Avatar:",x0,y0);
	y0+=50;
	int nro_textura = textura_face;
	gui_item *item_face = gui.InsertItem(GUI_FACE,"",x0,y0,60,60);
	item_face->nro_textura = nro_textura++;
	int primer_item_face = gui.cant_items-1;
	item_face = gui.InsertItem(GUI_FACE,"",x0+42,y0+30,60,60);
	item_face->nro_textura = nro_textura++;
	item_face = gui.InsertItem(GUI_FACE,"",x0+90,y0,60,60);
	item_face->nro_textura = nro_textura++;
	item_face = gui.InsertItem(GUI_FACE,"",x0+132,y0+30,60,60);
	item_face->nro_textura = nro_textura++;
	item_face = gui.InsertItem(GUI_FACE,"",x0,y0+60,60,60);
	item_face->nro_textura = nro_textura++;
	item_face = gui.InsertItem(GUI_FACE,"",x0+90,y0+60,60,60);
	item_face->nro_textura = nro_textura++;


	gui.face_sel = primer_item_face + player_one->nro_avatar;
	gui.items[gui.face_sel]->state |= GUI_CHECKED;
	gui.sel_color = _lst_colores[player_one->nro_color];
	
	// entro en loop
	GUI_loop();

	// tomo los datos del gui
	if(gui.rbt != -1)
		// recupero el nro del color, desde el texto del item gui
		player_one->SetColor(atoi(gui.items[gui.rbt]->text));


	// nombre del jugador y eslogan
	strcpy(player_one->nombre,item_nombre->text);
	strcpy(player_one->slogan,item_slogan->text);

	// avatar (cara del jugador)
	int nro_avatar = gui.face_sel - primer_item_face;
	if(nro_avatar>=0 && nro_avatar<6)
		player_one->nro_avatar = nro_avatar;

	// termina el dialogo
	gui.EndDialog();
	animacion_gui = GUI_ESCENARIO;
	return rta;
}


int CGameEngine::GUI_pausa()
{
	if(timer_modalless)
	{
		// todavia no habia terminado la pausa anterior
		timer_modalless = 0;
		// finalizo el dialogo no modal
		gui.EndDialog();
	}

	//  --------- menu PAUSA -------------
	modo_gui = TRUE;
	animacion_gui = GUI_GAME_PAUSED;
	gui.InitDialog();
	gui.total_delay = 0;
	int dy = 20;
	gui_item *pitem;

	int y = d3dpp.BackBufferHeight-40;
	int x = 10;
	int xf = d3dpp.BackBufferWidth-90-90;
	gui_item *radio = gui.InsertItem(GUI_TOOLBAR,"Radio",x,y,32,32);
	radio->nro_textura = 0;
	gui_item *s1 = gui.InsertItem(GUI_TOOLBAR,"Sonido Jugador #1",x+=40,y,32,32);
	s1->nro_textura = 1;
	gui_item *s2 = gui.InsertItem(GUI_TOOLBAR,"Sonido Jugador #2",x+=40,y,32,32);
	s2->nro_textura = 2;
	gui_item *vidas = gui.InsertItem(GUI_TOOLBAR,"Vidas Infinitas Jugador #1",x+=40,y,32,32);
	vidas->nro_textura = 3;
	gui_item *draft = gui.InsertItem(GUI_TOOLBAR,"Modo Render de baja Calidad",x+=40,y,32,32);
	draft->nro_textura = 4;
	gui_item *onda = gui.InsertItem(GUI_TOOLBAR,"Tono 440hz (Doppler Test)",x+=40,y,32,32);
	onda->nro_textura = 5;
	gui_item *debug = gui.InsertItem(GUI_TOOLBAR,"Debug IA",x+=40,y,32,32);
	debug->nro_textura = 6;

	pitem = gui.InsertItem(GUI_MENU_ITEM,"Seguir",xf,y,80,dy);
	pitem->flags &= (0xFFFF - GUI_CURSOR);
	pitem = gui.InsertItem(GUI_MENU_ITEM,"Salir",xf+90,y,80,dy);
	pitem->flags &= (0xFFFF - GUI_CURSOR);

	// init
	if(hay_radio)
		radio->state |= GUI_CHECKED;
	if(!P[0].mute)
		s1->state |= GUI_CHECKED;
	if(!P[1].mute)
		s2->state |= GUI_CHECKED;
	if(vidas_infinitas)
		vidas->state |= GUI_CHECKED;
	if(modo_draft)
		draft->state |= GUI_CHECKED;
	if(player_one->sound.tipo_mezcla!=2)
		onda->state |= GUI_CHECKED;
	if(debug_ia)
		debug->state |= GUI_CHECKED;

	BOOL seguir = TRUE;
	while(seguir)
	{
		// entro en loop
		GUI_loop();
		// tomo la respuesta
		if(gui.sel==gui.cant_items-2)
			// continuar el juego: termino el dialogo
			seguir = FALSE;
		else
		if(gui.sel==gui.cant_items-1)
			// termino el juego
			exit(0);
		else
		{
			int t = gui.sel-gui.item_0;
			switch(t)
			{
				case 0:
					// Radio on off
					GUI_elegir_midi();
					if(hay_radio)
						radio->state |= GUI_CHECKED;
					else
						radio->state &= (0xFFFF - GUI_CHECKED);
					break;
				case 1:
					// Player one mute on / off
					P[0].mute = !P[0].mute;
					if(!P[0].mute)
						s1->state |= GUI_CHECKED;
					else
						s1->state &= (0xFFFF - GUI_CHECKED);
					break;
				case 2:
					// Player 2 mute on / off
					P[1].mute = !P[1].mute;
					if(!P[1].mute)
						s2->state |= GUI_CHECKED;
					else
						s2->state &= (0xFFFF - GUI_CHECKED);
					break;

				case 3:
					// vidas infinitas
					vidas_infinitas = !vidas_infinitas;
					if(vidas_infinitas)
						vidas->state |= GUI_CHECKED;
					else
						vidas->state &= (0xFFFF - GUI_CHECKED);
					break;

				case 4:
					// pantalla s/shaders
					modo_draft = !modo_draft;
					if(modo_draft)
						draft->state |= GUI_CHECKED;
					else
						draft->state &= (0xFFFF - GUI_CHECKED);
					break;

				case 5:
					// prueba de efecto dopler
					if(player_one->sound.tipo_mezcla!=2)
					{
						player_one->sound.tipo_mezcla = 2;
						P[1].sound.tipo_mezcla = 2;
						onda->state &= (0xFFFF - GUI_CHECKED);
					}
					else
					{
						player_one->sound.tipo_mezcla = 1;
						P[1].sound.tipo_mezcla = 0;
						onda->state |= GUI_CHECKED;
					}
					break;

				case 6:
					// debug de IA 
					debug_ia = !debug_ia;
					if(debug_ia)
						debug->state |= GUI_CHECKED;
					else
						debug->state &= (0xFFFF - GUI_CHECKED);
					break;


			}
		}
	}

	modo_gui = FALSE;
	gui.EndDialog();
	animacion_gui = GUI_ESCENARIO;

	paused = FALSE;

	{
		// Inicio un dialogo modalless
		gui.InitDialog();
		int x0 = d3dpp.BackBufferWidth - 260;
		int y0 = 20;
		int dy = 20;
		int dx = 200;
		gui.total_delay = 0;
		gui.InsertItem(GUI_FRAME,"Tron Involution",x0,y0,250,150);
		char buffer[255];
		gui_item *item = gui.InsertItem(GUI_STATIC_TEXT,"Continua el juego...",x0+30,y0+110);
		//item->nro_textura = textura_menu;
		item = gui.InsertItem(GUI_STATIC_TEXT,"vs",x0+100,y0+55);
		gui_item *item_face = gui.InsertItem(GUI_FACE,"",x0+30,y0+40,60,60);
		item_face->nro_textura = textura_face + P[0].nro_avatar;
		item_face = gui.InsertItem(GUI_FACE,"",x0+140,y0+40,60,60);
		item_face->nro_textura = multiplayer?textura_face + P[1].nro_avatar:textura_face_enemigo;
		timer_modalless = 3;		// le doy 2 segundos de vida
	}

	return 0;
}


int CGameEngine::GUI_input_data(char *buffer,char *titulo,char *msg)
{
	BOOL rta = TRUE;
	gui.InitDialog();
	int x0 = 100;
	int y0 = 40;
	int dy = 20;
	gui.total_delay = 0;
	gui.InsertItem(GUI_FRAME,titulo,x0-50,y0-20,600,400);
	gui.InsertItem(GUI_BUTTON,"seguir",x0,300,200,dy);
	gui.InsertItem(GUI_STATIC_TEXT,msg,x0,y0+=40);
	gui_item *item_data = gui.InsertItem(GUI_EDIT,buffer,x0,y0+=40);

	// entro en loop
	GUI_loop();

	// tomo los datos del gui
	strcpy(buffer,item_data->text);

	// termina el dialogo
	gui.EndDialog();


	return rta;
}


void CGameEngine::cargarPlayList()
{
	char ant_dir[255];
	GetCurrentDirectory(255,ant_dir);
	SetCurrentDirectory("sounds");

	cant_midis = 0;
	midi_actual = 0;
	WIN32_FIND_DATA lpff;
	// Busco los archivos midis
	HANDLE ff = FindFirstFile("*.mid",&lpff);
	if(ff!=INVALID_HANDLE_VALUE)
	{
		BOOL flag = TRUE;
		while(flag)
		{
			if(lpff.cFileName[0]!='.' && lpff.cFileName[0] !='$' && 
				!(lpff.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
				strcpy(midi_files[cant_midis++],lpff.cFileName);
			// y paso al siguiente archivo
			flag = FindNextFile(ff,&lpff);
		}

		FindClose(ff);
	}
	SetCurrentDirectory(ant_dir);
}


int CGameEngine::GUI_elegir_midi()
{
	gui.InitDialog();
	int x0 = 100;
	int y0 = 70;
	int dy = 20;
	int dx = 300;
	gui.total_delay = 0;
	gui.InsertItem(GUI_FRAME,"Play List",x0-50,y0-50,600,400);
	gui.InsertItem(GUI_STATIC_TEXT,"Temas disponibles",x0,y0);
	for(int t = 0; t<cant_midis;++t)
	{
		gui.InsertItem(GUI_MENU_ITEM,midi_files[t],x0,y0+=50,dx,dy);
	}
	gui.InsertItem(GUI_MENU_ITEM,"Apagar Radio",x0,y0+=150,dx,dy);
	int sel = gui.sel = gui.item_0+2+midi_actual;
	
	// entro en loop
	GUI_loop();


	// tomo la respuesta
	t = gui.sel-gui.item_0-2;
	if(t>=0 && t<cant_midis)
	{

		hay_radio = TRUE;
		midi_actual = t;
		playMidi();
	}
	else
	{
		// apago la radio
		mciSendCommand(midi_DeviceID, MCI_PAUSE, 0, NULL);
		hay_radio = FALSE;
	}

	gui.EndDialog();
	return TRUE;
}

