#include "MainWindow.h"
#include <QSplitter>
#include "ObjectRenderView.h"
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Create a central widget and set it as the main window's central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create a horizontal splitter to hold both the OpenGL widget and the side panel
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    // OpenGL widget for rendering the 3D model
    ObjectRenderView* widget = new ObjectRenderView();
	widget->loadModel("./resources/cube.stl");  // Load the STL model

    // Side panel widget
    QWidget* sidePanel = new QWidget();
    QVBoxLayout* panelLayout = new QVBoxLayout(sidePanel);

    // Example widgets for the side panel
    QLabel* label = new QLabel("Model Controls", sidePanel);
    QPushButton* loadButton = new QPushButton("Load Model", sidePanel);
    QSlider* rotationSlider = new QSlider(Qt::Horizontal, sidePanel);
    rotationSlider->setRange(0, 360);

    // Add widgets to the panel layout
    panelLayout->addWidget(label);
    panelLayout->addWidget(loadButton);
    panelLayout->addWidget(rotationSlider);
    panelLayout->addStretch(); // Add stretch to push widgets to the top

    // Add OpenGL widget and side panel to the splitter
    splitter->addWidget(widget);
    splitter->addWidget(sidePanel);

    // Control the initial size ratios of the OpenGL widget and the side panel
    splitter->setStretchFactor(0, 3);  // OpenGL widget takes more space initially
    splitter->setStretchFactor(1, 1);  // Side panel takes less space initially

    // Set the splitter as the main layout in the central widget
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(splitter);

    // Connect the slider to control rotation of the 3D model in the OpenGL widget
    //connect(rotationSlider, &QSlider::valueChanged, widget, &ModelViewer::setRotation);
}