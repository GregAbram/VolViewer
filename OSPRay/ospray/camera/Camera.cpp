// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
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

// ospray 
#include "Camera.h"
// embree 
#include "../common/Library.h"
// stl 
#include <map>

namespace ospray {

  typedef Camera *(*creatorFct)();

  std::map<std::string, creatorFct> cameraRegistry;

  Camera *Camera::createCamera(const char *type)
  {
    std::map<std::string, Camera *(*)()>::iterator it = cameraRegistry.find(type);
    if (it != cameraRegistry.end())
      return it->second ? (it->second)() : NULL;
    
    if (ospray::logLevel >= 2) 
      std::cout << "#ospray: trying to look up camera type '" 
                << type << "' for the first time" << std::endl;

    std::string creatorName = "ospray_create_camera__"+std::string(type);
    creatorFct creator = (creatorFct)getSymbol(creatorName); //dlsym(RTLD_DEFAULT,creatorName.c_str());
    cameraRegistry[type] = creator;
    if (creator == NULL) {
      if (ospray::logLevel >= 1) 
        std::cout << "#ospray: could not find camera type '" << type << "'" << std::endl;
      return NULL;
    }
    Camera *camera = (*creator)();  camera->managedObjectType = OSP_CAMERA;
    return(camera);
  }

} // ::ospray

