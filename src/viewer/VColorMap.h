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

#pragma once

#include <string>
#include <vector>
#include <QtGui>
#include <ospray/ospray.h>
#include "TransferFunction.h"
#include "ColorMap.h"

class VColorMap
{
public:

  VColorMap(ColorMap c) { cmap = c; }
	VColorMap(std::string name) { cmap = ColorMap(name); }

	ColorMap getColorMap() { return cmap; }
  std::string getName() { return cmap.getName(); }
  std::vector<osp::vec3f> getColors() { return cmap.getColors(); }

	void commit(TransferFunction& tf) { cmap.commit(tf); }

  QImage getImage();

protected:

	ColorMap cmap;
};
