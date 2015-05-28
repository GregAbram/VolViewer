#pragma once

#include <ospray/ospray.h>
#include <vector>

#include "Window.h"
#include "Camera.h"
#include "Lights.h"
#include "TransferFunction.h"
#include "ColorMap.h"
#include "Slices.h"
#include "Isos.h"

#include "MyVolume.h"

using namespace std;

class Renderer
{
public:

	Renderer(bool shared_data);
	~Renderer();

	Window 		 			 &getWindow() 					{return window;}
	Camera 		 			 &getCamera() 					{return camera;}
	ColorMap	 			 &getColorMap() 				{return colorMap;}
	Lights 		 			 &getLights() 					{return lights;}
	Slices 		 			 &getSlices() 					{return slices;}
	Isos	 		 			 &getIsos() 						{return isos;}
	TransferFunction &getTransferFunction() {return transferFunction;}
	OSPRenderer 		 &getRenderer() 			  {return renderer;}
	MyVolume				 *getVolume()						{return &volume;}


	// Use this to load either a state file (with its data)
	// or a volume file
	void Load(std::string, bool with_data = true);

	// Use this to load a state file.   May or may not load data,
	// but will not munge camera state if it does
	void LoadState(std::string, bool with_data);

	// Use this to load a volume and set up a default camera
	void LoadVolume(std::string);

	// Call this when you've set up shared volume data
	void CommitVolume();

	void Render(std::string fname);

private:

  // Use this to load a volume without changing other state (e.g. camera)
	void LoadDataFromFile(std::string);
	
	Window   					window;
	Camera	 				 	camera;
	Lights   				 	lights;
	TransferFunction 	transferFunction;
	Slices					 	slices;
	Isos							isos;
	ColorMap					colorMap;

	OSPRenderer renderer;
	MyVolume volume;
};
