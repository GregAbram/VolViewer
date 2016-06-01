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

#include "Texture.h"
#include "../include/ospray/ospray.h"

namespace ospray {

  /*! \brief A Texture defined through a 2D Image. */
  struct Texture2D : public Texture {
    //! \brief common function to help printf-debugging 
    /*! Every derived class should overrride this! */
    virtual std::string toString() const { return "ospray::Texture2D"; }

    /*! \brief creates a Texture2D object with the given parameter */
    static Texture2D *createTexture(int width, int height, OSPDataType type, 
                                    void *data, int flags);

    int width;
    int height;
    OSPDataType type;
    void *data;
  };

} // ::ospray
