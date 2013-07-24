#include "stdafx.h"

#include <mmsystem.h>
#include "\Program Files (x86)\Microsoft DirectX SDK (April 2006)\include\d3dx9.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include "\msdev\games\clases\vectores.h"
#include "\msdev\games\clases\motor3d.h"


/////////////////////////////////////////////////////////////////////////////
// Sistema de Particulas
/////////////////////////////////////////////////////////////////////////////
CParticleSystem::CParticleSystem()
{
	g_pd3dDevice = NULL;
	g_pVB = NULL;
	cant_part = 0;
	r0 = 50;
	g0 = 100;
	b0 = 200;
	t0 = 1;		// tiempo a partir del cual las particulas empiezan a desaparecer
	t1 = 2;		// tiempo final, luego de t1 no hay mas particulas visibles
	gravedad = -1;
	textura = 0;
	cant_pasos = 0;
	rastro = FALSE;
}

CParticleSystem::~CParticleSystem()
{
	Release();
}


// simulacion stage
HRESULT  CParticleSystem::Create(DXEngine *p_model,TVector3d Pos,int cant,
									int nro_textura,char tipo_explosion)
{
	g_pd3dDevice = p_model->g_pd3dDevice;
	model = p_model;
	cant_part = cant;
	g_time = 0;
	cant_pasos = 0;
	P0 = Pos;		// posicion inicial del emisor
	tipo = tipo_explosion;
	textura = nro_textura;

	srand( (unsigned)time( NULL ) );
	int t = 0;
	double dk = 1;
	int cant_dir = cant_part/dk;

	for(int i=0;i<cant_dir;++i)
	{
		double alfa,beta;
		switch(tipo)
		{
			case 0:
			default:
				alfa = 2*M_PI*(double)rand()/(double)RAND_MAX;
				beta = M_PI/4 + M_PI*(double)rand()/(double)RAND_MAX/2;
				gravedad = -1;
				break;
			case 1:
				alfa = 2*M_PI*(double)rand()/(double)RAND_MAX;
				beta = M_PI*(double)rand()/(double)RAND_MAX/2;
				if(beta>M_PI/4)
					beta = M_PI - beta;
				gravedad = 1;
				break;
		}
		
		TVector3d Dir = TVector3d(cos(beta)*cos(alfa),sin(beta),cos(beta)*sin(alfa));		
		for(int j=0;j<dk;++j)
		{
			particle[t].Dir = Dir;
			particle[t].Pos = Pos;
			particle[t].rtime = 5;
			particle[t].vel_v =  0;
			if(rastro)
				particle[t].delay =  0;
			else
				particle[t].delay =  (double)rand()/(double)RAND_MAX;
			
			if(tipo==0)
			{
				particle[t].acel = dk*(double)rand()/(double)RAND_MAX;
				particle[t].vel =  2+5*(double)rand()/(double)RAND_MAX;
				particle[t].dq = 0.5+ 0.5*(dk-j)/dk + 0.3*(double)rand()/(double)RAND_MAX;
				particle[t].acel *= -1;
				particle[t].vel *= 1;
			}
			else
			{
				// explosion circular (semi-hemisferio)
				particle[t].vel =  8;
				particle[t].acel =  -5;
				particle[t].dq = 0.05;
			}


			++t;
		}
	}

	/*
	cant_part = 1;
	particle[0].Dir = TVector3d(-1,3,-1);
	particle[0].Dir.normalizar();
	particle[0].Pos = Pos;
	particle[0].rtime = 5;
	particle[0].acel = 0;
	particle[0].vel =  5;
	particle[0].vel_v =  0;
	particle[0].dq = 1;
	*/

	// tengo 2 seg. de explosion, a 100 fps maximo: 
	// si entra al update 200 veces: 
	int max_fps = 100;
	if(rastro)
	{
		max_pasos = 2*max_fps;
		vertex_size = sizeof(VERTEX_PARTICLE)*cant_part*2;
	}
	else
	{
		max_pasos = 1;
		vertex_size = sizeof(VERTEX_PARTICLE)*cant_part*4;
	}
	

	// Creo un vertex buffer para alojar las particulas
	SAFE_RELEASE(g_pVB);
	// Alloco memoria y creo un buffer para todos los vertices
	//vertex_size = sizeof(VERTEX_PARTICLE)*cant_part*4;
	if( FAILED( g_pd3dDevice->CreateVertexBuffer( vertex_size*max_pasos,
				0 , D3DFVF_VERTEX_PARTICLE, D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
		return E_FAIL;

	return S_OK;
}




HRESULT  CParticleSystem::Update(float time,TVector3d pos_emisor)
{
	if(rastro && cant_pasos+2>=max_pasos)
		return E_FAIL;		// se quedo sin memoria para las particulas 

	if(g_pVB == NULL)
		return E_FAIL;
	double elapsed_time = time-g_time;

	g_time = time;

	VERTEX_PARTICLE* pVertices;
	if(rastro)
	{
		if( FAILED( g_pVB->Lock( cant_pasos*vertex_size, 3*vertex_size, (void**)&pVertices, 0 ) ) )
			return E_FAIL;
	}
	else
	{
		if( FAILED( g_pVB->Lock( 0, vertex_size, (void**)&pVertices, 0 ) ) )
			return E_FAIL;
	}


	++cant_pasos;

	int t = 0;
	for(int i=0;i<cant_part;++i)
	{
		if(particle[i].delay>0)
		{
			particle[i].delay-=elapsed_time;
			// la particula esta latente, solo ajusto la pos para que siga al emisor
			particle[i].Pos = particle[i].Pos + pos_emisor - P0;
		}
		else
		{
			// actualizo los parametros de la particula
			particle[i].rtime-=time;
			particle[i].vel += particle[i].acel*elapsed_time; 
			if(particle[i].vel<0)
			{
				particle[i].acel = 0;
				particle[i].vel *= -1;
			}

			// efecto de agrandar el quads 
			if(!rastro)
				particle[i].dq += 5*elapsed_time; 

			TVector3d ant_Pos = particle[i].Pos;
			TVector3d Vel = particle[i].Dir*(particle[i].vel*elapsed_time);
			particle[i].Pos = particle[i].Pos + Vel;

			if(gravedad)
			{
				particle[i].vel_v-=10*elapsed_time*gravedad;
				particle[i].Pos.y += particle[i].vel_v*elapsed_time;
				if(particle[i].Pos.y<0)
				{
					particle[i].Pos.y *= -1;		// parche para que no se pase del suelo
					particle[i].vel_v *= -0.7;		// rebota y pierde fuerza
					particle[i].Dir.y *= -1;

				}
			}


			// efecto de ir contra la camara. 
			// Se lo saco x que Martin dice que queda como el culo
			/*
			if(rastro)
			{
				TVector3d dir_c = model->LF - particle[i].Pos;
				dir_c.normalizar();
				particle[i].Pos = particle[i].Pos + dir_c*(particle[i].vel*elapsed_time*1.5);
			}*/

			// corrijo la pos. del emisor
			particle[i].Pos = particle[i].Pos + pos_emisor - P0;
			// efecto de rotar sobre el eje de la moto. 
			//particle[i].Pos.rotar_xz(pos_emisor,elapsed_time*10);

			if(rastro)
			{
				// Genero un line 
				TVector3d P[2];
				P[0] = ant_Pos;
				P[1] = particle[i].Pos;

				for(int j=0;j<2;++j)
				{
					pVertices[t].position.x = P[j].x;
					pVertices[t].position.y = P[j].y;
					pVertices[t].position.z = P[j].z;

					pVertices[t].texcoord0.x = 0;
					pVertices[t].texcoord0.y = 0;

					pVertices[t].normal.x = particle[i].Dir.x;
					pVertices[t].normal.y = particle[i].Dir.y;
					pVertices[t].normal.z = particle[i].Dir.z;

					double a = 1;
					if(time>t0)
					{
						a = 1 - (time-t0)/(t1-t0);
						if(a<0)
							a = 0;
					}
					pVertices[t].color = D3DCOLOR_ARGB((BYTE)(a*255),r0,g0,b0);
					++t;
				}
			}
		}
	}
		

	// particulas pp dichas:
	for(i=0;i<cant_part;++i)
	if(particle[i].delay<=0)
	{
		// Genero un Quad
		double dq = rastro?0.1:particle[i].dq;
		TVector3d U = TVector3d(model->matView._11 ,model->matView._21 ,model->matView._31);
		TVector3d V = TVector3d(model->matView._12 ,model->matView._22 ,model->matView._32);
		TVector3d P[4];
		P[0] = particle[i].Pos + V*dq + U*dq;
		P[1] = particle[i].Pos + V*dq - U*dq;
		P[2] = particle[i].Pos - V*dq + U*dq;
		P[3] = particle[i].Pos - V*dq - U*dq;

		TVector2d T[4];
		T[0] = TVector2d(0,0);
		T[1] = TVector2d(0,1);
		T[2] = TVector2d(1,0);
		T[3] = TVector2d(1,1);

		for(int j=0;j<4;++j)
		{
			pVertices[t].position.x = P[j].x;
			pVertices[t].position.y = P[j].y;
			pVertices[t].position.z = P[j].z;

			pVertices[t].texcoord0.x = T[j].x;
			pVertices[t].texcoord0.y = T[j].y;

			pVertices[t].normal.x = particle[i].Dir.x;
			pVertices[t].normal.y = particle[i].Dir.y;
			pVertices[t].normal.z = particle[i].Dir.z;

			double a = 1;
			if(time>t0)
			{
				a = 1 - (time-t0)/(t1-t0);
				if(a<0)
					a = 0;
			}
			pVertices[t].color = D3DCOLOR_ARGB((BYTE)(a*255),r0,g0,b0);
			++t;
		}
	}

	P0 = pos_emisor;

	g_pVB->Unlock();
	return S_OK;
}


// rendering stage
void CParticleSystem::Render()
{
	if(g_pVB == NULL)
		return;
	
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
	g_pd3dDevice->SetTextureStageState(0,D3DTSS_ALPHAARG2,D3DTA_DIFFUSE);
	g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof(VERTEX_PARTICLE) );
	g_pd3dDevice->SetFVF( D3DFVF_VERTEX_PARTICLE);
	g_pd3dDevice->SetTexture( 0, NULL);


	// saco el lighting para que module la textura con el color del vertice 
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE);
	int t0 = 0;
	// rastro
	if(rastro)
	{
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);		// uso el color blanco del vertex
		for(int t=0;t<cant_pasos;++t)
		for(int i=0;i<cant_part;++i)
			g_pd3dDevice->DrawPrimitive( D3DPT_LINELIST,2*i+t*2*cant_part,1);

		// restuaro que el color del mateiral lo tome de la textura
		g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
		t0 = cant_part*cant_pasos*2;
	}

	//g_pd3dDevice->SetTexture( 0, model->g_pTexture[model->textura_explo+textura]);
	// guarda: hay que verificar que el nro de textura tenga sumado el textura explo!!!!
	g_pd3dDevice->SetTexture( 0, model->g_pTexture[textura]);
	for(int i=0;i<cant_part;++i)
		g_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP,t0+i*4,2);

	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE);


}

