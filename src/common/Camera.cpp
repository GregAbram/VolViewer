#include "Camera.h"
#define PI 3.1415926

void
Camera::setRenderer(OSPRenderer r)
{
	renderer = r;
	ospSetObject(renderer, "camera", ospCamera);
}

osp::vec3f
Camera::getPos()
{
	return xfmPoint(frame, osp::vec3f(0, 0, -eye_dist));
}

void
Camera::getPos(float* p) 
{
		osp::vec3f t;
		getPos(t);
		p[0] = t.x;
		p[1] = t.y;
		p[2] = t.z;
}

void
Camera::getPos(float& x, float& y, float& z)
{
		osp::vec3f t;
		getPos(t);
		x = t.x;
		y = t.y;
		z = t.z;
}

void
Camera::getPos(osp::vec3f &p) 
{
		p = getPos();
}

void
Camera::getCenter(float* d) 
{
		d[0] = frame.p.x;
		d[1] = frame.p.y;
		d[2] = frame.p.z;
}

void
Camera::getCenter(float& x, float& y, float& z)
{
		x = frame.p.x;
		y = frame.p.y;
		z = frame.p.z;
}

void
Camera::getCenter(osp::vec3f &p) 
{
		p = frame.p;
}

void
Camera::getUp(float& x, float& y, float& z)
{
		osp::vec3f u = getUp();
		x = up.x;
		y = up.y;
		z = up.z;
}

osp::vec3f
Camera::getUp()
{
	return frame.l.vy;
} 

void
Camera::getUp(float* u)
{
	osp::vec3f up = getUp();
	u[0] = up.x;
	u[1] = up.y;
	u[2] = up.z;
}

void 
Camera::getUp(osp::vec3f &u)
{
	u = frame.l.vy;
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
	return fovY;
}

void 
Camera::setFovY(float f)
{
	fovY   = f; modified = true;
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
Camera::setupFrame(osp::vec3f eye, osp::vec3f center, osp::vec3f up)
{
	osp::vec3f Y = normalize(up);
	osp::vec3f Z = normalize(center - eye);
	osp::vec3f X = normalize(cross(Y, Z));

	frame = osp::affine3f::AffineSpaceT(X, up, Z, center);

	eye_dist = length(center - eye);
}

void 
Camera::saveState(Document &doc, Value &section)
{
	Value cam(kObjectType), s(kObjectType);

	std::stringstream  p;
	p << frame.l.vx.x << " " << frame.l.vx.y << " " << frame.l.vx.z << " "
	  << frame.l.vy.x << " " << frame.l.vy.y << " " << frame.l.vy.z << " "
	  << frame.l.vz.x << " " << frame.l.vz.y << " " << frame.l.vz.z << " "
	  << frame.p.x << " " << frame.p.y << " " << frame.p.z;
	s.SetString(p.str().c_str(), doc.GetAllocator());
	cam.AddMember("frame of reference", s, doc.GetAllocator());

	std::stringstream  d;
	d << eye_dist;
	s.SetString(d.str().c_str(), doc.GetAllocator());
	cam.AddMember("distance", s, doc.GetAllocator());

	std::stringstream  f;
	f << fovY;
	s.SetString(f.str().c_str(), doc.GetAllocator());
	cam.AddMember("fovy", s, doc.GetAllocator());

	cameraLights.saveState(doc, cam);

	section.AddMember("Camera", cam, doc.GetAllocator());
}

void 
Camera::loadState(Value& cam)
{
	float v[3];

	std::stringstream p;
	p.str(cam["frame of reference"].GetString());
	p >> frame.l.vx.x >> frame.l.vx.y >> frame.l.vx.z
	  >> frame.l.vy.x >> frame.l.vy.y >> frame.l.vy.z
	  >> frame.l.vz.x >> frame.l.vz.y >> frame.l.vz.z
	  >> frame.p.x >> frame.p.y >> frame.p.z;

	std::stringstream d;
	d.str(cam["distance"].GetString());
	d >> eye_dist;

	std::stringstream f;
	f.str(cam["fovy"].GetString());
	f >> fovY;

	if (cam.HasMember("Lights"))
		cameraLights.loadState(cam["Lights"]);

	commit();
}

void 
Camera::commit()
{
	osp::vec3f e;
	if (phi != 0.0 || theta != 0.0)
	{
		osp::affine3f phi_theta_frame = osp::affine3f::rotate(frame.p, osp::vec3f(1.0, 0.0, 0.0), phi) * osp::affine3f::rotate(frame.p, osp::vec3f(0.0, 1.0, 0.0), -theta) * frame;
		e = xfmPoint(phi_theta_frame, osp::vec3f(0.0, 0.0, -eye_dist));
	}
	else
	{
		e = xfmPoint(frame, osp::vec3f(0.0, 0.0, -eye_dist));
	}
			
	osp::vec3f d = frame.p - e;

	ospSetVec3f(ospCamera,"pos", e);
	ospSetVec3f(ospCamera,"dir", d);
	ospSetVec3f(ospCamera,"up",  frame.l.vy);
	ospSetf(ospCamera,"aspect",  aspect);
	ospSetf(ospCamera,"fovy",    fovY);
	ospCommit(ospCamera);

	cameraLights.commit(renderer, frame);
}

void 
Camera::rotateFrame(float t, float p)
{
	frame = osp::affine3f::rotate(frame.p, osp::vec3f(1.0, 0.0, 0.0), p) * osp::affine3f::rotate(frame.p, osp::vec3f(0.0, 1.0, 0.0), -t) * frame;
}

void 
Camera::zoom(float dy)
{
	float motionSpeed = 0.012f;
	eye_dist *= (1 + dy*motionSpeed);
}

Lights *
Camera::getLights()
{ 
	return &cameraLights;
}
