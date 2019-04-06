#pragma once

#include <QtWidgets>
#include <QtGui>
#include <QAction>

#include "EnrichedComboBox.h"
#include "EnrichedSlider.h"


class RenderSettingsBox : public QGroupBox 
{
	Q_OBJECT
public:
	RenderSettingsBox();
	~RenderSettingsBox();	
	
	int getUserPrefSliderValue();
	int getRoughSliderValue();
	int getSpecSliderValue();
	int getLightComboBoxSelIndexValue();

signals:
	void checkDiffuse(bool arg);
	void checkNormal(bool arg);
	void checkSpecular(bool arg);
	void checkRoughness(bool arg);

	void checkPrintSignal(bool arg);
	void saveModel();
	void cancelExport();

	void userPrefVal();
	void roughSliderVal();
	void specSliderVal();

	void lightCombo();
	void resetPressed();
private slots: 
	void checkPrint(bool arg);
	void cancelExportSlot();
private:
	QVBoxLayout *vbox;

	QGroupBox* modelBox;
	QGroupBox* lightBox;
	QGroupBox* textureBox;
	QGroupBox* printingBox;

	QGridLayout* modelLayout;
	QGridLayout* lightLayout;
	QGridLayout* textureLayout;
	QGridLayout* printingLayout;

	QPushButton* resetButton;
	QPushButton* SaveModelBox;
	QPushButton* cancelPrintBox;
	
	QCheckBox* diffuseCheck;
	QCheckBox* normalCheck;
	QCheckBox* roughCheck;
	QCheckBox* specCheck;
	QCheckBox* print3D;

	EnrichedSlider* intensitySlider;
	EnrichedSlider* RoughSlider;
	EnrichedSlider* SpecSlider;
	EnrichedSlider* userPref;

	EnrichedComboBox* MeshBox;
	EnrichedComboBox* TBox;
	EnrichedComboBox* lightsBox;

	vector<string> lightsBoxva;
};

