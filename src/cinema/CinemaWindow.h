#pragma once

#include <ospray/ospray.h>
#include "cinema_cfg.h"

class CinemaWindow
{
public:

	CinemaWindow(int w = 1920, int h = 1080) : 
		width(w), height(h), show(false)
	{
		osp::vec2i sz(width, height);
		frameBuffer = ospNewFrameBuffer(sz, OSP_RGBA_I8);
	}

	~CinemaWindow()
	{
		ospFreeFrameBuffer(frameBuffer);
	}

	void render(OSPRenderer r) {ospRenderFrame(frameBuffer, r);}
	void save(std::string filename);

	void setShow(bool a) { show = a; }

	void getSize(int& w, int& h) { w = width; h = height; }

protected:
	bool createDisplay();

private:
	int width, height;
	OSPFrameBuffer frameBuffer;

	bool show;
};

