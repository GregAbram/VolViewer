#include <iostream>
#include "IsosEditor.h"

IsosEditor::IsosEditor()
{
	active = true;

	QVBoxLayout *vl = new QVBoxLayout();
  vl->setSizeConstraint(QLayout::SetMinimumSize);
  setLayout(vl);

	QWidget *w = new QWidget();
  QGridLayout *l = new QGridLayout();
  w->setLayout(l);

	vl->addWidget(w);

	l->addWidget(new QLabel("on/off"), 0, 0, Qt::AlignLeft);
	l->addWidget(new QLabel("offset"), 0, 1, Qt::AlignCenter);

	const char *labels[3] = { "", "", "" };

	for (int i = 0; i < 3; i++)
	{
	  onoffs[i] = new QCheckBox(labels[i]);
		connect(onoffs[i], SIGNAL(stateChanged(int)), this, SLOT(isoModified()));
		l->addWidget(onoffs[i], i+1, 0);

		sliders[i] = new QSlider(Qt::Horizontal);
		connect(sliders[i], SIGNAL(sliderMoved(int)), this, SLOT(sliderChanged(int)));
		l->addWidget(sliders[i], i+1, 1);
	}

}

void 
IsosEditor::loadState(std::istream& in)
{
	active = false;

	isos.loadState(in);
	for (int i = 0; i < 3; i++)
	{
		float v = isos.GetValue(i);
		int j = sliders[i]->minimum() + v * (sliders[i]->maximum() - sliders[i]->minimum());
		sliders[i]->setValue(j);

		if (isos.GetOnOff(i))
			onoffs[i]->setCheckState(Qt::Checked);
		else
			onoffs[i]->setCheckState(Qt::Unchecked);
	}

	active = true;
	isoModified();
}

void 
IsosEditor::saveState(std::ostream& out)
{
	isos.saveState(out);
}

void
IsosEditor::sliderChanged(int v)
{
  isoModified();
}

void
IsosEditor::isoModified()
{
	if (active)
		emit isosChanged();
}

void 
IsosEditor::commit(MyVolume *vol)
{
	for (int i = 0; i < 3; i++)
	{
		float v = ((float)(sliders[i]->value() - sliders[i]->minimum()))
												/ ((float)(sliders[i]->maximum() - sliders[i]->minimum()));
		isos.SetValue(i, v);
		isos.SetOnOff(i, onoffs[i]->isChecked());
	}

	isos.commit(vol);
}
