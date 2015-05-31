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

  Lights()
  {
		Light l;
		lights.push_back(l);
  }

	void addLight(float x, float y, float z, float r, float g, float b)
	{
		Light l(x, y, z, r, g, b);
	  lights.push_back(l);
	}

	void commit(OSPRenderer r, osp::affine3f frame)
	{
		std::vector<OSPLight> ospLights;
		for (int i = 0; i < lights.size(); i++)
		{
			OSPLight l = ospNewLight(NULL, "DirectionalLight");

			osp::vec3f d(lights[i].x, lights[i].y, lights[i].z);
			osp::vec3f xd = xfmPoint(frame, d);

			ospSet3f(l, "direction", xd.x, xd.y, xd.z);
			ospSet3f(l, "color", lights[i].r, lights[i].g, lights[i].b);
			ospCommit(l);
			ospLights.push_back(l);
		}
		ospSetData(r, "lights", ospNewData(ospLights.size(), OSP_OBJECT, ospLights.data()));
  }

	void commit(OSPRenderer r)
	{
		std::vector<OSPLight> ospLights;
		for (int i = 0; i < lights.size(); i++)
		{
			OSPLight l = ospNewLight(NULL, "DirectionalLight");
			ospSet3f(l, "direction", lights[i].x, lights[i].y, lights[i].z);
			ospSet3f(l, "color", lights[i].r, lights[i].g, lights[i].b);
			ospCommit(l);
			ospLights.push_back(l);
		}
		ospSetData(r, "lights", ospNewData(ospLights.size(), OSP_OBJECT, ospLights.data()));
  }

	void loadState(Value &section)
	{
		lights.clear();

		for (Value::ConstValueIterator itr = section.Begin(); itr != section.End(); ++itr)
		{
			float x, y, z, r, g, b;
			std::stringstream ss(itr->GetString());
			ss >> x >> y >> z >> r >> g >> b;
			addLight(x, y, z, r, g, b);
		}
  }

	void saveState(Document &doc, Value &section)
	{
		Value a(kArrayType);

		for (int i = 0; i < lights.size(); i++)
		{
			std::stringstream s;
			s << lights[i].x << " " << lights[i].y << " " << lights[i].z << " "  <<
						 lights[i].r << " " << lights[i].g << " " << lights[i].b;
			a.PushBack(Value().SetString(s.str().c_str(), doc.GetAllocator()), doc.GetAllocator());
		}

		section.AddMember("Lights", a, doc.GetAllocator());
	}

	void clear() { lights.clear(); }

private:
	std::vector<Light> lights;
};
	
