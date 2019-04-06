#include "RenderSettingsBox.h"
#include "RendererBox.h"

RenderSettingsBox::RenderSettingsBox() : QGroupBox(tr("Settings"))
{
	vbox = new QVBoxLayout;

	modelBox = new QGroupBox("Models");
	lightBox = new QGroupBox("Object manipulation");
	textureBox = new QGroupBox("Textures");
	printingBox = new QGroupBox("3D Printing");

	modelLayout = new QGridLayout();
	lightLayout = new QGridLayout();
	textureLayout = new QGridLayout();
	printingLayout = new QGridLayout();
		
	// model part
	vector<string> MeshBoxva;
	MeshBoxva.push_back("Model");
	MeshBox = new EnrichedComboBox(modelLayout, 0, MeshBoxva, 0, QString(tr("Model Selection")));
	vector<string> TBoxva;
	TBoxva.push_back("T0");
	TBox = new EnrichedComboBox(modelLayout, 0, TBoxva, 0, QString(tr("Time Selection")),2);


	//Light part
	resetButton = new QPushButton("Reset values");
	QObject::connect(resetButton, SIGNAL(pressed()), this, SIGNAL(resetPressed()));

	
	lightsBoxva.push_back("Model");
	lightsBoxva.push_back("Camera"); 
	lightsBoxva.push_back("light 1");
	lightsBoxva.push_back("light 2");
	lightsBoxva.push_back("light 3");
	lightsBoxva.push_back("light 4");
	lightsBox = new EnrichedComboBox(lightLayout, 0, lightsBoxva, 0, QString(tr("Object Selection")));
	QObject::connect(lightsBox, SIGNAL(updateValue()), this, SIGNAL(lightCombo()));

	intensitySlider = new EnrichedSlider(lightLayout, 0, 1, 100, 1, QString(tr("Light Intensity:")), 1., 2);
	lightLayout->addWidget(resetButton, 1, 3, Qt::AlignRight);

	
	//shader settings part
	diffuseCheck = new QCheckBox(tr("Diffuse Texture"));
	diffuseCheck->setCheckState(Qt::Checked);
	normalCheck = new QCheckBox(tr("Normal Texture"));
	normalCheck->setCheckState(Qt::Checked);
	roughCheck = new QCheckBox(tr("Roughness Texture"));
	roughCheck->setCheckState(Qt::Checked);
	specCheck = new QCheckBox(tr("Specular Texture"));
	specCheck->setCheckState(Qt::Checked);

	textureLayout->addWidget(diffuseCheck, 0, 0);
	textureLayout->addWidget(normalCheck, 0, 2);

	textureLayout->addWidget(roughCheck, 1, 0);
	RoughSlider = new EnrichedSlider(textureLayout, 1, 1, 100, 1, QString(tr("Roughness:")), 1., 1);
	RoughSlider->setEnabled(false);

	textureLayout->addWidget(specCheck, 2, 0);
	SpecSlider = new EnrichedSlider(textureLayout, 2, 1, 100, 1, QString(tr("Specular:")), 1., 1);
	SpecSlider->setEnabled(false);

	userPref = new EnrichedSlider(textureLayout, 3, 1, 5, 1, QString(tr("Highlighted layer:")), 1., 1);


	QObject::connect(RoughSlider, SIGNAL(updateValue()), this, SIGNAL(roughSliderVal()));
	QObject::connect(SpecSlider, SIGNAL(updateValue()), this, SIGNAL(specSliderVal()));
	QObject::connect(diffuseCheck, SIGNAL(toggled(bool)), this, SIGNAL(checkDiffuse(bool)));
	QObject::connect(normalCheck, SIGNAL(toggled(bool)), this, SIGNAL(checkNormal(bool)));
	QObject::connect(roughCheck, SIGNAL(toggled(bool)), this, SIGNAL(checkRoughness(bool)));
	QObject::connect(roughCheck, SIGNAL(toggled(bool)), RoughSlider, SLOT(setDisabled(bool)));
	QObject::connect(specCheck, SIGNAL(toggled(bool)), this, SIGNAL(checkSpecular(bool)));
	QObject::connect(specCheck, SIGNAL(toggled(bool)), SpecSlider, SLOT(setDisabled(bool)));
	QObject::connect(userPref, SIGNAL(updateValue()), this, SIGNAL(userPrefVal()));
	
	//3D prining part
	SaveModelBox = new QPushButton("Save");
	SaveModelBox->setDisabled(true);
	cancelPrintBox = new QPushButton("Cancel");
	cancelPrintBox->setDisabled(true);
	print3D = new QCheckBox(tr("Export Model"));
	print3D->setCheckState(Qt::Unchecked);

	printingLayout->addWidget(print3D, 0, 0);
	printingLayout->addWidget(SaveModelBox, 0, 1);
	printingLayout->addWidget(cancelPrintBox, 0, 2);

	QObject::connect(print3D, SIGNAL(toggled(bool)), this, SLOT(checkPrint(bool)));
	QObject::connect(print3D, SIGNAL(toggled(bool)), this, SIGNAL(checkPrintSignal(bool)));
	QObject::connect(SaveModelBox, SIGNAL(pressed()), this, SIGNAL(saveModel()));
	QObject::connect(SaveModelBox, SIGNAL(pressed()), this, SLOT(cancelExportSlot()));
	QObject::connect(cancelPrintBox, SIGNAL(pressed()), this, SIGNAL(cancelExport()));
	QObject::connect(cancelPrintBox, SIGNAL(pressed()), this, SLOT(cancelExportSlot()));


	modelBox->setLayout(modelLayout);
	lightBox->setLayout(lightLayout);
	textureBox->setLayout(textureLayout);
	printingBox->setLayout(printingLayout);

	vbox->addWidget(modelBox);
	vbox->addWidget(lightBox);
	vbox->addWidget(textureBox);
	vbox->addWidget(printingBox);
	setLayout(vbox);

}


RenderSettingsBox::~RenderSettingsBox()
{
	delete intensitySlider;
	delete RoughSlider;
	delete SpecSlider;
	delete userPref;
	delete resetButton;
	delete diffuseCheck;
	delete normalCheck;
	delete roughCheck;
	delete specCheck;
	delete MeshBox;
	delete TBox;
	delete lightsBox;

	delete modelLayout;
	delete lightLayout;
	delete textureLayout;

	delete modelBox;
	delete lightBox;
	delete textureBox;

	delete vbox;
}



int RenderSettingsBox::getLightComboBoxSelIndexValue(){
	return lightsBox->getIndexValue();
}


int RenderSettingsBox::getUserPrefSliderValue(){
	return userPref->getValue();
}

int RenderSettingsBox::getRoughSliderValue(){
	return RoughSlider->getValue();
}

int RenderSettingsBox::getSpecSliderValue(){
	return SpecSlider->getValue();
}

void RenderSettingsBox::checkPrint(bool arg){
	SaveModelBox->setDisabled(!arg);
	cancelPrintBox->setDisabled(!arg);
	if (arg){
		lightsBox->addItem("Selector");
	}
	else{
		lightsBox->removeItem("Selector");
	}
	
}

void RenderSettingsBox::cancelExportSlot(){
	print3D->setCheckState(Qt::Unchecked);
//	checkPrint(false);
}