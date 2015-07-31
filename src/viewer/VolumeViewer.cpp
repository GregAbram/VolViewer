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

VolumeViewer::VolumeViewer(bool showFrameRate) 
#if 1
  : renderer(NULL), 
    osprayWindow(NULL), 
		dataName("")
{
#else
{
	std::cerr << "here\n";
  renderer = NULL;
  osprayWindow = NULL;
	dataName = string("");
#endif

  //! Default window size.
  resize(1024, 768);

  //! Create an OSPRay renderer.
  renderer = ospNewRenderer("vis_renderer");  
	exitOnCondition(renderer == NULL, "could not create OSPRay renderer object");
	getTransferFunctionEditor().setRenderer(renderer);

	renderPropertiesEditor.setRenderer(renderer);

  //! Create an OSPRay window and set it as the central widget, but don't let it start rendering until we're done with setup.
  osprayWindow = new QOSPRayWindow(this, renderer, showFrameRate);  setCentralWidget(osprayWindow);

  //! Configure the user interface widgets and callbacks.
  initUserInterfaceWidgets();

  //! Commit the transfer function only after the initial colors and alphas have been set (workaround for Qt signalling issue).
	getTransferFunctionEditor().getTransferFunction().commit(renderer);

	// slicesEditor.commit(renderer, currentVolume);
	// isosEditor.commit(currentVolume);

  //! Show the window.
	raise();
	std::cerr << "Clearing?\n";
	osprayWindow->Clear();
  show();
}

void 
VolumeViewer::selectTimeStep(int t)
{
	if (t > volumeSeries.GetNumberOfMembers())
	{
		std::cerr << "t too big in selectTimeSeries\n";
		t = 0;
	}

	currentVolume = volumeSeries.GetMember(t);
	slicesEditor.commit(renderer, currentVolume);
	isosEditor.commit(currentVolume);

  OSPModel model = ospNewModel();
	ospAddVolume(model, currentVolume->getOSPVolume());
	ospCommit(model);  
	ospSetObject(renderer, "model", model);

	OSPModel dmodel = ospNewModel();
	ospCommit(dmodel);
	ospSetObject(renderer, "dynamic_model", dmodel);

	ospCommit(renderer);
	osprayWindow->setRenderingEnabled(true);
}

void VolumeViewer::importFromFile(const std::string &filename) {


	volumeSeries.Import(filename, getTransferFunctionEditor().getTransferFunction());

	dataName = filename;

	float min, max;
	volumeSeries.GetMinMax(min, max);

	isosEditor.setMinMax(min, max);
	getTransferFunctionEditor().setRange(min, max);

	resetCamera();

	timeEditor.setRange(volumeSeries.GetNumberOfMembers());

	selectTimeStep(0);
}

void
VolumeViewer::resetCamera()
{
	int x, y, z;
	volumeSeries.GetDimensions(x, y, z);

	int m = x > y ? x > z ? x : z : y > z ? y : z;

	osp::vec3f eye((x-1)/2.0, (y-1)/2.0, -(3*m - (z-1)/2.0));
	osp::vec3f center((x-1)/2.0, (y-1)/2.0, (z-1)/2.0);
	osp::vec3f up(0.0, 1.0, 0.0);

	getWindow()->getCameraEditor()->setupFrame(eye, center, up);
	getWindow()->getCameraEditor()->commit();
}

void
VolumeViewer::openVolume()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Load Volume"), ".", "volumes (*.vol)");

  if(filename.isEmpty())
    return;

	importFromFile(filename.toStdString());
}

void
VolumeViewer::openSeries()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Load Volume"), ".", "series (*.ser)");

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

	// Not vert
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

	importFromFile(doc["State"]["Volume"].GetString());

	if (doc["State"].HasMember("TransferFunction"))
		getTransferFunctionEditor().loadState(doc["State"]["TransferFunction"]);			// Encompases colormap and opacity

	if (doc["State"].HasMember("Slices"))
		getSlicesEditor().loadState(doc["State"]["Slices"]);
	
	if (doc["State"].HasMember("Isosurfaces"))
	{
		getIsosEditor().loadState(doc["State"]["Isosurfaces"]);
		getIsosEditor().commit(currentVolume);
		currentVolume->commit();
	}

	float vmin, vmax;
	currentVolume->GetMinMax(vmin, vmax);
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

	state.AddMember("Volume", Value().SetString(dataName.c_str(), doc.GetAllocator()), doc.GetAllocator());

	getWindow()->saveState(doc, state);
	getRenderProperties()->saveState(doc, state);

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

void VolumeViewer::commitLights()
{
	printf("commitLights\n");
}

void VolumeViewer::commitSlices()
{
	slicesEditor.commit(renderer, currentVolume);
	ospCommit(renderer);
	render();
}

void VolumeViewer::commitIsos()
{
	isosEditor.commit(currentVolume);
	currentVolume->commit();
	render();
}
	
