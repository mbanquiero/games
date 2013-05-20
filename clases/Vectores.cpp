
	#include<afxwin.h>
#ifndef _CONSOLE
	#include<afxpriv.h>
#endif

#include<stdio.h>
#include<math.h>
#include"vectores.h"

// retorna 1 si es positivo -1 si es negativo
char sign(double n)
{
	return(n<0?-1:1);
}

// Devuelve el entero mas cercano a x
int round(double x)
{
	int rta;
	int a = (int)floor(x);
	int b = a + 1;
	// luego
	// a <= x <= b
	if(fabs(a-x) < fabs(b-x))
		rta = a;
	else
		rta = b;

	return rta;
}

// redondea a 2 decimales
double round2(double x)
{
	char buffer[255];
	sprintf(buffer,"%10.2f",round(x*100)/100.0);
	return atof(buffer);
}

// redondea a 1 decimales
double round1(double x)
{
	char buffer[255];
	sprintf(buffer,"%10.1f",round(x*10)/10.0);
	return atof(buffer);
}


// Redondea un angulo, de tal forma que si es casi ortogonal, quede ortogonal.
// pero cualquier otro angulo, con la precision maxima que se le pueda dar
double round_dir(double x)
{
	double rta = x;
	int angulo = round(x*10);
	if(angulo%900==0)
		rta = angulo/10;
	return rta;
}




TVector3d::TVector3d(double a,double b,double c)
{
	x=a;
	y=b;
	z=c;
}

TVector3d::TVector3d(TVector2d p,double c)
{
	x = p.x;
	y = p.y;
	z = c;
}


TVector3d TVector3d::operator=(TVector3d p)
{
	x=p.x;
	y=p.y;
	z=p.z;
	return(*this);
}

TVector3d TVector3d::operator=(TVector2d p)
{
	x=p.x;
	y=p.y;
	z=0;
	return(*this);
}


// rotacion x-y es sobre el eje z
void TVector3d::rotar_xy(TVector3d o,double an)
{
	if(an==0)
		return;

	double ro=sqrt((x-o.x)*(x-o.x)+(y-o.y)*(y-o.y));
	double alfa;
	if((x-o.x)!=0)
		alfa=atan2(y-o.y,x-o.x);
	else
		alfa=(y-o.y)>=0?M_PI_2:-M_PI_2;
	alfa+=an;
	x=o.x+ro*cos(alfa);
	y=o.y+ro*sin(alfa);

	/*
	x=x*cos(an)+y*sin(an);
	y=y*cos(an)-x*sin(an); 
	*/
}

// rotacion xz es sobre el eje y
void TVector3d::rotar_xz(TVector3d o,double an)
{
	if(an==0)
		return;

	double ro=sqrt((x-o.x)*(x-o.x)+(z-o.z)*(z-o.z));
	double alfa;
	if((x-o.x)!=0)
		alfa=atan2(z-o.z,x-o.x);
	else
		alfa=(z-o.z)>=0?M_PI_2:-M_PI_2;
	alfa+=an;
	x=o.x+ro*cos(alfa);
	z=o.z+ro*sin(alfa); 

	/*
	x=x*cos(an)-z*sin(an); 
	z=x*sin(an)+z*cos(an);
	*/
}

// rotacion zy es sobre el eje x
void TVector3d::rotar_zy(TVector3d o,double an)
{
	if(an==0)
		return;
	double ro=sqrt((y-o.y)*(y-o.y)+(z-o.z)*(z-o.z));
	double alfa;
	if((y-o.y)!=0)
		alfa=atan2(z-o.z,y-o.y);
	else
		alfa=(z-o.z)>=0?M_PI_2:-M_PI_2;
	alfa+=an;
	y=o.y+ro*cos(alfa);
	z=o.z+ro*sin(alfa);

	/*
	y=y*cos(an)+z*sin(an);
	z=z*cos(an)-y*sin(an); 
	*/

}

// rotacion x-y es sobre el eje z
void TVector3d::rotar_xy(double an)
{
	double xr = x*cos(an)-y*sin(an);
	double yr = y*cos(an)+x*sin(an); 
	x = xr;
	y = yr;
}


