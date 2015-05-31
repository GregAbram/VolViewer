#pragma once

#include <ospray/ospray.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "common.h"

#include <math.h>


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

	void setRenderer(OSPRenderer r)
	{
		renderer = r;
		ospSetObject(renderer, "camera", ospCamera);
	}

	~Camera()
	{
		ospRelease(ospCamera);
	}


	osp::vec3f getPos()  	{return center + eye_dist * xfmPoint(frame, eye_dir); }
	void getPos(float* p) 
	{
		  osp::vec3f t;
			getPos(t);
			p[0] = t.x;
			p[1] = t.y;
			p[2] = t.z;
	}
	void getPos(osp::vec3f &p) 
	{
			p = getPos();
	}

	osp::vec3f getDir()  	{return -eye_dist * xfmPoint(frame, eye_dir); }
	void getDir(float* d) 
	{
		  osp::vec3f t;
			getDir(t);
			d[0] = t.x;
			d[1] = t.y;
			d[2] = t.z;
	}
	void getDir(osp::vec3f &d)
	{ 
			d = getDir();
	}

	osp::vec3f getUp() 	{return up; } 
	void getUp(float* u)  { u[0] = up.x;  u[1] = up.y;  u[2] = up.z; }

	void setPos(float *p) { setPos(p[0], p[1], p[2]); }
	void setPos(osp::vec3f f)  { setPos(f.x, f.y, f.z); }
	void setPos(float x, float y, float z) 
	{
		pos = osp::vec3f(x, y, z); 

		center = pos + dir; 
		eye_dist = length(dir); 
		eye_dir = osp::vec3f(-dir.x / eye_dist, -dir.y / eye_dist, -dir.z / eye_dist);

		modified = true;
  }

	void setDir(float *p) { setDir(p[0], p[1], p[2]); }
	void setDir(osp::vec3f f)  { setDir(f.x, f.y, f.z); }
	void setDir(float x, float y, float z) 
	{
		dir = osp::vec3f(x, y, z); 

		center = pos + dir; 
		eye_dist = length(dir); 
		eye_dir = osp::vec3f(-dir.x / eye_dist, -dir.y / eye_dist, -dir.z / eye_dist);

		modified = true;
  }

	void setUp(float *p) { setUp(p[0], p[1], p[2]); }
	void setUp(osp::vec3f f)  { setUp(f.x, f.y, f.z); }
	void setUp(float x, float y, float z)  { up  = osp::vec3f(x, y, z); modified = true; }

#define PI 3.1415926

	void setPhi(int iphi)
	{
		phi = 0.99 * PI * (iphi/180.0);
		setPhiTheta(theta, phi);
	}

	void setTheta(int itheta)
	{
		theta = PI * 2 * (itheta/360.0);
		setPhiTheta(theta, phi);
	}

	float			 getFovY() 	{return fovY;  } void setFovY(float f)		  { fovY   = f; modified = true;}
	float			 getAspect(){return aspect;} void setAspect(float a)	  { aspect = a; modified = true;}

  /*! set frame 'up' vector. if this vector is (0,0,0) the window will
   *not* apply the up-vector after camera manipulation */
  void snapUp()
  {
    if(fabsf(dot(up,frame.l.vz)) < 1e-3f)
      return;

    frame.l.vx = normalize(cross(frame.l.vy,up));
    frame.l.vz = normalize(cross(frame.l.vx,frame.l.vy));
    frame.l.vy = normalize(cross(frame.l.vz,frame.l.vx));
		modified = true;
  }

	void saveState(Document &doc)
	{
		Value cam(kObjectType), s(kObjectType);

		doc.AddMember("Camera", cam, doc.GetAllocator());

		float v[3];

		getPos(v);
		std::stringstream  p;
		p << v[0] << " " << v[1] << " " << v[2];
		s.SetString(p.str().c_str(), doc.GetAllocator());
		doc["Camera"].AddMember("viewpoint", s, doc.GetAllocator());
	
		getDir(v);
		std::stringstream  d;
		d << v[0] << " " << v[1] << " " << v[2];
		s.SetString(d.str().c_str(), doc.GetAllocator());
		doc["Camera"].AddMember("viewdir", s, doc.GetAllocator());

		std::stringstream  u;
		u << up[0] << " " << up[1] << " " << up[2];
		s.SetString(u.str().c_str(), doc.GetAllocator());
		doc["Camera"].AddMember("viewup", s, doc.GetAllocator());
	
		std::stringstream  a;
		a << aspect;
		s.SetString(a.str().c_str(), doc.GetAllocator());
		doc["Camera"].AddMember("aspect", s, doc.GetAllocator());
	
		std::stringstream  f;
		f << fovY;
		s.SetString(f.str().c_str(), doc.GetAllocator());
		doc["Camera"].AddMember("fovy", s, doc.GetAllocator());
	}

	void loadState(Document &doc)
	{
		if (! doc.HasMember("Camera"))
		{
			std::cerr << "No camera state\n";
		}
		else
		{
			float v[3];

			std::stringstream p;
			p.str(doc["Camera"]["viewpoint"].GetString());
			p >> v[0] >> v[1] >> v[2];
			setPos(v);

			std::stringstream d;
			d.str(doc["Camera"]["viewdir"].GetString());
			d >> v[0] >> v[1] >> v[2];
			setDir(v);

			std::stringstream u;
			u.str(doc["Camera"]["viewup"].GetString());
			u >> up[0] >> up[1] >> up[2];

			std::stringstream a;
			a.str(doc["Camera"]["aspect"].GetString());
			a >> aspect;

			std::stringstream f;
			f.str(doc["Camera"]["fovy"].GetString());
			f >> fovY;

			modified = true;
			commit();
		}
	}

	void commit()
	{
		if (modified)
		{

			osp::vec3f e = getPos(), d = getDir();

			ospSetVec3f(ospCamera,"pos", e);
			ospSetVec3f(ospCamera,"dir", d);
			ospSetVec3f(ospCamera,"up",  up);
			ospSetf(ospCamera,"aspect",  aspect);
			ospSetf(ospCamera,"fovy",    fovY);
			ospCommit(ospCamera);
			modified = false;
		}
	}

	void rotateCenter(float t, float p)
	{
    frame = frame * osp::affine3f::rotate(osp::vec3f(1.0, 0.0, 0.0), p) * osp::affine3f::rotate(osp::vec3f(0.0, 1.0, 0.0), -t);
		up 		= frame.l.vy;

    snapUp();
		modified = true;
	}

	void setPhiTheta(float t, float p)
	{
    frame = osp::affine3f::rotate(osp::vec3f(1.0, 0.0, 0.0), p) * osp::affine3f::rotate(osp::vec3f(0.0, 1.0, 0.0), -t);
		up 		= frame.l.vy;

    snapUp();
		modified = true;
	}

	void zoom(float dy)
	{
		float motionSpeed = 0.012f;
		eye_dist *= (1 + dy*motionSpeed);


		modified = true;
	}

private:
	OSPCamera ospCamera;
	OSPRenderer renderer;

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