void CParticleSystem::Release()
{
	SAFE_RELEASE(g_pVB);
	g_pVB = NULL;
	cant_part = 0;
}


/////////////////////////////////////////////////////////////////////////////
// Mallas 
/////////////////////////////////////////////////////////////////////////////
DXMesh::DXMesh()
{
	limpiar();

}

void DXMesh::limpiar()
{
	g_pMesh = NULL;
	g_pMeshMaterials = NULL;
	g_pMeshTextures = NULL;
	g_dwNumMaterials = 0;
	memset(fname,0,sizeof(fname));
}



// Carga el objeto desde un archivo .x y carga las texturas y materiales asociados
HRESULT  DXMesh::LoadMesh(DXEngine *m,char *file)
{
	modelo = m;
	strcpy(fname,file);
	LPDIRECT3DDEVICE9 g_pd3dDevice = m->g_pd3dDevice;

    // Load the mesh from the specified file
	LPD3DXBUFFER pD3DXMtrlBuffer;
	char file_name[MAX_PATH];
	sprintf(file_name,"texturas\\%s",file);

	// Archivo comun .X 
	if( FAILED( D3DXLoadMeshFromX( file_name, D3DXMESH_SYSTEMMEM|D3DXMESH_32BIT, g_pd3dDevice, NULL, &pD3DXMtrlBuffer, 
			NULL, &g_dwNumMaterials, &g_pMesh ) ) )
		return E_FAIL;

	// ya esta cargado, sigo con el resto del proceso
    // We need to extract the material properties and texture names from the  pD3DXMtrlBuffer
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
    g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
    if( g_pMeshMaterials == NULL )
        return E_OUTOFMEMORY;
    g_pMeshTextures  = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
    if( g_pMeshTextures == NULL )
        return E_OUTOFMEMORY;


    for( DWORD i=0; i<g_dwNumMaterials; i++ )
    {
        // Copy the material
        g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

        // Set the ambient color for the material (D3DX does not do this)
        g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;
        g_pMeshTextures[i] = NULL;

		// Create the texture
		char textura[MAX_PATH];
		sprintf(textura,"texturas\\%s",d3dxMaterials[i].pTextureFilename);
        if( d3dxMaterials[i].pTextureFilename != NULL && lstrlen(textura) > 0 )
		{
            //D3DXCreateTextureFromFile( g_pd3dDevice, textura, &g_pMeshTextures[i] );
			D3DXCreateTextureFromFileEx( g_pd3dDevice, textura, 
							D3DX_DEFAULT,    // default width
                            D3DX_DEFAULT,    // default height
                            0,    // automatic mipmaping: ojo....en este caso tengo cambiar toda la cadena de bitmaps....
                            NULL,    // regular usage
                            D3DFMT_A8R8G8B8,    // 32-bit pixels with alpha
                            D3DPOOL_MANAGED,    // typical memory handling
                            D3DX_DEFAULT,    // no filtering
                            D3DX_DEFAULT,    // no mip filtering
                            0,
                            NULL,    // no image info struct
                            NULL,    // not using 256 colors
							&g_pMeshTextures[i]);

		}
    }

    // Done with the material buffer
    pD3DXMtrlBuffer->Release();


	// Determino la extension del objeto
	D3DXVECTOR3 min,max;
	min = max = D3DXVECTOR3 (0,0,0);
	BYTE* pVertices=NULL; 
	g_pMesh->LockVertexBuffer(D3DLOCK_READONLY, (LPVOID*)&pVertices); 
	D3DXComputeBoundingBox((D3DXVECTOR3 *)pVertices,g_pMesh->GetNumVertices(),g_pMesh->GetNumBytesPerVertex(),&min,&max);
	g_pMesh->UnlockVertexBuffer(); 
	P0 = TVector3d(min.x,min.y,min.z);
	size = TVector3d(max.x,max.y,max.z) - P0;
	rot = TVector3d(0,0,0);

    return S_OK;
}





