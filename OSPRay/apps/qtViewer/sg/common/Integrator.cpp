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

#undef NDEBUG

// scene graph
#include "Integrator.h"

namespace ospray {
  namespace sg {

    void Integrator::commit()
    {
      if (!ospRenderer) {
        ospRenderer = ospNewRenderer(type.c_str());
        if (!ospRenderer) 
          throw std::runtime_error("#osp:sg:SceneGraph: could not create renderer (of type '"+type+"')");
      }
      if (lastCommitted >= lastModified) return;

      ospSet1i(ospRenderer,"spp",spp);

      // set world, camera, ...
      if (world ) { 
        world->commit();
        ospSetObject(ospRenderer,"world", world->ospModel);
      }
      if (camera) {
        camera->commit();
        ospSetObject(ospRenderer,"camera",camera->ospCamera);
      }

      lastCommitted = __rdtsc();
      ospCommit(ospRenderer);
      assert(ospRenderer); 
   }

    void Integrator::setSPP(size_t spp) { 
      this->spp = spp;
      if (ospRenderer) {
        ospSet1i(ospRenderer,"spp",spp);
      }
    }

  } // ::ospray::sg
} // ::ospray
