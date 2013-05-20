//
//  TileViewViewController.m
//  TileView
//
//  Created by user on 19/07/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "TileViewViewController.h"
#import "EAGLView.h"
#include <sys/time.h>
#define COMPUTE_USECS(a) (a).tv_sec * 1000000 + (a).tv_usec

// Uniform index.
enum {
    UNIFORM_TRANSLATE,
    NUM_UNIFORMS
};
GLint uniforms[NUM_UNIFORMS];

// Attribute index.
enum {
    ATTRIB_VERTEX,
    ATTRIB_COLOR,
    NUM_ATTRIBUTES
};

@interface TileViewViewController ()
@property (nonatomic, retain) EAGLContext *context;
@end

@implementation TileViewViewController

@synthesize context;

- (void)awakeFromNib
{
    EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    
    if (!aContext)
        NSLog(@"Failed to create ES context");
    else if (![EAGLContext setCurrentContext:aContext])
        NSLog(@"Failed to set ES context current");
    
	self.context = aContext;
	[aContext release];
	
    [(EAGLView *)self.view setContext:context];
    [(EAGLView *)self.view setFramebuffer];
	
	// inicializacion
	// Texture binding
	glBindTexture(GL_TEXTURE_2D, 13);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	BITMAPINFOHEADER header;
	BYTE *image = [self LoadBitmap: @"atlas" header: &header];
	glTexImage2D( GL_TEXTURE_2D,0,GL_RGB,header.biWidth,header.biHeight,0,GL_RGB,GL_UNSIGNED_BYTE,image);
	free(image);
	glEnable (GL_TEXTURE_2D);


	// inicializo el motor 2d
	/*
	motor.Create();
	motor.Init(WINDOW_WIDTH, WINDOW_HEIGHT);
	motor.LoadLevel(12,12);
	motor.cargar_escenario("mapa.dat");
	origen_y = motor.pos_y = 31 * 12;
	origen_x = motor.pos_x = 380 * 12;
	*/


	

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(1)
	{
		// Apaisado
		glRotatef(-90.0f, 0, 0, 1);
		glOrthof(0, WINDOW_HEIGHT, 0,WINDOW_WIDTH ,0, 1);
		glH = WINDOW_WIDTH;
		glW = WINDOW_HEIGHT;
	}
	else 
	{
		glOrthof(0, WINDOW_WIDTH, 0,WINDOW_HEIGHT, 0, 1);
		glW = WINDOW_WIDTH;
		glH = WINDOW_HEIGHT;
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	// [self cargar_mapa];
	tile_cant_col = 512/12;
	step = 0;

	animating = FALSE;
	
	// Creo un timer
	timer = [NSTimer scheduledTimerWithTimeInterval:0.1
											 target:self
										   selector:@selector(refreshScreen)
										   userInfo:nil
											repeats:YES];
	
	struct timeval time;
	gettimeofday(&time,NULL); 
	cur_time = COMPUTE_USECS(time);
	
	elapsed_time = 0;
	cant_frames = 0;
	
}

- (void)dealloc
{
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
    
    [context release];
    
    [super dealloc];
}


- (void)viewDidUnload
{
	[super viewDidUnload];
  
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	self.context = nil;	
}

-(void)startAnimation
{
	animating = TRUE;
}

- (void)stopAnimation
{
	animating = FALSE;
}



-(void) refreshScreen
{
	if(!animating)
		return;
	
	NSLog(@"Refreshing screen!");
	// teoricamente 0.1 segundos, pero el timer no es muy preciso
	
	struct timeval time;
	gettimeofday(&time,NULL); 
	long lTime = COMPUTE_USECS(time);
	float dt = (float)(lTime - cur_time)/1000000.0;
	cur_time = lTime;
	[self updateFrame: dt];	
}

-(void) updateFrame:(CFTimeInterval)timeElapsed
{
	elapsed_time = timeElapsed;
	++cant_frames;
	[self drawFrame];	
}


- (void)drawFrame
{
    [(EAGLView *)self.view setFramebuffer];
	
	++step;
	NSLog(@"%d",step);
	
 	glClearColor(1,0,0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

	glBindTexture(GL_TEXTURE_2D, 13);
	glEnable (GL_TEXTURE_2D);

	/*
	int ox = 0;
	int oy = 0;
	int tile_dx = 12;
	int tile_dy = 12;
	int cant_fil = 25;
	int cant_col = 40;
	int atlas_dx = 512;
	int atlas_dy = 512;
	QUADVERTEX vertices[4000*2*3];
	float du = (float)tile_dy/(float)atlas_dx;
	float dv = (float)tile_dx/(float)atlas_dy;
	int pos_x = [(EAGLView *)self.view pos_x];
	int pos_y = [(EAGLView *)self.view pos_y];

	int x0 = pos_x;
	int y0 = pos_y;
	
	int j0 = x0/tile_dx;
	float desf_x = x0%tile_dx;
	int i0 = y0/tile_dy;
	float desf_y = y0%tile_dy;
	
	
	int t = 0;
	for(int i=0;i<cant_fil;++i)
	{
		for(int j=0;j<cant_col;++j)
			if(i0+i>=0 && i0+i<MAX_TILE_Y && j0+j>=0 && j0+j<MAX_TILE_X)
			{
				
				int n = C[i0+i][j0+j].nro_tile;
				int fil = n / tile_cant_col;
				int col = n % tile_cant_col;
				
				
				float u = (float)col*du;
				float v = (float)fil*dv;
				
				
				vertices[t].x = ox + j*tile_dx-desf_x;
				vertices[t].y = h - (oy + i*tile_dy-desf_y);
				vertices[t].z = 0;
				vertices[t].tu = u;
				vertices[t].tv = 1-v;
				++t;
				
				vertices[t].x = ox + (j+1)*tile_dx-desf_x;
				vertices[t].y = h - (oy +  i*tile_dy-desf_y);
				vertices[t].z = 0;
				vertices[t].tu = u+du;
				vertices[t].tv = 1-v;
				++t;
				
				vertices[t].x = ox + (j+1)*tile_dx-desf_x;
				vertices[t].y = h - (oy +  (i+1)*tile_dy-desf_y);
				vertices[t].z = 0;
				vertices[t].tu = u+du;
				vertices[t].tv = 1-(v+dv);
				++t;
				
				
				vertices[t] = vertices[t-3];
				vertices[t+1] = vertices[t-1];
				vertices[t+2].x = ox + j*tile_dx-desf_x;
				vertices[t+2].y = h -(oy + (i+1)*tile_dy-desf_y);
				vertices[t+2].z = 0;
				vertices[t+2].tu = u;
				vertices[t+2].tv = 1-(v+dv);
				t+=3;
				
			}
	}
	
	
	glVertexPointer(3, GL_FLOAT, sizeof(struct QUADVERTEX), vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(struct QUADVERTEX), &(vertices[0].tu));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, t);
	*/
    [(EAGLView *)self.view presentFramebuffer];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}


- (BYTE *)LoadBitmap: (NSString *)filename header:(BITMAPINFOHEADER *)header
{
	return [self LoadBitmap: filename header:header mask:-1 ds:-1];
}


- (BYTE *)LoadBitmap: (NSString *)filename header:(BITMAPINFOHEADER *)header
			mask: (COLORREF)mask ds: (BYTE)ds

{
	BYTE *image = NULL; 
	NSString *fileLocation = [[NSBundle mainBundle] pathForResource:filename
															 ofType:@"bmp"];
	NSData *image_data = [NSData dataWithContentsOfFile:fileLocation];
	
	BYTE *fp = (BYTE *)[image_data bytes];
	
	DWORD i,j;
	int bpc = 3;					// bytes x canal
	BYTE mask_r,mask_g,mask_b;		// mask
	if(mask!=-1)
	{
		mask_r = GetRValue(mask);
		mask_g = GetGValue(mask);
		mask_b = GetBValue(mask);
		bpc = 4;					// 4 bytes x canal: RGBA
	}

	DWORD fpos = 0;
	// Salteo el file hearder
	fpos += sizeof_BITMAPFILEHEADER;
	// Leo algunos datos del info header
	memcpy(header,fp+fpos,sizeof_BITMAPINFOHEADER);

	// solo soporta paleta de 24 bits
	if(header->biBitCount==24)
	{
		DWORD size = (header->biWidth+2)*(header->biHeight+2)*bpc+1;
		BYTE r,g,b,a;
		// Aloco memoria para los colores
		image = (BYTE *)malloc(size);
		// Leo los datos de los colores
		fpos =  54;
		int t;
		int resto;
		for(i=0;i<header->biHeight;++i)
		{
			t = 0;
			for(j=0;j<header->biWidth;++j)
			{
				r = fp[fpos++];
				g = fp[fpos++];
				b = fp[fpos++];

				if(bpc==4)
				{
					// canal alpha
					if(abs(b-mask_b)<ds && abs(g-mask_g)<ds && abs(r-mask_r)<ds)		// es el mask transparente
						a = 0;
					else
						a = 255;
				}

				t+=3;
				image[(DWORD)(i*bpc*header->biWidth+bpc*j)] = b;
				image[(DWORD)(i*bpc*header->biWidth+bpc*j+1L)] = g;
				image[(DWORD)(i*bpc*header->biWidth+bpc*j+2L)] = r;

				// Alpha channel
				if(bpc==4)
					image[(DWORD)(i*bpc*header->biWidth+bpc*j+3L)] = a;

			}
				
			// t tiene que ser multiplo de 32 bits, si no es asi
			// leo resto's butes hasta completar el limite
				
			if((resto = (t%4))>0 && resto<4)
				fpos+=4-resto;
		}
	}
	return image;
	
}

-(void) cargar_mapa
{
	memset(C,0,sizeof(C));
	
	NSString *fileLocation = [[NSBundle mainBundle] pathForResource:@"mapa" ofType:@"dat"];
	CFile *file = [CFile alloc];

	if([file open:fileLocation])
	{
		char buffer[255];
		[file readln:buffer];			// salteo el flag de mapa
		for(int i=0;i<MAX_TILE_Y && ![file eof];++i)
		{
			for(int j=0;j<MAX_TILE_X && ![file eof];++j)
			{
				[file readln:buffer];
				C[i][j].nro_tile = atoi(buffer);	

				[file readln:buffer];
				C[i][j].flags = atoi(buffer);	
				
				[file readln:buffer];
				C[i][j].tipo = atoi(buffer);	
			}
		}
		[file readln:buffer];		// teoricamente flat /mapa
		[file close];
	}
	[file dealloc];
		
}


- (void) Create
{
	init = FALSE;
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
	status_seg = P_STATUS_UNKNOWN;
	screen_i = screen_j = -1;
	vel_v_seg = vel_v;
	vel_h_seg = vel_h;

}


- (void) Init: (SIZE)s
{

	screen_dx = glW;
	screen_dy = glH;

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);

	// Habilito el mapeo de texturas
	glEnable (GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	// Seteo los parametros de la proyeccion 2d
	glViewport(0, 0, glW, glH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, glW, 0,glH, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// alloco toda la memoria necesaria para los quads 
	int cant_v = cant_fil*cant_col*2*3;
	int cant_bytes = sizeof(QUADVERTEX)*cant_v;
	screen_quad = (QUADVERTEX*) malloc(cant_bytes);
	memset(screen_quad,0,cant_bytes);

}


- (void) CleanUp
{
	// libero la memoria 
	if(screen_quad)
	{
		free(screen_quad);
		screen_quad = NULL;
	}
}

- (int) cargar_textura: (NSString *)filename mask: (COLORREF) mask ds: (BYTE)ds
{
	// primero busco que la textura no este repetida
	int rta = -1;
	
	/*
	rtrim(filename);
	int i =0;
	while(i<cant_bmp && rta==-1)
		if(strcmp(bmp_fname[i],filename)==0)
			rta = i;
		else
			++i;
	*/

	if(rta==-1)
	{
		[bmp_fname[cant_bmp] initWithString: filename];
		bmp_mask[cant_bmp] = mask;
		bmp_ds[cant_bmp] = ds;
		rta = cant_bmp++;
	}
	return rta;
}

- (int) cargar_textura: (NSString *)filename mask: (COLORREF) mask
{
	return [self cargar_textura: filename mask:mask ds:4];
}

- (int) cargar_textura: (NSString *)filename;
{
	// el -1 indica que es opaco
	return [self cargar_textura: filename mask:-1 ds:-1];
}

- (void) LoadTextures
{
	// Primero libero cualquier textura anterior
	[self CleanTextures];

	// Genero los nombres
	glGenTextures(cant_bmp,tx_id);

	// Voy a cargar las texturas
	for(int i=0;i<cant_bmp;++i)
	{

		// Texture binding
		glBindTexture(GL_TEXTURE_2D, tx_id[i]);

		// Cargo el bitmap en memoria
		BITMAPINFOHEADER header;
		BYTE *image = [self LoadBitmap: bmp_fname[i] header: &header 
								mask:bmp_mask[i] ds:bmp_ds[i]];

		GLint format = bmp_mask[i]==-1?GL_RGB:GL_RGBA;
		glTexImage2D( GL_TEXTURE_2D,0,format,header.biWidth,header.biHeight,0,
			format,GL_UNSIGNED_BYTE,image);
		free(image);

		bmp_size[i].cx = header.biWidth;
		bmp_size[i].cy = header.biHeight;

		// Parametros de textura
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// paso a la siguiente textura
		++cant_texturas;
	}

}

- (void) CleanTextures
{
	glDeleteTextures(cant_texturas, tx_id);
	cant_texturas = 0;
}


// tiles:
- (BOOL) LoadLevel: (char)tdx tdy: (char)tdy
{

	// Cargo el nivel:
	atlas = [self cargar_textura: @"atlas.bmp" mask:RGB(0,0,0)];
	// Cargo el atlas de sprites
	sprites = [self cargar_textura: @"sprites.bmp" mask:RGB(255,0,255)];
	// Animacion del fuego (implementado como otro sprite
	fuego = [self cargar_textura: @"fuego.bmp" mask:RGB(0,0,0) ds:140];
	fuego_dx = 30;
	fuego_dy = 60;

	// Cargo las texturas pp dichas
	[self LoadTextures];

	// tamaño de los tiles
	tile_dx = tdx;
	tile_dy = tdy;

	atlas_dx = bmp_size[atlas].cx;
	atlas_dy = bmp_size[atlas].cx;
	tile_cant_col = atlas_dx/tile_dx;
	tile_cant_fil = atlas_dy/tile_dy;

	// Tamaño de los sprites (de momento fijos)
	sprite_dx = 24;
	sprite_dy = 36;

	// esqueleto
	cant_vertebras = 0;
	vertebra[0].x = 0;
/*	
	vertebra[cant_vertebras++] = {7,0};			// Pie Izquierdo
	vertebra[cant_vertebras++] = POINT(15,0);		// Pie Derecho
	vertebra[cant_vertebras++] = POINT(11,10);		// tronco
	vertebra[cant_vertebras++] = POINT(11,15);		// Cuello
	vertebra[cant_vertebras++] = POINT(11,29);		// Cabeza
	vertebra[cant_vertebras++] = POINT(6,15);		// Hombro Izquierdo
	vertebra[cant_vertebras++] = POINT(16,15);		// Hombro Derecho
	vertebra[cant_vertebras++] = POINT(2,8);			// Mano Izquierda
	vertebra[cant_vertebras++] = POINT(19,8);		// Mano Derecha
*/

	// inicializo con todo el atlas 
	for(int i=0;i<MAX_TILE_Y;++i)
		for(int j=0;j<MAX_TILE_X;++j)
		{
			C[i][j].flags = 0;
			C[i][j].nro_tile = 0;
		}

	// user interface
	cant_ui_items = 0;
	int sp = 105;

	// panel izquierda:
	// mover izquierda
	ui_items[cant_ui_items].x = 0;
	ui_items[cant_ui_items].y = 270;
	ui_items[cant_ui_items].dx = 100;
	ui_items[cant_ui_items].dy = 50;
	ui_items[cant_ui_items].nro_sprite = sp;
	ui_items[cant_ui_items].key = '4';
	cant_ui_items++;

	// saltar / mover arriba
	ui_items[cant_ui_items].x = 50;
	ui_items[cant_ui_items].y = 220;
	ui_items[cant_ui_items].dx = 50;
	ui_items[cant_ui_items].dy = 50;
	ui_items[cant_ui_items].nro_sprite = sp + 3;
	ui_items[cant_ui_items].key = '8';
	cant_ui_items++;

	// saltar diagonal izquierda
	ui_items[cant_ui_items].x = 0;
	ui_items[cant_ui_items].y = 220;
	ui_items[cant_ui_items].dx = 50;
	ui_items[cant_ui_items].dy = 50;
	ui_items[cant_ui_items].nro_sprite = sp + 4;
	ui_items[cant_ui_items].key = '7';
	cant_ui_items++;

	// mover abajo
	ui_items[cant_ui_items].x = 100;
	ui_items[cant_ui_items].y = 270;
	ui_items[cant_ui_items].dx = 100;
	ui_items[cant_ui_items].dy = 50;
	ui_items[cant_ui_items].nro_sprite = sp + 2;
	ui_items[cant_ui_items].key = '2';
	cant_ui_items++;



	// panel derecha
	// mover derecha
	ui_items[cant_ui_items].x = 370;
	ui_items[cant_ui_items].y = 270;
	ui_items[cant_ui_items].dx = 100;
	ui_items[cant_ui_items].dy = 50;
	ui_items[cant_ui_items].nro_sprite = sp + 1;
	ui_items[cant_ui_items].key = '6';
	cant_ui_items++;

	// saltar / mover arriba
	ui_items[cant_ui_items].x = 370;
	ui_items[cant_ui_items].y = 220;
	ui_items[cant_ui_items].dx = 50;
	ui_items[cant_ui_items].dy = 50;
	ui_items[cant_ui_items].nro_sprite = sp + 3;
	ui_items[cant_ui_items].key = '8';
	cant_ui_items++;

	// saltar diaagonal derecha
	ui_items[cant_ui_items].x = 420;
	ui_items[cant_ui_items].y = 220;
	ui_items[cant_ui_items].dx = 50;
	ui_items[cant_ui_items].dy = 50;
	ui_items[cant_ui_items].nro_sprite = sp + 5;
	ui_items[cant_ui_items].key = '9';
	cant_ui_items++;

	// mover abajo
	ui_items[cant_ui_items].x = 280;
	ui_items[cant_ui_items].y = 270;
	ui_items[cant_ui_items].dx = 100;
	ui_items[cant_ui_items].dy = 50;
	ui_items[cant_ui_items].nro_sprite = sp + 2;
	ui_items[cant_ui_items].key = '2';
	cant_ui_items++;



    return TRUE;

}


- (BOOL) Render: (int)x0 y0:(int)y0 ex:(float)ex ey:(float)ey
{
	// Quad para postprocess 2d
	int ox = 0;
	int oy = 0;

//	int cant_v = cant_fil*cant_col*2*3;
    QUADVERTEX *vertices = screen_quad;

    float desf = 0;
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
	float h = screen_dy;

	if(j0!=screen_j || i0!=screen_i)
	{
		// cambio de pantalla, aprovecho para guardar la pos. segura
		pos_seg_x = pos_x;
		pos_seg_y = pos_y;
		status_seg = status;
		vel_v_seg = vel_v;
		vel_h_seg = vel_h;
	}
	screen_j = j0;
	screen_i = i0;

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

			int n = [self que_tile: i0+i j:j0+j];
			int fil = n / tile_cant_col;
			int col = n % tile_cant_col;

			float u = (float)col*du;
			float v = (float)fil*dv;

			vertices[t].x = ox + ex*j*tile_dx-desf_x;
			vertices[t].y = h - (oy + ey*i*tile_dy-desf_y);
			vertices[t].z = 0.0f;
			vertices[t].tu = u;
			vertices[t].tv = 1-v;
			++t;

			vertices[t].x = ox + ex*(j+1)*tile_dx-desf_x;
			vertices[t].y = h - (oy + ey*i*tile_dy-desf_y);
			vertices[t].z = 0.0f;
			vertices[t].tu = u+du;
			vertices[t].tv = 1-v;
			++t;

			vertices[t].x = ox + ex*(j+1)*tile_dx-desf_x;
			vertices[t].y = h-(oy + ey*(i+1)*tile_dy-desf_y);
			vertices[t].z = 0.0f;
			vertices[t].tu = u+du;
			vertices[t].tv = 1-(v+dv);
			++t;

			
			vertices[t] = vertices[t-3];
			vertices[t+1] = vertices[t-1];
			vertices[t+2].x = ox + ex*j*tile_dx-desf_x;
			vertices[t+2].y = h - (oy + ey*(i+1)*tile_dy-desf_y);
			vertices[t+2].z = 0.0f;
			vertices[t+2].tu = u;
			vertices[t+2].tv = 1-(v+dv);
			t+=3;
		}
	}


	// Borro la pantalla
    glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Selecciono la textura del atlas
	glBindTexture(GL_TEXTURE_2D, tx_id[atlas]);

	glVertexPointer(3, GL_FLOAT, sizeof( QUADVERTEX), vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof( QUADVERTEX), &(vertices[0].tu));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, t);

	
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

			[self XplodeSprite: (2-timer_quema)*0.05 x:x y:y nro_sprite:nro_sprite ex:ex ey:ey];

			[self RenderSprite: x y:y-sprite_dy/2*ey nro_sprite:(int)(timer_quema*16)
					ex:ex ey:ey dx:fuego_dx dy:fuego_dy atlas:fuego];
			
			[self RenderSprite: x y:y+fuego_dy*ey+sprite_dy/2*ey 
				nro_sprite:(int)(timer_quema*16) ex:ex ey:-ey 
				dx:fuego_dx dy:fuego_dy atlas:fuego];
		}
		else
		if(timer_cadena)
		{
			if(sentido==1)
				// Espejo X
				[self XplodeSprite: 1-timer_cadena/2 x:x+2*tile_dx*ex y:y
				nro_sprite:nro_sprite ex:-ex ey:ey];
			else
				[self XplodeSprite: 1-timer_cadena/2 x:x y:y
				nro_sprite:nro_sprite ex:ex ey:ey];
		}
		else
		{
			if(sentido==1)
				// Espejo X
				[self RenderSprite: x+2*tile_dx*ex y:y nro_sprite:nro_sprite ex:-ex ey:ey];
			else
				[self RenderSprite: x y:y nro_sprite:nro_sprite ex:ex ey:ey];
		}
	}

	// dibujo los enemigos
	for(int i=0;i<cant_enemigos;++i)
	{

		int x = ox + (enemigo[i].pos_x - j0*tile_dx)*ex;
		int y = oy + (enemigo[i].pos_y - (i0+3)*tile_dy)*ey;

		if(timer_choco && i==enemigo_sel)
			[self XplodeSprite: 2-timer_choco x:x y:y nro_sprite:enemigo[i].nro_sprite[0]
				ex:ex ey:ey];
		else
			[self RenderSprite: x y:y nro_sprite:enemigo[i].nro_sprite[enemigo[i].psprite]
				ex:ex ey:ey];
	}


	// Dibujo los fuegos
	if(cant_fuegos)
	{
		for(int i=0;i<cant_fuegos;++i)
		{
			int x = ox + (fuego_col[i]-j0)*tile_dx*ex - fuego_dx*0.3*ex;
			int y = oy + (fuego_fil[i]-i0+1)*tile_dy*ey - fuego_dy*0.52*ey;
			[self RenderSprite:x y:y nro_sprite:(frame_fuego + i*11)%33
				ex:ex ey:ey dx:fuego_dx dy:fuego_dy atlas:fuego];
		}
	}

	if(cant_items)
	{
		// dibujo los items coleccionables
		int x = ox + 10*ex;
		int y = oy + 2*ey;
		for(int i =0;i<cant_items;++i)
		{
			switch(items[i])
			{
				case TILE_LLAVE_BLANCA:
					[self RenderTile: x y:y dx:tile_dx*ex dy:tile_dy*ey sel:967];
					[self RenderTile: x+tile_dx*ex y:y dx:tile_dx*ex dy:tile_dy*ey sel:970];
					[self RenderTile: x y:y+tile_dy*ey dx:tile_dx*ex dy:tile_dy*ey sel:966];
					[self RenderTile: x+tile_dx*ex y:y+tile_dy*ey dx:tile_dx*ex dy:tile_dy*ey sel:969];
					break;
				case TILE_LLAVE_ROJA:
					[self RenderTile:x y:y dx:tile_dx*ex dy:tile_dy*ey sel:888];
					[self RenderTile:x+tile_dx*ex y:y dx:tile_dx*ex dy:tile_dy*ey sel:890];
					[self RenderTile:x y:y+tile_dy*ey dx:tile_dx*ex dy:tile_dy*ey sel:887];
					[self RenderTile:x+tile_dx*ex y:y+tile_dy*ey dx:tile_dx*ex dy:tile_dy*ey sel:889];
					break;
				case TILE_LLAVE_AZUL:
					[self RenderTile:x y:y dx:tile_dx*ex dy:tile_dy*ey sel:1012];
					[self RenderTile:x+tile_dx*ex y:y dx:tile_dx*ex dy:tile_dy*ey sel:1014];
					[self RenderTile:x y:y+tile_dy*ey dx:tile_dx*ex dy:tile_dy*ey sel:1011];
					[self RenderTile:x+tile_dx*ex y:y+tile_dy*ey dx:tile_dx*ex dy:tile_dy*ey sel:1013];
					break;
				case TILE_ESPADA:
					[self RenderTile:x y:y dx:tile_dx*ex dy:tile_dy*ey sel:975];
					[self RenderTile:x+tile_dx*ex y:y dx:tile_dx*ex dy:tile_dy*ey sel:978];
					[self RenderTile:x y:y+tile_dy*ey dx:tile_dx*ex dy:tile_dy*ey sel:974];
					[self RenderTile:x+tile_dx*ex y:y+tile_dy*ey dx:tile_dx*ex dy:tile_dy*ey sel:977];
					break;

			}

			x += (tile_dx+10)*ex;
		}
	}


	{
		// dibujo las vidas
		int x = ox + 360*ex;
		int y = oy + 2*ey;
		for(int i =0;i<cant_vidas;++i)
		{
			[self RenderSprite:x y:y nro_sprite:0 ex:ex ey:ey];
			x += 24*ex;
		}
	}

	// user interface
	for(i=0;i<cant_ui_items;++i)
	{
		int x = ox + (ui_items[i].x + (ui_items[i].dx-sprite_dx)/2 ) *ex;
		int y = oy + (ui_items[i].y + (sprite_dy-ui_items[i].dy)/2 ) *ey;
		[self RenderSprite: x y:y nro_sprite:ui_items[i].nro_sprite ex:ex ey:ey];
	}


	return TRUE;

}


