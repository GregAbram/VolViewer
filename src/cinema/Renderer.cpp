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

static char xyzzy[10240];

void
Renderer::LoadState(std::string statefile, bool with_data)
{
  Document doc;

  std::ifstream in;
  in.open(statefile.c_str(), std::istream::in);
  in.seekg(0, std::ios::end);
  std::streamsize size = in.tellg();
  in.seekg(0, std::ios::beg);

  in.read(xyzzy, size);
  doc.Parse(xyzzy);
  in.close();

	if (! doc.HasMember("Volume"))
  {
    std::cerr << "no volume?\n";
    return;
  }

	getCamera().loadState(doc);
	getCamera().commit();

	getLights().loadState(doc);
	getLights().commit(getRenderer());

	getTransferFunction().loadState(doc);
	getColorMap().loadState(doc);
	getColorMap().commit(getTransferFunction());
	getTransferFunction().commit(getRenderer());

	getSlices().loadState(doc);
	getSlices().commit(getRenderer(), &volume);
	
	getIsos().loadState(doc);
	getIsos().commit(&volume);

	if (with_data)
		LoadDataFromFile(doc["Volume"].GetString());
	
	in.close();
}

void
Renderer::Render(std::string fname) 
{ 
	ospCommit(getRenderer());
  getWindow().render(getRenderer()); 
	getWindow().save(fname);
}
