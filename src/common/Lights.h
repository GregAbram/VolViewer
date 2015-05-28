#pragma once
// #include <ospray/lights/Light.h>
#include <ospray/ospray.h>
#include <iostream>
#include <fstream>
#include <vector>

struct Light
{
  Light(float x, float y, float z, float r, float g, float b) :
					x(x),
					y(y),
					z(z),
					r(x),
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
		std::cerr << "ADD LIGHT " << x << " " << y << " " << z << "\n";
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

	void loadState(std::istream &in)
	{
		lights.clear();

		int n;
		in >> n;
		for (int i = 0; i < n; i++)
		{
			float x, y, z, r, g, b;
			in >> x >> y >> z >> r >> g >> b;
			addLight(x, y, z, r, g, b);
		}
	}

	void saveState(std::ostream &out)
	{
		out << lights.size() << "\n";
		for (int i = 0; i < lights.size(); i++)
			out << lights[i].x << " " << lights[i].y << " " << lights[i].z << " "  <<
						 lights[i].r << " " << lights[i].g << " " << lights[i].b << "\n";
	}

	void clear() { lights.clear(); }

private:
	std::vector<Light> lights;
};
	
