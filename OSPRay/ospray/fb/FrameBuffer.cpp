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

#include "FrameBuffer.h"
#include "LocalFB_ispc.h"

namespace ospray {

  FrameBuffer::FrameBuffer(const vec2i &size,
                           ColorBufferFormat colorBufferFormat,
                           bool hasDepthBuffer,
                           bool hasAccumBuffer)
    : size(size),
      colorBufferFormat(colorBufferFormat),
      hasDepthBuffer(hasDepthBuffer),
      hasAccumBuffer(hasAccumBuffer),
      accumID(-1)
  {
    managedObjectType = OSP_FRAMEBUFFER;
    Assert(size.x > 0 && size.y > 0);
  };

  void LocalFrameBuffer::clear(const uint32 fbChannelFlags)
  {
    if (fbChannelFlags & OSP_FB_ACCUM) {
      ispc::LocalFrameBuffer_clearAccum(getIE());
      accumID = 0;
    }
  }

  LocalFrameBuffer::LocalFrameBuffer(const vec2i &size,
                                     ColorBufferFormat colorBufferFormat,
                                     bool hasDepthBuffer,
                                     bool hasAccumBuffer, 
                                     void *colorBufferToUse)
    : FrameBuffer(size, colorBufferFormat, hasDepthBuffer, hasAccumBuffer)
  { 
    Assert(size.x > 0);
    Assert(size.y > 0);
    if (colorBufferToUse) {
      colorBuffer = colorBufferToUse;
    }
    else {
      switch(colorBufferFormat) {
      case OSP_RGBA_NONE:
        colorBuffer = NULL;
        break;
      case OSP_RGBA_F32:
        colorBuffer = new vec4f[size.x*size.y];
        break;
      case OSP_RGBA_I8:
        colorBuffer = new uint32[size.x*size.y];
        break;
      default:
        throw std::runtime_error("color buffer format not supported");
      }
    }

    if (hasDepthBuffer)
      depthBuffer = new float[size.x*size.y];
    else
      depthBuffer = NULL;
    
    if (hasAccumBuffer)
      accumBuffer = new vec4f[size.x*size.y];
    else
      accumBuffer = NULL;
    ispcEquivalent = ispc::LocalFrameBuffer_create(this,size.x,size.y,
                                                   colorBufferFormat,
                                                   colorBuffer,
                                                   depthBuffer,
                                                   accumBuffer);
  }
  
  LocalFrameBuffer::~LocalFrameBuffer() 
  {
    if (depthBuffer) delete[] depthBuffer;

    if (colorBuffer)
      switch(colorBufferFormat) {
      case OSP_RGBA_F32:
        delete[] ((vec4f*)colorBuffer);
        break;
      case OSP_RGBA_I8:
        delete[] ((uint32*)colorBuffer);
        break;
      default:
        throw std::runtime_error("color buffer format not supported");
      }
    if (accumBuffer) delete[] accumBuffer;
  }

  const void *LocalFrameBuffer::mapDepthBuffer()
  {
    this->refInc();
    return (const void *)depthBuffer;
  }
  
  const void *LocalFrameBuffer::mapColorBuffer()
  {
    this->refInc();
    return (const void *)colorBuffer;
  }
  
  void LocalFrameBuffer::unmap(const void *mappedMem)
  {
    Assert(mappedMem == colorBuffer || mappedMem == depthBuffer );
    this->refDec();
  }

} // ::ospray
