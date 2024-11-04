#pragma once
#include <QMainWindow>
#include <QWidget>

/*
* Window that shows all widgets and controls in 1 place
* Contains RenderView and controls
*/

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget* parent = nullptr);
};

