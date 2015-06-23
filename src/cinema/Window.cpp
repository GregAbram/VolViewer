#include "Window.h"
#include "mypng.h"

#include <stdlib.h>
#include <iostream>
#include <stdio.h>

using namespace std;

void Window::save(std::string filename)
{
	unsigned int *mappedFrameBuffer = (unsigned int *)ospMapFrameBuffer(frameBuffer);
	write_png(filename.c_str(), width, height, mappedFrameBuffer);
  ospUnmapFrameBuffer(mappedFrameBuffer, frameBuffer);
}