// rotacion xz es sobre el eje y
void TVector3d::rotar_xz(double an)
{
	
	double xr=x*cos(an)+z*sin(an); 
	double zr=-x*sin(an)+z*cos(an);
	x = xr;
	z = zr;

}

// rotacion zy es sobre el eje x
void TVector3d::rotar_zy(double an)
{
	double yr=y*cos(an)-z*sin(an);
	double zr=z*cos(an)+y*sin(an); 
	y = yr;
	z = zr;
}


// Rotacion sobre un eje arbitrario
void TVector3d::rotar(TVector3d o,TVector3d eje,double theta)
{
	double a = o.x;
	double b = o.y;
	double c = o.z;
	double u = eje.x;
	double v = eje.y;
	double w = eje.z;

	double u2 = u*u;
    double v2 = v*v;
    double w2 = w*w;
	double cosT = cos(theta);
	double sinT = sin(theta);
	double l2 = u2 + v2 + w2;
    double l =  sqrt(l2);

	if(l2 < 0.000000001)		// el vector de rotacion es casi nulo
		return;

	double xr = a*(v2 + w2) + u*(-b*v - c*w + u*x + v*y + w*z) 
            + (-a*(v2 + w2) + u*(b*v + c*w - v*y - w*z) + (v2 + w2)*x)*cosT
            + l*(-c*v + b*w - w*y + v*z)*sinT;
	xr/=l2;

    double yr = b*(u2 + w2) + v*(-a*u - c*w + u*x + v*y + w*z) 
            + (-b*(u2 + w2) + v*(a*u + c*w - u*x - w*z) + (u2 + w2)*y)*cosT
            + l*(c*u - a*w + w*x - u*z)*sinT;
	yr/=l2;

     double zr = c*(u2 + v2) + w*(-a*u - b*v + u*x + v*y + w*z) 
            + (-c*(u2 + v2) + w*(a*u + b*v - u*x - v*y) + (u2 + v2)*z)*cosT
            + l*(-b*u + a*v - v*x + u*y)*sinT;
	zr/=l2;

	x = xr;
	y = yr;
	z = zr;
}

void TVector3d::rotar(TVector3d o,double an_Z,double an_X,double an_Y)
{
	TVector3d eje_x = TVector3d(1,0,0);
	TVector3d eje_y = TVector3d(0,1,0);
	TVector3d eje_z = TVector3d(0,0,1);
	TVector3d O = TVector3d(0,0,0);

	// primero roto en el eje Z (plano_xy)
	rotar(o,eje_z,an_Z);
	// y roto los ejes x,y tambien, la primer rotacion se puede hacer
	// directamente, pues los ejes todavia no estan rotados
	eje_x.rotar_xy(an_Z);
	eje_y.rotar_xy(an_Z);
	//eje_x.rotar(O,eje_z,an_Z);
	//eje_y.rotar(O,eje_z,an_Z);

	// sin embargo las siguientes tienen que ser sobre los ejes ya rotados
	// Ahoro roto sobre el eje X (plano zy)
	rotar(o,eje_x,an_X);		// el pto pp dicho
	// y los ejes 
	eje_y.rotar(O,eje_x,an_X);
	//eje_y.rotar_zy(an_X);

	// Y ahora roto sobre el eje Y (plano xz)
	rotar(o,eje_y,an_Y);		// el pto pp dicho

}



void TVector3d::rotar_inv(TVector3d o,double an_Z,double an_X,double an_Y)
{
	TVector3d eje_x = TVector3d(1,0,0);
	TVector3d eje_y = TVector3d(0,1,0);
	TVector3d eje_z = TVector3d(0,0,1);
	TVector3d O = TVector3d(0,0,0);
	
	// Revierto la rotacion en Y 
	rotar(o,eje_y,-an_Y);		// el pto pp dicho
	// y roto los ejes x,y tambien
	eje_x.rotar_xz(-an_Y);
	eje_z.rotar_xz(-an_Y);
	//eje_x.rotar(O,eje_y,-an_Y);
	//eje_z.rotar(O,eje_y,-an_Y);

	// Revierto la rot. sobre el eje X (plano zy)
	rotar(o,eje_x,-an_X);		// el pto pp dicho
	eje_z.rotar(O,eje_x,-an_X);
	//eje_z.rotar_zy(-an_X);

	// y revierto la rot en el eje Z (plano_xy)
	rotar(o,eje_z,-an_Z);

}