void DXMesh::Release()
{
    if( g_pMeshMaterials != NULL ) 
        delete[] g_pMeshMaterials;

    if( g_pMeshTextures )
    {
        for( DWORD i = 0; i < g_dwNumMaterials; i++ )
			SAFE_RELEASE(g_pMeshTextures[i]);
        delete[] g_pMeshTextures;
    }
    
	SAFE_RELEASE(g_pMesh);

	limpiar();

}


void DXMesh::DXRender(TVector3d Origen,TVector3d S,TVector3d Dir,TVector3d VUP)
{
	D3DXMATRIXA16 mat;
	CalcularMatriz(Origen,S,Dir,&mat,VUP);
	modelo->g_pd3dDevice->SetTransform( D3DTS_WORLD, &mat);
	modelo->g_pd3dDevice->SetTexture( 0, NULL);
	DXRender();
	// no restaura la matriz para ganar tiempo, que lo haga el caller al final de todo.
}


void DXMesh::DXRender()
{

	LPDIRECT3DDEVICE9 g_pd3dDevice = modelo->g_pd3dDevice;
	DWORD ant_zenable,ant_cullmode,ant_dmsource;
	g_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &ant_zenable);
	g_pd3dDevice->GetRenderState( D3DRS_CULLMODE, &ant_cullmode);
	g_pd3dDevice->GetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, &ant_dmsource);
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	// x defecto uso el color asociado al material
	g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
	//g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);


	// Calcula la transf. para ubicar, rotar y escalar el objeto 
	D3DXMATRIXA16 matWorld;
	//CalcularMatriz(Origen,S,R,&matWorld,pobj);
	//modelo->g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	///modelo->g_pd3dDevice->SetTexture( 0, NULL);
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
	
	LPDIRECT3DTEXTURE9 g_pTexture = NULL;			// textura actual

	for( int i=0; i<(int)g_dwNumMaterials; i++ )
    {
		if(g_pMeshMaterials)		
			g_pd3dDevice->SetMaterial(&g_pMeshMaterials[i] );

		if(g_pMeshTextures)
		{
			// si el modelo tiene textura, 
			//g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
			//g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
			

			g_pd3dDevice->SetTexture( 0, g_pTexture = g_pMeshTextures[i] );
		}

		if(modelo->hay_shader && modelo->g_pEffect)
		{
			// dibujo con efectos
			// seteo las texturas en el shadder
			modelo->g_pEffect->SetTexture( "g_Texture", g_pTexture);
			modelo->g_pEffect->SetBool("bTexture",g_pTexture?TRUE:FALSE);
			// idem con el material actual
			D3DMATERIAL9 aux_mat;
			g_pd3dDevice->GetMaterial(&aux_mat);
			modelo->g_pEffect->SetValue("matDiffuse", &aux_mat.Diffuse,sizeof(D3DCOLORVALUE));

			UINT cPasses;
			modelo->g_pEffect->Begin(&cPasses, 0);
			for(UINT pass = 0;pass<cPasses;++pass)
			{
				modelo->g_pEffect->BeginPass(0);
				g_pMesh->DrawSubset( i );
				modelo->g_pEffect->EndPass();
			}
			modelo->g_pEffect->End();
		}
		else
		g_pMesh->DrawSubset( i );

	}

	// restuaro los valores del render
	// Resturo el matWorld y z-buff
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, ant_zenable);
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, ant_cullmode);
	g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, ant_dmsource);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,TRUE);
	g_pd3dDevice->SetMaterial( &modelo->mtrl_std);

	// Restauro el Steam Source (me parece que DrawSubset lo cambia internamente)
	// (*) efectivamente lo cambia (ver nota en void DXMesh::DXRenderEdges(object *pobj)
	//modelo->g_pd3dDevice->SetStreamSource( 0, modelo->g_pVB, 0, sizeof(CUSTOMVERTEX) );
	//modelo->g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );

}




