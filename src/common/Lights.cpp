#include "Lights.h"

Lights::Lights()
{
	Light l;
	lights.push_back(l);
}

void
Lights::addLight(osp::vec3f d, osp::vec3f c)
{
	Light l(d.x, d.y, d.z, c.x, c.y, c.z);
	lights.push_back(l);
}

void
Lights::commit(OSPRenderer r, osp::affine3f frame)
{
	std::vector<OSPLight> ospLights;
	for (int i = 0; i < lights.size(); i++)
	{
		OSPLight l = ospNewLight(NULL, "DirectionalLight");

		osp::vec3f d(lights[i].x, lights[i].y, lights[i].z);
		osp::vec3f xd = xfmVector(frame, d);

		ospSet3f(l, "direction", xd.x, xd.y, xd.z);
		ospSet3f(l, "color", lights[i].r, lights[i].g, lights[i].b);
		ospCommit(l);
		ospLights.push_back(l);
	}
	ospSetData(r, "lights", ospNewData(ospLights.size(), OSP_OBJECT, ospLights.data()));
	ospCommit(r);
}

void
Lights::commit(OSPRenderer r)
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

void 
Lights::loadState(Value &section)
{
	lights.clear();

	for (Value::ConstValueIterator itr = section.Begin(); itr != section.End(); ++itr)
	{
		osp::vec3f d, c;
		std::stringstream ss(itr->GetString());
		ss >> d.x >> d.y >> d.z >> c.x >> c.y >> c.z;
		addLight(d, c);
	}
}

void
Lights::saveState(Document &doc, Value &section)
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

void
Lights::clear()
{
	lights.clear();
}