- (BOOL) RenderTile: (int)x0 y:(int) y0 dx:(int)dx dy:(int)dy sel: (int)sel
{
    QUADVERTEX vertices[4];
    float desf = 0.5f;
	float du = (float)tile_dx / (float)atlas_dx;
	float dv = (float)tile_dy / (float)atlas_dy;

	int i = sel / tile_cant_col;
	int j = sel % tile_cant_col;

	float u = (float)j*du;
	float v = (float)i*dv;
	float h = screen_dy;


	vertices[0].x = x0-desf;
	vertices[0].y = h - (y0-desf);
	vertices[0].z = 0.0f;
	vertices[0].tu = u;
	vertices[0].tv = 1-v;

	vertices[1].x = x0+dx-desf;
	vertices[1].y = h - (y0-desf);
	vertices[1].z = 0.0f;
	vertices[1].tu = u+du;
	vertices[1].tv = 1-v;

	vertices[3].x = x0+dx-desf;
	vertices[3].y = h - (y0+dy-desf);
	vertices[3].z = 0.0f;
	vertices[3].tu = u+du;
	vertices[3].tv = 1-(v+dv);

	vertices[2].x = x0-desf;
	vertices[2].y = h - (y0+dy-desf);
	vertices[2].z = 0.0f;
	vertices[2].tu = u;
	vertices[2].tv = 1-(v+dv);


	// Selecciono la textura del atlas
	glBindTexture(GL_TEXTURE_2D, tx_id[atlas]);

	glVertexPointer(3, GL_FLOAT, sizeof( QUADVERTEX), vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof( QUADVERTEX), &(vertices[0].tu));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	return TRUE;

}

