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
class SliceRenderView;
class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();
private:
	ObjectRenderView* widget;
	SliceRenderView* sliceRenderWidget;
	QDoubleSpinBox* slicerHeightInputBox;
	QPushButton* sliceButton;

};

