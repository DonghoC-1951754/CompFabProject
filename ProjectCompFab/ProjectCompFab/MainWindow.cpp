#include "MainWindow.h"
#include <QSplitter>
#include "ObjectRenderView.h"
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QLineEdit>
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
    sidePanel = new QWidget();
    panelLayout = new QVBoxLayout(sidePanel);

    // Example widgets for the side panel
    QLabel* label = new QLabel("Model Controls", sidePanel);
    loadButton = new QPushButton("Load Model", sidePanel);

    //QSlider* rotationSlider = new QSlider(Qt::Horizontal, sidePanel);
    //rotationSlider->setRange(0, 360);
    
	createBedDimensions();


    // Add the layout to the panelLayout

    slicerHeightInputBox = new QDoubleSpinBox(sidePanel);
    slicerHeightInputBox->setRange(-100.0, 100.0);
	slicerHeightInputBox->setSingleStep(widget->getSlicer()->getLayerHeight());
	slicerHeightInputBox->setValue(0.0);
	slicerHeightInputBox->setDisabled(true);

    sliceButton = new QPushButton("Slice Model", sidePanel);

    sliceWindow = new SliceWindow();
	createSlicingParameterWidgets();

    //connect(slicerHeightInputBox, &QDoubleSpinBox::valueChanged, widget, &ObjectRenderView::setSliderSlicerHeight);
    connect(slicerHeightInputBox, &QDoubleSpinBox::valueChanged, this, &MainWindow::changeSlicerHeight);
	connect(sliceButton, &QPushButton::clicked, this, &MainWindow::openSliceWindow);
	connect(loadButton, &QPushButton::clicked, this, &MainWindow::openLoadModelDialog);

    
    // Add widgets to the panel layout
    panelLayout->addWidget(label);
    panelLayout->addWidget(loadButton);
    panelLayout->addWidget(slicerHeightInputBox);
	panelLayout->addWidget(sliceButton);
    panelLayout->addWidget(gridWidget);
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

void MainWindow::changeSlicerHeight(double height) {
	SlicerPlane* slicer = widget->getSlicer();
	double layerHeight = slicer->getLayerHeight();
    
    //widget->setSliderSlicerHeight(height + layerHeight);
	widget->setSlicerHeight(height);
    if (allCompiledSlices.size() > height / layerHeight) {
        sliceWindow->setSLiceDataClipper(allCompiledSlices[(height / layerHeight)]);
	}
}

MainWindow::~MainWindow() {
	delete widget;
}

void MainWindow::changeLayerHeight(double layerHeight)
{
	//widget->getSlicer()->setLayerHeight(layerHeight);
	//slicerHeightInputBox->setSingleStep(layerHeight);
}