// Devuelve el tile correspondiente a la fila i, col j, teniendo en cuenta si hay animacion
- (int) que_tile: (int) i j:(int)j
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

// Escenario
- (void) cargar_escenario: (NSString *) fname
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
	int sp[] = {63,64,65,66,67};
	POINT pt = {372*tile_dx-1,45*tile_dy-1};
	[enemigo[0] Create: self withPos: pt withCantSprites:5 withSprites: sp];

	cant_enemigos++;

	// cargo el mapa pp dicho
	[self cargar_mapa: fname];
}


- (void) cargar_mapa: (NSString *) fname
{
	memset(C,0,sizeof(C));
	
	NSString *fileLocation = [[NSBundle mainBundle] pathForResource:fname ofType:@"dat"];
	CFile *file = [CFile alloc];

	if([file open:fileLocation])
	{
		char buffer[255];
		[file readln:buffer];			// salteo el flag de mapa
		for(int i=0;i<MAX_TILE_Y && ![file eof];++i)
		{
			for(int j=0;j<MAX_TILE_X && ![file eof];++j)
			{
				[file readln:buffer];
				C[i][j].nro_tile = atoi(buffer);	

				[file readln:buffer];
				C[i][j].flags = atoi(buffer);	
				
				[file readln:buffer];
				C[i][j].tipo = atoi(buffer);	
			}
		}
		[file readln:buffer];		// teoricamente flat /mapa
		[file close];
	}
	[file dealloc];

	
	// Post proceso los tiles:
	// 1era pasada: determino el flag y el tipo de tiple
	for(int i=0;i<MAX_TILE_Y;++i)
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
	for(int i=0;i<MAX_TILE_Y;++i)
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



// Sprites
- (BOOL) RenderSprite: (int) x0 
					y:(int) y0 nro_sprite:(int)nro_sprite
					ex: (float)ex ey:(float) ey 
					dx: (int)dx dy:(int) dy atlas:(int) atlas
{
	// tomo los parametros x defecto
	int atlas_dx = 512;
	int atlas_dy = 512;
	if(dx==-1)
		dx = sprite_dx;
	if(dy==-1)
		dy = sprite_dy;
	if(atlas==-1)
		atlas = sprites;
	else
		atlas_dx = 256;


    QUADVERTEX vertices[4];
    float desf = 0.5f;
	float du = (float)dx / (float)atlas_dx;
	float dv = (float)dy / (float)atlas_dy;
	int cant_col = atlas_dx / dx;

	int i = nro_sprite / cant_col;
	int j = nro_sprite % cant_col;

	float u = (float)j*du;
	float v = (float)i*dv;

	float h = screen_dy;

	vertices[0].x = x0-desf;
	vertices[0].y = h - (y0-desf);
	vertices[0].z = 0.0f;
	vertices[0].tu = u;
	vertices[0].tv = 1 -v;

	vertices[1].x = x0+sprite_dx*ex-desf;
	vertices[1].y = h - (y0-desf);
	vertices[1].z = 0.0f;
	vertices[1].tu = u+du;
	vertices[1].tv = 1 -v;
	
	vertices[3].x = x0+sprite_dx*ex-desf;
	vertices[3].y = h - (y0+sprite_dy*ey-desf);
	vertices[3].z = 0.0f;
	vertices[3].tu = u+du;
	vertices[3].tv = 1- (v+dv);

	vertices[2].x = x0-desf;
	vertices[2].y = h - (y0+sprite_dy*ey-desf);
	vertices[2].z = 0.0f;
	vertices[2].tu = u;
	vertices[2].tv = 1-(v+dv);


	// Selecciono la textura correspondiente (atlas, sprites o fuego de momento)
	glBindTexture(GL_TEXTURE_2D, tx_id[atlas]);

	glVertexPointer(3, GL_FLOAT, sizeof(QUADVERTEX), vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(QUADVERTEX), &(vertices[0].tu));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	return TRUE;
}



- (BOOL) XplodeSprite: (float)elapsed_time
					x: (int) x0 y:(int) y0 
					nro_sprite:(int)nro_sprite
					ex: (float)ex ey:(float) ey
					dx: (int)dx dy:(int) dy atlas:(int) atlas
{


	float an = elapsed_time*2;
	elapsed_time = sin(elapsed_time);
	// tomo los parametros x defecto
	int atlas_dx = 512;
	int atlas_dy = 512;
	if(dx==-1)
		dx = sprite_dx;
	if(dy==-1)
		dy = sprite_dy;
	if(atlas==-1)
		atlas = sprites;
	else
		atlas_dx = 256;


	int cant_part = 24;
    QUADVERTEX *vertices = screen_quad;
    float desf = 0.5f;
	float du = (float)dx / (float)atlas_dx;
	float dv = (float)dy / (float)atlas_dy;
	int cant_col = atlas_dx / dx;

	int i = nro_sprite / cant_col;
	int j = nro_sprite % cant_col;

	float u = (float)j*du;
	float v = (float)i*dv;

	float h = screen_dy;

	/*
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


			vertices[t].x = X0;
			vertices[t].y = h - Y0;
			vertices[t].z = 0.0f;
			vertices[t].tu = u + su*du;
			vertices[t].tv = 1- (v + sv*dv);
			++t;


			vertices[t].x = X1;
			vertices[t].y = h - Y0;
			vertices[t].z = 0.0f;
			vertices[t].tu = u + (su+su1)*du;
			vertices[t].tv = 1- (v + sv*dv);
			++t;

			
			vertices[t].x = X1;
			vertices[t].y = h - Y1;
			vertices[t].z = 0.0f;
			vertices[t].tu = u + (su+su1)*du;
			vertices[t].tv = 1- (v + sv*dv);
			++t;

			vertices[t] = vertices[t-3];
			vertices[t+1] = vertices[t-1];


			vertices[t+2].x = X0;
			vertices[t+2].y = h - Y1;
			vertices[t+2].z = 0.0f;
			vertices[t+2].tu = u + su*du;
			vertices[t+2].tv = 1 - (v + (sv+sv1)*dv);

			t+=3;

		}
	 */

	glBindTexture(GL_TEXTURE_2D, tx_id[atlas]);

	glVertexPointer(3, GL_FLOAT, sizeof(QUADVERTEX), vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(QUADVERTEX), &(vertices[0].tu));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, 2*cant_part*cant_part);


	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// Fisica basica
/////////////////////////////////////////////////////////////////////////////

- (BOOL) colision: (POINT) p0 p1:(POINT) p1
{
	coli_mask = 0;
	item_collected = -1;
	int dx = p1.x-p0.x;
	int dy = p1.y-p0.y;

	if(!dx && !dy)
		return FALSE;		// no se movio

	BOOL rta = FALSE;

	// guardo el pto actual: 
	//int pos_j = coli_j = [self nearest_x:Ip.x = p0.x];
	//int pos_i = coli_i = [self nearest_y:Ip.y = p0.y];


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
				int tj = [self nearest_x:px];
				int ti = [self nearest_y:py];


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
				int tj = [self nearest_x:px];
				int ti = [self nearest_y:py];

				// primero proceso la pos intermedia: esto permite recoger
				// objetos y abrir puertas y demas en el camino de la colision
				[self procesar_posicion:ti tj:tj];

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
				int tj = [self nearest_x:px];
				int ti = [self nearest_y:py];

				// primero proceso la pos intermedia: esto permite recoger
				// objetos y abrir puertas y demas en el camino de la colision
				[self procesar_posicion:ti tj:tj];

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
			int tj = [self nearest_x:x + sprite_dx/2];
			int ti = [self nearest_y:y];

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
- (void) procesar_posicion: (int) ti tj:(int) tj
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
		char rta = [self tiene_item:llave];
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
			[self borrar_item:rta];
		}
	}

}