void VolumeViewer::initUserInterfaceWidgets() {

	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	
	QAction *openVolumeAct = new QAction(tr("Open Volume"), this);
	fileMenu->addAction(openVolumeAct);
	connect(openVolumeAct, SIGNAL(triggered()), this, SLOT(openVolume()));

	QAction *openSeriesAct = new QAction(tr("Open Series"), this);
	fileMenu->addAction(openSeriesAct);
	connect(openSeriesAct, SIGNAL(triggered()), this, SLOT(openSeries()));

	QAction *openStateAct = new QAction(tr("Open State"), this);
	fileMenu->addAction(openStateAct);
	connect(openStateAct, SIGNAL(triggered()), this, SLOT(openState()));

	QAction *saveStateAct = new QAction(tr("Save State"), this);
	fileMenu->addAction(saveStateAct);
	connect(saveStateAct, SIGNAL(triggered()), this, SLOT(saveState()));

  QDockWidget *renderPropertiesEditorDockWidget = new QDockWidget("Render Properties", this);
	renderPropertiesEditorDockWidget->setFloating(true);
	renderPropertiesEditorDockWidget->hide();
  renderPropertiesEditorDockWidget->setWidget(&renderPropertiesEditor);
  addDockWidget(Qt::LeftDockWidgetArea, renderPropertiesEditorDockWidget);
	connect(&renderPropertiesEditor, SIGNAL(renderPropertiesChanged()), this, SLOT(render()));

  QDockWidget *transferFunctionEditorDockWidget = new QDockWidget("Transfer Function Editor", this);
	transferFunctionEditorDockWidget->setFloating(true);
	transferFunctionEditorDockWidget->hide();
  transferFunctionEditorDockWidget->setWidget(&transferFunctionEditor);
  connect(&transferFunctionEditor, SIGNAL(transferFunctionChanged()), this, SLOT(commitVolume()));
  connect(&transferFunctionEditor, SIGNAL(transferFunctionChanged()), this, SLOT(render()));
  addDockWidget(Qt::LeftDockWidgetArea, transferFunctionEditorDockWidget);

  //! Set the transfer function editor widget to its minimum allowed height, to leave room for other dock widgets.
  transferFunctionEditor.setMaximumHeight(transferFunctionEditor.minimumSize().height());

	QDockWidget *slicesEditorDockWidget = new QDockWidget("Slice Planes Editor", this);
	slicesEditorDockWidget->setFloating(true);
	slicesEditorDockWidget->hide();
	slicesEditorDockWidget->setWidget(&slicesEditor);
	connect(&slicesEditor, SIGNAL(slicesChanged()), this, SLOT(commitSlices()));
  addDockWidget(Qt::LeftDockWidgetArea, slicesEditorDockWidget);
  slicesEditor.setMaximumHeight(slicesEditor.minimumSize().height());

	QDockWidget *isosEditorDockWidget = new QDockWidget("Isovalues Editor", this);
	isosEditorDockWidget->setFloating(true);
	isosEditorDockWidget->hide();
	isosEditorDockWidget->setWidget(&isosEditor);
	connect(&isosEditor, SIGNAL(isosChanged()), this, SLOT(commitIsos()));
  addDockWidget(Qt::LeftDockWidgetArea, isosEditorDockWidget);
  isosEditor.setMaximumHeight(isosEditor.minimumSize().height());

	QDockWidget *timeEditorDockWidget = new QDockWidget("Time Manager", this);
	timeEditorDockWidget->setFloating(true);
	timeEditorDockWidget->hide();
	timeEditorDockWidget->setWidget(&timeEditor);
	connect(&timeEditor, SIGNAL(newTimeStep(int)), this, SLOT(selectTimeStep(int)));
  addDockWidget(Qt::LeftDockWidgetArea, timeEditorDockWidget);
  timeEditor.setMaximumHeight(timeEditor.minimumSize().height());

	QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
	
	QAction *renderPropertiesAction = new QAction(tr("Render Properties"), this);
	toolsMenu->addAction(renderPropertiesAction);
	connect(renderPropertiesAction, SIGNAL(triggered()), renderPropertiesEditorDockWidget, SLOT(show()));
	
	QAction *transferFunctionAction = new QAction(tr("Transfer Function"), this);
	toolsMenu->addAction(transferFunctionAction);
	connect(transferFunctionAction, SIGNAL(triggered()), transferFunctionEditorDockWidget, SLOT(show()));

	QAction *slicesAction = new QAction(tr("Slices"), this);
	toolsMenu->addAction(slicesAction);
	connect(slicesAction, SIGNAL(triggered()), slicesEditorDockWidget, SLOT(show()));

	QAction *isosAction = new QAction(tr("Isosurfaces"), this);
	toolsMenu->addAction(isosAction);
	connect(isosAction, SIGNAL(triggered()), isosEditorDockWidget, SLOT(show()));

	QAction *cameraAction = new QAction(tr("Camera/Lights"), this);
	toolsMenu->addAction(cameraAction);
	connect(cameraAction, SIGNAL(triggered()), osprayWindow->getCameraEditor(), SLOT(Open()));

	QAction *timeStepAction = new QAction(tr("Time Manager"), this);
	toolsMenu->addAction(timeStepAction);
	connect(timeStepAction, SIGNAL(triggered()), timeEditorDockWidget, SLOT(show()));

}
