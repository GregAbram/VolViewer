#include <ospray/ospray.h>

#include "common.h"

class RenderProperties 
{

public:
	RenderProperties()
	{
		ambient = 0.4;
		n_samples = 0;
		radius = 1; 
  }

	void setRenderer(OSPRenderer r) { renderer = r; }

	void loadState(Value& rp)
	{
	  int i;
		float f;

		std::stringstream a;
		a.str(rp["ambient"].GetString());
		a >> f;
		setAmbient(f);

		std::stringstream c;
		c.str(rp["ao radius"].GetString());
		c >> f;
		setAORadius(f);

		std::stringstream d;
		d.str(rp["ao sample count"].GetString());
		d >> i;
		setNumAOSamples(i);
	}

  void saveState(Document& doc, Value& section)
	{
		Value rp(kObjectType), s(kObjectType);

		std::stringstream a;
		a << getAmbient();
		s.SetString(a.str().c_str(), doc.GetAllocator());
		rp.AddMember("ambient", s, doc.GetAllocator());

		std::stringstream b;
		b << getAORadius();
		s.SetString(b.str().c_str(), doc.GetAllocator());
		rp.AddMember("ao radius", s, doc.GetAllocator());

		std::stringstream d;
		d << getNumAOSamples();
		s.SetString(d.str().c_str(), doc.GetAllocator());
		rp.AddMember("ao sample count", s, doc.GetAllocator());

		section.AddMember("Render Properties", rp, doc.GetAllocator());
	}


	float getAmbient() { return ambient; }
	float getAORadius() { return radius; }
	int		getNumAOSamples() { return n_samples; }

	void setAmbient(float a) { ambient = a; }
	void setAORadius(float r) { radius = r; }
	void setNumAOSamples(int n) { n_samples = n; }

	void commit()
	{
		ospSet1i(renderer, "AO number", getNumAOSamples());
		ospSet1f(renderer, "AO radius", getAORadius());
		ospSet1f(renderer, "ambient", getAmbient());
		ospCommit(renderer);
	}
	

private:
	float ambient;
	float radius;
	int   n_samples;

	OSPRenderer renderer;
};
	