// Ubica y rota con respecto al centro de gravedad. 
void DXMesh::CalcularMatriz(TVector3d Origen,TVector3d S,TVector3d Dir,D3DXMATRIXA16 *matWorld,TVector3d VUP)
{

    D3DXMatrixIdentity(matWorld);

	// lo llevo al cero en el espacio del objeto pp dicho
	// La traslacion T0 hace que el centro del objeto quede en el cero
	// size = tamaño y P0 punto inicial, en coordenadas del objeto. 
	D3DXMATRIXA16 T0;
	D3DXMatrixTranslation(&T0,-P0.x-size.x/2,-P0.y-size.y/2,-P0.z-size.z/2);
	D3DXMatrixMultiply(matWorld,matWorld,&T0);

	// calculo la escala 
	D3DXMATRIXA16 Es;
	double kx = size.x && S.x?S.x/size.x:1;
	double ky = size.y && S.y?S.y/size.y:1;
	double kz = size.z && S.z?S.z/size.z:1;
	D3DXMatrixScaling(&Es,kx,ky,kz);
	D3DXMatrixMultiply(matWorld,matWorld,&Es);

	// determino la orientacion
	TVector3d U = VUP*Dir;
	U.normalizar();
	TVector3d V = Dir*U;
	D3DXMATRIXA16 Orientacion = 
				D3DXMATRIXA16(	
								U.x,		U.y,		U.z,		0,				
								V.x,		V.y,		V.z,		0,				
								Dir.x,		Dir.y,		Dir.z,		0,
								0,			0,			0,			1
								);
	D3DXMatrixMultiply(matWorld,matWorld,&Orientacion);

	// Lo traslado a la posicion final 
	D3DXMATRIXA16 T;
	D3DXMatrixTranslation(&T,Origen.x,Origen.y,Origen.z);
	D3DXMatrixMultiply(matWorld,matWorld,&T);


}

