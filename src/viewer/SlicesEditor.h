#include <ospray/ospray.h>
#include <QtGui>
#include "MyVolume.h"
#include "Slices.h"

#include "../common/common.h"

class SlicesEditor : public QWidget {

	Q_OBJECT

public:
	SlicesEditor();
	Slices& getSlices() { return slices; }

	void commit(OSPRenderer r, MyVolume *v);
	void update();

signals:
	void slicesChanged();

public slots:
	void slicesModified();
	void sliderChanged(int);


public:
	void loadState(Value&);
  void saveState(Document&, Value&);

private:
	void  setup();
	bool 	active;
	Slices slices;

	QCheckBox *onoffs[3];
	QSlider   *sliders[3];
	QCheckBox *clips[3];
	QCheckBox *flips[3];
	QCheckBox *visibility[3];
};
	