- (void) Update: (float) elapsed_time
{
	ftime += elapsed_time;
	[self UpdateScene:elapsed_time];

	// timers 
	if(timer_caida>0)
	{
		timer_caida -= elapsed_time;
		if(timer_caida<0)
		{
			timer_caida = 0;
			[self morir];
		}
		return;
	}

	if(timer_cadena>0)
	{
		timer_cadena-= elapsed_time;
		if(timer_cadena<0)
		{
			timer_cadena = 0;
			[self morir];
		}
		return;
	}


	if(timer_choco>0)
	{
		timer_choco -= elapsed_time;
		if(timer_choco<0)
		{
			timer_choco = 0;
			[self morir];
			// Tambien muere el enemigo
			[self matar_enemigo:enemigo_sel];
		}
		return;
	}

	if(timer_quema>0)
	{
		timer_quema -= elapsed_time;
		if(timer_quema<0)
		{
			timer_quema = 0;
			[self morir];
		}
		return;
	}


	int ant_pos_x = pos_x;
	int ant_pos_y = pos_y;
	int ant_pos_j = [self nearest_x:pos_x];
	int ant_pos_i = [self nearest_y:pos_y];

	if(flag_tubo && ![self esta_en_tubo] && ![self esta_en_soga])
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
		int pos_i_sig = [self nearest_y: pos_y+4];

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
		if(![self esta_en_escalera])
			status = P_STATUS_UNKNOWN;		// salio de la escalera
	}
	else
	if(status==P_EN_TUBO)
	{
		// si estaba en la tubo: verifico si sigue estando?
		if(![self esta_en_tubo])
			status = P_STATUS_UNKNOWN;		// salto del tubo
	}
	else
	if(status==P_EN_SOGA)
	{
		// si estaba en la soga : verifico si sigue estando?
		if(![self esta_en_soga])
			status = P_STATUS_UNKNOWN;		// salto de la soga
	}


	// mov. relativo (al pto anterior)
	//int dx = pos_x - ant_pos_x;
	//int dy = pos_y - ant_pos_y;


	// Verifico las colisiones en la nueva posicion
	POINT p0 = {ant_pos_x,ant_pos_y};
	POINT p1 = {pos_x,pos_y};
	
	if([self colision:p0  p1:p1])
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
				vel_v = 0;
				status = P_SOBRE_PISO;
			}

		}

		// verifico si esta sobre el fuego
		if(C[coli_i-1][coli_j].tipo==TILE_FUEGO || C[coli_i-1][coli_j+1].tipo==TILE_FUEGO)
		{
			timer_quema = 2;
			timer_caida = 0;		// la animacion de quemar tiene prioridad
		}

	}


	if(item_collected!=-1 && cant_items<10)
	{
		// paso por arriba de un item pickable
		[self recoger_item:item_i j:item_j];
	}


	// Update de enemigos
	for(int i=0;i<cant_enemigos;++i)
	{
		[enemigo[i] Update:elapsed_time];
		// verifico si colisiona contra el enemigo
		POINT pt = {pos_x,pos_y};
		if([enemigo[i] colision: pt])
		{
			timer_choco = 2;
			enemigo_sel = i;		// dejo guardo que enemigo fue el que me estrolo
			break;
		}
	}

}