double TVector3d::distancia(TVector3d q)
{
	double dx=x-q.x;
	double dy=y-q.y;
	double dz=z-q.z;
	return(sqrt(fabs(dx*dx+dy*dy+dz*dz)));
}

double TVector3d::mod()
{
	return(sqrt(fabs(x*x+y*y+z*z)));
}

double TVector3d::mod_xy()
{
	return(sqrt(fabs(x*x+y*y)));
}



void TVector3d::normalizar()
{
	double m = mod();
	if(m!=0)
	{
		x/=m;
		y/=m;
		z/=m;
	}
}


// suma de vectores
TVector3d TVector3d::operator+(TVector3d &q)
{
	return(TVector3d(x+q.x,y+q.y,z+q.z));
}

// diferencia de vectores
TVector3d TVector3d::operator-(TVector3d &q)
{
	return(TVector3d(x-q.x,y-q.y,z-q.z));
}

// escalar x vector
TVector3d TVector3d::operator*(double k)
{
	return(TVector3d(k*x,k*y,k*z));
}

// producto vectorial
TVector3d TVector3d::operator*(TVector3d &q)
{
	double a=y*q.z-z*q.y;
	double b=z*q.x-x*q.z;
	double c=x*q.y-y*q.x;
	return(TVector3d(a,b,c));
}

// producto escalar
double TVector3d::operator>>(TVector3d &q)
{
	double rta=x*q.x+y*q.y+z*q.z;
	return(rta);
}


// ----------------------------------------------
// 	VECTORES EN 2 DIMENSIONES
// ----------------------------------------------
TVector2d::TVector2d(double a,double b)
{
	x=a;
	y=b;
}

TVector2d::TVector2d(POINT p)
{
	x = p.x;
	y = p.y;
}


TVector2d TVector2d::operator=(TVector2d p)
{
	x=p.x;
	y=p.y;
	return(*this);
}


void TVector2d::rotar(TVector2d o,double an)
{
	double ro=sqrt((x-o.x)*(x-o.x)+(y-o.y)*(y-o.y));
	double alfa;
	if((x-o.x)!=0)
		alfa=atan2(y-o.y,x-o.x);
	else
		alfa=(y-o.y)>=0?M_PI_2:-M_PI_2;
	alfa+=an;
	x=o.x+ro*cos(alfa);
	y=o.y+ro*sin(alfa);
	return;
}


void TVector2d::rotar(double an)
{
	double xp=x*cos(an)-y*sin(an);
	double yp=x*sin(an)+y*cos(an);
	x=xp;
	y=yp;
	return;
}



double TVector2d::distancia(TVector2d q)
{
	double dx=x-q.x;
	double dy=y-q.y;
	return(sqrt(fabs(dx*dx+dy*dy)));
}


double TVector2d::mod()
{
	return(sqrt(fabs(x*x+y*y)));
}

// suma de vectores
TVector2d TVector2d::operator+(TVector2d &q)
{
	return(TVector2d(x+q.x,y+q.y));
}

// diferencia de vectores
TVector2d TVector2d::operator-(TVector2d &q)
{
	return(TVector2d(x-q.x,y-q.y));
}

// escalar x vector
TVector2d TVector2d::operator*(double k)
{
	return(TVector2d(k*x,k*y));
}

// producto escalar
double TVector2d::operator>>(TVector2d &q)
{
	return(x*q.x+y*q.y);
}


// retorna un vector normal a si mismo 
TVector2d TVector2d::normal()
{
	return(TVector2d(-y,x));
}


double TVector2d::angulo()
{
	double rta;
	if(x!=0)
		rta = atan2(y,x);
	else
		rta = y>=0?M_PI/2:-M_PI/2;
	return(rta);

}


// devuelve el angulo entre cero y 2pi
double TVector2d::angulo2()
{
	double rta = angulo();
	if(rta<0)
		rta+=2*M_PI;
	return rta;
}

