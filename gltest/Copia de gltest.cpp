#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>

#include "C:\Archivos de programa\NVIDIA Corporation\SDK 9.5\inc\GL\glut.h"

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   700
#define WINDOW_X        0
#define WINDOW_Y        0
#define WINDOW_TITLE    "Open GL Test"

#define FOV_ANGLE       30

#define CENTER_X        0.0
#define CENTER_Y        0.0
#define CENTER_Z        0.0

#define VIEWER_X        0.0
#define VIEWER_Y        0.0
#define VIEWER_Z        -2.1

#define UP_X            0.0
#define UP_Y            1.0
#define UP_Z            0.0

#define CLIPPLANE_NEAR  1.0
#define CLIPPLANE_FAR   20.0


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif



GLfloat rot_x = 0.0, rot_y = 0.0;
GLfloat saved_x, saved_y;


LARGE_INTEGER F,T0,T1;   // address of current frequency
int cant_frames = 0;
float frame_time =0;
GLfloat w,h;


struct cell
{
	int nro_tile;
	int flags;
	char tipo;
};

#define MAX_TILE_X		800
#define MAX_TILE_Y		250
cell C[MAX_TILE_Y][MAX_TILE_X];
int tile_cant_col = 512/12;
int pos_y = 31*12;
int pos_x = 380*12;

void cargar_mapa();
void MainLoop();
void draw_scene(void);
BYTE *LoadBitmap(char *filename,BITMAPINFOHEADER *header);


void output(GLfloat x, GLfloat y, char *text)
{
	char *p;
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(0.2, 0.2, 1);
	for (p = text; *p; p++)
	  glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
	glPopMatrix();
}



int main(int argc, char **argv) 
{
    
	// inicializo el OPENGL
	glutInit(&argc, argv);
    // Create a window 
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(WINDOW_X, WINDOW_Y);
    glutCreateWindow(WINDOW_TITLE);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH);

	// Texture binding
	glBindTexture(GL_TEXTURE_2D, 13);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	BITMAPINFOHEADER header;
	BYTE *image = LoadBitmap("atlas.bmp",&header);
	glTexImage2D( GL_TEXTURE_2D,0,3,header.biWidth,header.biHeight,0,GL_RGB,GL_UNSIGNED_BYTE,image);
	delete image;
	glEnable (GL_TEXTURE_2D);

	w = WINDOW_WIDTH;
	h = WINDOW_HEIGHT;

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0,h, 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	cargar_mapa();

	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);


    glutDisplayFunc(MainLoop);
    //glutMouseFunc(save_position);
    //glutMotionFunc(rotate);
    glutMainLoop();

    /* Not reached */
    return 0;
}


void MainLoop()
{
	// redibujo:
	draw_scene();


	QueryPerformanceCounter(&T1);
	float elapsed_time = (double)(T1.LowPart - T0.LowPart) / (double)F.LowPart;
	T0 = T1;

/*
	char buffer[255];
	++cant_frames;
	frame_time += elapsed_time;
	if(frame_time)
	{
		float fps = (float)cant_frames/frame_time;
		sprintf(buffer,"FPS: %.2f",fps);
		glColor3f(0,0,0);
		output(0,0,buffer);
		glColor3f(1,1,1);

		if(frame_time>1)
		{
			frame_time = 0;
			cant_frames = 0;
		}
	}
*/

	// Proceso la entrada
	if(GetAsyncKeyState(VK_LBUTTON))
	{
		POINT pt;
		GetCursorPos(&pt);
		pos_x -= pt.x - saved_x;
		pos_y -= pt.y - saved_y;
		saved_x = pt.x;
		saved_y = pt.y;
	}

}

