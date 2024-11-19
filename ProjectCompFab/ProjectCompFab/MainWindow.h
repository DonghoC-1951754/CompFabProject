#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <clipper2/clipper.h>
#include <QLabel>
#include "GcodeCreator.h"
/*
* Window that shows all widgets and controls in 1 place
* Contains RenderView and controls
*/
class ObjectRenderView;
class SliceWindow;
class GcodeCreator;
class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();
public slots:
	void changeSlicerHeight(double height);
	void changeLayerHeight(double layerHeight);
private slots:
	void openSliceWindow();
	void openLoadModelDialog();
	void setBedDimensions();
	void updateBedText();
private:
	ObjectRenderView* widget;
	QDoubleSpinBox* slicerHeightInputBox;
	QPushButton* sliceButton;
	SliceWindow* sliceWindow;
	QPushButton* loadButton;
	QVBoxLayout* panelLayout;
	QWidget* gridWidget;
	QWidget* sidePanel;
	std::vector<QDoubleSpinBox*> slicingParameterInputBoxes;
	//std::vector<Clipper2Lib::PathsD> allCompiledSlices;
	std::vector<Clipper2Lib::PathsD> erodedSlices;
	std::vector<Clipper2Lib::PathsD> erodedSlicesWithShells;
	std::vector<Clipper2Lib::PathsD> infill;
	std::vector<Clipper2Lib::PathsD> mostInnerShells;
	GcodeCreator* gcodeCreator;
	

	std::string modelFilePath;
	void createSlicingParameterWidgets();
	void createBedDimensions();

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
};