void DXMesh::SetColor(D3DCOLOR color)
{
	BYTE r =  (BYTE) ((color>>16) & 0xFF);
	BYTE g =  (BYTE) ((color>>8) & 0xFF);
	BYTE b =  (BYTE) (color&0xFF);

	for(int i=0;i<g_dwNumMaterials;++i)
	{
		g_pMeshMaterials[i].Ambient.r = g_pMeshMaterials[i].Diffuse.r = (float)r/255.;
		g_pMeshMaterials[i].Ambient.g = g_pMeshMaterials[i].Diffuse.g = (float)g/255.;
		g_pMeshMaterials[i].Ambient.b = g_pMeshMaterials[i].Diffuse.b = (float)b/255.;
		g_pMeshMaterials[i].Ambient.a = g_pMeshMaterials[i].Diffuse.a = 1.;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Motor pp dicho
/////////////////////////////////////////////////////////////////////////////

void DXEngine::Create()
{
	init = FALSE;

	g_pD3D = NULL;
	g_pd3dDevice = NULL;
	g_pVB = NULL;
	g_pVBFocos = NULL;
	g_pQuad = NULL;
	g_pEffect = NULL;			// D3DX effect interface
	hay_shader = FALSE;

	cant_bmp = 0;
	cant_mesh = 0;


	near_plane = 1;
	far_plane = 50000;
	aspect_ratio = 1;

}



// inicializa el Direct X
HRESULT DXEngine::DXInit( HWND hWnd)
{

	if(g_pD3D)
		return S_OK;

	if(hWnd==NULL)
		hWnd = AfxGetMainWnd()->m_hWnd;
	m_hWnd = hWnd;

	g_pVB = NULL;
	g_pVBFocos = NULL;
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
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);		
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
	g_pd3dDevice->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_LINEAR);

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

	// LUCES
	ZeroMemory( &light, sizeof(light) );
	light[0].Type = D3DLIGHT_DIRECTIONAL;
	light[0].Diffuse.r = 1;
	light[0].Diffuse.g = 1;
	light[0].Diffuse.b = 1;
	light[0].Specular.r = 1;
	light[0].Specular.g = 1;
	light[0].Specular.b = 1;
	light[0].Position = D3DXVECTOR3(0,500,0);
	light[0].Direction = D3DXVECTOR3(0,-1,0);
	light[0].Range = 500000;
	//light[0].Attenuation0 = 1.0f; 
	//light[0].Theta = 0.5;
	//light[0].Phi = 1;
	g_pd3dDevice->SetLight( 0, &light[0] );
	g_pd3dDevice->LightEnable( 0, TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE);
	g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_XRGB(128,128,128));		// LUZ AMBIENTE


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

	HRESULT hr = g_pd3dDevice->CreateTexture(sdx,sdy,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,&g_pTxLigthPath,NULL);
	if(FAILED(hr) )
		return hr;

	hr = g_pd3dDevice->CreateTexture(sdx,sdy,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,&g_pTxLigthPath2,NULL);
	if(FAILED(hr) )
		return hr;

	hr = g_pd3dDevice->CreateTexture(d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,&g_pRenderTarget,NULL);
	if(FAILED(hr) )
		return hr;

	hr = g_pd3dDevice->CreateTexture(d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,&g_pRenderTarget2,NULL);
	if(FAILED(hr) )
		return hr;
	hr = g_pd3dDevice->CreateTexture(d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,&g_pRenderTarget3,NULL);
	if(FAILED(hr) )
		return hr;
    
	// Experimento anti-aliasing
	// Cuando se hacer un render a una textura no se puede usar el anti-alias, con lo cual se complica la implementacion
	// de shaders que trabajan sobre texturas, para hacer las pasadas. 
	// La tecnica downsample, consiste en dibujar sobre un RenderTarget (que no es una textura) que si soporta multisample
	// y luego, copiar la imagen a la textura pp dicha (downsample) o bien al backbuffer (en ese caso fue al pedo, porque 
	// el backbuffer si soporta multisample), pero me ahorro tener que crear y limpiar el Device, o crear un nuevo
	// device para cambiar el paraemtro ddpresent.
	dxcaps_downsampling = TRUE;
	g_pTargetZ = g_pTargetB = NULL;
	if(FAILED(g_pd3dDevice->CreateDepthStencilSurface(d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,
		D3DFMT_D24S8,D3DMULTISAMPLE_2_SAMPLES,0,TRUE,&g_pTargetZ,NULL)))
		dxcaps_downsampling = FALSE;
	else
	if(FAILED(g_pd3dDevice->CreateRenderTarget(d3dpp.BackBufferWidth,d3dpp.BackBufferHeight,
		D3DFMT_X8R8G8B8,D3DMULTISAMPLE_2_SAMPLES,0,FALSE,&g_pTargetB,NULL)))
		dxcaps_downsampling = FALSE;
	dxcaps_downsampling = FALSE;


	
	return S_OK;
}




