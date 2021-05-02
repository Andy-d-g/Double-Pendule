#include <time.h>
#include <math.h>
#include <GL4D/gl4dp.h>
#include <GL4D/gl4duw_SDL2.h>
#include "moteur.h"

static void  init   (void);
static void  draw   (void);
static void  sortie (void);
static void  key    (int keycode);
static float absolu (float a);

/* Position Camera */
/*
static float _ycam = 0.0f;
static float _zcam = 0.0f;
static float _xcam = 0.0f;
*/

/* Scale Obj */
static int scale_sphere = 10;
static int scale_cube   = 3;

/* Width Window */
static const int _wW = 1000; 
/* Height Window */
static const int _hW = 1000;

/* Angle(s) */
static float a1;
static float a2;
/* Longueur(s) */
static float l1;
static float l2;
/* Poids */
static float m1;
static float m2;
/* Velocity */
static float a1_velocity;
static float a2_velocity;

/* Pesanteur */
static float g = 1.0f;

/* Cube(s) */
static surface_t * _cube1 = NULL;

/* Sphere(s) */
static surface_t * _sphere1 = NULL;
static surface_t * _sphere2 = NULL;

int main (int argc, char ** argv) {
	if (!gl4duwCreateWindow(argc, argv, "Double Pendule", 10, 10, _wW, _hW, GL4DW_SHOWN)) return 1;

	/* Pour forcer la desactivation de la synchronisation verticale */
	/* Activation de la synchronisation verticale sinon ca va super vite et les 
	 * differences de nombre d'affichage entre chaque frame peux donner un rendu bizzare */
	//SDL_GL_SetSwapInterval(0);

	init();

	if (argc==1){
		printf ("Conservation des parametre de base\n");
	}
	else if (argc==3){
		a1 = (float) atoi(argv[1]);
		a2 = (float) atoi(argv[2]);
	}
	else if (argc==5){
		a1 = (float) atoi(argv[1]);
		a2 = (float) atoi(argv[2]);

		l1 = (float) atoi(argv[3]);
		l2 = (float) atoi(argv[4]);
	}
	else if (argc==7){
		a1 = (float) atoi(argv[1]);
		a2 = (float) atoi(argv[2]);

		l1 = (float) atoi(argv[3]);
		l2 = (float) atoi(argv[4]);

		m1 = (float) atoi(argv[5]);
		m2 = (float) atoi(argv[6]);
	}
	else {
		printf ("Le nombre d'argument n'est pas le bon\n");
	}

	printf ("Parametres :\n\t- a1 : %f\n\t- a2 : %f\n\t- l1 : %f\n\t- l2 : %f\n\t- m1 : %f\n\t- m2 : %f\n\n\n",a1,a2,l1,l2,m1,m2);

  	gl4dpInitScreen();
  	gl4duwKeyDownFunc(key);
  	gl4duwDisplayFunc(draw);
  	gl4duwMainLoop();

	return 0;
}

void init (void) {
	GLuint id;
	vec4 r = {1.0f, 0.0f, 0.0f, 1}, b = {0.1f, 0.1f, 0.7f, 1};

	/* Angle de départ */
	a1=15.0f;
	a2=45.0f;


	/* Angle velocity */
	a1_velocity = 0.0f;
	a2_velocity = 0.0f;

	/* Longueur des branchement entre spheres */
	l1 = 200.0f;
	l2 = 100.0f;

	/* Masse des sphere de depart */
	m1 = 30.0f;
	m2 = 30.0f;

	/* Creation des cubes */
	_cube1 = mkCube();

	/* Creation des sphere */
	_sphere1 = mkSphere(12, 12);
	_sphere2 = mkSphere(12, 12);

	/* Definition des couleurs (Sphere / Cube) */
	_sphere1->dcolor = r;
	_sphere2->dcolor = r;
	_cube1->dcolor = b;

	/* Definition des textures (Sphere / Cube) */
  	id = getTexFromBMP("images/tex.bmp");
  	setTexId(_sphere1, id);
  	setTexId(_sphere2, id);
  	setTexId(_cube1,   id);

  	/* Affichage des textures (Sphere / Cube ) */
  	enableSurfaceOption(_sphere1, SO_USE_TEXTURE);
  	enableSurfaceOption(_sphere2, SO_USE_TEXTURE);
  	enableSurfaceOption(_cube1,   SO_USE_TEXTURE);

  	/* Affichage des ombres (Spheres / Cube) */
  	enableSurfaceOption(_sphere1, SO_USE_LIGHTING);
  	enableSurfaceOption(_sphere2, SO_USE_LIGHTING);
  	enableSurfaceOption(_cube1,   SO_USE_LIGHTING);

  	/* Fonction de sortie */
  	atexit(sortie);
}

float absolu(float a){
	if (a<0.0f){
		return a*-1;
	}
	return a;
}

