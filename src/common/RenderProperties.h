#include <ospray/ospray.h>

#include "common.h"

class RenderProperties 
{

public:
	RenderProperties();
	void setRenderer(OSPRenderer r);
	void loadState(Value& rp);
  void saveState(Document& doc, Value& section);
	float getAmbient();
	float getAORadius();
	int		getNumAOSamples();
	void setAmbient(float a);
	void setAORadius(float r);
	void setNumAOSamples(int n);
	void commit();

private:
	float ambient;
	float radius;
	int   n_samples;

	OSPRenderer renderer;
};
	


