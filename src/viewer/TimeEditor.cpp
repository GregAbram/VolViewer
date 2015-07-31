#include <iostream>
#include "TimeEditor.h"

TimeEditor::TimeEditor()
{
	QVBoxLayout *vl = new QVBoxLayout();
  vl->setSizeConstraint(QLayout::SetMinimumSize);
  setLayout(vl);

	QWidget *w = new QWidget();
  QGridLayout *l = new QGridLayout();
  w->setLayout(l);

	vl->addWidget(w);
	
	slider.setProperty("orientation", Qt::Horizontal);
	l->addWidget(&slider, 0, 0, 3, 1);
	connect(&slider, SIGNAL(sliderMoved(int)), this, SLOT(sliderChanged(int)));

	text.setValidator(new QIntValidator(0, 1));
	l->addWidget(&text, 0, 3);
	connect(&text, SIGNAL(returnPressed()), this, SLOT(textChanged()));

	setRange(1);
}

void 
TimeEditor::setRange(int n)
{
	slider.setRange(0, n-1);
	slider.setValue(0);

	((QIntValidator *)text.validator())->setTop(n-1);
	text.setText(QString::number(0));
}

void
TimeEditor::sliderChanged(int v)
{
	text.setText(QString::number(v));
	emit newTimeStep(v);
}

void
TimeEditor::textChanged()
{
	int v = atoi(text.text().toStdString().c_str());
	slider.setValue(v);
	emit newTimeStep(v);
}
