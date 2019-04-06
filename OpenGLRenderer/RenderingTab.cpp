#include "RenderingTab.h"


RenderingTab::RenderingTab()
	:QWidget()
{
	vbox = new QVBoxLayout();
	topsplit = new QSplitter;
	
	renderSettingsBox = new RenderSettingsBox();
	topsplit->addWidget(renderSettingsBox);;
	rendererBox = new RendererBox;
	topsplit->addWidget(rendererBox);
	
	QObject::connect(renderSettingsBox, SIGNAL(checkDiffuse(bool)), this, SLOT(checkpressed(bool)));
	QObject::connect(renderSettingsBox, SIGNAL(checkNormal(bool)), this, SLOT(checkNormalpressed(bool)));
	QObject::connect(renderSettingsBox, SIGNAL(checkRoughness(bool)), this, SLOT(checkRoughnesspressed(bool)));
	QObject::connect(renderSettingsBox, SIGNAL(checkSpecular(bool)), this, SLOT(checkSpecularpressed(bool)));

	QObject::connect(renderSettingsBox, SIGNAL(resetPressed()), this, SLOT(resetPressed()));

	QObject::connect(renderSettingsBox, SIGNAL(userPrefVal()), this, SLOT(userPrefSlider()));
	QObject::connect(renderSettingsBox, SIGNAL(roughSliderVal()), this, SLOT(roughSliderValue()));
	QObject::connect(renderSettingsBox, SIGNAL(specSliderVal()), this, SLOT(specSliderValue()));

	QObject::connect(renderSettingsBox, SIGNAL(lightCombo()), this, SLOT(lightComboBoxSel()));

	QObject::connect(renderSettingsBox, SIGNAL(checkPrintSignal(bool)), this, SLOT(checkPrintPressed(bool)));
	QObject::connect(renderSettingsBox, SIGNAL(saveModel()), this, SLOT(SavePressed()));
	

	QList<int> list1;
	list1.append(500);
	list1.append(1300);
	topsplit->setSizes(list1);
	vbox->addWidget(topsplit); 
	setLayout(vbox);
}


RenderingTab::~RenderingTab()
{
	delete rendererBox;
	delete renderSettingsBox;
	
	delete topsplit;
	delete vbox;
}


void RenderingTab::checkPrintPressed(bool arg){
	rendererBox->printCheck(arg);
}

void RenderingTab::checkpressed(bool arg){
	rendererBox->diffuseCheck(arg);
}

void RenderingTab::checkNormalpressed(bool arg){
	rendererBox->normalCheck(arg);
}

void RenderingTab::checkRoughnesspressed(bool arg){
	rendererBox->roughnessCheck(arg);
}

void RenderingTab::checkSpecularpressed(bool arg){
	rendererBox->specularCheck(arg);
}


void RenderingTab::resetPressed(){
	rendererBox->resetPressed();
}


void RenderingTab::userPrefSlider(){
	int val = renderSettingsBox->getUserPrefSliderValue();
	rendererBox->userPrefSlider(val);
}

void RenderingTab::roughSliderValue(){
	int val = renderSettingsBox->getRoughSliderValue();
	rendererBox->roughnessSliderVal(val);
}

void RenderingTab::specSliderValue(){
	int val = renderSettingsBox->getSpecSliderValue();
	rendererBox->specularSliderVal(val);
}

void RenderingTab::lightComboBoxSel(){
	int val = renderSettingsBox->getLightComboBoxSelIndexValue();
	rendererBox->lightCombo(val);
}

void RenderingTab::SavePressed(){
	rendererBox->savePressed();
}