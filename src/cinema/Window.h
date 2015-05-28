#pragma once

#include <ospray/ospray.h>

class Window
{
public:

	Window(int w = 1920, int h = 1080) : width(w), height(h)
	{
		osp::vec2i sz(width, height);
		frameBuffer = ospNewFrameBuffer(sz, OSP_RGBA_I8);
	}

	~Window()
	{
		ospFreeFrameBuffer(frameBuffer);
	}

	void resize(int w, int h)
	{
		ospFreeFrameBuffer(frameBuffer);

		width = w; height = h;
		osp::vec2i sz(width, height);
		frameBuffer = ospNewFrameBuffer(sz, OSP_RGBA_I8);
	}

	void render(OSPRenderer r) { ospRenderFrame(frameBuffer, r); }
	void save(std::string filename);

private:
	int width, height;
	OSPFrameBuffer frameBuffer;
};

