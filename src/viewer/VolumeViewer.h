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

#include "QOSPRayWindow.h"
#include <QtGui>
#include <string>
#include <vector>

#include "CameraEditor.h"
#include "RenderPropertiesEditor.h"
#include "TransferFunctionEditor.h"
#include "SlicesEditor.h"
#include "IsosEditor.h"
#include "TimeEditor.h"
#include "Volume.h"

#include "../common/common.h"

class VolumeViewer : public QMainWindow {

  Q_OBJECT

  public:

  //! Constructor.
  VolumeViewer(bool showFrameRate);

  //! Destructor.
  ~VolumeViewer() {};

  //! Get the OSPRay output window.
  QOSPRayWindow *getWindow() { return(osprayWindow); }

  //! Get the transfer function editor.
  TransferFunctionEditor &getTransferFunctionEditor() { return transferFunctionEditor; }
  SlicesEditor &getSlicesEditor() { return slicesEditor; }

  IsosEditor &getIsosEditor() { return isosEditor; }

  //! A string description of this class.
  std::string toString() const { return("VolumeViewer"); }

  //! Load an data from a file
  void importFromFile(const std::string &filename);

	void loadState(std::string statename);

	RenderPropertiesEditor *getRenderProperties() { return &renderPropertiesEditor; }

	void ImportGeometry(std::string);
	void UpdateModel();

public slots:

	void openVolume();
	void openSeries();
	void openGeometry();
	void loadColorMap();
	void openState();
	void saveState();
	void record();

	void commitVolume() { currentVolume->commit(); }

  //! Force the OSPRay window to be redrawn.
  void render() { if (osprayWindow != NULL) osprayWindow->updateGL(); }

	void commitSlices();
	void commitIsos();
	void commitLights();

	void resetCamera();
	void selectTimeStep(int);

protected:
	std::string dataName;
	Volume *currentVolume;
	VolumeSeries volumeSeries;

  //! OSPRay renderer.
  OSPRenderer renderer;

  //! The OSPRay output window.
  QOSPRayWindow *osprayWindow;

  //! The transfer function editor.
  TransferFunctionEditor transferFunctionEditor;

	CameraEditor cameraEditor;

	//! The slices editor
  SlicesEditor slicesEditor;

  IsosEditor isosEditor;

  //! Print an error message.
  void emitMessage(const std::string &kind, const std::string &message) const
  { std::cerr << "  " + toString() + "  " + kind + ": " + message + "." << std::endl; }

  //! Error checking.
  void exitOnCondition(bool condition, const std::string &message) const
  { if (!condition) return;  emitMessage("ERROR", message);  exit(1); }


  //! Create and configure the OSPRay state.
  void initObjects(const std::vector<std::string> &filenames);

  //! Create and configure the user interface widgets and callbacks.
  void initUserInterfaceWidgets();

  //! The transfer function editor.
  RenderPropertiesEditor renderPropertiesEditor;

	//! The timestep manager
	TimeEditor timeEditor;

	void ImportVTP(OSPTriangleMesh& mesh, std::string);

	OSPTriangleMesh currentMesh;
};

