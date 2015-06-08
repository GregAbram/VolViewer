#include <ospray/ospray.h>
#include <QtGui>

#include "RenderProperties.h"

class RenderPropertiesEditor : public QWidget {

  Q_OBJECT

public:
  RenderPropertiesEditor();

  void commit();
  void setRenderer(OSPRenderer r) { renderProperties.setRenderer(r); }

  void loadState(Value&);
  void saveState(Document&, Value&);

  float getAmbient();
  float getAORadius();
  int   getNumAOSamples();
  float getAORadiusMax();

  void setAmbient(float);
  void setAORadius(float);
  void setNumAOSamples(int);
  void setAORadiusMax(float);

signals:
  void renderPropertiesChanged();

private slots:
  void ambientSliderChanged(int);
  void ambientTextChanged();
  void numAOSamplesSliderChanged(int);
  void numAOSamplesTextChanged();
  void AORadiusSliderChanged(int);
  void AORadiusTextChanged();

private:
  QSlider ambient_slider;
  QLineEdit ambient_current;

  QSlider ao_number_of_samples_slider;
  QLineEdit ao_number_of_samples_current;

  QSlider ao_radius_slider;
  QLineEdit ao_radius_current;
  QLineEdit ao_radius_max;

	RenderProperties renderProperties;
};



