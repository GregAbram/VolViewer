#include <ospray/ospray.h>
#include <QtGui>
#include "Isos.h"
#include "MyVolume.h"

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
  void saveState(std::ostream&);

private:
	bool 	active;
	Isos isos;

	QCheckBox *onoffs[3];
	QSlider   *sliders[3];
};
	