void draw_scene() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, 13);

	int ox = 10;
	int oy = 10;
	int tile_dx = 12;
	int tile_dy = 12;
	int cant_fil = 25;
	int cant_col = 40;
	int atlas_dx = 512;
	int atlas_dy = 512;
	int cant_v = cant_fil*cant_col*2*3;
	int size_vert = sizeof(QUADVERTEX)*cant_v;
    QUADVERTEX vertices[4000*2*3];
	float du = (float)tile_dy/(float)atlas_dx;
	float dv = (float)tile_dx/(float)atlas_dy;

	int x0 = pos_x;
	int y0 = pos_y;

	int j0 = x0/tile_dx;
	float desf_x = x0%tile_dx;
	int i0 = y0/tile_dy;
	float desf_y = y0%tile_dy;


	int t = 0;
	for(int i=0;i<cant_fil*2;++i)
	{
		for(int j=0;j<cant_col*2;++j)
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



    glutSwapBuffers();
    glutPostRedisplay();

}



BYTE *LoadBitmap(char *filename,BITMAPINFOHEADER *header)
{
	BYTE *image = NULL;
	int maxcolor;
	RGBQUAD paleta[256];
	long importancia[256];

	DWORD i,j;
	FILE *fp = fopen(filename,"rb");
	if(!fp)
		return image;
	
	int s1 = sizeof(BITMAPFILEHEADER);
	int s2 = sizeof(BITMAPINFOHEADER);

	// Salteo el file hearder
	fseek(fp,sizeof(BITMAPFILEHEADER),SEEK_SET);
	// Leo algunos datos del info header
	fread(header,sizeof(BITMAPINFOHEADER),1,fp);
	// leo la paleta
	switch(header->biBitCount)
	{
		case 1:		// 2 colores
			fread(&paleta,sizeof(RGBQUAD)*2,1,fp);
			break;
		case 4:		// 16 colores
			fread(&paleta,sizeof(RGBQUAD)*16,1,fp);
			break;
		case 8:		// 256 Colores
			fread(&paleta,sizeof(RGBQUAD)*256,1,fp);
			break;
	}


	DWORD size = (header->biWidth+2)*(header->biHeight+2)*3+1;
	switch(header->biBitCount)
	{
		case 24:		// true color
		{
			BYTE r,g,b;
			// Aloco memoria para los colores
			image = (BYTE *)new char[size];
			// Leo los datos de los colores
			fseek(fp,54,SEEK_SET);
			int t;
			DWORD n=0;
			int resto;
			for(i=0;i<header->biHeight && !feof(fp);++i)
			{
		   		t = 0;
				for(j=0;j<header->biWidth && !feof(fp);++j)
				{
		      		fread(&r,sizeof(r),1,fp);
					fread(&g,sizeof(g),1,fp);
					fread(&b,sizeof(b),1,fp);
					t+=3;
					image[(DWORD)(i*3L*header->biWidth+3L*j)] = b;
					image[(DWORD)(i*3L*header->biWidth+3L*j+1L)] = g;
					image[(DWORD)(i*3L*header->biWidth+3L*j+2L)] = r;
				}

				// t tiene que ser multiplo de 32 bits, si no es asi
				// leo resto's butes hasta completar el limite
		      
				if((resto = (t%4))>0 && resto<4)
				{
					BYTE pad[10];
					fread(pad,sizeof(BYTE),4-resto,fp);
				} 
			}
		}
		break;

      case 8:		// 256 colores
			{
				BYTE r,b,g;
				// Aloco memoria para los colores
				image = (BYTE *)new char[size];
	
				// borro la paleta
				for(i=0;i<256;++i)
					importancia[i] = 0;
			
				// Leo los datos de los colores
			   BYTE index;
				int t;
			   DWORD n=0;
				int resto;
				for(i=0;i<header->biHeight && !feof(fp);++i)
				{
			   	t = 0;
					for(j=0;j<header->biWidth && !feof(fp);++j)
			      {
			      	fread(&index,sizeof(index),1,fp);
						t++;
			         r = paleta[index].rgbRed;
						g = paleta[index].rgbGreen;
						b = paleta[index].rgbBlue;
						importancia[index]++;
						image[(DWORD)(i*3L*header->biWidth+3L*j)] = r;
						image[(DWORD)(i*3L*header->biWidth+3L*j+1L)] = g;
						image[(DWORD)(i*3L*header->biWidth+3L*j+2L)] = b;
					}
		
					// t tiene que ser multiplo de 32 bits, si no es asi
					// leo mas bytes hasta completar el limite
	      
					if((resto = (t%4))>0 && resto<4)
					{
						BYTE pad[10];
						fread(pad,sizeof(BYTE),4-resto,fp);
					}
				}
			}
			break;

		case 4:		// 16 colores
			{
			   BYTE r,b,g;
				// Aloco memoria para los colores
				image = (BYTE *)new char[size];
	
			// borro la paleta
				for(i=0;i<256;++i)
					importancia[i] = 0;
			
				// Leo los datos de los colores
			   BYTE index;
				int t;
			   DWORD n=0;
				int resto;
				for(i=0;i<header->biHeight && !feof(fp);++i)
				{
			   	t = 0;
					for(j=0;j<header->biWidth && !feof(fp);j+=2)
					{
						BYTE byte;
						fread(&byte,sizeof(byte),1,fp);
						// cada byte contiene 2 pixels
                  // 1er pixel
						index = byte>>4;
						t++;
			         r = paleta[index].rgbRed;
						g = paleta[index].rgbGreen;
						b = paleta[index].rgbBlue;
						importancia[index]++;
						image[(DWORD)(i*3L*header->biWidth+3L*j)] = r;
						image[(DWORD)(i*3L*header->biWidth+3L*j+1L)] = g;
						image[(DWORD)(i*3L*header->biWidth+3L*j+2L)] = b;
						// 2do pixel
						index = byte&15;
						r = paleta[index].rgbRed;
						g = paleta[index].rgbGreen;
						b = paleta[index].rgbBlue;
						importancia[index]++;
						image[(DWORD)(i*3L*header->biWidth+3L*j+3L)] = r;
						image[(DWORD)(i*3L*header->biWidth+3L*j+4L)] = g;
						image[(DWORD)(i*3L*header->biWidth+3L*j+5L)] = b;


					}
		
					// t tiene que ser multiplo de 32 bits, si no es asi
					// leo mas bytes hasta completar el limite
	      
					if((resto = (t%4))>0 && resto<4)
					{
						BYTE pad[10];
						fread(pad,sizeof(BYTE),4-resto,fp);
					}
				}
			}
			break;

	}

	fclose(fp);


	return image;
}


