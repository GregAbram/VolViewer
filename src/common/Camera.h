#pragma once

#include <ospray/ospray.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "common.h"

#include <math.h>

#include "Lights.h"

class Camera
{
public:
  Camera() :
				pos(0,-1,0),
				dir(0,1,0),
				up(0,0,1),
				aspect(1.f),
				fovY(40.f),
				modified(true)
  {
    frame = osp::affine3f(embree::one);
		ospCamera = ospNewCamera("perspective");
		modified = true;
  }

	~Camera()
	{
		ospRelease(ospCamera);
	}

	void setRenderer(OSPRenderer r);
	osp::vec3f getPos();
	void getPos(float* p);
	void getPos(osp::vec3f &p) ;
	osp::vec3f getDir();
	void getDir(float* d);
	void getDir(osp::vec3f &d);

	osp::vec3f getUp();
	void getUp(float* u);

	void setPos(float *p);
	void setPos(osp::vec3f f);
	void setPos(float x, float y, float z);

	void setDir(float *p);
	void setDir(osp::vec3f f);
	void setDir(float x, float y, float z);

	void setUp(float *p);
	void setUp(osp::vec3f f);
	void setUp(float x, float y, float z);

#define PI 3.1415926

	void setPhi(int iphi);
	void setTheta(int itheta);

	float			 getFovY();
	float			 getAspect();

	void setFovY(float f);
	void setAspect(float a);

  void snapUp();

	void saveState(Document &doc, Value &section);
	void loadState(Value& cam);

	void commit();

	void rotateCenter(float t, float p);
	void setPhiTheta(float t, float p);
	void zoom(float dy);
	Lights *getLights();

private:
	OSPCamera ospCamera;
	OSPRenderer renderer;

	Lights cameraLights;

	float phi, theta; 

  osp::affine3f frame;

  osp::vec3f pos;
  osp::vec3f dir;
  osp::vec3f up;

	float eye_dist;
	osp::vec3f center;
	osp::vec3f eye_dir;

  float fovY;
  float aspect;
  bool modified;
};
