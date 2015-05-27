#pragma once
#include <ospray/ospray.h>
#include <iostream>
#include <fstream>

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

	osp::vec3f getPos()  	{return pos;   } 
	osp::vec3f getDir()  	{return dir;   } 
	osp::vec3f getUp()   	{return up;    } 

	void getPos(float* p) { p[0] = pos.x; p[1] = pos.y; p[2] = pos.z; }
	void getDir(float* d) { d[0] = dir.x; d[1] = dir.y; d[2] = dir.z; }
	void getUp(float* u)  { u[0] = up.x;  u[1] = up.y;  u[2] = up.z; }

	void setPos(osp::vec3f f)  { pos    = f; modified = true;}
	void setDir(osp::vec3f a)  { dir    = a; modified = true;}
	void setUp(osp::vec3f u)   { up     = u; modified = true;}

	void setPos(float *p) { pos = osp::vec3f(p[0], p[1], p[2]); modified = true; }
	void setDir(float *d) { dir = osp::vec3f(d[0], d[1], d[2]); modified = true; }
	void setUp(float *u)  { up  = osp::vec3f(u[0], u[1], u[2]); modified = true; }

#define PI 3.1415926

	void setPhi(int iphi)
	{
		phi = 0.99 * PI * (iphi/180.0);
		float xy = cos(phi);
		osp::vec3f xyz(xy*cos(theta), xy*sin(theta), sin(phi));
		pos = center + dist * xyz;
		dir = -dist * xyz;
		modified = true;
	}

	void setTheta(int itheta)
	{
		theta = PI * 2 * (itheta/360.0);
		float xy = cos(phi);
		osp::vec3f xyz(xy*cos(theta), xy*sin(theta), sin(phi));
		pos = center + dist * xyz;
		dir = -dist * xyz;
		modified = true;
	}

	void setPhiTheta(int iphi, int itheta)
	{
		setPhi(iphi);
		setTheta(itheta);
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

	void saveState(std::ostream& out)
	{
		out << "pos " << pos[0] << " " << pos[1] << " " << pos[2] << "\n";
		out << "dir " << dir[0] << " " << dir[1] << " " << dir[2] << "\n";
		out << "up " << up[0] << " " << up[1] << " " << up[2] << "\n";
		out << "aspect " << aspect << "\n";
		out << "fovy " << fovY << "\n";
	}

	void loadState(std::istream& in)
	{
		std::string cmd;
		in >> cmd >> pos[0] >> pos[1] >> pos[2];
		in >> cmd >> dir[0] >> dir[1] >> dir[2];
		in >> cmd >> up[0] >> up[1] >> up[2];
		in >> cmd >> aspect;
		in >> cmd >> fovY;

		center = pos + dir;

		dist   = length(dir);
		modified = true;
		commit();
	}

	void commit()
	{
		if (modified)
		{
			// fprintf(stderr, "pos %f %f %f dir %f %f %f\n", pos.x, pos.y, pos.z, dir.x, dir.y, dir.z);

			ospSetVec3f(ospCamera,"pos", pos);
			ospSetVec3f(ospCamera,"dir", dir);
			ospSetVec3f(ospCamera,"up",  up);
			ospSetf(ospCamera,"aspect",  aspect);
			ospSetf(ospCamera,"fovy",    fovY);
			ospCommit(ospCamera);
			modified = false;
		}
	}

	void rotateCenter(float t, float p)
	{
		osp::affine3f xfm = osp::affine3f::rotate(osp::vec3f(1.0, 0.0, 0.0), p);
    frame = frame * xfm;

		xfm = osp::affine3f::rotate(osp::vec3f(0.0, 1.0, 0.0), -t);
    frame = frame * xfm;

    pos   = center + dist*xfmPoint(frame, osp::vec3f(0.0, 0.0, -1.0));

    dir   = center - pos;
		up 		= frame.l.vy;

    snapUp();
		modified = true;
	}

	void zoom(float dy)
	{
		float motionSpeed = 0.012f;
		dist *= (1 + dy*motionSpeed);

    pos   = center + dist*xfmPoint(frame, osp::vec3f(0.0, 0.0, -1.0));

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

	float dist;
	osp::vec3f center;

  float fovY;
  float aspect;
  bool modified;
};
