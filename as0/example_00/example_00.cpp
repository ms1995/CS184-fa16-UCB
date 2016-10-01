// CS184 Simple OpenGL Example
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

#define PI 3.14159265

using namespace std;

//****************************************************
// Some Classes
//****************************************************
class Viewport {
	public:
		int w, h; // width and height
};


//****************************************************
// Global Variables
//****************************************************
Viewport    viewport;

//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
	viewport.w = w;
	viewport.h = h;

	glViewport(0,0,viewport.w,viewport.h);// sets the rectangle that will be the window
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();                // loading the identity matrix for the screen

	//----------- setting the projection -------------------------
	// glOrtho sets left, right, bottom, top, zNear, zFar of the chord system


	// glOrtho(-1, 1 + (w-400)/200.0 , -1 -(h-400)/200.0, 1, 1, -1); // resize type = add
	// glOrtho(-w/400.0, w/400.0, -h/400.0, h/400.0, 1, -1); // resize type = center

	glOrtho(0, 1, 1, 0, 1, -1);    // resize type = stretch

	//------------------------------------------------------------
}


//****************************************************
// sets the window up
//****************************************************
void initScene(){
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f); // Clear to black, fully transparent

	myReshape(viewport.w,viewport.h);
}

int status = 3, passed = 0, lastScored = -1, ct_timer = 0;

const int BARRIER = 5;
const float BARRIER_SPAN = 1.0f / (BARRIER - 1);
const float BARRIER_WIDTH = BARRIER_SPAN * 0.2f;
int curr_barrier = 0;
float barriers[BARRIER], curr_pos = 0.0f;

const float gravity = 0.0001f, half_bird_size = 0.02f;
float bird_y = 0.5f, bird_velocity = 0.0f;

float getRand() {
	return (float)(rand() % 20 + 40) / 100;
}

void drawBitmapText(const char *c, float x, float y, float z) {
	glColor3f(1.0f, 0.0f, 0.0f);
	glRasterPos3f(x, y, z);
	do {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	} while (*c++);
}

//***************************************************
// function that does the actual drawing
//***************************************************
void myDisplay() {
	glClear(GL_COLOR_BUFFER_BIT);                // clear the color buffer (sets everything to black)

	glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
	glLoadIdentity();                            // make sure transformation is "zero'd"

	int i, j, k;

	// Draw bird
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_POLYGON);
	glVertex3f(0.5f - half_bird_size, bird_y - half_bird_size, 0.0f);
	glVertex3f(0.5f + half_bird_size, bird_y - half_bird_size, 0.0f);
	glVertex3f(0.5f + half_bird_size, bird_y + half_bird_size, 0.0f);
	glVertex3f(0.5f - half_bird_size, bird_y + half_bird_size, 0.0f);
	glEnd();

	// Draw barriers
	glColor3f(0.0f, 0.0f, 1.0f);

	for (i = 0; i != BARRIER; ++i) {
		j = (i + curr_barrier) % BARRIER;
		glBegin(GL_POLYGON);
		float upper_left = BARRIER_SPAN * i + (BARRIER_SPAN - BARRIER_WIDTH) / 2 - curr_pos;
		glVertex3f(upper_left, 0.0f, 0.0f);
		glVertex3f(upper_left + BARRIER_WIDTH, 0.0f, 0.0f);
		glVertex3f(upper_left + BARRIER_WIDTH, barriers[j], 0.0f);
		glVertex3f(upper_left, barriers[j], 0.0f);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex3f(upper_left, barriers[j] + 0.1f, 0.0f);
		glVertex3f(upper_left + BARRIER_WIDTH, barriers[j] + 0.1f, 0.0f);
		glVertex3f(upper_left + BARRIER_WIDTH, 1.0f, 0.0f);
		glVertex3f(upper_left, 1.0f, 0.0f);
		glEnd();
		if ((upper_left < 0.5f + half_bird_size && upper_left + BARRIER_WIDTH > 0.5f + half_bird_size)
			|| (upper_left < 0.5f - half_bird_size && upper_left + BARRIER_WIDTH > 0.5f - half_bird_size)) {
			// crossing or crashing into the barrier
			if (bird_y - half_bird_size < barriers[j] || bird_y + half_bird_size > barriers[j] + 0.1f) {
				// crossing
				status = 1;
			} else if (lastScored != j) {
				++passed;
				lastScored = j;
			}
		}
	}

	if (++ct_timer == 100)
		ct_timer = 0;
	if (status == 0 || status == 3 || ct_timer < 50) {
		// Show status
		char buf[256];
		sprintf(buf, "Score: %d", passed);
		drawBitmapText(buf, 0.01f, 0.05f, 0.0f);
	}

	if (!status) {
		bird_y += bird_velocity;
		if (bird_y + half_bird_size > 1.0f)
			status = 2;
		bird_velocity += gravity;
	}

	if (!status) {
		curr_pos += 0.001f;
		if (curr_pos >= BARRIER_SPAN) {
			curr_pos = 0.0f;
			barriers[curr_barrier++] = getRand();
			curr_barrier %= BARRIER;
		}
	}

