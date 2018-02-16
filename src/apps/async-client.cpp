#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <glut.h>
#include <pthread.h>

#include "Debug.h"
#include "Socket.h"
#include "async.h"

#include "Pixel.h"

using namespace std;

#define WIDTH  500
#define HEIGHT 500
int width = WIDTH, height = HEIGHT;

float*       pixels = NULL;
int*         frameids = NULL;
volatile int frame = -1;
pthread_t 	 receiver_tid;

Socket *skt;

void
SendStart(int w, int h, string statefile)
{
	int sz = 3*sizeof(int) + statefile.length() + 1;
	char *buf = new char[sz];

	char *p = buf;
	*(int *)p = START;
	p += sizeof(int);
	*(int *)p = w;
	p += sizeof(int);
	*(int *)p = h;
	p += sizeof(int);
	memcpy(p, statefile.c_str(), statefile.length()+1);

	skt->Send(buf, sz);
	delete[] buf;
}

void
SendMouseDown(float x, float y)
{
	std::cerr << "md " << x << " " << y << "\n";
	int sz = sizeof(int) + 2*sizeof(float);
	char *buf = new char[sz];

	char *p = buf;
	*(int *)p = MOUSEDOWN;
	p += sizeof(int);
	*(float *)p = x;
	p += sizeof(float);
	*(float *)p = y;

	skt->Send(buf, sz);
	delete[] buf;
}

void
SendMouseMotion(float x, float y)
{
	std::cerr << "mm " << x << " " << y << "\n";
	int sz = sizeof(int) + 2*sizeof(float);
	char *buf = new char[sz];

	char *p = buf;
	*(int *)p = MOUSEMOTION;
	p += sizeof(int);
	*(float *)p = x;
	p += sizeof(float);
	*(float *)p = y;

	skt->Send(buf, sz);
	delete[] buf;
}

void
SendQuit()
{
	int sz = sizeof(int);
	char *buf = new char[sz];

	char *p = buf;
	*(int *)p = QUIT;

	skt->Send(buf, sz);
	delete[] buf;

	pthread_join(receiver_tid, NULL);
}

void
draw(void)
{
	if (pixels)
	{
		glDrawPixels(width, height, GL_RGBA, GL_FLOAT, pixels);
		glutSwapBuffers();
	}
}

void
keyboard(unsigned char ch, int x, int y)
{
  switch (ch)
	{
    case 0x1B: 
			SendQuit();
      exit(0);
		case 0x40:
			glutPostRedisplay();
			break;
  }
}

void
menu(int item)
{
  switch (item) {
    case 0x1B: 
			SendQuit();
      exit(0);
  }
}

void
mousefunc(int k, int s, int x, int y)
{
  if (s == GLUT_DOWN)
		SendMouseDown(-1.0 + 2.0*(float(x)/width), -1.0 + 2.0*(float(y)/height));
}
  
void
motionfunc(int x, int y)
{
	SendMouseMotion(-1.0 + 2.0*(float(x)/width), -1.0 + 2.0*(float(y)/height));
}

void *
receiver_thread(void *)
{
	char *buf; int n;
  
	while (true)
	{
		skt->Recv(buf, n);

		char *ptr = buf;
		int knt = *(int *)ptr;
		ptr += sizeof(int);
		int frame = *(int *)ptr;
		ptr += sizeof(int);
		Pixel *p = (Pixel *)ptr;

		for (int i = 0; i < knt; i++, p++)
		{
			size_t offset = (((height-1)-(p->y))*width + ((width-1)-(p->x)));
			float *pix = pixels + (offset<<2);
			if (frameids[offset] < frame)
			{
				pix[0] = 0;
				pix[1] = 0;
				pix[2] = 0;
				pix[3] = 0;
			}
			*pix++ += p->r;
			*pix++ += p->g;
			*pix++ += p->b;
			*pix++ += p->o;
		}

		free(buf);
	}

	pthread_exit(NULL);
}

void
syntax(char *a)
{
  cerr << "syntax: " << a << " [options] statefile\n";
  cerr << "options:\n";
  cerr << "  -D         run debugger\n";
	cerr << "  -H host    host (localhost)\n";
	cerr << "  -P port		port (5001)\n";
  cerr << "  -A         wait for attachment\n";
  cerr << "  -s w h     image size (512 x 512)\n";
  exit(1);
}

int
main(int argc, char *argv[])
{
  bool dbg = false, atch = false;
	string host = "localhost";
	string statefile = "";
	int port = 5001;

  for (int i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "-A")) dbg = true, atch = true;
    else if (!strcmp(argv[i], "-D")) dbg = true, atch = false;
    else if (!strcmp(argv[i], "-H")) host = argv[++i];
    else if (!strcmp(argv[i], "-P")) port = atoi(argv[++i]);
    else if (!strcmp(argv[i], "-s"))
    {
      width  = atoi(argv[++i]);
      height = atoi(argv[++i]);
    }
    else if (statefile == "")
			statefile = argv[i];
		else
      syntax(argv[0]);
  }

	if (statefile == "")
		syntax(argv[0]);

  Debug *d = dbg ? new Debug(argv[0], atch) : NULL;
	skt = new Socket((char *)host.c_str(), port);

	pixels   = (float *)malloc(width*height*4*sizeof(float));
	frameids = (int *)malloc(width*height*sizeof(int));

	memset(frameids, 0, width*height*sizeof(int));
	for (int i = 0; i < 4*width*height; i++)
			pixels[i] = 0.0;

	pthread_t receiver_tid;
	pthread_create(&receiver_tid, NULL, receiver_thread, NULL);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowSize(width, height);
  glutCreateWindow("render");
  glutIdleFunc(draw);
  glutDisplayFunc(draw);
  glutMotionFunc(motionfunc);
  glutMouseFunc(mousefunc);
  glutKeyboardFunc(keyboard);
  glutCreateMenu(menu);
  glutAddMenuEntry("Quit", QUIT);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
  glClearDepth(1.0);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glMatrixMode(GL_PROJECTION);
  glOrtho(-1, 1, -1, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glRasterPos2i(-1, -1);
  frame = 1;

	SendStart(width, height, statefile);

  glutMainLoop();

  return 0;
}