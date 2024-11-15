#include "MainWindow.h"
#include <QSplitter>
#include "ObjectRenderView.h"
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include "SliceWindow.h"
#include <QFileDialog>
#include "GcodeCreator.h"
//#include "clipper2/clipper.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Create a central widget and set it as the main window's central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Create a horizontal splitter to hold both the OpenGL widget and the side panel
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    // OpenGL widget for rendering the 3D model
    widget = new ObjectRenderView();
    widget->loadModel("./resources/hole-test(easy).stl");
	  // Load the STL model

    // Side panel widget
    QWidget* sidePanel = new QWidget();
    QVBoxLayout* panelLayout = new QVBoxLayout(sidePanel);

    // Example widgets for the side panel
    QLabel* label = new QLabel("Model Controls", sidePanel);
    loadButton = new QPushButton("Load Model", sidePanel);

    //QSlider* rotationSlider = new QSlider(Qt::Horizontal, sidePanel);
    //rotationSlider->setRange(0, 360);

    slicerHeightInputBox = new QDoubleSpinBox(sidePanel);
    slicerHeightInputBox->setRange(-100.0, 100.0);
	slicerHeightInputBox->setSingleStep(0.1);
	slicerHeightInputBox->setValue(0.0);

    sliceButton = new QPushButton("Slice Model", sidePanel);

    sliceWindow = new SliceWindow();

    connect(slicerHeightInputBox, &QDoubleSpinBox::valueChanged, widget, &ObjectRenderView::setSliderSlicerHeight);
	connect(sliceButton, &QPushButton::clicked, this, &MainWindow::openSliceWindow);
	connect(loadButton, &QPushButton::clicked, this, &MainWindow::openLoadModelDialog);

    // Add widgets to the panel layout
    panelLayout->addWidget(label);
    panelLayout->addWidget(loadButton);
    panelLayout->addWidget(slicerHeightInputBox);
	panelLayout->addWidget(sliceButton);
    panelLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	panelLayout->addWidget(sliceWindow, Qt::AlignBottom);
    //panelLayout->addStretch(); // Add stretch to push widgets to the top

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

MainWindow::~MainWindow() {
	delete widget;
}

void MainWindow::openSliceWindow() {
	auto orderedLineSegments = widget->sliceMesh();
    GcodeCreator GCreator;
    GCreator.generateGCode(orderedLineSegments, "Test");
	sliceWindow->setSliceData(orderedLineSegments);
    SlicerPlane* slicer = widget->getSlicer();
	slicer->setContours(orderedLineSegments);
	Clipper2Lib::PathsD polygons = slicer->compilePolygons();
}

void MainWindow::openLoadModelDialog() {
	QFileDialog dialog(this);
    dialog.setDirectory("./resources");
    QString filePath = dialog.getOpenFileName(this, "Open File", "", "STL Files (*.stl)");
    if (!filePath.isEmpty()) {
		modelFilePath = filePath.toStdString();
        widget->loadModel(modelFilePath);
        widget->resetRendering();
    }
    qDebug() << "Empty filepath!";  // Print file path to console

}