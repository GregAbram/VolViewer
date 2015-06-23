#include <iostream>
#include <fstream>

#include "Renderer.h"
#include "Importer.h"

Renderer::Renderer(bool shared) :
	volume(shared)
{
	renderer = ospNewRenderer("vis_renderer");
	camera.setRenderer(renderer);
	renderProperties.setRenderer(renderer);

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

  getCamera().setPos(x/2.0, y/2.0, -(1.5*m - z/2.0));
  getCamera().setDir(0.0, 0.0, 1.5*m);
  getCamera().commit();

	getLights().commit(getRenderer());
	getTransferFunction().commit(getRenderer());
	getSlices().commit(getRenderer(), &volume);
	getIsos().commit(&volume);
	renderProperties.commit();
	
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

	if (! doc.IsObject() ||  ! doc.HasMember("State"))
  {
    std::cerr << "invalud state file\n";
    return;
  }

	if (doc["State"].HasMember("Camera")) 
	{
		getCamera().loadState(doc["State"]["Camera"]);
		getCamera().commit();
	}

	if (doc["State"].HasMember("Lights")) 
	{
		getLights().loadState(doc["State"]["Lights"]);
		getLights().commit(getRenderer());
	}

	if (doc["State"].HasMember("TransferFunction")) 
	{
		getTransferFunction().loadState(doc["State"]["TransferFunction"]);
		getTransferFunction().commit(getRenderer());
	}

	if (doc["State"].HasMember("Slices")) 
	{
		getSlices().loadState(doc["State"]["Slices"]);
		getSlices().commit(getRenderer(), &volume);
	}
	
	if (doc["State"].HasMember("Isosurfaces")) 
	{
		getIsos().loadState(doc["State"]["Isosurfaces"]);
		getIsos().commit(&volume);
	}

	if (with_data)
		LoadDataFromFile(doc["State"]["Volume"].GetString());
	
	in.close();
}

void
Renderer::Render(std::string fname) 
{ 
	ospCommit(getRenderer());
  getWindow().render(getRenderer()); 
	getWindow().save(fname);
}
