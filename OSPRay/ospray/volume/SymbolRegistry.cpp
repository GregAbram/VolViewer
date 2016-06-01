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

#include "ospray/volume/BlockBrickedVolume.h"
#include "ospray/volume/SharedStructuredVolume.h"

namespace ospray {

  //! A volume type with 64-bit addressing and multi-level bricked storage order.
  OSP_REGISTER_VOLUME(BlockBrickedVolume, block_bricked_volume);

  //! A volume type with 32-bit addressing and XYZ storage order. The voxel data is provided by the application via a shared data buffer.
  OSP_REGISTER_VOLUME(SharedStructuredVolume, shared_structured_volume);

} // ::ospray