/*
	//----------------------- ----------------------- -----------------------
	// This is a quick hack to add a little bit of animation.
	static float tip = 0.5f;
	const  float stp = 0.001f;
	const  float beg = 0.1f;
	const  float end = 0.9f;

	tip += stp;
	if (tip>end) tip = beg;
	//----------------------- ----------------------- -----------------------

	//----------------------- code to draw objects --------------------------
	// Rectangle Code
	//glColor3f(red component, green component, blue component);
	glColor3f(1.0f,0.0f,0.0f);                   // setting the color to pure red 90% for the rect
	glBegin(GL_POLYGON);                         // draw rectangle 
	//glVertex3f(x val, y val, z val (won't change the point because of the projection type));
	glVertex3f(-0.8f, 0.0f, 0.0f);               // bottom left corner of rectangle
	glVertex3f(-0.8f, 0.5f, 0.0f);               // top left corner of rectangle
	glVertex3f( 0.0f, 0.5f, 0.0f);               // top right corner of rectangle
	glVertex3f( 0.0f, 0.0f, 0.0f);               // bottom right corner of rectangle
	glEnd();
	// Triangle Code
	glColor3f(1.0f,0.5f,0.0f);                   // setting the color to orange for the triangle

	float basey = sqrt(0.48f);                  // height of triangle = sqrt(.8^2-.4^2)
	glBegin(GL_POLYGON);
	glVertex3f(tip,  0.0f, 0.0f);                // top tip of triangle
	glVertex3f(0.1f, basey, 0.0f);               // lower left corner of triangle
	glVertex3f(0.9f, basey, 0.0f);               // lower right corner of triangle
	glEnd();
	//-----------------------------------------------------------------------
*/

	glFlush();
	glutSwapBuffers();                           // swap buffers (we earlier set double buffer)
}

void keyPressed (unsigned char key, int x, int y) {
	// printf("%c\n", key);
	switch (status) {
		case 0:
			if (key == ' ')
				// bird_velocity -= 0.002f;
				bird_velocity = -0.002f;
			else
				status = 3;
			break;
		case 3:
			status = 0;
			break;
		default:
			if (key == 'r') {
				bird_y = 0.5f;
				curr_pos = bird_velocity = 0.0f;
				passed = 0;
				lastScored = -1;
				status = 0;
			}
	}
}

//****************************************************
// called by glut when there are no messages to handle
//****************************************************
void myFrameMove() {
	//nothing here for now
#ifdef _WIN32
	Sleep(10);                                   //give ~10ms back to OS (so as not to waste the CPU)
#endif
	glutPostRedisplay(); // forces glut to call the display function (myDisplay())
}


//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
	srand((unsigned)time(NULL));

	for (int i = 0; i < BARRIER; ++i)
		barriers[i] = getRand();

	//This initializes glut
	glutInit(&argc, argv);

	//This tells glut to use a double-buffered window with red, green, and blue channels 
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

	// Initalize theviewport size
	viewport.w = 400;
	viewport.h = 400;

	//The size and position of the window
	glutInitWindowSize(viewport.w, viewport.h);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Yet Another Flappy Bird for CS184");

	initScene();                                 // quick function to set up scene

	glutDisplayFunc(myDisplay);                  // function to run when its time to draw something
	glutReshapeFunc(myReshape);                  // function to run when the window gets resized
	glutKeyboardFunc(keyPressed);
	glutIdleFunc(myFrameMove);                   // function to run when not handling any other task
	glutMainLoop();                              // infinite loop that will keep drawing and resizing and whatever else

	return 0;
}
