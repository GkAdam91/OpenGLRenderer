#include "RendererBox.h"


RendererBox::RendererBox() : QGroupBox(tr("Renderer"))
{
	vbox4 = new QGridLayout;
	qOpenGLScene = new viewer::GLWidget();
	vbox4->addWidget(qOpenGLScene);
	setLayout(vbox4);
}

RendererBox::~RendererBox()
{
	delete qOpenGLScene;
	delete vbox4;
}

void RendererBox::printCheck(bool value){
	qOpenGLScene->printCheck(value);
}

void RendererBox::diffuseCheck(bool value){
	qOpenGLScene->diffTextureCheck(value);
}

void RendererBox::normalCheck(bool value){
	qOpenGLScene->normTextureCheck(value);
}

void RendererBox::specularCheck(bool value){
	qOpenGLScene->specTextureCheck(value);
}

void RendererBox::roughnessCheck(bool value){
	qOpenGLScene->roughTextureCheck(value);
}


void RendererBox::zoomSlider(int val){
	qOpenGLScene->scaleSlider(val);
}

void RendererBox::userPrefSlider(int val){
	qOpenGLScene->showIndexSlider(val);
}

void RendererBox::roughnessSliderVal(int val){
	qOpenGLScene->roughSlider(val);
}

void RendererBox::specularSliderVal(int val){
	qOpenGLScene->specSlider(val);
}

void RendererBox::lightCombo(int val){
	qOpenGLScene->lightComboBox(val);
}

void RendererBox::resetPressed(){
	qOpenGLScene->resetValues();
}

void RendererBox::savePressed(){
	qOpenGLScene->export3DModel();
}
