// ======================================================================== //
// Copyright 2009-2014 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include <iostream>
#include <ctype.h>
#include <sstream>

#include "Cinema.h"

using namespace std;

int main(int argc, char *argv[]) {
		int w = 1920, h = 1080;
		int ni = 32;

  //! Initialize Cinema
	Cinema cinema(&argc, (const char **)argv);

  //! Print the command line usage.
  if (argc < 2) {

    std::cerr << " "                                                                               << std::endl;
    std::cerr << "  USAGE:  " << argv[0] << "[options] statefile"			                             << std::endl;
    std::cerr << " "                                                                               << std::endl;
    std::cerr << "  Options:"                                                                      << std::endl;
    std::cerr << "    -s w h                      : size of images (1920x1080)"                    << std::endl;
    std::cerr << "    -n nImages                  : number of images to render (32)"               << std::endl;
    std::cerr << " "                                                                               << std::endl;
    return(1);
  }

	char *filename = NULL;

  for (int i= 1 ; i < argc ; i++) {

    std::string arg = argv[i];

    if (!strcmp(argv[i], "-s"))
		{
      if (i + 2 >= argc) throw std::runtime_error("missing size arguments");
			w = atoi(argv[++i]);
			h = atoi(argv[++i]);
    }
		else if (!strcmp(argv[i], "-n"))
		{
      if (i + 1 >= argc) throw std::runtime_error("missing number of images argument");
			ni = atoi(argv[++i]);
		}
		else
		{
			if (filename != NULL)
				throw std::runtime_error("improperly formatted <width>x<height> argument");

			filename = argv[i];
		}
  }

  Renderer renderer(false);
	renderer.getWindow().resize(w, h);
	renderer.Load(std::string(filename));

#if 1
	vector<int> phis;
	int d = 20 / 2;
	for (int i = 0; i < 3; i++)
		phis.push_back(i*d);

	vector<int> thetas;
	d = 90 / 15;
	for (int i = 0; i < 12; i++)
		thetas.push_back(i*d);
#else
	vector<int> phis;
	phis.push_back(30);

	vector<int> thetas;
	thetas.push_back(50);
#endif

	CameraVariable *camvar = new CameraVariable(phis, thetas);
	cinema.AddVariable(camvar);

#if 0
	{
		vector<int> clips;
		clips.push_back(1);

		vector<int> visible;
		visible.push_back(1);

		vector<int> flip;
		flip.push_back(0);
		flip.push_back(1);

		vector<int> values;
		for (int i = 0; i < 10; i++)
		{
			int v = (int)((i / 9.0) * 100);
			std::cerr << "XXXX: " << v << "\n";
			values.push_back(v);
		}

		SlicePlaneVariable *clip = new SlicePlaneVariable(string("X"), clips, visible, flip, values);
		cinema.AddVariable(clip);
	}
#endif

#if 1
	{
		vector<int> isovalues;
		for (int i = 0; i < 3; i++)
			isovalues.push_back((int)(20 + (i / 2.0)*60));

		float min, max;
		renderer.getVolume()->GetMinMax(min, max);
		renderer.getIsos().SetMinMax(min, max);

		IsosurfaceVariable *iso = new IsosurfaceVariable(string("Iso"), isovalues);
		cinema.AddVariable(iso);
	}
#endif

#if 1
	vector<int> doVR;
	doVR.push_back(0);
	// doVR.push_back(1);
	VolumeRenderingVariable *vrvar = new VolumeRenderingVariable(doVR);
	cinema.AddVariable(vrvar);
#endif

	std::cerr << "Requires " << cinema.Count() << " images\n";
	camvar->ResetCount();

	cinema.Render(renderer, 0);
	cinema.WriteInfo();

	return(0);
}
