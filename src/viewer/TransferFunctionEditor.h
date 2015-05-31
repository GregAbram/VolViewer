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

#include "VColorMap.h"
#include "TransferFunction.h"
#include "LinearTransferFunctionWidget.h"
#include <QtGui>
#include <ospray/ospray.h>

#include "../common/common.h"

class TransferFunctionEditor : public QWidget {

  Q_OBJECT

public:

  TransferFunctionEditor();
	TransferFunction& getTransferFunction() { return transferFunction; }

	void commit() { getTransferFunction().commit(renderer); }

	void setRenderer(OSPRenderer r) {renderer = r;}


signals:

  void transferFunctionChanged();

public slots:

  void loadState(Document&);
  void saveState(Document&);
  void loadColorMap();

protected slots:

  void alphaWidgetChanged();
  void modifiedTransferFunction();
  void setColorMapIndex(int index);

protected:

  void loadColorMapFile(std::string);
  void addColorMap(VColorMap);
  void loadOpacityMapFile(std::string);

  void loadColorMaps();

	TransferFunction transferFunction;

  //
  //! Color maps.
  std::vector<VColorMap> colorMaps;

  //! Color map selection widget.
  QComboBox colorMapComboBox;

  //! Transfer function widget for opacity.
  LinearTransferFunctionWidget transferFunctionAlphaWidget;

  //! Slider for scaling transfer function opacities.
  QSlider transferFunctionAlphaScalingSlider;

	OSPRenderer renderer;
};
