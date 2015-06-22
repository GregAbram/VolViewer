#include "CameraEditor.h"
#include "QOSPRayWindow.h"

LightEditor::LightEditor(CameraEditor *ce)
{
	cameraEditor = ce;
	setup();
}

LightEditor::LightEditor(CameraEditor *ce, float X, float Y, float Z, float R, float G, float B)
{
	cameraEditor = ce;
	setup();
	x.setText(QString::number(X));
	y.setText(QString::number(Y));
	z.setText(QString::number(Z));
	r.setText(QString::number(R));
	g.setText(QString::number(G));
	b.setText(QString::number(B));
}
	
void
LightEditor::setup()
{
	QGridLayout *l = new QGridLayout();
	l->setSizeConstraint(QLayout::SetMinimumSize);
	setLayout(l);

	QPushButton *remove = new QPushButton();
	remove->setText("x");
	remove->setDefault(false);
	remove->setAutoDefault(false);
	l->addWidget(remove, 0, 0);

	l->addWidget(new QLabel("Direction"), 0, 1, 1, 1);
	l->addWidget(new QLabel("X"), 0, 2, 1, 1);
	x.setFixedWidth(60); l->addWidget(&x, 0, 3, 1, 1); x.setValidator(new QDoubleValidator());
	l->addWidget(new QLabel("Y"), 0, 4, 1, 1);
	y.setFixedWidth(60); l->addWidget(&y, 0, 5, 1, 1); y.setValidator(new QDoubleValidator());
	l->addWidget(new QLabel("Z"), 0, 6, 1, 1);
	z.setFixedWidth(60); l->addWidget(&z, 0, 7, 1, 1); z.setValidator(new QDoubleValidator());

	l->addWidget(new QLabel("Color"), 1, 1, 1, 1);
	l->addWidget(new QLabel("R"), 1, 2, 1, 1);
	r.setFixedWidth(60); l->addWidget(&r, 1, 3, 1, 1); r.setValidator(new QDoubleValidator());
	l->addWidget(new QLabel("G"), 1, 4, 1, 1);
	g.setFixedWidth(60); l->addWidget(&g, 1, 5, 1, 1); g.setValidator(new QDoubleValidator());
	l->addWidget(new QLabel("B"), 1, 6, 1, 1);
	b.setFixedWidth(60); l->addWidget(&b, 1, 7, 1, 1); b.setValidator(new QDoubleValidator());

	connect(remove, SIGNAL(clicked()), this, SLOT(removeMe()));
}

LightEditor::~LightEditor()
{
}

void
LightEditor::removeMe()
{
	cameraEditor->removeLight(this);
}

bool
LightEditor::getXYZRGB(osp::vec3f& dir, osp::vec3f& color)
{
	if (x.text().length() &&
			y.text().length() &&
			z.text().length() &&
			r.text().length() &&
			g.text().length() &&
			b.text().length())
	{
		dir.x = x.text().toFloat();
		dir.y = y.text().toFloat();
		dir.z = z.text().toFloat();
		color.x = r.text().toFloat();
		color.y = g.text().toFloat();
		color.z = b.text().toFloat();
		return true;
	}
	else
		return false;
}

CameraEditor::CameraEditor() 
{
	dialog = new QDialog(this);
	QVBoxLayout *panel = new QVBoxLayout();
	panel->setSizeConstraint(QLayout::SetMinimumSize);
	dialog->setLayout(panel);

	QWidget *cameraArea = new QWidget();
	QGridLayout *g = new QGridLayout();
	cameraArea->setLayout(g);

	g->addWidget(new QLabel("Eye:"), 0, 0, 1, 1, Qt::AlignLeft);
	g->addWidget(&eyeX, 0, 1, 1, 1); eyeX.setValidator(new QDoubleValidator()); eyeX.setText(QString::number(0.0));
	g->addWidget(&eyeY, 0, 2, 1, 1); eyeY.setValidator(new QDoubleValidator()); eyeY.setText(QString::number(0.0));
	g->addWidget(&eyeZ, 0, 3, 1, 1); eyeZ.setValidator(new QDoubleValidator()); eyeZ.setText(QString::number(-1.0));

	g->addWidget(new QLabel("Center:"), 1, 0, 1, 1, Qt::AlignLeft);
	g->addWidget(&cenX, 1, 1, 1, 1); cenX.setValidator(new QDoubleValidator()); cenX.setText(QString::number(0.0));
	g->addWidget(&cenY, 1, 2, 1, 1); cenY.setValidator(new QDoubleValidator()); cenY.setText(QString::number(0.0));
	g->addWidget(&cenZ, 1, 3, 1, 1); cenZ.setValidator(new QDoubleValidator()); cenZ.setText(QString::number(0.0));

	g->addWidget(new QLabel("Up:"), 2, 0, 1, 1, Qt::AlignLeft);
	g->addWidget(&upX, 2, 1, 1, 1); cenX.setValidator(new QDoubleValidator()); upX.setText(QString::number(0.0));
	g->addWidget(&upY, 2, 2, 1, 1); cenY.setValidator(new QDoubleValidator()); upY.setText(QString::number(1.0));
	g->addWidget(&upZ, 2, 3, 1, 1); cenZ.setValidator(new QDoubleValidator()); upZ.setText(QString::number(0.0));

	g->addWidget(new QLabel("FOV:"), 3, 0, 1, 1, Qt::AlignLeft);
	g->addWidget(&fov, 3, 1, 1, 1); fov.setValidator(new QDoubleValidator()); fov.setText(QString::number(45.0));
	
	panel->addWidget(cameraArea);

	QWidget *activeLightsWidget = new QWidget();
	activeLights.setSizeConstraint(QLayout::SetMinimumSize);
	activeLightsWidget->setLayout(&activeLights);
	panel->addWidget(activeLightsWidget);

	QWidget *buttonRow = new QWidget();
	QHBoxLayout *buttonRowLayout = new QHBoxLayout();
	buttonRowLayout->setSizeConstraint(QLayout::SetMinimumSize);
	buttonRow->setLayout(buttonRowLayout);

	QPushButton *add = new QPushButton();
	add->setText("Add");
	add->setDefault(false);
	add->setAutoDefault(false);
	connect(add, SIGNAL(clicked()), SLOT(addLight()));
	buttonRowLayout->addWidget(add);

	QPushButton *apply = new QPushButton();
	apply->setText("Apply");
	apply->setDefault(false);
	apply->setAutoDefault(false);
	connect(apply, SIGNAL(clicked()), SLOT(Apply()));
	buttonRowLayout->addWidget(apply);

	panel->addWidget(buttonRow);

	addLight(-1, 2, -3, 1, 1, 1);
}

