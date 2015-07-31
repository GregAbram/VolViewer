#include <ospray/ospray.h>
#include <QtGui>
#include "Volume.h"
#include "Time.h"

#include "../common/common.h"

class TimeEditor : public QWidget {

	Q_OBJECT

public:
	TimeEditor();
	
	void setRange(int);

signals:
	void newTimeStep(int);

private slots:
	void sliderChanged(int);
	void textChanged();

private:

	QLineEdit text;
	QSlider   slider;
};
	


