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
#include "cinema_cfg.h"

using namespace std;

int main(int argc, char *argv[]) {
		int w = 1920, h = 1080;
		int ni = 32;
		bool show = false;
		bool saveState = false;


  //! Initialize Cinema
	Cinema cinema(&argc, (const char **)argv);

  //! Print the command line usage.
  if (argc < 2) {

    std::cerr << " "                                                                               << std::endl;
    std::cerr << "  USAGE:  " << argv[0] << "[options] statefile"                                  << std::endl;
    std::cerr << " "                                                                               << std::endl;
    std::cerr << "  Options:"                                                                      << std::endl;
#if WITH_DISPLAY_WINDOW == ON
    std::cerr << "    -S                          : show rendered images"                          << std::endl;
#endif
    std::cerr << "    -F                          : save state files"		                           << std::endl;
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
		else if (!strcmp(argv[i], "-F"))
    { saveState = true;
    }
		else if (!strcmp(argv[i], "-S"))
    {
      show = true;
    }
		else
		{
			if (filename != NULL)
				throw std::runtime_error("improperly formatted <width>x<height> argument");

			filename = argv[i];
		}
  }

  Renderer renderer(w, h, false);

	cinema.Load(renderer, filename, true);

#if WITH_DISPLAY_WINDOW
	renderer.getWindow()->setShow(show);
#endif

	cinema.setSaveState(saveState);
	cinema.Render(renderer, 0);
	cinema.WriteInfo();

	return(0);
}
