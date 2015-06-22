#pragma once

#include <QtGui>
#include "../common/common.h"
#include "Camera.h"

class CameraEditor;
class QOSPRayWindow;

class LightEditor : public QWidget
{
	Q_OBJECT

public:
	LightEditor(CameraEditor *, float x, float y, float z, float r, float g, float b);
	LightEditor(CameraEditor *);
	~LightEditor();

	bool getXYZRGB(osp::vec3f&, osp::vec3f&);

private slots:
	void removeMe();

private:
	void setup();
	QLineEdit x, y, z, r, g, b;
	CameraEditor *cameraEditor;
};

class CameraEditor : public QWidget
{
	Q_OBJECT

public:
	CameraEditor();
	~CameraEditor();

	void setWindow(QOSPRayWindow *win) { window = win; }
	void render();
	void commit();

	void addLight(float, float, float, float, float, float);

	void setEye(osp::vec3f);
	void setCenter(osp::vec3f);
	void setUp(osp::vec3f);
	void setFOV(float);

	bool getEye(osp::vec3f&);
	bool getCenter(osp::vec3f&);
	bool getUp(osp::vec3f&);
	bool getFOV(float&);

	void setupFrame(osp::vec3f, osp::vec3f, osp::vec3f);

	Camera *getCamera() { return &camera; }

  void saveState(Document &doc, Value &section);
  void loadState(Value& cam);

	void rotateFrame(float, float);
	void zoom(float);

public slots:
	void Apply();
	void Open();
	void addLight();
	void removeLight(QWidget *);

private:
	void setup();
	void updateEditorFromCamera();

	QWidget *dialog;
	QVBoxLayout activeLights;
	QLineEdit eyeX, eyeY, eyeZ;
	QLineEdit cenX, cenY, cenZ;
	QLineEdit upX, upY, upZ;
	QLineEdit fov;

	Camera camera;
	QOSPRayWindow *window;
};