CameraEditor::~CameraEditor()
{
}

void
CameraEditor::render()
{
	if (window)
		window->render();
}

void
CameraEditor::Open()
{
	dialog->show();
	dialog->raise();
	dialog->activateWindow();
}

void
CameraEditor::Apply()
{	
	commit();
	render();
}

void
CameraEditor::addLight()
{
	LightEditor *l = new LightEditor(this);
	activeLights.addWidget(l);
}

void 
CameraEditor::addLight(float x, float y, float z, float r, float g, float b)
{
	LightEditor *l = new LightEditor(this, x, y, z, r, g, b);
	activeLights.addWidget(l);
}

void
CameraEditor::removeLight(QWidget *light)
{
	activeLights.removeWidget(light);
	delete light;
}

void CameraEditor::setEye(osp::vec3f eye)
{
	eyeX.setText(QString::number(eye.x));
	eyeY.setText(QString::number(eye.y));
	eyeZ.setText(QString::number(eye.z));
}

void CameraEditor::setCenter(osp::vec3f center)
{
	cenX.setText(QString::number(center.x));
	cenY.setText(QString::number(center.y));
	cenZ.setText(QString::number(center.z));
}

void CameraEditor::setUp(osp::vec3f up)
{
	upX.setText(QString::number(up.x));
	upY.setText(QString::number(up.y));
	upZ.setText(QString::number(up.z));
}

void CameraEditor::setFOV(float f)
{
	fov.setText(QString::number(f));
}


bool CameraEditor::getEye(osp::vec3f& eye)
{
  if (eyeX.text().length() &&
      eyeY.text().length() &&
      eyeZ.text().length())
  {
    eye.x = eyeX.text().toFloat();
    eye.y = eyeY.text().toFloat();
    eye.z = eyeZ.text().toFloat();
    return true;
  }
  else
    return false;
}

bool CameraEditor::getCenter(osp::vec3f& center)
{
  if (cenX.text().length() &&
      cenY.text().length() &&
      cenZ.text().length())
  {
    center.x = cenX.text().toFloat();
    center.y = cenY.text().toFloat();
    center.z = cenZ.text().toFloat();
    return true;
  }
  else
    return false;
}

bool CameraEditor::getUp(osp::vec3f& up)
{
  if (upX.text().length() &&
      upY.text().length() &&
      upZ.text().length())
  {
    up.x = upX.text().toFloat();
    up.y = upY.text().toFloat();
    up.z = upZ.text().toFloat();
    return true;
  }
  else
    return false;
}

bool CameraEditor::getFOV(float& f)
{
  if (fov.text().length())
  {
    f = fov.text().toFloat();
    return true;
  }
  else
    return false;
}

void 
CameraEditor::setupFrame(osp::vec3f e, osp::vec3f c, osp::vec3f u)
{
	setEye(e);
	setCenter(c);
	setUp(u);
	camera.setupFrame(e, c, u);
}

void CameraEditor::rotateFrame(float du, float dv)
{
	camera.rotateFrame(du, dv);
	updateEditorFromCamera();
	camera.commit();
}

void CameraEditor::zoom(float d)
{
  camera.zoom(d);
	updateEditorFromCamera();
	camera.commit();
}

void CameraEditor::updateEditorFromCamera()
{
	osp::vec3f e, c, u;

	camera.getPos(e);
	camera.getCenter(c);
	camera.getUp(u);

	setEye(e);
	setCenter(c);
	setUp(u);
}

void CameraEditor::commit()
{
	osp::vec3f eye, center, up;
	float f;

	if (getEye(eye) && getCenter(center) && getUp(up) && getFOV(f))
	{
		camera.setupFrame(eye, center, up);
		camera.setFovY(f);
	}
	else
		std::cerr << "Camera specification is incomplete\n";

	camera.getLights()->clear();
	for (int i = 0; i < activeLights.count(); i++)
	{
		osp::vec3f d, c;
		LightEditor *le = (LightEditor *)activeLights.itemAt(i)->widget();
		if (le->getXYZRGB(d, c))
			camera.getLights()->addLight(d, c);
	}

	camera.commit();
}

void
CameraEditor::saveState(Document &doc, Value &section)
{
	camera.saveState(doc, section);
}

void
CameraEditor::loadState(Value& cam)
{
	camera.loadState(cam);
	updateEditorFromCamera();
}
