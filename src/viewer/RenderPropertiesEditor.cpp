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

	l->addWidget(new QLabel("Ambient / Lambertian ratio"), 0, 0, 1, 4);

	ambient_slider.setProperty("orientation", Qt::Horizontal);
	ambient_slider.setRange(0, 100);
	ambient_slider.setValue((int)(100*renderProperties.getAmbient()));

	ambient_current.setValidator(new QDoubleValidator(0.0, 1.0, 10));
	ambient_current.setText(QString::number(renderProperties.getAmbient()));

	connect(&ambient_slider, SIGNAL(sliderMoved(int)), this, SLOT(ambientSliderChanged(int)));
	connect(&ambient_current, SIGNAL(returnPressed()), this, SLOT(ambientTextChanged()));
	
	l->addWidget(&ambient_slider, 1, 0, 1, 2);
	l->addWidget(&ambient_current, 1, 2, 1, 1);

	l->addWidget(new QLabel("Number of AO samples"),  2, 0, 1, 4);

	ao_number_of_samples_slider.setProperty("orientation", Qt::Horizontal);
	ao_number_of_samples_slider.setRange(0, 64);
	ao_number_of_samples_slider.setValue(0);

	ao_number_of_samples_current.setValidator(new QIntValidator(0, 64));
	ao_number_of_samples_current.setText(QString::number(renderProperties.getNumAOSamples()));

	connect(&ao_number_of_samples_slider, SIGNAL(sliderMoved(int)), this, SLOT(numAOSamplesSliderChanged(int)));
	connect(&ao_number_of_samples_current, SIGNAL(returnPressed()), this, SLOT(numAOSamplesTextChanged()));
	
	l->addWidget(&ao_number_of_samples_slider, 3, 0, 1, 2);
	l->addWidget(&ao_number_of_samples_current, 3, 2, 1, 1);

	l->addWidget(new QLabel("AO radius"),  4, 0, 1, 4);

	ao_radius_slider.setProperty("orientation", Qt::Horizontal);
	ao_radius_slider.setRange(0, 100);
	ao_radius_slider.setValue((int)(100*renderProperties.getAORadius()));
	l->addWidget(&ao_radius_slider, 5, 0, 1, 2);

	ao_radius_current.setText(QString::number(renderProperties.getAORadius()));
	ao_radius_current.setValidator(new QIntValidator());
	ao_radius_max.setText(QString::number(4*renderProperties.getAORadius()));
	ao_radius_max.setValidator(new QIntValidator());

	l->addWidget(&ao_radius_current, 5, 2);
	l->addWidget(&ao_radius_max, 5, 3);

	connect(&ao_radius_slider, SIGNAL(sliderMoved(int)), this, SLOT(AORadiusSliderChanged(int)));
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

float
RenderPropertiesEditor::getAORadiusMax()
{
	return atof(ao_radius_max.text().toStdString().c_str());
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
	float r = atof(ao_radius_max.text().toStdString().c_str());
	ao_radius_current.setText(QString::number(c));
	ao_radius_slider.setValue(100*(int)(((float)c)/(float)r));
}

void   
RenderPropertiesEditor::setNumAOSamples(int i)
{
	renderProperties.setNumAOSamples(i);
	ao_number_of_samples_current.setText(QString::number(i));
	ao_number_of_samples_slider.setValue(i);
}

void
RenderPropertiesEditor::setAORadiusMax(float r)
{
	int c = atoi(ao_radius_current.text().toStdString().c_str());
	ao_radius_max.setText(QString::number(r));
	ao_radius_slider.setValue(100*(int)(((float)c)/(float)r));
}

void 
RenderPropertiesEditor::commit()
{
	renderProperties.commit();
	emit renderPropertiesChanged();
}

void
RenderPropertiesEditor::numAOSamplesSliderChanged(int k)
{
	renderProperties.setNumAOSamples(k);
	ao_number_of_samples_current.setText(QString::number(k));
	commit();
}

void
RenderPropertiesEditor::numAOSamplesTextChanged()
{
	int v = atoi(ao_number_of_samples_current.text().toStdString().c_str());
	renderProperties.setNumAOSamples(v);
	ao_number_of_samples_slider.setValue(v);
	commit();
}

void
RenderPropertiesEditor::AORadiusTextChanged()
{
	int r = atoi(ao_radius_max.text().toStdString().c_str());
	int c = atoi(ao_radius_current.text().toStdString().c_str());
	ao_radius_slider.setValue(100*(int)(((float)c)/(float)r));
	renderProperties.setAORadius(r);
	commit();
}

void
RenderPropertiesEditor::AORadiusSliderChanged(int k)
{
	int r = atoi(ao_radius_max.text().toStdString().c_str());
	renderProperties.setAORadius(r);
	float v = (k/100.0) * r;
	ao_radius_current.setText(QString::number(v));
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