void DXEngine::DXCleanTextures()
{
	// Libera las Texturas
	for(int i=0;i<cant_texturas;++i)
		SAFE_RELEASE(g_pTexture[i]);
	cant_texturas = 0;
}

// Carga la textura si no esta, devuelve el nro de textura
int DXEngine::cargar_textura(char *filename,int K)
{
	if(K<=0)
		K = 1000;
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


HRESULT DXEngine::DXLoadTextures()
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
		{
			// abro el bmp comun y corriente
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


HRESULT SetAlphaChannel(LPDIRECT3DTEXTURE9  g_pTexture,BYTE r0,BYTE g0,BYTE b0)
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

				if(abs(b-b0)<4 && abs(g-g0)<4 && abs(r-r0)<4)		// es el mask transparente
					*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(0,r,g,b);
				else
					*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(255,r,g,b);

			}
		}
		g_pTexture->UnlockRect(i);
	}
    return S_OK;
}

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
				if(b<10 && g<10 && r<10)		// es el mask transparente
					*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(0,r,g,b);
				else
				*(((DWORD *)lockedRect.pBits)+(dwOffset+x)) = D3DCOLOR_ARGB(alpha,r,g,b);

			}
		}
		g_pTexture->UnlockRect(i);
	}
    return S_OK;
}



