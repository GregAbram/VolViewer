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

#include "DirectionalLight.h"
#include "DirectionalLight_ispc.h"

namespace ospray {
  DirectionalLight::DirectionalLight()
    : direction(0.f, 0.f, 1.f)
    , color(1.f)
    , intensity(1.f)
  {
    ispcEquivalent = ispc::DirectionalLight_create(this);
  }

  //! Commit parameters understood by the DirectionalLight
  void DirectionalLight::commit() {
    direction = getParam3f("direction", vec3f(0.f, 0.f, 1.f));
    color     = getParam3f("color", vec3f(1.f));
    intensity = getParam1f("intensity", 1.f);

    vec3f radiance = color * intensity;

    ispc::DirectionalLight_set(getIE(), (ispc::vec3f&)direction, (ispc::vec3f&)radiance);
  }

  OSP_REGISTER_LIGHT(DirectionalLight, DirectionalLight);
}