void draw (void) {
	float 
		mvMat[16], projMat[16], nmv[16],
		//acceleration_num, acceleration_denom,
		a1_acceleration, a2_acceleration,
		x1, x2, y1, y2, x, y,
		max, i,
		num1, num2, num3, num4, den;

	gl4dpClearScreen();
	clearDepth();

  	MFRUSTUM(projMat, -0.05f, 0.05f, -0.05f, 0.05f, 0.1f, 1000.0f);
  	MIDENTITY(mvMat);
  	/* on place la caméra : je ne sais pas où mais ici c'est bien */
  	lookAt(mvMat, 0, 0, 700, 0, -60, 0, 0, 1, 0); 

  	/* Calcul acceleration A1 */

  	num1 = -g * (2 * m1 + m2) * sin(a1);
  	num2 = -m2 * g * sin(a1-2*a2);
  	num3 = -2*sin(a1-a2)*m2;
  	num4 = a2_velocity*a2_velocity*l2+a1_velocity*a1_velocity*l1*cos(a1-a2);
  	den = l1 * (2*m1+m2-m2*cos(2*a1-2*a2));
  	a1_acceleration = (num1 + num2 + num3*num4) / den;

  	/*
  	acceleration_num = -g * (2 * m1 + m2) * sin(a1) - m2 * g * sin(a1 - 2 * a2) - 2 * sin(a1 - a2) * m2 * (a2_velocity * a2_velocity * l2 + a1_velocity * a1_velocity * l1 * cos(a1 - a2));
  	acceleration_denom = l1 * (2 * m1 + m2 - m2 * cos(2 * a1 - 2 * a2));
  	a1_acceleration = acceleration_num / acceleration_denom;
	*/

    /* Calcul acceleration A2 */

  	num1 = 2 * sin(a1-a2);
 	num2 = (a1_velocity*a1_velocity*l1*(m1+m2));
  	num3 = g * (m1 + m2) * cos(a1);
  	num4 = a2_velocity*a2_velocity*l2*m2*cos(a1-a2);
  	den = l2 * (2*m1+m2-m2*cos(2*a1-2*a2));
  	a2_acceleration = (num1*(num2+num3+num4)) / den;

  	/*
  	acceleration_num = 2 * sin(a1 - a2) * (a1_velocity * a1_velocity * l1 * (m1 + m2) + g * (m1 + m2) * cos(a1) + a2_velocity * a2_velocity * l2 * m2 * cos(a1 - a2));
  	acceleration_denom = l2 * (2 * m1 + m2 - m2 * cos(2 * a1 - 2 * a2));
	a2_acceleration = acceleration_num / acceleration_denom;
	*/

  	/* Calcul next position */ 

	x1 = l1 * sin(a1);
	y1 = l1 * cos(a1);

	x2 = x1 + l2 * sin(a2);
	y2 = y1 + l2 * cos(a2);

	/* Cube(s) Drawing */
	
	max=30;
	x=0.0f;
	y=0.0f;


	for (i=0; i<max; i++){
		x+=x1/max;
		y+=y1/max;
		memcpy(nmv, mvMat, sizeof nmv);
		translate(nmv, x, -y, 0.0f);
		scale(nmv, scale_cube, scale_cube, scale_cube);
		transform_n_raster(_cube1, nmv, projMat);
	}

	x=x1;
	y=y1;

	for (i=0; i<max; i++){
		if (x2<x1) x-=absolu(x1-x2)/max;
		else x+=absolu(x2-x1)/max;
		if (y2>y1) y+=absolu(y2-y1)/max;
		else y-=absolu(y1-y2)/max;
		memcpy(nmv, mvMat, sizeof nmv);
		translate(nmv, x, -y, 0.0f);
		scale(nmv, scale_cube, scale_cube, scale_cube);
		transform_n_raster(_cube1, nmv, projMat);
	}

	/* Sphere Drawing */

	memcpy(nmv, mvMat, sizeof nmv);
	translate(nmv, x1, -y1, 0.0f);
	scale(nmv, scale_sphere, scale_sphere, scale_sphere);
	transform_n_raster(_sphere1, nmv, projMat);

	memcpy(nmv, mvMat, sizeof nmv);
	translate(nmv, x2, -y2, 0.0f);
	scale(nmv, scale_sphere, scale_sphere, scale_sphere);
	transform_n_raster(_sphere2, nmv, projMat);
	
	/* Calcul velocity / acceleration / angle */

	a1_velocity += a1_acceleration;
	a2_velocity += a2_acceleration;

	a1 += a1_velocity;
	a2 += a2_velocity;
	
	a1_velocity *= 0.999;
	a2_velocity *= 0.999;
	
	//a1 += 0.1;
	//a2 -= 0.2;
	

	gl4dpScreenHasChanged();
	gl4dpUpdateScreen(NULL);
}

void key (int keycode) {
	/* Gestion des interactions */
	/*
	switch (keycode){
		case GL4DK_UP:
			_zcam += 50.0f;
			printf ("Forward\n");
			break;
		case GL4DK_DOWN:
			_zcam -= 50.0f;
			printf ("Back\n");
			break;
		case GL4DK_LEFT:
			_xcam += 50.0f;
			_zcam -= 50.0f;
			printf ("Right Around\n");
			break;
		case GL4DK_RIGHT:
			_xcam -= 50.0f;
			_zcam += 50.0f;
			printf ("Left Around\n");
			break;
		case GL4DK_a:
			scale_sphere += 1;
			scale_cube += 1;
			printf ("scale +\n");
			break;
		case GL4DK_z:
			scale_sphere -= 1;
			scale_cube -= 1;
			printf ("Scale -\n");
			break;
		case GL4D_BUTTON_LEFT:
			printf ("left\n");
			break;
		case GL4D_BUTTON_RIGHT:
			printf ("right\n");
			break;
		case GL4D_BUTTON_MIDDLE:
			printf ("right\n");
			break;
		
		default:
			break;
	}
	*/
}

void sortie(void){
	gl4duClean(GL4DU_ALL);
	freeSurface(_sphere1);
	freeSurface(_sphere2);
	freeSurface(_cube1);
}