void DXEngine::DXSetTrans(double alpha)
{
	if(alpha)
	{
		// selecciono el material transparente y asingo el kt apropiado
		mtrl_tx.Diffuse.a = mtrl_tx.Ambient.a = 1-alpha;
		g_pd3dDevice->SetMaterial( &mtrl_tx);
	}
	else
	{
		// selecciono el material opaco
		g_pd3dDevice->SetMaterial( &mtrl_std);
	}

}


HRESULT DXEngine::LoadFx(char *fx_file)
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
void DXEngine::DXCleanup()
{

	if(g_pD3D==NULL)
		return;

	SAFE_RELEASE(pSprite);
	SAFE_RELEASE(ppLine);
    SAFE_RELEASE(g_pFont);
    SAFE_RELEASE(g_pFontb);
    SAFE_RELEASE(g_pFont2);

	// Clean Meshs
	for(int i=0;i<cant_mesh;++i)
		Mesh[i].Release();
	cant_mesh = 0;


	// borro el buffer de vertices
	SAFE_RELEASE(g_pVB);
	SAFE_RELEASE(g_pVBFocos);
	SAFE_RELEASE(g_pQuad);

	// LightPath
	SAFE_RELEASE( g_pTxLigthPath);
	SAFE_RELEASE( g_pTxLigthPath2);

	// Render Target
	SAFE_RELEASE( g_pRenderTarget);
	SAFE_RELEASE( g_pRenderTarget2);
	SAFE_RELEASE( g_pRenderTarget3);

	// Downsampling
	SAFE_RELEASE(g_pTargetZ);
	SAFE_RELEASE(g_pTargetB);

	// el dipositivo 
	SAFE_RELEASE(g_pd3dDevice );
	SAFE_RELEASE(g_pD3D);
	SAFE_RELEASE(g_pEffect);

}



HRESULT DXEngine::DXSetupRender()
{

	D3DXVECTOR3 vUpVec(0,1,0);
	D3DXMatrixIdentity(&matWorld);
	D3DXVECTOR3 vEyePt(LF.x,LF.y,LF.z);
	D3DXVECTOR3 vLookatPt(LA.x,LA.y,LA.z);
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, 
			(double)d3dpp.BackBufferWidth/(double)d3dpp.BackBufferHeight*aspect_ratio, near_plane, far_plane);

    g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	ant_LF = LF;
	ant_LA = LA;

	return S_OK;
}



// Render states
void DXEngine::SaveRenderStates()
{
	g_pd3dDevice->GetRenderState( D3DRS_ZENABLE, &ant_zenable);
	g_pd3dDevice->GetRenderState( D3DRS_CULLMODE, &ant_cullmode);
	g_pd3dDevice->GetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, &ant_dmsource);
	g_pd3dDevice->GetRenderState( D3DRS_ALPHABLENDENABLE,&ant_alpha);
}

void DXEngine::RestoreRenderStates()
{

	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, ant_zenable);
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, ant_cullmode);
	g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, ant_dmsource);
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,ant_alpha);

}


// pone el modo negro, para borrar cosas de la escane
void DXEngine::DXSetModoNegro()
{
	SaveRenderStates();
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE);
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE);
	// x defecto uso el color asociado al material
	g_pd3dDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
	// blend disable
	g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE,FALSE);
	g_pd3dDevice->SetMaterial( &mtrl_negro);
}

