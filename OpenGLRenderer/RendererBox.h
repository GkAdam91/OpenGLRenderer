#pragma once

#include <QtWidgets>
#include <QtGui>
#include <QAction>
#include "glwidget.h"

class RendererBox : public QGroupBox 
{
	Q_OBJECT
public:
	RendererBox();
	~RendererBox();
public slots:
	void diffuseCheck(bool value);
	void normalCheck(bool value);
	void specularCheck(bool value);
	void roughnessCheck(bool value);

	void zoomSlider(int val);

	void userPrefSlider(int val);
	void roughnessSliderVal(int val);
	void specularSliderVal(int val);

	void printCheck(bool value);
	void savePressed();
	

	void lightCombo(int val);

	void resetPressed();
private:
	viewer::GLWidget *qOpenGLScene;
	QGridLayout *vbox4;
};

