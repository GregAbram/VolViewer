
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

	renderProperties.setRenderer(renderer);

  //! Create an OSPRay window and set it as the central widget, but don't let it start rendering until we're done with setup.
  osprayWindow = new QOSPRayWindow(this, renderer, showFrameRate);  setCentralWidget(osprayWindow);

	// lights.commit(renderer);
  
  //! Configure the user interface widgets and callbacks.
  initUserInterfaceWidgets();

  //! Commit the transfer function only after the initial colors and alphas have been set (workaround for Qt signalling issue).
	getTransferFunctionEditor().getTransferFunction().commit(renderer);
	slicesEditor.commit(renderer, &volume);
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

	float vmin, vmax;
	volume.GetMinMax(vmin, vmax);
	getTransferFunctionEditor().getTransferFunction().SetMin(vmin);
	getTransferFunctionEditor().getTransferFunction().SetMax(vmax);
	getTransferFunctionEditor().commit();

	int x, y, z;
	volume.GetDimensions(x, y, z);

	int m = x > y ? x > z ? x : z : y > z ? y : z;

	getWindow()->getCamera().setPos(x/2.0, y/2.0, -(3*m - z/2.0));
	getWindow()->getCamera().setDir(0.0, 0.0, 3*m);
	getWindow()->getCamera().commit();

	osprayWindow->setRenderingEnabled(true);
	render();
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

static char xyzzy[10240];
void VolumeViewer::loadState(std::string statename)
{
	Document doc;

  std::ifstream in; 
	in.open(statename.c_str(), std::istream::in);
  in.seekg(0, std::ios::end);
  std::streamsize size = in.tellg();
  in.seekg(0, std::ios::beg);

  in.read(xyzzy, size);
  doc.Parse(xyzzy);
	in.close();

	if ((! doc.IsObject()) || (! doc.HasMember("State")))
	{
		std::cerr << "invalid state file\n";
		return;
	}

	if (! doc["State"].HasMember("Volume"))
	{
    std::cerr << "no volume?\n";
		return;
	}

	if (doc["State"].HasMember("Render Properties") )
	{
		getRenderProperties()->loadState(doc["State"]["Render Properties"]);
		getRenderProperties()->commit();
	}

	if (doc["State"].HasMember("Camera") )
	{
		getWindow()->loadState(doc["State"]["Camera"]);
		getWindow()->commit();
	}

	if (doc["State"].HasMember("Lights"))
	{
		getLights().loadState(doc["State"]["Lights"]);
		getLights().commit(renderer);
	}

	importFromFile(doc["State"]["Volume"].GetString());

	if (doc["State"].HasMember("TransferFunction"))
		getTransferFunctionEditor().loadState(doc["State"]["TransferFunction"]);			// Encompases colormap and opacity

	if (doc["State"].HasMember("Slices"))
		getSlicesEditor().loadState(doc["State"]["Slices"]);
	
	if (doc["State"].HasMember("Isosurfaces"))
	{
		getIsosEditor().loadState(doc["State"]["Isosurfaces"]);
		getIsosEditor().commit(&volume);
		volume.commit();
	}

	float vmin, vmax;
	volume.GetMinMax(vmin, vmax);
	getTransferFunctionEditor().getTransferFunction().SetMin(vmin);
	getTransferFunctionEditor().getTransferFunction().SetMax(vmax);
	getTransferFunctionEditor().commit();

	osprayWindow->setRenderingEnabled(true);
	render();
}

void VolumeViewer::saveState()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save state", ".", "State files (*.state)");
	if(filename.isNull())
		return;

	//! Make sure the filename has the proper extension.
	if(!filename.endsWith(".state"))
		filename += ".state";

	Document doc;
	doc.Parse("{}");

	Value state(kObjectType);

	state.AddMember("Volume", Value().SetString(volumeName.c_str(), doc.GetAllocator()), doc.GetAllocator());

	getWindow()->saveState(doc, state);
	getRenderProperties()->saveState(doc, state);

	getLights().saveState(doc, state);
	getTransferFunctionEditor().saveState(doc, state);
	getSlicesEditor().saveState(doc, state);
	getIsosEditor().saveState(doc, state);

	doc.AddMember("State", state, doc.GetAllocator());

  StringBuffer sbuf;
  PrettyWriter<StringBuffer> writer(sbuf);
  doc.Accept(writer);
	
  std::ofstream out;
	out.open(filename.toStdString().c_str(), std::ofstream::out);
  out << sbuf.GetString() << "\n";
	out.close();
}

void VolumeViewer::commitSlices()
{
	slicesEditor.commit(renderer, &volume);
	ospCommit(renderer);
	render();
}

void VolumeViewer::commitIsos()
{
	isosEditor.commit(&volume);
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
  QDockWidget *renderPropertiesDockWidget = new QDockWidget("Render Properties", this);
  renderPropertiesDockWidget->setWidget(&renderProperties);
  addDockWidget(Qt::LeftDockWidgetArea, renderPropertiesDockWidget);
	connect(&renderProperties, SIGNAL(renderPropertiesChanged()), this, SLOT(render()));

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
