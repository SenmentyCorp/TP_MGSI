/********/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <armadillo>
#include <ctime>
#include <cstdlib>

#include <glm/vec3.hpp>					// glm::vec3
#include <glm/vec4.hpp>					// glm::vec4
#include <glm/mat4x4.hpp>				// glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <GL/glut.h>

#define WX 1000
#define WY 790
#define GAP 10
#define PI 3.14159
using namespace arma;

// variables globales pour OpenGL
int window, width, height, F3D, temoin;
const int NMAX = 100;
int N = 0;
int mp = -1, droite = 0, gauche = 0;
float s = 0.5f;
int presse = 0;
int anglex = 0, angley = 0, xold, yold;
int NP = 50;

bool isCamPanoramique = false, isHelico = false, isFPS = false;

float theta = 0.0f;

enum TypeBouton
{
	action1 = 0,
	action2,
	action3
} bouton_action = action3;

struct Point
{
	double x, y;
	Point(double a = 0, double b = 0) { set(a, b); }
	void set(double a, double b)
	{
		x = a;
		y = b;
	}
};

struct Point3D
{
	double x, y, z;
	Point3D(double a = 0, double b = 0, double c = 0) { set(a, b, c); }
	void set(double a, double b, double c)
	{
		x = a;
		y = b;
		z = c;
	}
};

struct CouleurRVB
{
	double r, v, b;
	CouleurRVB(double a = 0, double b = 0, double c = 0) { set(a, b, c); }
	void set(double a, double b, double c)
	{
		r = a;
		v = b;
		b = c;
	}
};

Point P[NMAX];

const int col = 200;
const int nbPoints = col * col;
Point3D P3D[nbPoints];
CouleurRVB Couleur[nbPoints];

static void menu(int item)
{
	bouton_action = static_cast<TypeBouton>(item);
	glutPostRedisplay();
}
// ---
const int NB_POINTS = 16;
const int DISCRET = 10;
const int N_Parcours = NB_POINTS + 3;
Point P_Parcours[NMAX];
Point rails[NB_POINTS * DISCRET];
int init = 0;

float map(float value, float istart, float istop, float ostart, float ostop)
{
	return ostart + (ostop - ostart) * ((value - istart) / (istop - istart));
}

void initPointsParcours()
{
	P_Parcours[0] = {171., 472.};
	P_Parcours[1] = {260., 471.};
	P_Parcours[2] = {316., 426.};
	P_Parcours[3] = {297., 343.};
	P_Parcours[4] = {254., 262.};
	P_Parcours[5] = {263., 195.};
	P_Parcours[6] = {304., 136.};
	P_Parcours[7] = {297., 63.};
	P_Parcours[8] = {188., 50.};
	P_Parcours[9] = {142., 104.};
	P_Parcours[10] = {67., 121.};
	P_Parcours[11] = {62., 218.};
	P_Parcours[12] = {129., 263.};
	P_Parcours[13] = {143., 351.};
	P_Parcours[14] = {104., 399.};
	P_Parcours[15] = {110., 457.};
}

void catmullromParcours()
{
	mat m = mat(4, 4);
	m << -s << 2 - s << s - 2 << s << endr
	  << 2 * s << s - 3 << 3 - 2 * s << -s << endr
	  << -s << 0 << s << 0 << endr
	  << 0 << 1 << 0 << 0 << endr;

	if (init == 0)
	{
		initPointsParcours();
		init = 1;
	}

	int i;
	for (i = 0; i < N_Parcours; i++)
	{
		colvec vectX = colvec(4);
		vectX << P_Parcours[i % NB_POINTS].x
			  << P_Parcours[(i + 1) % NB_POINTS].x
			  << P_Parcours[(i + 2) % NB_POINTS].x
			  << P_Parcours[(i + 3) % NB_POINTS].x;

		colvec vectY = colvec(4);
		vectY << P_Parcours[i % NB_POINTS].y
			  << P_Parcours[(i + 1) % NB_POINTS].y
			  << P_Parcours[(i + 2) % NB_POINTS].y
			  << P_Parcours[(i + 3) % NB_POINTS].y;

		mat prodMPx = m * vectX;
		mat prodMPy = m * vectY;

		glColor3f(1.0, 0.0, 0.0);
		glBegin(GL_LINE_STRIP);

		int t;
		for (t = 0; t <= DISCRET; t++)
		{
			double a = (double)t / 10.;
			rowvec rv = rowvec(4);
			rv << (double)(pow(a, 3.)) << (double)(pow(a, 2.)) << a << 1.;
			mat cX = rv * prodMPx;
			mat cY = rv * prodMPy;

			glVertex2f(cX(0, 0), cY(0, 0));
			if (t != DISCRET)
			{
				rails[(i % NB_POINTS) * DISCRET + (int)(a * DISCRET)] = {(double)cX(0, 0) * 2. - 500., (double)cY(0, 0) * 2. - 500.};
			}
		}
		glEnd();
	}
}

