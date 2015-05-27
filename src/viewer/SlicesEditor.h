#include <ospray/ospray.h>
#include <QtGui>
#include "Slices.h"

class SlicesEditor : public QWidget {

	Q_OBJECT

public:
	SlicesEditor();
	Slices& getSlices() { return slices; }

	void commit(OSPRenderer r);
	void update();

signals:
	void slicesChanged();

public slots:
	void slicesModified();
	void sliderChanged(int);


public:
	void loadState(std::istream&);
  void saveState(std::ostream&);

private:
	bool 	active;
	Slices slices;

	QCheckBox *onoffs[3];
	QSlider   *sliders[3];
	QCheckBox *clips[3];
	QCheckBox *flips[3];
	QCheckBox *visibility[3];
};
	