void MainWindow::openSliceWindow() {
 //   widget->getSlicer()->setLayerHeight(slicingParameterInputBoxes[0]->value());
 //   slicerHeightInputBox->setSingleStep(slicingParameterInputBoxes[0]->value());
 //   allCompiledSlices = widget->getAllSlices();
	//slicerHeightInputBox->setEnabled(true);

	//slicerHeightInputBox->setValue(widget->getSlicer()->getLayerHeight());
	//sliceWindow->setSLiceDataClipper(allCompiledSlices[0]);

 //   // Slices for GCode
	//GcodeCreator GCreator;
	//auto erodedSlices = GCreator.erodeSlicesForGCode(allCompiledSlices);
	//auto erodedSlicesWithShells = GCreator.addShells(erodedSlices, 2);

    widget->getSlicer()->setLayerHeight(slicingParameterInputBoxes[0]->value());
    slicerHeightInputBox->setSingleStep(slicingParameterInputBoxes[0]->value());
    allCompiledSlices = widget->getAllSlices();
    

    // Slices for GCode
    GcodeCreator GCreator;
    auto erodedSlices = GCreator.erodeSlicesForGCode(allCompiledSlices);
    auto erodedSlicesWithShells = GCreator.addShells(erodedSlices, 2);
    allCompiledSlices = erodedSlicesWithShells;

    slicerHeightInputBox->setEnabled(true);

    slicerHeightInputBox->setValue(widget->getSlicer()->getLayerHeight());
    sliceWindow->setSLiceDataClipper(allCompiledSlices[0]);


    
    
	//auto orderedLineSegments = widget->sliceMesh();
	//sliceWindow->setSliceData(orderedLineSegments);

 //   SlicerPlane* slicer = widget->getSlicer();
	//slicer->setContours(orderedLineSegments);
	//Clipper2Lib::PathsD polygons = slicer->compilePolygons();

	/*auto orderedLineSegments = widget->sliceMesh();
    GcodeCreator GCreator;
    GCreator.generateGCode(orderedLineSegments, "Test");
	sliceWindow->setSliceData(orderedLineSegments);
    SlicerPlane* slicer = widget->getSlicer();
	slicer->setContours(orderedLineSegments);*/
	//Clipper2Lib::PathsD polygons = slicer->compilePolygons();
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

void MainWindow::createSlicingParameterWidgets()
{
    gridWidget = new QWidget;
    QGridLayout* gridLayout = new QGridLayout(gridWidget);
    std::vector<QLabel*> labels;
    
    labels.push_back(new QLabel(QString("Layer height").arg(1)));
    labels.push_back(new QLabel(QString("Number of shells").arg(2)));
    labels.push_back(new QLabel(QString("Nozzle diameter").arg(3)));
    labels.push_back(new QLabel(QString("Printing temperature").arg(4)));
    labels.push_back(new QLabel(QString("Printing speed").arg(5)));
    labels.push_back(new QLabel(QString("Infill density").arg(6)));
    labels.push_back(new QLabel(QString("Enable/disable supports").arg(7)));

    for (int row = 0; row < 7; ++row) {
		slicingParameterInputBoxes.push_back(new QDoubleSpinBox);
        gridLayout->addWidget(labels[row], row, 0);
        gridLayout->addWidget(slicingParameterInputBoxes[row], row, 1);
    }

	slicingParameterInputBoxes[0]->setValue(widget->getSlicer()->getLayerHeight());
    connect(slicingParameterInputBoxes[0], &QDoubleSpinBox::valueChanged, this, &MainWindow::changeLayerHeight);
    
    gridWidget->setLayout(gridLayout);
}

void MainWindow::createBedDimensions() {
    // Create a vertical layout for the main bed dimensions section
    QVBoxLayout* bedDimensionsMainLayout = new QVBoxLayout();

    // Add the label for the bed dimensions
    QLabel* bedLabel = new QLabel("Bed Dimensions (Width x Depth):", sidePanel);
    bedDimensionsMainLayout->addWidget(bedLabel);

    // Create a horizontal layout for the width and depth inputs
    QHBoxLayout* bedDimensionsLayout = new QHBoxLayout();

    // Create the Width input
    QLineEdit* bedWidthInput = new QLineEdit(sidePanel);
    bedWidthInput->setPlaceholderText("Width");
    bedWidthInput->setText("180.0");
    bedWidthInput->setValidator(new QDoubleValidator(0.0, 1000.0, 2, sidePanel));  // Restrict input to valid double
    bedWidthInput->setMinimumWidth(80);  // Make the input wider
    bedDimensionsLayout->addWidget(bedWidthInput);

    // Add the "mm" label for Width
    QLabel* widthUnitLabel = new QLabel("mm", sidePanel);
    bedDimensionsLayout->addWidget(widthUnitLabel);

    // Add the "x" label between Width and Depth
    QLabel* xLabel = new QLabel("x", sidePanel);
    bedDimensionsLayout->addWidget(xLabel);

    // Create the Depth input
    QLineEdit* bedDepthInput = new QLineEdit(sidePanel);
    bedDepthInput->setPlaceholderText("Depth");
    bedDepthInput->setText("180.0");
    bedDepthInput->setValidator(new QDoubleValidator(0.0, 1000.0, 2, sidePanel));  // Restrict input to valid double
    bedDepthInput->setMinimumWidth(80);  // Make the input wider
    bedDimensionsLayout->addWidget(bedDepthInput);

    // Add the "mm" label for Depth
    QLabel* depthUnitLabel = new QLabel("mm", sidePanel);
    bedDimensionsLayout->addWidget(depthUnitLabel);

    // Add the layout containing the width and depth to the main layout
    bedDimensionsMainLayout->addLayout(bedDimensionsLayout);

    // Add the final layout to the panelLayout (assumed to be a member of MainWindow)
    panelLayout->addLayout(bedDimensionsMainLayout);
}