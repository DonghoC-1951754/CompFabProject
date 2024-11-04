#include "MainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.resize(800, 600); // Set the initial size of the window
    mainWindow.show();
    //ObjectRenderView widget;
    //widget.loadModel("./resources/cube.stl");  // Load the STL model
    //widget.resize(800, 600);
    //widget.show();

    return app.exec();
}
