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

#include <QtGui>
#include <QGLWidget>
#include <ospray/ospray.h>
#include <iostream>
#include <fstream>

#include "../common/common.h"

#include "CameraEditor.h"
#include "Camera.h"

class QOSPRayWindow : public QGLWidget
{
public:

  QOSPRayWindow(QMainWindow *parent, OSPRenderer renderer, bool showFrameRate);
  virtual ~QOSPRayWindow();

	void saveState(Document&, Value&);
	void loadState(Value&);

  void setRenderingEnabled(bool renderingEnabled);
  void setRotationRate(float rotationRate);
  void setBenchmarkParameters(int benchmarkWarmUpFrames, int benchmarkFrames);

  CameraEditor    *getCameraEditor() { return &cameraEditor; }

  OSPFrameBuffer& getFrameBuffer() { return frameBuffer; }

	void Clear();

	void commit() { cameraEditor.commit(); }
	void render() { updateGL(); }

	void saveImage(std::string filename);

protected:

  /*! Parent Qt window. */
  QMainWindow *parent;

  /*! Display the frame rate in the main window title bar. */
  bool showFrameRate;

  virtual void paintGL();
  virtual void resizeGL(int width, int height);
  virtual void mousePressEvent(QMouseEvent * event);
  virtual void keyPressEvent(QKeyEvent * event);
  virtual void mouseReleaseEvent(QMouseEvent * event);
  virtual void mouseMoveEvent(QMouseEvent * event);

  /*! frame counter */
  long frameCount;

  /*! only render when this flag is true. this allows the window to be created before all required components are ospCommit()'d. */
  bool renderingEnabled;

  /*! rotation rate to automatically rotate view. */
  float rotationRate;

  /*! benchmarking: number of warm-up frames */
  int benchmarkWarmUpFrames;

  /*! benchmarking: number of frames over which to measure frame rate */
  int benchmarkFrames;

  /*! benchmarking: timer to measure elapsed time over benchmark frames */
  QTime benchmarkTimer;

  /*! Timer to measure elapsed time over a single frame. */
  QTime renderFrameTimer;

  osp::vec2i windowSize;
  QPoint lastMousePosition;

  OSPFrameBuffer frameBuffer;
  OSPRenderer renderer;

	CameraEditor cameraEditor;

	int current_width, current_height;
};
