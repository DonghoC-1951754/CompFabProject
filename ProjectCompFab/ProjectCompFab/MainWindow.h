#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>

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
private:
	ObjectRenderView* widget;
	QDoubleSpinBox* slicerHeightInputBox;
	QPushButton* sliceButton;
	SliceWindow* sliceWindow;
	QPushButton* loadButton;
	QVBoxLayout* panelLayout;
	QWidget* gridWidget;
	std::vector<QDoubleSpinBox*> slicingParameterInputBoxes;
	

	std::string modelFilePath;
	void createSlicingParameterWidgets();

};

