#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <clipper2/clipper.h>
#include <QLabel>
#include <QSlider>
#include "GcodeCreator.h"
#include <QProgressBar>
#include <QCheckBox>
/*
* Window that shows all widgets and controls in 1 place
* Contains RenderView and controls
*/
class ObjectRenderView;
class SliceWindow;
class GcodeCreator;
class SliceOperations;
class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();
	int getSliceAmount() { return sliceAmount; }
public slots:
	void changeSlicerHeight(double height);
	void sliderValueChanged(int value);
private slots:
	void sliceModel();
	void openLoadModelDialog();
	void openGCodeDialog();
	void setBedDimensions();
	void updateBedText();
	void generateGcode();
	void limitInfillDensity();
	void updateSpeedLabel();
	void showColorGuide();
private:
	ObjectRenderView* widget;
	QSlider* slider;
	QDoubleSpinBox* slicerHeightInputBox;
	QPushButton* sliceButton;
	SliceWindow* sliceWindow;
	QPushButton* loadButton;
	QPushButton* gcodeButton;
	QVBoxLayout* panelLayout;
	QWidget* gridWidget;
	QWidget* sidePanel;
	QProgressBar* progressBar;
	QLabel* progressText;
	std::vector<QLabel*> labels;
	std::vector<QDoubleSpinBox*> slicingParameterInputBoxes;
	QCheckBox* enableSupport;
	//std::vector<Clipper2Lib::PathsD> allCompiledSlices;
	std::vector<Clipper2Lib::PathsD> erodedSlices;
	std::vector<std::vector<Clipper2Lib::PathsD>> shells;
	std::vector<Clipper2Lib::PathsD> infill;
	std::vector<std::vector<Clipper2Lib::PathsD>> floors;
	std::vector<std::vector<Clipper2Lib::PathsD>> roofs;
	std::vector<Clipper2Lib::PathsD> mostInnerShells;
	// Rectangular Support
	std::vector<Clipper2Lib::PathsD> erodedSupportPerimeter;
	std::vector<Clipper2Lib::PathsD> supportInfill;
	// Zigzag Support
	std::vector<Clipper2Lib::PathsD> zigzagSupportInfill;
	GcodeCreator* gcodeCreator;
	SliceOperations* sliceOperations;
	

	std::string modelFilePath;
	void loadModel(std::string path);
	void createSlicingParameterWidgets();
	void createBedDimensions();
	void createSlicerHeightInput();
	void createObjectRenderView();
	void createProgressBar();

    // Create a vertical layout for the main bed dimensions section
	QVBoxLayout* bedDimensionsMainLayout;
	QLabel* bedLabel;
	QHBoxLayout* bedDimensionsLayout;
	QLineEdit* bedWidthInput;
	QLabel* widthUnitLabel;
	QLabel* xLabel;
	QLineEdit* bedDepthInput;
	QLabel* depthUnitLabel;
	QHBoxLayout* setDimLayout;
	QPushButton* setDimButton;

	void drawCompleteSlice(int index);
	void calculateSlices();

	int sliceAmount = 0;

	QLineEdit* filenameInput;
	QDoubleSpinBox* printbedTempInput;
	QDoubleSpinBox* nozzleTempInput;
	QDoubleSpinBox* speedMultiplierInput;
	QCheckBox* enableRetraction;


	QLabel* printbedTempLabel;
	QLabel* nozzleTempLabel;
	QLabel* speedMultiplierLabel;
	QLabel* retractionLabel;

	QString gCodeFileName;
	float speedMultiplier;
	float printBedTemp;
	float nozzleTemp;
	bool retractionToggle;

};

