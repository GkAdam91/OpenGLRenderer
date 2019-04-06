#pragma once

#include <QtWidgets>

#include "RenderSettingsBox.h"
#include "RendererBox.h"

class RenderingTab : public QWidget
{
	Q_OBJECT
public:
	RenderingTab();
	~RenderingTab();
public slots:
	void checkpressed(bool arg);
	void checkNormalpressed(bool arg);
	void checkSpecularpressed(bool arg);
	void checkRoughnesspressed(bool arg);
	void roughSliderValue();
	void specSliderValue();
	
	void checkPrintPressed(bool arg);

	void SavePressed();

	void userPrefSlider();
	void lightComboBoxSel();
	void resetPressed();
private:
	QVBoxLayout *vbox;
	QSplitter *topsplit;
	
	RendererBox* rendererBox;
	RenderSettingsBox *renderSettingsBox;
};