//----------------------------------------------------
// MATRIZ 2x2
//----------------------------------------------------

TMatriz2x2::TMatriz2x2(	double a,double b,double c,double d )
{
	a11=a;
	a12=b;
	a21=c;
	a22=d;
}

double TMatriz2x2::det()
{
	double rta = a11*a22 - a12*a21;
	return rta;
}


double TMatriz2x2::X(TVector2d B)
{
	double x = (a22*B.x-a12*B.y)/det();
	return x;
}

double TMatriz2x2::Y(TVector2d B)
{
	double y = (a11*B.y-a21*B.x)/det();
	return y;
}


void TVector2d::normalizar()
{
	double m = mod();
	if(m!=0)
	{
		double k = 1/mod();
		*this=*this*k;
	}
}

void TVector2d::ortonormalizar()
{
	double ep =0.00001; 
	
	if(fabs(x)<ep)
		x = 0;
	if(fabs(y)<ep)
		y = 0;
	
	if(fabs(x-1)<ep)
		x = 1;
	if(fabs(y-1)<ep)
		y = 1;

	if(fabs(x+1)<ep)
		x = -1;
	if(fabs(y+1)<ep)
		y = -1;
}


//----------------------------------------------
TMatriz3x3::TMatriz3x3(TVector3d A,TVector3d B,TVector3d C)
{
	a11 = A.x;
	a21 = A.y;
	a31 = A.z;

	a12 = B.x;
	a22 = B.y;
	a32 = B.z;

	a13 = C.x;
	a23 = C.y;
	a33 = C.z;
}


TMatriz3x3::TMatriz3x3(	double a,double b, double c,
						double d,double e, double f,
						double g,double h, double i)
{
	a11=a;
	a12=b;
	a13=c;
	a21=d;
	a22=e;
	a23=f;
	a31=g;
	a32=h;
	a33=i;
}

double TMatriz3x3::det()
{
	double rta=a11*a22*a33 + a12*a23*a31 + a21*a32*a13 -
			(a13*a22*a31 + a21*a12*a33 + a32*a23*a11);
	return rta;
}


TMatriz3x3 TMatriz3x3::operator=(TMatriz3x3 M)
{
	a11=M.a11;
	a12=M.a12;
	a13=M.a13;
	
	a21=M.a21;
	a22=M.a22;
	a23=M.a23;

	a31=M.a31;
	a32=M.a32;
	a33=M.a33;
	
	return *this;
}



double TMatriz3x3::det(TVector3d B,int col)
{
	double D=0;
	switch(col)
	{
		case 1:

			D = B.x*a22*a33 + a12*a23*B.z + B.y*a32*a13 -
			(a13*a22*B.z + B.y*a12*a33 + a32*a23*B.x);

			//D=TMatriz3x3(	B.x,a12,a13,
								//B.y,a22,a23,
								//B.z,a32,a33	).det();
			break;
		case 2:
			D = a11*B.y*a33 + B.x*a23*a31 + a21*B.z*a13 -
			(a13*B.y*a31 + a21*B.x*a33 + B.z*a23*a11);

			//D=TMatriz3x3(	a11,B.x,a13,
								//a21,B.y,a23,
								//a31,B.z,a33	).det();
			break;
		case 3:
			D = a11*a22*B.z + a12*B.y*a31 + a21*a32*B.x -
			(B.x*a22*a31 + a21*a12*B.z+ a32*B.y*a11);

			//D=TMatriz3x3(	a11,a12,B.x,
								//a21,a22,B.y,
								//a31,a32,B.z	).det();
			break;
	}
	return D;
}

// Multiplicar un vector3d x una matriz de 3x3 da como resultado un vector3d
// Matriz x Vector
TVector3d TMatriz3x3::operator*(TVector3d p)
{
	TVector3d r;
	r.x = p.x*a11 + p.y*a12 + p.z*a13;
	r.y = p.x*a21 + p.y*a22 + p.z*a23;
	r.z = p.x*a31 + p.y*a32 + p.z*a33;
	return r;
}

