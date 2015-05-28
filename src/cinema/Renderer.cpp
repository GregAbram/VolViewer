#include <iostream>
#include <fstream>

#include "Renderer.h"
#include "Importer.h"

Renderer::Renderer(bool shared) :
	volume(shared)
{
	renderer = ospNewRenderer("vis_renderer");
	camera.setRenderer(renderer);

	float clips[12] = 
		{
			 1.0,  0.0,  0.0, -0.9,
			 1.0, -1.0,  0.0,  0.0,
			-1.0,  0.0,  0.0,  0.1
		};

	ospSetData(renderer, "clips", ospNewData(sizeof(clips) / (4 * sizeof(float)), OSP_FLOAT4, clips));

	float slice[4] = {5.0, 6.0, 7.0, 8.0};
	ospSetData(renderer, "slices", ospNewData(1, OSP_FLOAT4, slice));

	OSPModel dmodel = ospNewModel();
	ospCommit(dmodel);
	ospSetObject(renderer, "dynamic_model", dmodel);
	std::cerr << "Here DM\n";
}

Renderer::~Renderer()
{
	ospRelease(renderer);
}

void
Renderer::Load(std::string name, bool with_data)
{
	if (name.substr(name.rfind('.')) == ".state")
		LoadState(name, with_data);
	else
		LoadVolume(name);
}

void 
Renderer::CommitVolume()
{
	volume.commit();

	OSPModel model = ospNewModel();
	ospAddVolume(model, volume.getOSPVolume());
	ospCommit(model);
	ospSetObject(getRenderer(), "model", model);
	ospCommit(getRenderer());
}

void
Renderer::LoadDataFromFile(std::string volumeName)
{
	importVolume(volume, volumeName, getTransferFunction());
	CommitVolume();
}

void
Renderer::LoadVolume(std::string volumeName)
{
	LoadDataFromFile(volumeName);

	int x, y, z;
  volume.GetDimensions(x, y, z);

  int m = x > y ? x > z ? x : z : y > z ? y : z;

  getCamera().setPos(x/2.0, y/2.0, -(3*m - z/2.0));
  getCamera().setDir(0.0, 0.0, 3*m);
  getCamera().commit();
}

void
Renderer::LoadState(std::string statefile, bool with_data)
{
	std::ifstream in;
	in.open(statefile.c_str(), std::ifstream::in);

	std::string volumeName;
	in >> volumeName;


	getCamera().loadState(in);
	getCamera().commit();

	getLights().loadState(in);
	getLights().commit(getRenderer());

	getTransferFunction().loadState(in);
	getColorMap().loadState(in);
	getColorMap().commit(getTransferFunction());
	getTransferFunction().commit(getRenderer());

	getSlices().loadState(in);
	getSlices().commit(getRenderer(), &volume);
	
	getIsos().loadState(in);
	getIsos().commit(&volume);

	if (with_data)
		LoadDataFromFile(volumeName);
	
	in.close();
}

void
Renderer::Render(std::string fname) 
{ 
	ospCommit(getRenderer());
  getWindow().render(getRenderer()); 
	getWindow().save(fname);
}
