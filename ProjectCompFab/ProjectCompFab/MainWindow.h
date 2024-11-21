#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <clipper2/clipper.h>
#include <QLabel>
#include "GcodeCreator.h"
#include <QProgressBar>
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
private slots:
	void sliceModel();
	void openLoadModelDialog();
	void setBedDimensions();
	void updateBedText();
	void generateGcode();
	void limitInfillDensity();
private:
	ObjectRenderView* widget;
	QDoubleSpinBox* slicerHeightInputBox;
	QPushButton* sliceButton;
	SliceWindow* sliceWindow;
	QPushButton* loadButton;
	QPushButton* gcodeButton;
	QVBoxLayout* panelLayout;
	QWidget* gridWidget;
	QWidget* sidePanel;
	QProgressBar* progressBar;
	std::vector<QDoubleSpinBox*> slicingParameterInputBoxes;
	//std::vector<Clipper2Lib::PathsD> allCompiledSlices;
	std::vector<Clipper2Lib::PathsD> erodedSlices;
	std::vector<std::vector<Clipper2Lib::PathsD>> shells;
	std::vector<Clipper2Lib::PathsD> infill;
	std::vector<Clipper2Lib::PathsD> mostInnerShells;
	GcodeCreator* gcodeCreator;
	SliceOperations* sliceOperations;
	

	std::string modelFilePath;
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
};

