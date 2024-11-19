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
    QLabel* label = new QLabel("Slicer Controls", sidePanel);
    loadButton = new QPushButton("Load Model", sidePanel);

    //QSlider* rotationSlider = new QSlider(Qt::Horizontal, sidePanel);
    //rotationSlider->setRange(0, 360);
    
	createBedDimensions();


    // Add the layout to the panelLayout

    slicerHeightInputBox = new QDoubleSpinBox(sidePanel);
    slicerHeightInputBox->setRange(-100.0, 100.0);
	slicerHeightInputBox->setSingleStep(widget->getSlicer()->getLayerHeight());
	slicerHeightInputBox->setValue(0.2);
	slicerHeightInputBox->setDisabled(true);
	widget->setSlicerHeight(0.2);

    sliceButton = new QPushButton("Slice Model", sidePanel);
	progressBar = new QProgressBar(sidePanel);
	progressBar->setRange(0, 100);
	progressBar->setValue(0);

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
	panelLayout->addWidget(progressBar);
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
    if (erodedSlices.size() > height / layerHeight) {
		drawCompleteSlice(height / layerHeight);
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
    widget->getSlicer()->setLayerHeight(slicingParameterInputBoxes[0]->value());
    slicerHeightInputBox->setSingleStep(slicingParameterInputBoxes[0]->value());
    auto allCompiledSlices = widget->getAllSlices();
    
    // Slices for GCode
    gcodeCreator = new GcodeCreator();
    erodedSlices = gcodeCreator->erodeSlicesForGCode(allCompiledSlices, slicingParameterInputBoxes[2]->value());
    progressBar->setValue(progressBar->value() + 40);
    erodedSlicesWithShells = gcodeCreator->addShells(erodedSlices, slicingParameterInputBoxes[1]->value(), slicingParameterInputBoxes[2]->value());
    progressBar->setValue(progressBar->value() + 30);
	mostInnerShells = gcodeCreator->getMostInnerShells();
    infill = gcodeCreator->generateInfill(mostInnerShells, erodedSlices);
	
    // GUI Controls
    double maxSlicerHeight = allCompiledSlices.size() * widget->getSlicer()->getLayerHeight();
	double minSlicerHeight = widget->getSlicer()->getLayerHeight();
    if (minSlicerHeight <= maxSlicerHeight) {
        slicerHeightInputBox->setEnabled(true);
        slicerHeightInputBox->setValue(widget->getSlicer()->getLayerHeight());
        slicerHeightInputBox->setRange(minSlicerHeight, maxSlicerHeight);
		drawCompleteSlice(0);
        progressBar->setValue(progressBar->value() + 30);
    }
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
        widget->getSlicer()->setLayerHeight(0.2);
        slicingParameterInputBoxes[0]->setValue(0.2);
        slicerHeightInputBox->setValue(0.2);
		widget->setSlicerHeight(0.2);

        slicerHeightInputBox->setDisabled(true);
		progressBar->setValue(0);
    }
    else {
        qDebug() << "Empty filepath!";
    }
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

    // Layer height controls
	slicingParameterInputBoxes[0]->setValue(widget->getSlicer()->getLayerHeight());
	slicingParameterInputBoxes[0]->setRange(0.10, 1.0);
	slicingParameterInputBoxes[0]->setSingleStep(0.02);

    // Shell amount controls
	slicingParameterInputBoxes[1]->setDecimals(0);
	slicingParameterInputBoxes[1]->setSingleStep(1);

	// Nozzle diameter controls
	slicingParameterInputBoxes[2]->setDecimals(2);
	slicingParameterInputBoxes[2]->setRange(0.2, 1.0);
	slicingParameterInputBoxes[2]->setSingleStep(0.2);

    connect(slicingParameterInputBoxes[0], &QDoubleSpinBox::valueChanged, this, &MainWindow::changeLayerHeight);
    
    gridWidget->setLayout(gridLayout);
}

void MainWindow::createBedDimensions() {
    bedDimensionsMainLayout = new QVBoxLayout();

    bedLabel = new QLabel("Bed Dimensions (Width x Depth):", sidePanel);
    bedDimensionsMainLayout->addWidget(bedLabel);

    bedDimensionsLayout = new QHBoxLayout();

    bedWidthInput = new QLineEdit(sidePanel);
    bedWidthInput->setPlaceholderText("Width");
    bedWidthInput->setText("180.0");
    bedWidthInput->setValidator(new QDoubleValidator(0.0, 1000.0, 2, sidePanel));
    bedWidthInput->setMinimumWidth(80);
    bedDimensionsLayout->addWidget(bedWidthInput);

    widthUnitLabel = new QLabel("mm", sidePanel);
    bedDimensionsLayout->addWidget(widthUnitLabel);

    xLabel = new QLabel("x", sidePanel);
    bedDimensionsLayout->addWidget(xLabel);

    bedDepthInput = new QLineEdit(sidePanel);
    bedDepthInput->setPlaceholderText("Depth");
    bedDepthInput->setText("180.0");
    bedDepthInput->setValidator(new QDoubleValidator(0.0, 1000.0, 2, sidePanel));
    bedDepthInput->setMinimumWidth(80);
    bedDimensionsLayout->addWidget(bedDepthInput);



    setDimLayout = new QHBoxLayout();
    setDimButton = new QPushButton("Dimensions are: " + bedWidthInput->text() + " x " + bedDepthInput->text(), sidePanel);
	setDimButton->setDisabled(true);
	setDimLayout->addWidget(setDimButton);
    connect(setDimButton, &QPushButton::clicked, this, &MainWindow::setBedDimensions);
    connect(setDimButton, &QPushButton::clicked, this, &MainWindow::updateBedText);
	connect(bedWidthInput, &QLineEdit::textChanged, this, &MainWindow::updateBedText);
	connect(bedDepthInput, &QLineEdit::textChanged, this, &MainWindow::updateBedText);

    depthUnitLabel = new QLabel("mm", sidePanel);
    bedDimensionsLayout->addWidget(depthUnitLabel);

    bedDimensionsMainLayout->addLayout(bedDimensionsLayout);
	bedDimensionsMainLayout->addLayout(setDimLayout);

    panelLayout->addLayout(bedDimensionsMainLayout);
}

void MainWindow::drawCompleteSlice(int index)
{
    sliceWindow->setSLiceDataClipper(erodedSlices[index]);
    sliceWindow->setSliceShells(erodedSlicesWithShells[index]);
    sliceWindow->setSliceInfill(infill[index]);
}

void MainWindow::setBedDimensions() {
	double width = bedWidthInput->text().toDouble();
	double depth = bedDepthInput->text().toDouble();
	widget->setPlateWidth(width);
	widget->setPlateDepth(depth);
}

void MainWindow::updateBedText() {
    double width = bedWidthInput->text().toDouble();
    double depth = bedDepthInput->text().toDouble();
    setDimButton->setDisabled(true);
    setDimButton->setText("Dimensions are: " + QString::number(width) + " x " + QString::number(depth));
    if (!(widget->getPlateWidth() == width && widget->getPlateDepth() == depth)) {
        setDimButton->setEnabled(true);
        setDimButton->setText("Set dimensions");
	}
}