int param(int ri, int rj)
{
	return 10.0 * (sinf(2.0 * (ri - rj)) * cosf(ri - rj * ri) * cosf(3.0 * (rj - ri)) * PI * ri - rj + sqrt(2 * ri) * abs(sinf(ri * ri)));
}

int calculHauteur(int i)
{
	double x = rails[i].x / 10.;
	double z = rails[i].y / 10.;

	float ri = (x + 50) / 100.0;
	ri *= PI;
	float rj = (z + 50) / 100.0;
	rj *= PI;

	int temp = param(ri, rj);
	return temp;
}

void parcours3D()
{
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINE_STRIP);
	int i, temp;
	for (i = 0; i < NB_POINTS * DISCRET; i++)
	{
		temp = calculHauteur(i);
		glVertex3d(rails[i].x, temp, rails[i].y);
	}
	temp = calculHauteur(0);
	glVertex3d(rails[0].x, temp, rails[0].y);
	glEnd();
} //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

void camPanoramique()
{

	gluLookAt(300, 300, 0, 0, 0, 0, 0, 1, 0);
}

float sigmoid(float x)
{
	float exp_value;
	float return_value;
	exp_value = exp((double)-x);
	return_value = 1 / (1 + exp_value);
	return return_value;
}
float npara(float x, float mult)
{
	float c = mult * mult;
	return map(-x * x, -c, c, 0, 1);
}
float norm(float x)
{
	return x / 255.0;
}

