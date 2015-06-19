#include <iostream>
#include "RenderPropertiesEditor.h"

RenderPropertiesEditor::RenderPropertiesEditor()
{
	QVBoxLayout *vl = new QVBoxLayout();
  vl->setSizeConstraint(QLayout::SetMinimumSize);
  setLayout(vl);

	QWidget *w = new QWidget();
  QGridLayout *l = new QGridLayout();
  w->setLayout(l);

	vl->addWidget(w);

	l->addWidget(new QLabel("A/L ratio"), 0, 0, 1, 1);

	ambient_slider.setProperty("orientation", Qt::Horizontal);
	ambient_slider.setRange(0, 100);
	ambient_slider.setValue((int)(100*renderProperties.getAmbient()));

	ambient_current.setValidator(new QDoubleValidator(0.0, 1.0, 10));
	ambient_current.setText(QString::number(renderProperties.getAmbient()));

	connect(&ambient_slider, SIGNAL(sliderMoved(int)), this, SLOT(ambientSliderChanged(int)));
	connect(&ambient_current, SIGNAL(returnPressed()), this, SLOT(ambientTextChanged()));
	
	l->addWidget(&ambient_slider, 0, 1, 1, 2);
	l->addWidget(&ambient_current, 0, 3, 1, 1);

	l->addWidget(new QLabel("AO sample count"),  1, 0, 1, 1);

	ao_number_of_samples_current.setValidator(new QIntValidator(0, 64));
	ao_number_of_samples_current.setText(QString::number(renderProperties.getNumAOSamples()));

	connect(&ao_number_of_samples_current, SIGNAL(returnPressed()), this, SLOT(numAOSamplesTextChanged()));
	
	l->addWidget(&ao_number_of_samples_current, 1, 1, 1, 1);

	l->addWidget(new QLabel("radius"),  1, 2, 1, 1);
	ao_radius_current.setText(QString::number(renderProperties.getAORadius()));
	ao_radius_current.setValidator(new QIntValidator());

	l->addWidget(&ao_radius_current, 1, 3, 1, 1);

	connect(&ao_radius_current, SIGNAL(returnPressed()), this, SLOT(AORadiusTextChanged()));
}

float 
RenderPropertiesEditor::getAmbient()
{
	return renderProperties.getAmbient();
}

float 
RenderPropertiesEditor::getAORadius()
{
	return renderProperties.getAORadius();
}

int   
RenderPropertiesEditor::getNumAOSamples()
{
	return renderProperties.getNumAOSamples();
}

void 
RenderPropertiesEditor::setAmbient(float a)
{
	renderProperties.setAmbient(a);
	ambient_current.setText(QString::number(a));
	ambient_slider.setValue((int)(100*a));
}

void 
RenderPropertiesEditor::setAORadius(float c)
{
	renderProperties.setAORadius(c);
	ao_radius_current.setText(QString::number(c));
}

void   
RenderPropertiesEditor::setNumAOSamples(int i)
{
	renderProperties.setNumAOSamples(i);
	ao_number_of_samples_current.setText(QString::number(i));
}

void 
RenderPropertiesEditor::commit()
{
	renderProperties.commit();
	emit renderPropertiesChanged();
}

void
RenderPropertiesEditor::numAOSamplesTextChanged()
{
	int v = atoi(ao_number_of_samples_current.text().toStdString().c_str());
	renderProperties.setNumAOSamples(v);
	commit();
}

void
RenderPropertiesEditor::AORadiusTextChanged()
{
	int r = atoi(ao_radius_current.text().toStdString().c_str());
	renderProperties.setAORadius(r);
	commit();
}

void
RenderPropertiesEditor::ambientTextChanged()
{
	float a = atof(ambient_current.text().toStdString().c_str());
	renderProperties.setAmbient(a);
	ambient_slider.setValue((int)(100*a));
	commit();
}

void
RenderPropertiesEditor::ambientSliderChanged(int k)
{
	float v = k / 100.0;
	renderProperties.setAmbient(v);
	ambient_current.setText(QString::number(v));
	commit();
}

void 
RenderPropertiesEditor::loadState(Value& rp)
{
	renderProperties.loadState(rp);

	setAmbient(renderProperties.getAmbient());
	setAORadius(renderProperties.getAORadius());
	setNumAOSamples(renderProperties.getNumAOSamples());
}

void 
RenderPropertiesEditor::saveState(Document& doc, Value &section)
{
	renderProperties.saveState(doc, section);
}
