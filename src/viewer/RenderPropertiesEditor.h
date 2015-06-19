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

private:

  float getAmbient();
  float getAORadius();
  int   getNumAOSamples();

  void setAmbient(float);
  void setAORadius(float);
  void setNumAOSamples(int);

signals:
  void renderPropertiesChanged();

private slots:
  void ambientSliderChanged(int);
  void ambientTextChanged();
  void numAOSamplesTextChanged();
  void AORadiusTextChanged();

private:
  QSlider ambient_slider;
  QLineEdit ambient_current;

  QLineEdit ao_number_of_samples_current;
  QLineEdit ao_radius_current;

	RenderProperties renderProperties;
};