void initializePoints()
{
	int cpt = 0;
	float range = col / 2;
	float max = -1000, min = 1000;
	for (int i = -range; i < range; i++)
	{
		for (int j = -range; j < range; j++)
		{

			float ri = (i + range) / col;
			ri *= PI * 1;
			float rj = (j + range) / col;
			rj *= PI * 1;

			int temp = param(ri, rj);
			P3D[cpt].x = i * (10 / (col / 100));
			P3D[cpt].y = temp;
			P3D[cpt].z = j * (10 / (col / 100));
			cpt++;
			if (max < temp)
				max = temp;
			if (min > temp)
				min = temp;
		}
	}

	float yrange = max - min;
	//cout << max << " " << min << "=" << yrange << endl;
	for (int i = 0; i < nbPoints; i++)
	{
		float c1[3] = {79, 66, 37};
		float c2[3] = {70, 108, 29};
		float c3[3] = {132, 165, 119};
		float c4[4] = {255, 255, 255};
		int palier1 = -1;
		int palier2 = 20;

		Point3D p = P3D[i];
		if (p.y <= palier1)
		{
			Couleur[i].r = map(p.y, min, palier1, norm(c1[0]), norm(c2[0]));
			Couleur[i].v = map(p.y, min, palier1, norm(c1[1]), norm(c2[1]));
			Couleur[i].b = map(p.y, min, palier1, norm(c1[2]), norm(c2[2]));
		}
		else if (p.y >= palier1 && p.y <= palier2)
		{
			Couleur[i].r = map(p.y, palier1, palier2, norm(c2[0]), norm(c3[0]));
			Couleur[i].v = map(p.y, palier1, palier2, norm(c2[1]), norm(c3[1]));
			Couleur[i].b = map(p.y, palier1, palier2, norm(c2[2]), norm(c3[2]));
		}
		else
		{
			Couleur[i].r = map(p.y, palier2, max, norm(c3[0]), norm(c4[0]));
			Couleur[i].v = map(p.y, palier2, max, norm(c3[1]), norm(c4[1]));
			Couleur[i].b = map(p.y, palier2, max, norm(c3[2]), norm(c4[2]));
		}
	}
}
void tracePointsParcours()
{
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_POINTS);
	for (int i = 0; i < N_Parcours - 3; i++)
	{
		glVertex2f(P_Parcours[i].x, P_Parcours[i].y);
	}
	glEnd();
} //&&&&&&&&&&&&&&&&&&
void TracePoints()
{

	float r, v, b;
	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i < nbPoints - col; i++)
	{

		r = Couleur[i].r;
		v = Couleur[i].v;
		b = Couleur[i].b;
		glColor4f(r, v, b, 1);

		if ((i + 1) % col == 0)
		{
			glEnd();
			glBegin(GL_TRIANGLE_STRIP);
		}
		else
		{
			glVertex3f(P3D[i].x, P3D[i].y, P3D[i].z);
			glVertex3f(P3D[i + col].x, P3D[i + col].y, P3D[i + col].z);
			glVertex3f(P3D[i + 1].x, P3D[i + 1].y, P3D[i + 1].z);
			glVertex3f(P3D[i + col + 1].x, P3D[i + col + 1].y, P3D[i + col + 1].z);
		}
	}
	glEnd();
}
void main_reshape(int width, int height)
{
	GLint viewport[4];

	glViewport(0, 0, width, height);
	glGetIntegerv(GL_VIEWPORT, viewport);
	float prof = viewport[2] > viewport[3] ? viewport[2] : viewport[3];

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, viewport[2], 0.0, viewport[3], -prof, prof);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void main_display(void)
{
	glClearColor(0.5, 0.5, 0.5, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glutPostRedisplay();
	glutSwapBuffers();
}

void Mouse(int button, int state, int x, int y)
{
	GLint viewport[4];

	glutSetCursor(GLUT_CURSOR_CROSSHAIR);
	glGetIntegerv(GL_VIEWPORT, viewport);

	if (button == GLUT_RIGHT_BUTTON)
	{
		/*droite = 0;
		gauche = 1;
		glColor3f(0.0, 1.0, 0.0);
		glPointSize(3.0);
		glInitNames();
		glPushName(1);

		P[N].x = x;
		P[N].y = viewport[3] - y;

		glLoadName(N);
		glBegin(GL_POINTS);
		glVertex2f(P[N].x, P[N].y);
		glEnd();

		if (state == GLUT_UP)
			N++;
		glutPostRedisplay();
	}

	if (glutGetModifiers() != GLUT_ACTIVE_CTRL && button == GLUT_LEFT_BUTTON)
	{*/
		gauche = 0;
		droite = 1;
		if (state == GLUT_DOWN)
		{
			GLuint *selectBuf = new GLuint[200];
			GLuint *ptr;
			GLint hits;

			glSelectBuffer(200, selectBuf);
			glRenderMode(GL_SELECT);

			glPushMatrix();
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPickMatrix(x, viewport[3] - y, 5.0, 5.0, viewport);
			glOrtho(0.0, viewport[2], 0.0, viewport[3], -50.0, 50.0);

			glColor3f(0.0, 1.0, 0.0);
			glPointSize(3.0);
			glInitNames();
			glPushName(1);

			for (int i = 0; i < N_Parcours; i++)
			{
				glLoadName(i);
				glBegin(GL_POINTS);
				glVertex2f(P_Parcours[i].x, P_Parcours[i].y);
				glEnd();
			}

			glPopMatrix();
			glFlush();

			hits = glRenderMode(GL_RENDER);
			if (hits)
			{
				ptr = (GLuint *)selectBuf;
				ptr += 3;
				mp = *ptr;
			}
		}

		if (state == GLUT_UP)
			mp = -1;

		main_reshape(viewport[2], viewport[3]);
		glutPostRedisplay();
	}
}

void Motion(int x, int y)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	if ((droite) && (mp != -1))
	{
		int i = mp;
		P_Parcours[i].x = x;
		P_Parcours[i].y = viewport[3] - y;
		tracePointsParcours();

		glutPostRedisplay();
	}
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'f': /* affichage du carre plein */
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glutPostRedisplay();
		break;
	case 'e': /* affichage en mode fil de fer */
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glutPostRedisplay();
		break;
	case 'o':
		if (isCamPanoramique)
		{
			theta += 0.05f;
		}
		break;
	case 'p':
		if (isCamPanoramique)
		{
			theta -= 0.05f;
		}
		break;
	}
}