- (void) UpdateScene: (float) elapsed_time;
{
	// actualizo los tiles intermitentes
	// por ahora hay menos tiles intermitentes que tiles x pantalla
	// con lo cual conviene actualizar todos los tiles, en lugar de recorrer por pantalla
	// y actualizar solo los que estoy viendo. 
	// (***) Hay que seguir analizando cuando este  terminado el juego, si sigue cierto.

	int pos_j = [self nearest_x:pos_x];
	int pos_i = [self nearest_y:pos_y];

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


- (void) Move: (int) dx dy:(int)dy
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
		int pos_j = [self nearest_x:pos_x+12];
		int pos_i = [self nearest_y:pos_y];
		if(C[pos_i+1][pos_j].tipo==TILE_ESCALERA)
			return;
		else
		{
			status = P_SOBRE_PISO;		// salio de la escalera
			pos_y = (pos_i+1) * tile_dy - 1;  
		}


	}

	// Verifico las colisiones en la nueva posicion
	POINT p0 = {pos_x,pos_y};
	POINT p1 = {pos_x+dx,pos_y+dy};
	if(![self colision:p0 p1:p1] )
	{
		pos_x+=dx;
		pos_y+=dy;


		// verifico la nueva posicion
		if(status==P_SOBRE_PISO && dx)
		{
			// Si estaba sobre el piso, y se movio para la derecha o para la izquierda
			// se puede caer, (si no hay mas piso en el nuevo lugar)
			if(![self esta_sobre_piso])
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
- (void) Bajar: (int) dy
{

	// Verifico las colisiones en la nueva posicion
	POINT p0 = {pos_x,pos_y};
	POINT p1 = {pos_x,pos_y+dy};
	if([self colision:p0 p1:p1])
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


// Falta: si hay items juntos, uno al lado de otro sin separacion,
// se confunde los tiles. Eso es porque cada item tiene 4 tiles, y deberia guardar
// que esquina es cada tile. Por ejemplo  
//			(1) (2)
//			(3) (4)
// Entonces si toca contra la esquina (3), sabe que tiene borrar arriba y a la derecha

- (void) recoger_item: (int) ti j:(int) tj

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



- (char) tiene_item: (char) item
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
- (void) borrar_item: (int) i
{
	int t;
	for(t=i;t<cant_items;++t)
		items[t] = items[t+1];
	items[t] = 0;
	cant_items--;
}


- (void) matar_enemigo: (int) i
{
	for(int t=i;t<cant_enemigos-1;++t)
		enemigo[t] = enemigo[t+1];
	cant_enemigos--;
}

// Descuenta una vida, y lleva al personaje a un lugar seguro
- (void) morir;					
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
	status = status_seg;
	vel_v = vel_v_seg;
	vel_h = vel_h_seg;
}

// Helpers relacion con el escenario
- (BOOL) esta_sobre_piso
{
	BOOL rta = FALSE;
	// pos_x,pos_y es la esquina inferior izquierda
	// nearest point: 
	int pos_j = [self nearest_x:pos_x+vertebra[0].x];
	int pos_i = [self nearest_y:pos_y+vertebra[0].y+1];
	int mask = F_PISO | F_PIEDRA;
	if(C[pos_i][pos_j].flags&mask)
		rta = TRUE;		// el pie izquierdo apoya
	else
	{
		pos_j = [self nearest_x:pos_x+vertebra[1].x];
		pos_i = [self nearest_y:pos_y+vertebra[1].y+1];
		if(C[pos_i][pos_j].flags&mask)
			rta = TRUE;		// el pie derecho
	}

	return rta;
}


- (BOOL) esta_en_escalera: (int) x y:(int) y;
{
	BOOL rta = FALSE;
	int pos_j = [self nearest_x:x+6];
	int pos_i = [self nearest_y:y];
	// todo el cuerpo tiene que estar en la escalera
	if(C[pos_i][pos_j].tipo==TILE_ESCALERA)
	{
		pos_j = [self nearest_x:x+sprite_dx-6];
		if(C[pos_i][pos_j].tipo==TILE_ESCALERA)
			rta = TRUE;
	}
	return rta;
}


- (BOOL) esta_en_escalera;
{
	return [self esta_en_escalera:pos_x y:pos_y];
}


- (BOOL) esta_en_soga: (int) x y:(int) y;
{
	BOOL rta = FALSE;
	int pos_j = [self nearest_x:x+sprite_dx/2];
	int pos_i = [self nearest_y:y];
	if(C[pos_i][pos_j].tipo == TILE_SOGA)
		rta = TRUE;
	else
	{
		// todavia puede quedar colgado con las manos de la soga
		pos_i = [self nearest_y:y-sprite_dy+6];
		if(C[pos_i][pos_j].tipo == TILE_SOGA)
			rta = TRUE;
	}

	return rta;
}


- (BOOL) esta_en_soga;
{
	return [self esta_en_soga:pos_x y:pos_y];
}


- (BOOL) esta_en_tubo: (int) x y:(int) y;
{
	BOOL rta = FALSE;
	int pos_j = [self nearest_x:x+sprite_dx/2];
	int pos_i = [self nearest_y:y];
	if(C[pos_i][pos_j].tipo == TILE_TUBO)
		rta = TRUE;		// el pie izquierdo apoya
	return rta;
}


- (BOOL) esta_en_tubo;
{
	return [self esta_en_tubo:pos_x y:pos_y];
}


- (char *)que_status;

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


// Input Process

// devuelve el nro de ui sobre el que toca, o bien -1 si no toca contra nada
- (int) toca_sobre: int x y:int y
{
	int rta = -1;
	int i =0;
	while(i<cant_ui_items && rta==-1)
		if(x>=ui_items[i].x && x<=ui_items[i].x+ui_items[i].dx 
			&& y>=ui_items[i].y && y<=ui_items[i].y+ui_items[i].dy)
			rta = i;
		else
			++i;

	return rta;
}


- (BOOL) ProcessTouch: x y:int y
{

	BOOL redraw = FALSE;
	// verifico si toco sobre algun elemento de ui
	int ui_sel = [self toca_sobre: x y:y];
	if(ui_sel!=-1)
		// Sintetizo el comando
		redraw = [self OnChar: ui_items[ui_sel].key nRepCnt:1 nFlags:0];
	return redraw;
}


- (BOOL) OnChar: (int)nChar nRepCnt:(int)nRepCnt nFlags:(int)nFlags
{

	BOOL redraw = FALSE;
	switch(nChar)
	{

		case '4':
			flag_tubo = [self esta_en_tubo]?TRUE:FALSE;
			Move(-4);
			redraw = TRUE;
			break;
		case '6':
			flag_tubo = [self esta_en_tubo]?TRUE:FALSE;
			Move(4);
			redraw = TRUE;
			break;
		case '8':
			if([self esta_en_escalera])
			{
				// sube la escalera
				if(status!=P_EN_ESCALERA)
				{
					// recien entra a la escalera
					status = P_EN_ESCALERA;
					sprite_sel = 0;
					sentido = 0;
					pos_y -= 10;
					vel_h = 0;

				}
				else
					sprite_sel++;
				pos_y -= 4;

				// verifico si sigue en la escalera: 
				if(![self esta_en_escalera])
				{
					status = P_SOBRE_PISO;		// salio de la escalera
					// me posiciono justo sobre el piso. (Es el tile de arriba de la escalera)
					int pos_i = [self nearest_y:pos_y];
					pos_y = (pos_i+1) * tile_dy - 1;  
				}

				redraw = TRUE;
			}
			else
			if(status==P_EN_SOGA)
			{
				sprite_sel++;
				pos_y -= 4;
				// verifico si sigue en la soga (tomo la parte de arriba del sprite)
				if(![self esta_en_soga])
				{
					status = P_SOBRE_PISO;		// salio de la escalera
					// me posiciono justo sobre el piso. (Es el tile de arriba de la soga)
					int pos_i = [self nearest_y:pos_y];
					pos_y = (pos_i+1) * tile_dy - 1;  
				}
				redraw = TRUE;
			}
			else
			if(status==P_SOBRE_PISO || status==P_SOBRE_CINTA)
			{
				// salta
				vel_v = -150;
				status = P_SALTANDO;
				redraw = TRUE;
			}
			break;

		case '2':
			{

				int pos_j = [self nearest_x:pos_x+sprite_dx/2];
				int pos_i = [self nearest_y:pos_y];
				if([self esta_en_escalera:pos_x y:pos_y + 4])
				{
					// si el tile de abajo, es una escalera, puede empezar a bajar
					// o continuar bajando la escalera: 
					[self Bajar];
					sprite_sel++;
					status = P_EN_ESCALERA;
					redraw = TRUE;
				}
				else
				if([self esta_en_soga:pos_x y:pos_y + 4])
				{
					// si el tile de abajo, es una soga , puede empezar a bajar
					// o continuar bajando la soga: 
					[self Bajar];
					sprite_sel++;
					status = P_EN_SOGA;
					redraw = TRUE;
				}
				else
				if(status==P_EN_SOGA)
				{
					// si esta en la soga
					[self Bajar];
					sprite_sel++;
					status = P_EN_SOGA;
					// verifico si sigue en la soga: 
					pos_i = [self nearest_y:pos_y];
					if(![self esta_en_soga])
					{
						status = P_STATUS_UNKNOWN;		// salio de la soga
						pos_y = (pos_i+1) * tile_dy - 1;  
					}

					redraw = TRUE;
				}
				else
				if(status == P_EN_ESCALERA)
				{
					// si el tile de abajo no es una escalera, el personaje
					// esta en la en la escalera, lo hago salir de la misma
					// (es el ultimo escalon)
					status = P_SOBRE_PISO;		// salio de la escalera
					pos_y = (pos_i+1) * tile_dy - 1;  
					redraw = TRUE;
				}

			}
			break;


		case '9':
			if(status==P_SOBRE_PISO || status==P_SOBRE_CINTA 
				|| status==P_EN_TUBO || status==P_EN_SOGA)
			{
				// salto derecha
				flag_tubo = status==P_EN_TUBO || status==P_EN_SOGA?TRUE:FALSE;
				vel_v = -150;
				vel_h = 100;
				if(status==P_SOBRE_CINTA)
					vel_h += vel_cinta;
				status = P_SALTANDO;
				sentido = 0;
				redraw = TRUE;
			}
			break;

		case '7':
			if(status==P_SOBRE_PISO || status==P_SOBRE_CINTA 
				|| status==P_EN_TUBO || status==P_EN_SOGA)
			{
				// salto izquierda
				flag_tubo = status==P_EN_TUBO || status==P_EN_SOGA?TRUE:FALSE;
				vel_v = -150;
				vel_h = -100;
				if(status==P_SOBRE_CINTA)
					vel_h += vel_cinta;
				status = P_SALTANDO;
				sentido = 1;
				redraw = TRUE;
			}
			break;
	}

	return redraw;
}


@end

	
@interface CFile()
@end
	
@implementation CFile
	
- (BOOL) open: (NSString *)fileLocation
{
	BOOL rta = FALSE;
	cant_bytes = fp = 0;
	NSError *error;
	file  = [[NSString alloc] initWithContentsOfFile:fileLocation
													  encoding:NSUTF8StringEncoding
														 error:&error];
	if(file!=nil)
	{
		cant_bytes = [file length];
		rta = TRUE;
	}
	
	return rta;
	
}
	
- (void) close
{
	[file dealloc];
}

- (BOOL) eof
{
	return fp>=cant_bytes?TRUE:FALSE;
}

- (void) readln: (char *)buffer
{
	BOOL mas = TRUE;
	int t = 0;
	while(fp<cant_bytes && mas)
	{
		int c = [file characterAtIndex:fp];
		++fp;
		if(c=='\r') 
		{
			++fp;
			buffer[t] = 0;
			mas = FALSE;
		}
		else
			buffer[t++] = c;
	}
}


@end

@interface CEnemigo()
@end

@implementation CEnemigo



- (void) Create: (TileViewViewController *)m withPos: (POINT )pt withCantSprites: (int)cant withSprites: (int*)sp
{

	motor = m;
	pos_ini_x = pos_x = pt.x;
	pos_ini_y = pos_y = pt.y;
	pos_fin_x = pos_ini_x + 200;
	pos_fin_y = pos_ini_y;
	
	cant_sprites = cant;
	for(int i=0;i<cant_sprites;++i)
		nro_sprite[i] = sp[i];
	psprite = 0;
	vel_v = 50;
	vel_h = 0;
	sprite_frame = 0.1;
	timer_ani = 0;
	
}

- (BOOL) colision: (POINT) pt
{
	int sprite_dx = 24;		// [motor sprite_dx]
	int sprite_dy = 36;		// [motor sprite_dx]
	BOOL rta = FALSE;
	if(abs(pt.x-pos_x)< sprite_dx/2 && abs(pt.y-pos_y)< sprite_dy/2)
		rta = TRUE;
	
	return rta;
}



- (void) Update: (float) elapsed_time
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

@end





