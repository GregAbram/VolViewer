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
#include <QtGui>
#include <ctype.h>
#include <sstream>
#include "ospray/ospray.h"
#include "ospray/include/ospray/ospray.h"
#include "VolumeViewer.h"
#include "TransferFunctionEditor.h"


static void
syntax(char *a)
{
	if (a)
		std::cerr << "bad arg: " << a << "\n";

	std::cerr << " "                                                                                        << std::endl;
	std::cerr << "  USAGE:  " << a << " [volfile | statefile] [options]" 						                        << std::endl;
	std::cerr << " "                                                                                        << std::endl;
	std::cerr << "  Options:"                                                                               << std::endl;
	std::cerr << " "                                                                                        << std::endl;
	std::cerr << "    -benchmark <warm-up frames> <frames> : run benchmark and report overall frame rate"   << std::endl;
	std::cerr << "    -dt <dt>                             : use ray cast sample step size 'dt'"            << std::endl;
	std::cerr << "    -transferfunction <filename>         : load transfer function from 'filename'"        << std::endl;
	std::cerr << "    -viewsize <width>x<height>           : force OSPRay view size to 'width'x'height'"    << std::endl;
	std::cerr << "    -viewup <x> <y> <z>                  : set viewport up vector to ('x', 'y', 'z')"     << std::endl;
	std::cerr << "    -module <moduleName>                 : load the module 'moduleName'"                  << std::endl;
	std::cerr << " "                                                                                        << std::endl;
	exit(1);
}

int main(int argc, char *argv[]) {

  //! Initialize OSPRay.
  ospInit(&argc, (const char **) argv);

  //! Initialize Qt.
  QApplication *app = new QApplication(argc, argv);

  string volname("");
  string statename("");

  //! Default values for the optional command line arguments.
  float dt = 0.0f;
  std::vector<std::string> plyFilenames;
  float rotationRate = 0.0f;
  std::vector<std::string> sliceFilenames;
  std::string transferFunctionFilename;
  int benchmarkWarmUpFrames = 0;
  int benchmarkFrames = 0;
  int viewSizeWidth = 0;
  int viewSizeHeight = 0;
  osp::vec3f viewUp(0.f);
  bool showFrameRate = false;

  //! Parse the optional command line arguments.
  for (int i=1 ; i < argc ; i++) {

    std::string arg = argv[i];

    if (arg == "-dt") {

      if (i + 1 >= argc) throw std::runtime_error("missing <dt> argument");
      dt = atof(argv[++i]);
      std::cout << "got dt = " << dt << std::endl;

    } else if (arg == "-showframerate") {

      showFrameRate = true;
      std::cout << "set show frame rate" << std::endl;

    } else if (arg == "-transferfunction") {

      if (i + 1 >= argc) throw std::runtime_error("missing <filename> argument");
      transferFunctionFilename = std::string(argv[++i]);
      std::cout << "got transferFunctionFilename = " << transferFunctionFilename << std::endl;

    } else if (arg == "-benchmark") {

      if (i + 2 >= argc) throw std::runtime_error("missing <warm-up frames> <frames> arguments");
      benchmarkWarmUpFrames = atoi(argv[++i]);
      benchmarkFrames = atoi(argv[++i]);
      std::cout << "got benchmarkWarmUpFrames = " << benchmarkWarmUpFrames << ", benchmarkFrames = " << benchmarkFrames << std::endl;

    } else if (arg == "-viewsize") {

      if (i + 1 >= argc) throw std::runtime_error("missing <width>x<height> argument");
      std::string arg2(argv[++i]);
      size_t pos = arg2.find("x");

      if (pos != std::string::npos) {

        arg2.replace(pos, 1, " ");
        std::stringstream ss(arg2);
        ss >> viewSizeWidth >> viewSizeHeight;
        std::cout << "got viewSizeWidth = " << viewSizeWidth << ", viewSizeHeight = " << viewSizeHeight << std::endl;

      } else throw std::runtime_error("improperly formatted <width>x<height> argument");

    } else if (arg == "-viewup") {

      if (i + 3 >= argc) throw std::runtime_error("missing <x> <y> <z> arguments");

      viewUp.x = atof(argv[++i]);
      viewUp.y = atof(argv[++i]);
      viewUp.z = atof(argv[++i]);

      std::cout << "got viewup = " << viewUp.x << " " << viewUp.y << " " << viewUp.z << std::endl;

    } else if (arg == "-module") {

      if (i + 1 >= argc) throw std::runtime_error("missing <moduleName> argument");
      std::string moduleName = argv[++i];
      std::cout << "loading module '" << moduleName << "'." << std::endl;
      error_t error = ospLoadModule(moduleName.c_str());

      if(error != 0) {
        std::ostringstream ss;
        ss << error;
        throw std::runtime_error("could not load module " + moduleName + ", error " + ss.str());
      }

    } 
		else
		{
			if (arg.find_last_of(".") == std::string::npos)
        throw std::runtime_error("invalid input file: " + arg + " (no extension?)");

			if (arg.substr(arg.find_last_of(".")) == ".state")
			{
				if (volname != "" || statename != "")
					throw std::runtime_error("more than one input files?");
				
				statename = arg;
			}
			else 
			{
				if (volname != "" || statename != "")
					throw std::runtime_error("more than one input files?");
				
				volname = arg;
			}
		}
	}

  //! Create the OSPRay state and viewer window.
  VolumeViewer *volumeViewer = new VolumeViewer(showFrameRate);

  //! Set benchmarking parameters.
  volumeViewer->getWindow()->setBenchmarkParameters(benchmarkWarmUpFrames, benchmarkFrames);

  //! Set the window size if specified.
  if (viewSizeWidth != 0 && viewSizeHeight != 0) volumeViewer->getWindow()->setFixedSize(viewSizeWidth, viewSizeHeight);

  //! Set the view up vector if specified.
  if(viewUp != osp::vec3f(0.f)) volumeViewer->getWindow()->getCamera().setUp(viewUp);

	if (volname != "")
		volumeViewer->importFromFile(volname);
	else if (statename != "")
		volumeViewer->loadState(statename);

  //! Enter the Qt event loop.
  app->exec();

  //! Cleanup
  delete volumeViewer;  return(0);

}