void cargar_mapa()
{
	FILE *fp = fopen("mapa.dat","rt");

	if(!fp)
		return;
	memset(C,0,sizeof(C));

	char buffer[255];
	fgets(buffer,sizeof(buffer),fp);		// flag MAPA

	for(int i=0;i<MAX_TILE_Y;++i)
		for(int j=0;j<MAX_TILE_X;++j)
		{
			fgets(buffer,sizeof(buffer),fp);
			C[i][j].nro_tile = atoi(buffer);
			
			fgets(buffer,sizeof(buffer),fp);
			C[i][j].flags = atoi(buffer);

			fgets(buffer,sizeof(buffer),fp);
			C[i][j].tipo = atoi(buffer);
		}

	fgets(buffer,sizeof(buffer),fp);		// flag /MAPA

	fclose(fp);
}





/*
 * Save the position of the mouse pointer where the bottun press occured
void save_position(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        saved_x = x;
        saved_y = y;
    }
}

 * Calculate the angle the object has rotated by since the last update
void rotate(int x, int y) 
{
    //rot_y = (GLfloat)(x - saved_x) * ROTATION_SPEED;
    //rot_x = (GLfloat)(y - saved_y) * ROTATION_SPEED;

    pos_x -= x - saved_x;
    pos_y -= y - saved_y;
    saved_x = x;
    saved_y = y;
    
    glutPostRedisplay();
}

 */
