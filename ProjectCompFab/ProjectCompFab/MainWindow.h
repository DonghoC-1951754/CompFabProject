#pragma once
#include <QMainWindow>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>

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

private slots:
	void openSliceWindow();
	void openLoadModelDialog();
private:
	ObjectRenderView* widget;
	QDoubleSpinBox* slicerHeightInputBox;
	QPushButton* sliceButton;
	SliceWindow* sliceWindow;
	QPushButton* loadButton;

	std::string modelFilePath;

};

