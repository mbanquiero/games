#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <Afxwin.h>

#include "C:\Archivos de programa\NVIDIA Corporation\SDK 9.5\inc\GL\glut.h"

#define WINDOW_WIDTH    960
#define WINDOW_HEIGHT   600
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

#include "glmotor2d.h"

GLEngine2d motor;

GLfloat rot_x = 0.0, rot_y = 0.0;
GLfloat pos_y=31*12,pos_x=380*12;
GLfloat saved_x = pos_x, saved_y = pos_y;
LARGE_INTEGER F,T0,T1;   // address of current frequency
int cant_frames = 0;
float frame_time =0;
BOOL running = TRUE;
BOOL scrolling = FALSE;
int ant_x = 0,ant_y = 0;
int origen_x = 0,origen_y = 0;

// Callbacks
void OnClick(int button, int state, int x, int y);
void OnMouseMove(int x, int y);
void OnKey(unsigned char key, int x, int y);



void MainLoop();

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

	// inicializo el motor 2d
	motor.Create();
	motor.Init(WINDOW_WIDTH, WINDOW_HEIGHT);
	motor.LoadLevel(12,12);
	motor.cargar_escenario("mapa.dat");
	origen_y = motor.pos_y = 31 * 12;
	origen_x = motor.pos_x = 380 * 12;


	QueryPerformanceFrequency(&F);
	QueryPerformanceCounter(&T0);


    glutDisplayFunc(MainLoop);
    glutMouseFunc(OnClick);
    glutMotionFunc(OnMouseMove);
	glutKeyboardFunc(OnKey);
    glutMainLoop();

    /* Not reached */
    return 0;
}


void MainLoop()
{
	// redibujo:
	if(running)
	{
		// calcula el origen desde la posicion del sprite
		int tdx = motor.tile_dx;
		int tdy = motor.tile_dy;
		/*
		int pos_j = motor.nearest_x(motor.pos_x + motor.sprite_dx/2);
		int pos_i = motor.nearest_y(motor.pos_y - motor.sprite_dy/2);
		int pant_fil = 	pos_i/motor.cant_fil;
		int pant_col = 	pos_j/motor.cant_col;
		int f = pant_fil * motor.cant_fil;
		int c = pant_col * motor.cant_col;
		motor.Render(c*tdx,f*tdy);
		*/

		int x = motor.pos_x;
		int y = motor.pos_y;

		motor.Render(x - motor.cant_col/2*tdx,y - motor.cant_fil/2*tdy);



		origen_x = motor.pos_x;
		origen_y = motor.pos_y;

		 
	}
	else
	{
		// dibuja desde la pos. del mouse
		motor.Render(pos_x,pos_y);
	}


	QueryPerformanceCounter(&T1);
	float elapsed_time = (double)(T1.LowPart - T0.LowPart) / (double)F.LowPart;
	// ~30 fps
	if(elapsed_time*30>=1)
	{
		T0 = T1;
		motor.Update(elapsed_time);
	}

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
	/*
	if(GetAsyncKeyState(VK_LBUTTON))
	{
		POINT pt;
		GetCursorPos(&pt);
		pos_x -= pt.x - saved_x;
		pos_y -= pt.y - saved_y;
		saved_x = pt.x;
		saved_y = pt.y;
	}*/

    glutSwapBuffers();
    glutPostRedisplay();


}

// Callbacks
void OnClick(int button, int state, int x, int y) 
{

	// calcula el origen desde la posicion del sprite
	int pos_j = motor.nearest_x(motor.pos_x);
	int pos_i = motor.nearest_y(motor.pos_y);
	int pant_fil = 	pos_i/motor.cant_fil;
	int pant_col = 	pos_j/motor.cant_col;
	int f = pant_fil * motor.cant_fil;
	int c = pant_col * motor.cant_col;
	float kx = (float)motor.screen_dx / (float)(motor.cant_col*motor.tile_dx);		
	float ky = (float)motor.screen_dy / (float)(motor.cant_fil*motor.tile_dy);		
	int tdx = motor.tile_dx*kx;
	int tdy = motor.tile_dy*ky;
	int j = x/tdx + c;
	int i = y/tdy + f;
	if(state == GLUT_DOWN && button==GLUT_RIGHT_BUTTON) 
	{
		if(i>=0 && i<MAX_TILE_Y && j>=0 && j<MAX_TILE_X)
		{
			// llevo el sprite al tile seleccionado
			motor.pos_x = j*motor.tile_dx;
			motor.pos_y = (i+1)*motor.tile_dy-1;
		}
		glutPostRedisplay();
	}
	else
	if(button==GLUT_LEFT_BUTTON) 
	{
		//scrolling = state==GLUT_DOWN?TRUE:FALSE;
		//ant_x = x;
		//ant_y = y;

		// Sintetizo un touch del iphone
		float kx = (float)motor.screen_dx / (float)(motor.cant_col*motor.tile_dx);		// si coincide exacto kx = 1, por ejemplo 40 col x 8 = 320 px
		float ky = (float)motor.screen_dy / (float)(motor.cant_fil*motor.tile_dy);		// si coincide exacto ky = 1, por ejemplo 25 col x 8 = 200 px
		motor.ProcessTouch(x/kx,y/ky);

		glutPostRedisplay();
	}
}


void OnMouseMove(int x, int y)
{
	if(scrolling)
	{

		// la pantalla tiene cant_fil x cant_col, y eso ocupa screen_dy x screen_dx pixels, 
		float kx = (float)motor.screen_dx / (float)(motor.cant_col*motor.tile_dx);		// si coincide exacto kx = 1, por ejemplo 40 col x 8 = 320 px
		float ky = (float)motor.screen_dy / (float)(motor.cant_fil*motor.tile_dy);		// si coincide exacto ky = 1, por ejemplo 25 col x 8 = 200 px

		origen_x += (x-ant_x)/kx;
		origen_y += (y-ant_y)/ky;
		ant_x = x;
		ant_y = y;
		motor.pos_x = origen_x;
		motor.pos_y = origen_y;
		glutPostRedisplay();
	}

}


void OnKey(unsigned char key, int x, int y)
{
	motor.OnChar(key,0,0);
}