// ojo este es al revez :
// Vector x Matriz 
TVector3d TVector3d ::operator*(TMatriz3x3 &M)
{
	TVector3d r;
	r.x = x*M.a11 + y*M.a21 + z*M.a31;
	r.y = x*M.a12 + y*M.a22 + z*M.a32;
	r.z = x*M.a13 + y*M.a23 + z*M.a33;
	return r;
}



void TVector3d::swap_xy()
{
	double aux = x;
	x = y;
	y = aux;
}




TMatriz3x3 TMatriz3x3::inversa()
{

	TMatriz3x3 Adj;
	double det_A = det();
	if(det_A!=0)
	{
		double k = 1/det_A;

		Adj.a11 = (a22*a33-a32*a23) * k;
		Adj.a21 = -(a21*a33-a31*a23)* k;
		Adj.a31 = (a21*a32-a31*a22)* k;

		Adj.a12 = -(a12*a33-a32*a13)*k;
		Adj.a22 = (a11*a33-a31*a13)*k;
		Adj.a32 = -(a11*a32-a31*a12)*k;

		Adj.a13 = (a12*a23-a22*a13)*k;
		Adj.a23 = -(a11*a23-a21*a13)*k;
		Adj.a33 = (a11*a22-a21*a12)*k;
	}
	return Adj;
}

TMatriz3x3 TMatriz3x3::operator*(TMatriz3x3 B)
{
	TMatriz3x3 C;
	C.a11 = a11*B.a11+a12*B.a21+a13*B.a31;
	C.a12 = a11*B.a12+a12*B.a22+a13*B.a32;
	C.a13 = a11*B.a13+a12*B.a23+a13*B.a33;

	C.a21 = a21*B.a11+a22*B.a21+a23*B.a31;
	C.a22 = a21*B.a12+a22*B.a22+a23*B.a32;
	C.a23 = a21*B.a13+a22*B.a23+a23*B.a33;

	C.a31 = a31*B.a11+a32*B.a21+a33*B.a31;
	C.a32 = a31*B.a12+a32*B.a22+a33*B.a32;
	C.a33 = a31*B.a13+a32*B.a23+a33*B.a33;

	return C;
}
	


// helpers, devuelve las coordenadas baricentricas de un pto p
// http://en.wikipedia.org/wiki/Barycentric_coordinates_(mathematics)
TVector2d barycentric(TVector3d v1,TVector3d v2,TVector3d v3,TVector3d p)
{
	double b,g;

	double A = v1.x - v3.x;
	double B = v2.x - v3.x;
	double C = v3.x - p.x;

	double D = v1.y - v3.y;
	double E = v2.y - v3.y;
	double F = v3.y - p.y;

	double G = v1.z - v3.z;
	double H = v2.z - v3.z;
	double I = v3.z - p.z;

	if(A==0 && B==0)
	{
		swap(&A,&D);
		swap(&B,&E);
		swap(&C,&F);
	}


	b = (B*(F+I) - C*(E+H)) / (A*(E+H) - B*(D+G));
	g = (A*(F+I) - C*(D+G)) / (B*(D+G) - A*(E+H));


	return TVector2d(b,g);
}



BOOL interseccion_2rectas(TVector2d p0,TVector2d dir_0,
							TVector2d p1,TVector2d dir_1,TVector2d *Ip)
{
	BOOL rta = FALSE;
	TMatriz2x2 M(	dir_0.x,	-dir_1.x,
					dir_0.y,	-dir_1.y	
				);

	if(fabs(M.det())>0.000001)
	{
		double t = M.X(TVector2d(p1.x-p0.x,p1.y-p0.y));
		*Ip = p0 + dir_0*t;
		rta = TRUE;
	}

	return rta;

}


	/*
	// ojo, este es un ejemplo de rectas casi paralelas..el |det| > epsilon
	TVector2d p0,p1,q0,q1;
	p0.x = 0.13557858765125;
	p0.y = -2.1086437702179;
	p1.x = -0.20429508388042;
	p1.y = -4.6071920394897;
	q0.x = 0.19325295090675;
	q0.y = -1.6846562623978;
	q1.x = 0.22212965786457;
	q1.y = -1.4723719358444;
	CDC *pDC = &dc;
	pDC->MoveTo(500+50*p0.x,400+50*p0.y);
	pDC->LineTo(500+50*p1.x,400+50*p1.y);
	pDC->MoveTo(500+50*q0.x,400+50*q0.y);
	pDC->LineTo(500+50*q1.x,400+50*q1.y);
	*/


