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

#pragma once

#include "ospray/volume/StructuredVolume.h"

namespace ospray {

  //! \brief A concrete implementation of the StructuredVolume class
  //!  with 32-bit addressing in which the voxel data is laid out in
  //!  memory in XYZ order and provided via a shared data buffer.
  //!
  class SharedStructuredVolume : public StructuredVolume {
  public:

    //! Constructor.
    SharedStructuredVolume() {};

    //! Destructor.
    virtual ~SharedStructuredVolume() {};

    //! A string description of this class.
    virtual std::string toString() const { return("ospray::SharedStructuredVolume<" + voxelType + ">"); }

    //! Allocate storage and populate the volume, called through the OSPRay API.
    virtual void commit();

    //! Copy voxels into the volume at the given index; not allowed on SharedStructuredVolume.
    virtual int setRegion(const void *source, const vec3i &index, const vec3i &count) {
      exitOnCondition(true, "setRegion() not allowed on this volume type; volume data must be provided via the voxelData parameter");
      return 0;
    }

  protected:

    //! Create the equivalent ISPC volume container.
    virtual void createEquivalentISPC();

  };

} // ::ospray