void F3D_reshape(int x, int y)
{
	if (x < y)
		glViewport(0, (y - x) / 2, x, x);
	else
		glViewport((x - y) / 2, 0, y, y);
}

void temoin_reshape(int width, int height)
{
	GLint viewport[4];
	glViewport(0, 0, width, height);
	glGetIntegerv(GL_VIEWPORT, viewport);
	float prof = viewport[2] > viewport[3] ? viewport[2] : viewport[3];

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, viewport[2], 0.0, viewport[3], -prof, prof);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
} //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

void F3D_affichage()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(-5000, 5000, -5000, 5000, -10000, 10000);
	glRotatef(-(float)0.0, 1.0, 0.0, 0.0);
	glRotatef(-(float)0.0, 0.0, 1.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	if (isCamPanoramique)
	{
		gluLookAt(sinf(theta) * 600, 230, cosf(theta) * 600, 0, 0, 0, 0, 1, 0);
	}
	if (isHelico)
	{
		gluLookAt(1, 550, 0, 0, 0, 0, 0, 1, 0);
	}
	if (isFPS)
	{
	}

	glutPostRedisplay();

	//catmullRom3D();
	//glutSolidCube(500);
	TracePoints();
	parcours3D();

	glutSwapBuffers();
}

void temoin_affichage()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (bouton_action)
	{
	case action1:
		isCamPanoramique = false;
		isHelico = false;
		isFPS = true;
		break;
	case action2:
		isCamPanoramique = false;
		isHelico = true;
		isFPS = false;
		break;
	case action3:
		isCamPanoramique = true;
		isHelico = false;
		isFPS = false;
		break;
	}

	glColor3f(0.0, 1.0, 0.0);
	glPointSize(3.0);
	glInitNames();
	glPushName(1);
	for (int i = 0; i < N_Parcours; i++)
	{
		glLoadName(i);
		glBegin(GL_POINTS);
		glVertex2f(P_Parcours[i].x, P_Parcours[i].y);
		glEnd();
	}
	tracePointsParcours();
	catmullromParcours();

	//TracePoints();
	glutPostRedisplay();

	glutSwapBuffers();
}

void F3D_motion(int x, int y)
{
	{
		if (presse) /* si le bouton gauche est presse */
		{
			/* on modifie les angles de rotation de l'objet
           en fonction de la position actuelle de la souris et de la derniere
           position sauvegardee */
			anglex = anglex + (x - xold);
			angley = angley + (y - yold);
			glutPostRedisplay(); /* on demande un rafraichissement de l'affichage */
		}

		xold = x; /* sauvegarde des valeurs courante de le position de la souris */
		yold = y;
	}
}

void F3D_mouse(int button, int state, int x, int y)
{
	/* si on appuie sur le bouton gauche */
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		presse = 1; /* le booleen presse passe a 1 (vrai) */
		xold = x;   /* on sauvegarde la position de la souris */
		yold = y;
	}
	/* si on relache le bouton gauche */
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		presse = 0; /* le booleen presse passe a 0 (faux) */
}

int main(int argc, char **argv)
{
	srand(static_cast<unsigned>(time(0)));
	initializePoints();

	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WX, WY);
	glutInitWindowPosition(0, 0);
	glutInit(&argc, argv);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	window = glutCreateWindow("Tchouk Tchouk");
	glutReshapeFunc(main_reshape);
	glutDisplayFunc(main_display);

	//Fenetre 3D
	F3D = glutCreateSubWindow(window, GAP, GAP, 780, 780);

	glClearColor(0.43, 0.75, 0.7, 1);
	glutReshapeFunc(F3D_reshape);
	glutDisplayFunc(F3D_affichage);
	glutMotionFunc(F3D_motion);
	glutMouseFunc(F3D_mouse);
	glutKeyboardFunc(keyboard);

	//Fenetre 2D
	temoin = glutCreateSubWindow(window, 785 + GAP, GAP, 200, 200);
	glutReshapeFunc(temoin_reshape);
	glutDisplayFunc(temoin_affichage);
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);

	glutCreateMenu(menu);
	glutAddMenuEntry("FPS", action1);
	glutAddMenuEntry("Helico", action2);
	glutAddMenuEntry("Vue panoramique", action3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	glutPostRedisplay();

	glutMainLoop();
	return (1);
}
