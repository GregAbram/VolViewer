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

#include "ospray/common/Material.h"
#include "ThinDielectric_ispc.h"

namespace ospray {
  namespace pathtracer {
    struct ThinDielectric : public ospray::Material {
      //! \brief common function to help printf-debugging 
      /*! Every derived class should overrride this! */
      virtual std::string toString() const { return "ospray::pathtracer::ThinDielectric"; }
      
      //! \brief commit the material's parameters
      virtual void commit() {
        if (getIE() != NULL) return;

        const vec3f& transmission
          = getParam3f("transmission",vec3f(1.f)); //vec3f(0.19,0.45,1.5));
        const float eta
          = getParamf("eta",1.4f); //vec3f(.4f,0.f,0.f));
        const float thickness
          = getParamf("thickness",1.f);
        
        ispcEquivalent = ispc::PathTracer_ThinDielectric_create
          ((const ispc::vec3f&)transmission,eta,thickness);
      }
    };

    OSP_REGISTER_MATERIAL(ThinDielectric,PathTracer_ThinDielectric);
    OSP_REGISTER_MATERIAL(ThinDielectric,PathTracer_ThinGlass);
  }
}
