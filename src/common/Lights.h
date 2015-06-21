#pragma once
// #include <ospray/lights/Light.h>
#include <ospray/ospray.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "common.h"

struct Light
{
  Light(float x, float y, float z, float r, float g, float b) :
					x(x),
					y(y),
					z(z),
					r(r),
					g(g),
					b(b) 
	{}
  Light() :
					x(1.0),
					y(-2.0),
					z(-3.0),
					r(1.0),
					g(1.0),
					b(1.0) 
	{}

	float x, y, z, r, g, b;
};

class Lights {
public:

  Lights();

	void addLight(float x, float y, float z, float r, float g, float b);
	void commit(OSPRenderer r, osp::affine3f frame);
	void commit(OSPRenderer r);
	void loadState(Value &section);
	void saveState(Document &doc, Value &section);
	void clear();

private:
	std::vector<Light> lights;
};
	
