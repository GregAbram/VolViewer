
// ======================================================================== //
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

#include <algorithm>
#include <iostream>
#include <fstream>
#include "VolumeViewer.h"
#include "ospray/ospray.h"

#include "Importer.h"

VolumeViewer::VolumeViewer(bool showFrameRate) 
  : renderer(NULL), 
    osprayWindow(NULL), 
		volumeName(""),
		volume(false)
{
  //! Default window size.
  resize(1024, 768);

  //! Create an OSPRay renderer.
  renderer = ospNewRenderer("vis_renderer");  exitOnCondition(renderer == NULL, "could not create OSPRay renderer object");
	getTransferFunctionEditor().setRenderer(renderer);

  //! Create an OSPRay window and set it as the central widget, but don't let it start rendering until we're done with setup.
  osprayWindow = new QOSPRayWindow(this, renderer, showFrameRate);  setCentralWidget(osprayWindow);

	lights.commit(renderer);
  
  //! Configure the user interface widgets and callbacks.
  initUserInterfaceWidgets();

  //! Commit the transfer function only after the initial colors and alphas have been set (workaround for Qt signalling issue).
	getTransferFunctionEditor().getTransferFunction().commit(renderer);
	slicesEditor.commit(renderer);
	isosEditor.commit(&volume);

  //! Show the window.
  show();

}

void VolumeViewer::importFromFile(const std::string &filename) {

  TransferFunction tf = getTransferFunctionEditor().getTransferFunction();

	importVolume(volume, filename, tf);
	volumeName = filename;

	float min, max;
	volume.GetMinMax(min, max);
	isosEditor.setMinMax(min, max);

  OSPModel model = ospNewModel();
	ospAddVolume(model, volume.getOSPVolume());
	ospCommit(model);  
	ospSetObject(renderer, "model", model);

	OSPModel dmodel = ospNewModel();
	ospCommit(dmodel);
	ospSetObject(renderer, "dynamic_model", dmodel);

	ospCommit(renderer);

}

void
VolumeViewer::openVolume()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Load Volume"), ".", "volumes (*.vol)");

  if(filename.isEmpty())
    return;

	importFromFile(filename.toStdString());
}

void VolumeViewer::loadColorMap()
{
	getTransferFunctionEditor().loadColorMap();
}

void VolumeViewer::openState()
{
	QString filename = QFileDialog::getOpenFileName(this, "Load state", ".", "Load files (*.state)");
	if(filename.isNull())
		return;

	//! Make sure the filename has the proper extension.
	if(!filename.endsWith(".state"))
		filename += ".state";

	loadState(filename.toStdString());
}

void VolumeViewer::loadState(std::string statename)
{
  std::ifstream in; 
	in.open(statename.c_str(), std::istream::in);

	in >> volumeName;
	importFromFile(volumeName);
	getWindow()->loadState(in);
	getWindow()->commit();

	getLights().loadState(in);
	getLights().commit(renderer);
	
	getTransferFunctionEditor().loadState(in);			// Encompases colormap and opacity
	getSlicesEditor().loadState(in);

	getIsosEditor().loadState(in);

	float vmin, vmax;
	volume.GetMinMax(vmin, vmax);
	getTransferFunctionEditor().getTransferFunction().SetMin(vmin);
	getTransferFunctionEditor().getTransferFunction().SetMax(vmax);
	getTransferFunctionEditor().commit();

	in.close();

	osprayWindow->setRenderingEnabled(true);
}

void VolumeViewer::saveState()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save state", ".", "State files (*.state)");
	if(filename.isNull())
		return;

	//! Make sure the filename has the proper extension.
	if(!filename.endsWith(".state"))
		filename += ".state";

  std::ofstream out;
	out.open(filename.toStdString().c_str(), std::ofstream::out);
	out << volumeName << "\n";

	getWindow()->saveState(out);

	getLights().saveState(out);
	getTransferFunctionEditor().saveState(out);
	getSlicesEditor().saveState(out);
	getIsosEditor().saveState(out);
	out.close();
}

void VolumeViewer::commitSlices()
{
	slicesEditor.commit(renderer);
	ospCommit(renderer);
	render();
}

void VolumeViewer::commitIsos()
{
	isosEditor.commit(&volume);
	// ospCommit(renderer);
	volume.commit();
	render();
}
	
void VolumeViewer::initUserInterfaceWidgets() {

	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	
	QAction *openAct = new QAction(tr("Open"), this);
	fileMenu->addAction(openAct);
	connect(openAct, SIGNAL(triggered()), this, SLOT(openVolume()));

	QAction *openStateAct = new QAction(tr("Open State"), this);
	fileMenu->addAction(openStateAct);
	connect(openStateAct, SIGNAL(triggered()), this, SLOT(openState()));

	QAction *saveStateAct = new QAction(tr("Save State"), this);
	fileMenu->addAction(saveStateAct);
	connect(saveStateAct, SIGNAL(triggered()), this, SLOT(saveState()));

  //! Create the transfer function editor dock widget, this widget modifies the transfer function directly.
  QDockWidget *transferFunctionEditorDockWidget = new QDockWidget("Transfer Function Editor", this);
  transferFunctionEditorDockWidget->setWidget(&transferFunctionEditor);
  connect(&transferFunctionEditor, SIGNAL(transferFunctionChanged()), this, SLOT(commitVolume()));
  connect(&transferFunctionEditor, SIGNAL(transferFunctionChanged()), this, SLOT(render()));
  addDockWidget(Qt::LeftDockWidgetArea, transferFunctionEditorDockWidget);

  //! Set the transfer function editor widget to its minimum allowed height, to leave room for other dock widgets.
  transferFunctionEditor.setMaximumHeight(transferFunctionEditor.minimumSize().height());

	QDockWidget *slicesEditorDockWidget = new QDockWidget("Slice Planes Editor", this);
	slicesEditorDockWidget->setWidget(&slicesEditor);
	connect(&slicesEditor, SIGNAL(slicesChanged()), this, SLOT(commitSlices()));
  addDockWidget(Qt::LeftDockWidgetArea, slicesEditorDockWidget);
  slicesEditor.setMaximumHeight(slicesEditor.minimumSize().height());

	QDockWidget *isosEditorDockWidget = new QDockWidget("Isovalues Editor", this);
	isosEditorDockWidget->setWidget(&isosEditor);
	connect(&isosEditor, SIGNAL(isosChanged()), this, SLOT(commitIsos()));
  addDockWidget(Qt::LeftDockWidgetArea, isosEditorDockWidget);
  isosEditor.setMaximumHeight(isosEditor.minimumSize().height());

}
