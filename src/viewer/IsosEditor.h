#include <ospray/ospray.h>
#include <QtGui>
#include "Isos.h"
#include "../common/common.h"
#include "MyVolume.h"

#include <math.h>

class IsosEditor : public QWidget {

	Q_OBJECT

public:
	IsosEditor();
	Isos& getIsos() { return isos; }

	void commit(MyVolume *v);
	void setMinMax(float min, float max) { isos.SetMinMax(min, max); }

signals:
	void isosChanged();

public slots:
	void isoModified();
	void sliderChanged(int);


public:
	void loadState(std::istream&);
	void loadState(Document &);
  void saveState(std::ostream&);
  void saveState(Document &);

private:
	void setup();

	bool 	active;
	Isos isos;

	QCheckBox *onoffs[3];
	QSlider   *sliders[3];
};
	


