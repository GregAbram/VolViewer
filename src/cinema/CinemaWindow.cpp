#include "CinemaWindow.h"
#include "mypng.h"
#include "cinema_cfg.h"

#include <iostream>

#if WITH_DISPLAY_WINDOW

#define GL_GLEXT_PROTOTYPES
#include <X11/Xlib.h>
#include <GL/glx.h>

Display *dpy;
GLXContext ctx;
Window win;

bool
CinemaWindow::createDisplay()
{
	dpy = XOpenDisplay(getenv("DISPLAY"));
	if (! dpy)
	{
		std::cerr << "error opening display\n";
		return false;
	}

	int wattr[] = {
			GLX_RGBA,
			GLX_RED_SIZE,    8,
			GLX_GREEN_SIZE,  8,
			GLX_BLUE_SIZE,   8,
			None
	};

	XVisualInfo *vinf = glXChooseVisual(dpy, DefaultScreen(dpy), wattr);
	if (! vinf)
	{
		std::cerr << "error getting visual\n";
		return false;
	}

	ctx = glXCreateContext(dpy, vinf, NULL, 1);
	if (! ctx)
	{
		std::cerr << "unable to create glX context\n";
		return false;
  }

	XSetWindowAttributes swaattr;
	swaattr.colormap = XCreateColormap(dpy, DefaultRootWindow(dpy), vinf->visual, AllocNone);
	swaattr.override_redirect = False;

	win = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, width, height, 0,
		24, InputOutput, vinf->visual, CWColormap | CWOverrideRedirect, &swaattr);

	if (! win)
	{
			std::cerr << "unable to create window\n";
			return false;
	}

	XMapRaised(dpy, win);
	return true;
}
#else
bool
CinemaWindow::createDisplay() {}
#endif

void CinemaWindow::save(std::string filename)
{
	unsigned int *mappedFrameBuffer = (unsigned int *)ospMapFrameBuffer(frameBuffer);

#if WITH_DISPLAY_WINDOW
	if (show && !dpy)
		show = createDisplay();

	if (show)
	{
		glXMakeCurrent(dpy, win, ctx);
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)mappedFrameBuffer);
    glFlush();
	}
#endif
		
	unsigned int *buf = new unsigned int[width*height];
	for (int i = 0; i < width*height; i++)
		buf[i] = (mappedFrameBuffer[i] == 0) ? 0xff010101 : mappedFrameBuffer[i];

	write_png(filename.c_str(), width, height, buf);

	delete[] buf;

	ospUnmapFrameBuffer(mappedFrameBuffer, frameBuffer);
}
