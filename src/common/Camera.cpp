#include <unistd.h>
#include <stdlib.h>
#include "Camera.h"
#define PI 3.1415926

void
Camera::setRenderer(OSPRenderer r)
{
	renderer = r;
	ospSetObject(renderer, "camera", ospCamera);
}

void
Camera::setPCU(osp::vec3f p, osp::vec3f c, osp::vec3f u)
{
	pos = p;
	center = c;
	up = u;
	setupFrame();
}

osp::vec3f
Camera::getPos()
{
	return pos;
}

void
Camera::getPos(float* p) 
{
	p[0] = pos.x;
	p[1] = pos.y;
	p[2] = pos.z;
}

void
Camera::getPos(float& x, float& y, float& z)
{
	x = pos.x;
	y = pos.y;
	z = pos.z;
}

void
Camera::getPos(osp::vec3f &p) 
{
	p = pos;
}

void
Camera::getCenter(float* d) 
{
	d[0] = center.x;
	d[1] = center.y;
	d[2] = center.z;
}

void
Camera::getCenter(float& x, float& y, float& z)
{
	x = center.x;
	y = center.y;
	z = center.z;
}

void
Camera::getCenter(osp::vec3f &p) 
{
	p = center;
}

void
Camera::getUp(float& x, float& y, float& z)
{
	x = up.x;
	y = up.y;
	z = up.z;
}

osp::vec3f
Camera::getUp()
{
	return up;
} 

void
Camera::getUp(float* u)
{
	u[0] = up.x;
	u[1] = up.y;
	u[2] = up.z;
}

void 
Camera::getUp(osp::vec3f &u)
{
	u = up;
}

void 
Camera::setPhi(int iphi)
{
	phi = 0.99 * PI * (iphi/180.0);
}

void 
Camera::setTheta(int itheta)
{
	theta = PI * 2 * (itheta/360.0);
}

float			 
Camera::getFovY()
{
	return aov;
}

void 
Camera::setFovY(float f)
{
	aov = f; modified = true;
}

float			 
Camera::getAspect()
{
	return aspect;
}

void 
Camera::setAspect(float a)
{
	aspect = a; modified = true;
}

void 
Camera::setupFrame()
{
	up = normalize(up);
	dir = normalize(center - pos);

	right = cross(up, dir);
	right = normalize(right);

	frame = osp::affine3f::AffineSpaceT(right, up, dir, center);
}

void 
Camera::saveState(Document &doc, Value &section)
{
	Value cam(kObjectType), s(kObjectType);

	std::stringstream  p;

	p << center.x <<  " " << center.y << " " << center.z;
	s.SetString(p.str().c_str(), doc.GetAllocator());
	cam.AddMember("center", s, doc.GetAllocator());

	p.clear();
	p << pos.x <<  " " << pos.y << " " << pos.z;
	s.SetString(p.str().c_str(), doc.GetAllocator());
	cam.AddMember("viewpoint", s, doc.GetAllocator());

	p.clear();
	p << up.x <<  " " << up.y << " " << up.z;
	s.SetString(p.str().c_str(), doc.GetAllocator());
	cam.AddMember("viewup", s, doc.GetAllocator());

	p.clear();
	p << aov;
	s.SetString(p.str().c_str(), doc.GetAllocator());
	cam.AddMember("aov", s, doc.GetAllocator());

	cameraLights.saveState(doc, cam);

	section.AddMember("Camera", cam, doc.GetAllocator());
}

void 
Camera::loadState(Value& cam)
{
	float v[3];

	std::stringstream p;

	p.str(cam["center"].GetString());
	p >> center.x >> center.y >> center.z;

	p.str(cam["viewpoint"].GetString());
	p >> pos.x >> pos.y >> pos.z;

	p.str(cam["viewup"].GetString());
	p >> up.x >> up.y >> up.z;

	p.str(cam["aov"].GetString());
	p >> aov;

	if (cam.HasMember("Lights"))
		cameraLights.loadState(cam["Lights"]);

	commit();
}

void 
Camera::commit()
{
	ospSetVec3f(ospCamera,"pos", pos);
	ospSetVec3f(ospCamera,"dir", dir);
	ospSetVec3f(ospCamera,"up",  up);
	ospSetf(ospCamera,"aspect",  aspect);
	ospSetf(ospCamera,"fovy",    aov);
	ospCommit(ospCamera);

	cameraLights.commit(renderer, frame);
}

static void
rotmat(float theta, osp::vec3f axis, float *mat)
{
	float ct = cos(theta);
	float st = sin(theta);

	mat[0] = ct + axis.x*axis.x*(1-ct); 		mat[1] = axis.x*axis.y*(1-ct) - axis.z*st; 	mat[2] = axis.x*axis.z*(1-ct) + axis.y*st;
	mat[3] = axis.x*axis.y*(1-ct) + axis.z*st; 	mat[4] = ct + axis.y*axis.y*(1-ct);	 	mat[5] = axis.y*axis.z*(1-ct) - axis.x*st;
	mat[6] = axis.x*axis.z*(1-ct) - axis.y*st; 	mat[7] = axis.y*axis.z*(1-ct) + axis.x*st; 	mat[8] = ct + axis.z*axis.z*(1-ct);
}

static void
matmat(float *a, float *b, float *c)
{
	c[0] = a[0]*b[0] + a[1]*b[3] + a[2]*b[6];
	c[1] = a[0]*b[1] + a[1]*b[4] + a[2]*b[7];
	c[2] = a[0]*b[2] + a[1]*b[5] + a[2]*b[8];

	c[3] = a[3]*b[0] + a[4]*b[3] + a[5]*b[6];
	c[4] = a[3]*b[1] + a[4]*b[4] + a[5]*b[7];
	c[5] = a[3]*b[2] + a[4]*b[5] + a[5]*b[8];

	c[6] = a[5]*b[0] + a[7]*b[3] + a[8]*b[6];
	c[7] = a[5]*b[1] + a[7]*b[4] + a[8]*b[7];
	c[8] = a[5]*b[2] + a[7]*b[5] + a[8]*b[8];
}

static void
matvec(float *m, osp::vec3f& v)
{
	float x, y, z;

	x = m[0]*v.x + m[1]*v.y + m[2]*v.z;
	y = m[3]*v.x + m[4]*v.y + m[5]*v.z;
	z = m[6]*v.x + m[7]*v.y + m[8]*v.z;
	
	v.x = x;
	v.y = y;
	v.z = z;
}

static float theta = 0;

void 
Camera::rotateFrame(float dx, float dy)
{
	float X[9], Y[9], YX[9];

	dx = -dx;

	rotmat(dx, up, X);
	rotmat(dy, right, Y);
	matmat(Y, X, YX);

	dir = center - pos;

	float d = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
	dir.x /= d;
	dir.y /= d;
	dir.z /= d;

	matvec(YX, dir);
	dir = normalize(dir);

	dir.x *= d;
	dir.y *= d;
	dir.z *= d;

	pos = center - dir;

	matvec(YX, up);
	up = normalize(up);

	matvec(YX, right);
	right = normalize(right);

	// right = cross(up, dir);
	// right = normalize(right);
}

void 
Camera::zoom(float dy)
{
	float zoomSpeed = 0.012f;
	aov *= (1 + dy*zoomSpeed);
}

Lights *
Camera::getLights()
{ 
	return &cameraLights;
}
