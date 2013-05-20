
#pragma once

#define M_PI		3.14159265359 
#define M_PI_2		1.570796326795
#define M_2PI		6.28318530718 
#define EPSILON	0.0001
#define IGUAL(x,y) fabs((x)-(y))<EPSILON?TRUE:FALSE
char sign(double n);		// retorna 1 si es positivo -1 si es negativo
int round(double x);		// Devuelve el entero mas cercano a x
double round_dir(double x);	// Redondea un angulo
double round2(double x);
double round1(double x);
class TMatriz3x3;
class TMatriz2x2;
class TVector2d;
class TVector3d;

#pragma warning( disable : 4244 )

class TVector2d
{
	public:
		double x,y;

		TVector2d(double a=0,double b=0);
		TVector2d(POINT p);
		TVector2d operator=(TVector2d p);
		virtual void rotar(TVector2d o,double an);
		virtual void rotar(double an);
		double distancia(){return(sqrt(x*x+y*y));};
		double distancia(TVector2d q);
		// operaciones entre vectores y escalres
		TVector2d operator-(TVector2d &q);
		TVector2d operator+(TVector2d &q);
		TVector2d operator*(double k);
		double operator>>(TVector2d &q);
		double mod();			// modulo
		double angulo();			// angulo -pi y pi
		double angulo2();			// angulo 0 y 2pi
		TVector2d normal();
		void normalizar();
		void ortonormalizar();
		operator CPoint() const { return CPoint(x,y);};
};


class TMatriz2x2
{
	public:
		double a11;
		double a12;
		double a21;
		double a22;
		TMatriz2x2(	double a=0,double b=0,double c=0,double d=0);
		virtual double det();
		virtual double X(TVector2d B);
		virtual double Y(TVector2d B);

};


class TVector3d
{
	public:
		double x,y,z;

		TVector3d(double a=0,double b=0,double c=0);
		TVector3d(TVector2d p,double c =0);
		TVector3d operator=(TVector3d p);
		TVector3d operator=(TVector2d p);
		virtual void rotar_xy(TVector3d o,double an);
		virtual void rotar_xz(TVector3d o,double an);
		virtual void rotar_zy(TVector3d o,double an);
		virtual void rotar_xy(double an);
		virtual void rotar_xz(double an);
		virtual void rotar_zy(double an);
		virtual void rotar(TVector3d o,TVector3d eje,double an);
		virtual void rotar(TVector3d o,double an_x,double an_y,double an_z);
		virtual void rotar_inv(TVector3d o,double an_x,double an_y,double an_z);

		double distancia(){return(sqrt(x*x+y*y+z*z));};
		double distancia(TVector3d q);
		// operaciones entre vectores y escalres
		TVector3d operator-(TVector3d &q);
		TVector3d operator+(TVector3d &q);
		TVector3d operator*(double k);
		TVector3d operator*(TVector3d &q);
		TVector3d operator*(TMatriz3x3 &M);

		double operator>>(TVector3d &q);
		double mod();			// modulo
		double mod_xy();		// distancia plana (sobre x,y)
		void normalizar();
		void swap_xy();
		TVector2d pxy(){return TVector2d(x,y);};
		TVector2d pxz(){return TVector2d(x,z);};
};



class TMatriz3x3
{
	public:
		double a11;
		double a12;
		double a13;
		double a21;
		double a22;
		double a23;
		double a31;
		double a32;
		double a33;
		TMatriz3x3(	double a=0,double b=0, double c=0,
						double d=0,double e=0, double f=0,
						double g=0,double h=0, double i=0);
		TMatriz3x3(	TVector3d A,TVector3d B,TVector3d C);
		TMatriz3x3 operator=(TMatriz3x3 M);
		virtual double det();
		virtual double det(TVector3d B,int col);
		virtual TMatriz3x3 inversa();
		
		TVector3d operator*(TVector3d p);
		TMatriz3x3 operator*(TMatriz3x3 B);

};


// helpers geometrias
TVector2d barycentric(TVector3d v1,TVector3d v2,TVector3d v3,TVector3d p);

BOOL interseccion_2rectas(TVector2d p0,TVector2d dir_0,
							TVector2d p1,TVector2d dir_1,TVector2d *Ip);

BOOL interseccion_2segmentos(TVector2d p0,TVector2d p1,TVector2d q0,TVector2d q1,
									TVector2d *Ip,BOOL extremos=TRUE);

double interseccion_2segmentos(TVector2d p0,TVector2d p1,TVector2d q0,TVector2d q1,BOOL extremos=TRUE);
BOOL pto_inside_tri(TVector2d pt,TVector2d p0,TVector2d p1,TVector2d p2);

BOOL pto_inside_poly(TVector2d pt,TVector2d P[]);


// Varios 
char *rtrim(char *string);		// version ansi
void swap(int *a,int *b);
void swap(long *a,long *b);
void swap(double *a,double *b);
void swap(char *a,char *b);

