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

#include "TransferFunctionEditor.h"
#include "TransferFunction.h"

static std::string cmap_name(std::string name)
{
	size_t p = name.find_last_of('/');
	if (p != std::string::npos)
			name = name.substr(p+1);
	p = name.find_last_of(".cmap");
	if (p != std::string::npos)
		name = name.substr(0, p-4);
  return name;
}

TransferFunctionEditor::TransferFunctionEditor()
{
  //! Load color maps.
  loadColorMaps();

  //! Setup UI elments.
  QVBoxLayout * layout = new QVBoxLayout();
  layout->setSizeConstraint(QLayout::SetMinimumSize);
  setLayout(layout);

  //! Form layout.
  QWidget * formWidget = new QWidget();
  QFormLayout * formLayout = new QFormLayout();
  formWidget->setLayout(formLayout);
  layout->addWidget(formWidget);

  //! Color map choice.
  for(unsigned int i=0; i<colorMaps.size(); i++)
	{
		std::string name = cmap_name(colorMaps[i].getName());
    colorMapComboBox.addItem(name.c_str());
	}

	formLayout->addRow("Colormap", &colorMapComboBox);

  //! Widget containing opacity transfer function widget and scaling slider.
  QWidget * transferFunctionAlphaGroup = new QWidget();
  QHBoxLayout * hboxLayout = new QHBoxLayout();
  transferFunctionAlphaGroup->setLayout(hboxLayout);

  //! Opacity transfer function widget.
  hboxLayout->addWidget(&transferFunctionAlphaWidget);

  //! Opacity scaling slider, defaults to median value in range.
  transferFunctionAlphaScalingSlider.setValue(int(transferFunction.GetScale() * (transferFunctionAlphaScalingSlider.minimum() + transferFunctionAlphaScalingSlider.maximum())));
  transferFunctionAlphaScalingSlider.setOrientation(Qt::Vertical);
  hboxLayout->addWidget(&transferFunctionAlphaScalingSlider);

  layout->addWidget(transferFunctionAlphaGroup);

  connect(&colorMapComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setColorMapIndex(int)));
  connect(&transferFunctionAlphaWidget, SIGNAL(transferFunctionChanged()), this, SLOT(alphaWidgetChanged()));
  connect(&transferFunctionAlphaScalingSlider, SIGNAL(valueChanged(int)), this, SLOT(alphaWidgetChanged()));

	setColorMapIndex(0);
}

void TransferFunctionEditor::alphaWidgetChanged()
{
	std::vector< osp::vec2f > alphas;
	QVector< QPointF > widgetAlphas = transferFunctionAlphaWidget.getPoints();

	for (int i = 0; i < widgetAlphas.size(); i++)
		alphas.push_back(osp::vec2f(widgetAlphas[i].x(), widgetAlphas[i].y()));

	getTransferFunction().SetAlphas(alphas);

	float scale = float(transferFunctionAlphaScalingSlider.value() - transferFunctionAlphaScalingSlider.minimum()) / float(transferFunctionAlphaScalingSlider.maximum() - transferFunctionAlphaScalingSlider.minimum());


	getTransferFunction().SetScale(scale);

	modifiedTransferFunction();
}

void TransferFunctionEditor::modifiedTransferFunction() {
	if (renderer) transferFunction.commit(renderer);
  emit transferFunctionChanged();
}

void TransferFunctionEditor::loadColorMap()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Load colormap"), ".", "colormap files (*.cmap)");

  if(filename.isEmpty())
    return;

	loadColorMapFile(filename.toStdString());
}

void TransferFunctionEditor::loadColorMapFile(string filename)
{
	VColorMap cmap(filename);
	addColorMap(cmap);
}

void TransferFunctionEditor::addColorMap(VColorMap cmap)
{
	colorMaps.push_back(cmap);
  colorMapComboBox.addItem(cmap_name(colorMaps[colorMaps.size()-1].getName()).c_str());
  colorMapComboBox.setCurrentIndex(colorMaps.size()-1);
}

void TransferFunctionEditor::loadState(std::istream& in)
{

	transferFunction.loadState(in);

  transferFunctionAlphaScalingSlider.setValue(int(transferFunction.GetScale() * (transferFunctionAlphaScalingSlider.minimum() + transferFunctionAlphaScalingSlider.maximum())));

	QVector<QPointF> points;
	for (int i = 0; i < transferFunction.GetAlphas().size(); i++)
		points.push_back(QPointF(transferFunction.GetAlphas()[i].x, transferFunction.GetAlphas()[i].y));

  transferFunctionAlphaWidget.setPoints(points);

	ColorMap cmap;
	cmap.loadState(in);

	int colorMapIndex;
  for(colorMapIndex = 0; colorMapIndex < colorMaps.size(); colorMapIndex++)
		if (colorMaps[colorMapIndex].getName() == cmap.getName())
			break;

	if (colorMapIndex == colorMaps.size())
		addColorMap(VColorMap(cmap));

	colorMapComboBox.setCurrentIndex(colorMapIndex);

}

void TransferFunctionEditor::loadState(Document& in)
{

	transferFunction.loadState(in);

  transferFunctionAlphaScalingSlider.setValue(int(transferFunction.GetScale() * (transferFunctionAlphaScalingSlider.minimum() + transferFunctionAlphaScalingSlider.maximum())));

	QVector<QPointF> points;
	for (int i = 0; i < transferFunction.GetAlphas().size(); i++)
		points.push_back(QPointF(transferFunction.GetAlphas()[i].x, transferFunction.GetAlphas()[i].y));

  transferFunctionAlphaWidget.setPoints(points);

	ColorMap cmap;
	cmap.loadState(in);

	int colorMapIndex;
  for(colorMapIndex = 0; colorMapIndex < colorMaps.size(); colorMapIndex++)
		if (colorMaps[colorMapIndex].getName() == cmap.getName())
			break;

	if (colorMapIndex == colorMaps.size())
		addColorMap(VColorMap(cmap));

	colorMapComboBox.setCurrentIndex(colorMapIndex);

}

void TransferFunctionEditor::saveState(Document& out)
{
	transferFunction.saveState(out);
	colorMaps[colorMapComboBox.currentIndex()].getColorMap().saveState(out);
}

void TransferFunctionEditor::saveState(std::ostream& out)
{
	transferFunction.saveState(out);
	colorMaps[colorMapComboBox.currentIndex()].getColorMap().saveState(out);
}

void TransferFunctionEditor::setColorMapIndex(int index)
{
	colorMaps[index].commit(transferFunction);
  transferFunctionAlphaWidget.setBackgroundImage(colorMaps[index].getImage());
	modifiedTransferFunction();
}

void TransferFunctionEditor::loadColorMaps() {

	std::vector< ColorMap > cmaps = ColorMap::load_colormap_directory();

  for (std::vector< ColorMap >::iterator it = cmaps.begin(); it != cmaps.end(); ++it)
		colorMaps.push_back(VColorMap(*it));
}
