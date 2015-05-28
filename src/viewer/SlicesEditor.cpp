#include <iostream>
#include "SlicesEditor.h"

SlicesEditor::SlicesEditor()
{
	active = true;

	QVBoxLayout *vl = new QVBoxLayout();
  vl->setSizeConstraint(QLayout::SetMinimumSize);
  setLayout(vl);

	QWidget *w = new QWidget();
  QGridLayout *l = new QGridLayout();
  w->setLayout(l);

	vl->addWidget(w);

	l->addWidget(new QLabel("offset"),  0, 0, Qt::AlignCenter);
	l->addWidget(new QLabel("clip"),    0, 1, Qt::AlignRight);
	l->addWidget(new QLabel("flip"),    0, 2, Qt::AlignRight);
	l->addWidget(new QLabel("visible"), 0, 3, Qt::AlignRight);

	const char *labels[3] = { "X", "Y", "Z" };

	for (int i = 0; i < 3; i++)
	{
		sliders[i] = new QSlider(Qt::Horizontal);
		connect(sliders[i], SIGNAL(sliderMoved(int)), this, SLOT(sliderChanged(int)));
		l->addWidget(sliders[i], i+1, 0);

		clips[i] = new QCheckBox("");
		connect(clips[i], SIGNAL(stateChanged(int)), this, SLOT(slicesModified()));
		l->addWidget(clips[i], i+1, 1);

		flips[i] = new QCheckBox("");
		connect(flips[i], SIGNAL(stateChanged(int)), this, SLOT(slicesModified()));
		l->addWidget(flips[i], i+1, 2);

		visibility[i] = new QCheckBox("");
		connect(visibility[i], SIGNAL(stateChanged(int)), this, SLOT(slicesModified()));
		l->addWidget(visibility[i], i+1, 3);
	}

}

void 
SlicesEditor::loadState(std::istream& in)
{
	active = false;

	slices.loadState(in);
	for (int i = 0; i < 3; i++)
	{
		float v = slices.GetValue(i);
		int j = sliders[i]->minimum() + v * (sliders[i]->maximum() - sliders[i]->minimum());
		sliders[i]->setValue(j);

		if (slices.GetClip(i))
			flips[i]->setCheckState(Qt::Checked);
		else
			flips[i]->setCheckState(Qt::Unchecked);

		if (slices.GetFlip(i))
			flips[i]->setCheckState(Qt::Checked);
		else
			flips[i]->setCheckState(Qt::Unchecked);

		if (slices.GetVisible(i))
			visibility[i]->setCheckState(Qt::Checked);
		else
			visibility[i]->setCheckState(Qt::Unchecked);
	}

	active = true;
	slicesModified();
}

void 
SlicesEditor::saveState(std::ostream& out)
{
	update();
	slices.saveState(out);
}

void
SlicesEditor::sliderChanged(int v)
{
  slicesModified();
}

void
SlicesEditor::slicesModified()
{
	if (active)
		emit slicesChanged();
}

void 
SlicesEditor::update()
{
	for (int i = 0; i < 3; i++)
	{
		float v = ((float)(sliders[i]->value() - sliders[i]->minimum()))
												/ ((float)(sliders[i]->maximum() - sliders[i]->minimum()));
		slices.SetValue(i, v);
		slices.SetClip(i, clips[i]->isChecked());
		slices.SetFlip(i, flips[i]->isChecked());
		slices.SetVisible(i, visibility[i]->isChecked());
	}
}

void 
SlicesEditor::commit(OSPRenderer r, MyVolume *v)
{
	update();
	slices.commit(r, v);
}
