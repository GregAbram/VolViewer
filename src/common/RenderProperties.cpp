#include <ospray/ospray.h>

#include "RenderProperties.h"

RenderProperties::RenderProperties()
{
	ambient = 0.4;
	n_samples = 0;
	radius = 1; 
	stepScale = 1;
}

void
RenderProperties::setRenderer(OSPRenderer r) { renderer = r; }

void
RenderProperties::loadState(Value& rp)
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

	std::stringstream e;
	e.str(rp["step scale"].GetString());
	e >> f;
	setStepScale(f);
}

void
RenderProperties::saveState(Document& doc, Value& section)
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

	std::stringstream c;
	c << getNumAOSamples();
	s.SetString(c.str().c_str(), doc.GetAllocator());
	rp.AddMember("ao sample count", s, doc.GetAllocator());

	std::stringstream d;
	d << getStepScale();
	s.SetString(d.str().c_str(), doc.GetAllocator());
	rp.AddMember("step scale", s, doc.GetAllocator());

	section.AddMember("Render Properties", rp, doc.GetAllocator());
}

float
RenderProperties::getAmbient() { return ambient; }

float
RenderProperties::getAORadius() { return radius; }

int
RenderProperties::getNumAOSamples() { return n_samples; }

void 
RenderProperties::setAmbient(float a) { ambient = a; }

void
RenderProperties::setAORadius(float r) { radius = r; }

void
RenderProperties::setNumAOSamples(int n) { n_samples = n; }

void
RenderProperties::setStepScale(float ss) { stepScale = ss; }

float
RenderProperties::getStepScale() { return stepScale; }

void
RenderProperties::commit()
{
	ospSet1i(renderer, "AO number", getNumAOSamples());
	ospSet1f(renderer, "AO radius", getAORadius());
	ospSet1f(renderer, "ambient", getAmbient());
	ospSet1f(renderer, "stepScale", getStepScale());
	ospCommit(renderer);
}