BOOL interseccion_2segmentos(TVector2d p0,TVector2d p1,TVector2d q0,TVector2d q1,
									TVector2d *Ip,BOOL extremos)
{
	BOOL rta = FALSE;
	// La recta p = p0+t*p
	TVector2d p = p1-p0;
	// La recta q = q0+k*q
	TVector2d q = q1-q0;

	TMatriz2x2 M(p.x,-q.x,p.y,-q.y);
	if(fabs(M.det())>0.000001)
	{
		double t = M.X(TVector2d(q0.x-p0.x,q0.y-p0.y));
		double k = M.Y(TVector2d(q0.x-p0.x,q0.y-p0.y));
		double ep = extremos?-0.0001:0.0001;
		// si incluye los extremos t,k E [0,1], si no debe ser t,k E (0,1)
		// con lo cual el ep es negativo si incluye los extremos 
		if(t>ep && t<1-ep && k>ep && k<1-ep)
		{
			*Ip = p0+p*t;
			rta = TRUE;
		}
	}
	
	return rta;
}

double interseccion_2segmentos(TVector2d p0,TVector2d p1,TVector2d q0,TVector2d q1,BOOL extremos)
{
	double rta = -1;
	// La recta p = p0+t*p
	TVector2d p = p1-p0;
	// La recta q = q0+k*q
	TVector2d q = q1-q0;

	TMatriz2x2 M(p.x,-q.x,p.y,-q.y);
	if(fabs(M.det())>0.000001)
	{
		double t = M.X(TVector2d(q0.x-p0.x,q0.y-p0.y));
		double k = M.Y(TVector2d(q0.x-p0.x,q0.y-p0.y));
		double ep = extremos?-0.0001:0.0001;
		// si incluye los extremos t,k E [0,1], si no debe ser t,k E (0,1)
		// con lo cual el ep es negativo si incluye los extremos 
		if(t>ep && t<1-ep && k>ep && k<1-ep)
		{
			//*Ip = p0+p*t;
			rta = t;
		}
	}
	
	return rta;
}


// esta el pto pt adentro del triangulo
BOOL pto_inside_tri(TVector2d pt,TVector2d p0,TVector2d p1,TVector2d p2)
{
	BOOL rta = FALSE;
	// verifico si el pto esta adentro del triangulo
	TVector2d B = barycentric(p0,p1,p2,pt);
	// Check if point is in triangle
	if((B.x >= 0) && (B.y >= 0) && (B.x + B.y <= 1))
		rta = TRUE;
	return rta;
}


// esta el pto pt adentro del poligono
BOOL pto_inside_poly(TVector2d pt,TVector2d P[])
{
	BOOL rta = FALSE;
	// verifico si el pto esta adentro del triangulo
	TVector2d B = barycentric(P[0],P[1],P[2],pt);
	// Check if point is in triangle
	if((B.x >= 0) && (B.y >= 0) && (B.x + B.y <= 1))
		rta = TRUE;
	else
	{
		// todavia puede esta adentro del otro tiangulo
		TVector2d B = barycentric(P[0],P[2],P[3],pt);
		// Check if point is in triangle
		if((B.x >= 0) && (B.y >= 0) && (B.x + B.y <= 1))
			rta = TRUE;
	}

	return rta;
}



// varios (algunos al pedo, vienen del Borland C en DOS)
char *rtrim(char *string)
{
	int l=strlen(string)-1;
	while(l>=0 && (*(string+l)==' ' || *(string+l)=='\t'))
	{
		*(string+l)='\0';
		--l;
	}
	return(string);
}

void swap(int *a,int *b)
{
	int x = *a;
	*a = *b;
	*b = x;
}

void swap(long *a,long *b)
{
	long x = *a;
	*a = *b;
	*b = x;
}

void swap(double *a,double *b)
{
	double c = *a;
	*a = *b;
	*b = c;
}

void swap(char *a,char *b)
{
	char c = *a;
	*a = *b;
	*b = c;
}
