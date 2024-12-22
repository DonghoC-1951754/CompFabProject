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
#include "clipper2/clipper.h"
#include "SliceOperations.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
	modelFilePath = "./resources/hole-test(easy).stl";
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    sidePanel = new QWidget();
    panelLayout = new QVBoxLayout(sidePanel);

    QLabel* label = new QLabel("Slicer Controls", sidePanel);
    loadButton = new QPushButton("Load Model", sidePanel);

    slider = new QSlider(Qt::Vertical, this);
    slider->setRange(0, 100); // Set the range of the slider (0-100)
    //slider->setTickPosition(QSlider::TicksLeft); // Optional: Add ticks to the slider
    //slider->setTickInterval(10);
	slider->setDisabled(true);

	gcodeCreator = new GcodeCreator();

	createObjectRenderView();
	createBedDimensions();
	createSlicerHeightInput();
    createProgressBar();
    sliceButton = new QPushButton("Slice Model", sidePanel);
    sliceWindow = new SliceWindow();
	createSlicingParameterWidgets();

    gcodeButton = new QPushButton("Create GCode", sidePanel);
	gcodeButton->setDisabled(true);

    connect(slicerHeightInputBox, &QDoubleSpinBox::valueChanged, this, &MainWindow::changeSlicerHeight);
	connect(sliceButton, &QPushButton::clicked, this, &MainWindow::sliceModel);
	connect(loadButton, &QPushButton::clicked, this, &MainWindow::openLoadModelDialog);
	connect(gcodeButton, &QPushButton::clicked, this, &MainWindow::generateGcode);
	connect(slicingParameterInputBoxes[2], &QDoubleSpinBox::valueChanged, this, &MainWindow::limitInfillDensity);
	connect(slicingParameterInputBoxes[5], &QDoubleSpinBox::valueChanged, this, &MainWindow::updateSpeedLabel);
    // Add widgets to the panel layout
    panelLayout->addWidget(label);
    panelLayout->addWidget(loadButton);
    panelLayout->addWidget(slicerHeightInputBox);
	panelLayout->addWidget(sliceButton);
	panelLayout->addWidget(progressBar);
	panelLayout->addWidget(gcodeButton);
    panelLayout->addWidget(gridWidget);
    panelLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	panelLayout->addWidget(sliceWindow, Qt::AlignBottom);

    splitter->addWidget(widget);
    splitter->addWidget(slider);
    splitter->addWidget(sidePanel);

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(splitter);

    gcodeCreator = new GcodeCreator();
	sliceOperations = new SliceOperations();
}

void MainWindow::changeSlicerHeight(double height) {
	SlicerPlane* slicer = widget->getSlicer();
	double layerHeight = slicer->getLayerHeight();
    
    widget->setSlicerHeight(height);
	int stepNumber = std::round(height / layerHeight)-1;
	qDebug() << "Step number: " << stepNumber << "height:" << height << "layerHeight:" << layerHeight;
    qDebug() << "Calc: " << (height / layerHeight) - 1;
    if (erodedSlices.size() > stepNumber) {
		drawCompleteSlice(stepNumber);
	}
}

MainWindow::~MainWindow() {
	delete widget;
	delete gcodeCreator;
}

void MainWindow::sliceModel() {
	SlicerPlane* slicer = widget->getSlicer();

	// slicingParameterInputBoxes[0] == layer height
    slicer->setLayerHeight(slicingParameterInputBoxes[0]->value());
    slicerHeightInputBox->setSingleStep(slicingParameterInputBoxes[0]->value());

    //slider->setTickInterval(slicingParameterInputBoxes[0]->value());
    
    progressBar->setValue(0);
    calculateSlices();

	gcodeButton->setDisabled(false);
}

void MainWindow::openLoadModelDialog() {
	QFileDialog dialog(this);
    dialog.setDirectory("./resources");
    QString filePath = dialog.getOpenFileName(this, "Open File", "", "STL Files (*.stl)");
    if (!filePath.isEmpty()) {
		loadModel(filePath.toStdString());
    }
    else {
        qDebug() << "Empty filepath!";
    }
}

void MainWindow::loadModel(std::string path) {
    widget->loadModel(path);
    widget->resetRendering();
    widget->getSlicer()->setLayerHeight(0.2);
    slicingParameterInputBoxes[0]->setValue(0.2);
    slicerHeightInputBox->setValue(0.2);
    widget->setSlicerHeight(0.2);

    slicerHeightInputBox->setDisabled(true);
	slider->setDisabled(true);
    progressBar->setValue(0);
}

void MainWindow::createSlicingParameterWidgets()
{
    gridWidget = new QWidget;
    QGridLayout* gridLayout = new QGridLayout(gridWidget);
    
    labels.push_back(new QLabel(QString("Layer height").arg(1)));
    labels.push_back(new QLabel(QString("Number of shells").arg(2)));
    labels.push_back(new QLabel(QString("Nozzle diameter").arg(3)));
    labels.push_back(new QLabel(QString("Printbed temperature").arg(4)));
    labels.push_back(new QLabel(QString("Nozzle temperature").arg(5)));
    labels.push_back(new QLabel(QString("Speed multiplier (F2200)").arg(6)));
    labels.push_back(new QLabel(QString("Infill density").arg(7)));
    labels.push_back(new QLabel(QString("Enable/disable supports").arg(8)));
    labels.push_back(new QLabel(QString("Floor amount").arg(9)));
    labels.push_back(new QLabel(QString("Roof amount").arg(10)));
	labels.push_back(new QLabel(QString("filament diameter").arg(11)));

    for (int row = 0; row < labels.size(); ++row) {
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
    slicingParameterInputBoxes[2]->setValue(0.4);
	
    // Printbed temperature controls
	slicingParameterInputBoxes[3]->setDecimals(0);
	slicingParameterInputBoxes[3]->setRange(0, 100);
	slicingParameterInputBoxes[3]->setSingleStep(1);
	slicingParameterInputBoxes[3]->setValue(60);

	// Nozzle temperature controls
	slicingParameterInputBoxes[4]->setDecimals(0);
	slicingParameterInputBoxes[4]->setRange(0, 300);
	slicingParameterInputBoxes[4]->setSingleStep(1);
	slicingParameterInputBoxes[4]->setValue(200);

	// Speed multiplier controls
	slicingParameterInputBoxes[5]->setDecimals(1);
	slicingParameterInputBoxes[5]->setRange(0, 5);
	slicingParameterInputBoxes[5]->setSingleStep(0.1);
	slicingParameterInputBoxes[5]->setValue(1.0);

	// Infill density controls
	slicingParameterInputBoxes[6]->setDecimals(2);
	slicingParameterInputBoxes[6]->setValue(2.0);
	slicingParameterInputBoxes[6]->setRange(0.2, 100.0);
	slicingParameterInputBoxes[6]->setSingleStep(0.2);

    // Floor controls
	slicingParameterInputBoxes[8]->setDecimals(0);
    slicingParameterInputBoxes[8]->setSingleStep(1);
    slicingParameterInputBoxes[8]->setValue(2);

	// Roof controls
	slicingParameterInputBoxes[9]->setDecimals(0);
	slicingParameterInputBoxes[9]->setSingleStep(1);
	slicingParameterInputBoxes[9]->setValue(2);

	// Filament diameter controls
	slicingParameterInputBoxes[10]->setDecimals(2);
	slicingParameterInputBoxes[10]->setRange(0, 5.0);
	slicingParameterInputBoxes[10]->setSingleStep(0.05);
	slicingParameterInputBoxes[10]->setValue(1.75);
    


    
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

void MainWindow::createSlicerHeightInput()
{
    slicerHeightInputBox = new QDoubleSpinBox(sidePanel);
    slicerHeightInputBox->setRange(-100.0, 100.0);
    slicerHeightInputBox->setSingleStep(widget->getSlicer()->getLayerHeight());
    slicerHeightInputBox->setValue(0.2);
    slicerHeightInputBox->setDisabled(true);
}

void MainWindow::createObjectRenderView()
{
    widget = new ObjectRenderView();
    widget->loadModel(modelFilePath);
    widget->setSlicerHeight(0.2);
}

void MainWindow::createProgressBar()
{
    progressBar = new QProgressBar(sidePanel);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
}

void MainWindow::drawCompleteSlice(int index)
{
    sliceWindow->setSLiceDataClipper(erodedSlices[index]);
    sliceWindow->setSliceShells(shells[index]);
    sliceWindow->setSliceInfill(infill[index]);
	sliceWindow->setSliceFloorInfill(floors[index]);
    sliceWindow->setSliceRoofInfill(roofs[index]);
    sliceWindow->setBasicSupportPerimeter(erodedSupportPerimeter[index]);
    sliceWindow->setBasicSupportInfill(supportInfill[index]);
}

void MainWindow::calculateSlices()
{
    // Layer height controls slicingParameterInputBoxes[0]
    // Shell amount controls slicingParameterInputBoxes[1]
    // Nozzle diameter controls slicingParameterInputBoxes[2]
    // Printbed temperature controls  slicingParameterInputBoxes[3]
    // Nozzle temperature controls slicingParameterInputBoxes[4]
    // Speed multiplier controls slicingParameterInputBoxes[5]
    // Infill density controls slicingParameterInputBoxes[6]
    // Floor controls slicingParameterInputBoxes[8]
    // Roof controls slicingParameterInputBoxes[9]
    // Filament diameter controls slicingParameterInputBoxes[10]
    auto allCompiledSlices = widget->getAllSlices();
	sliceAmount = allCompiledSlices.size();
    // Contour
    erodedSlices = sliceOperations->erodeSlicesForGCode(allCompiledSlices, slicingParameterInputBoxes[2]->value());
    progressBar->setValue(progressBar->value() + 60);
    // Shell
    shells = sliceOperations->addShells(erodedSlices, slicingParameterInputBoxes[1]->value(), slicingParameterInputBoxes[2]->value());
    progressBar->setValue(progressBar->value() + 30);

    // Floor
    floors = sliceOperations->generateRoofsAndFloorsInfill(erodedSlices, slicingParameterInputBoxes[8]->value(), true, slicingParameterInputBoxes[2]->value());
    // Roof
	roofs = sliceOperations->generateRoofsAndFloorsInfill(erodedSlices, slicingParameterInputBoxes[9]->value(), false, slicingParameterInputBoxes[2]->value());
    
    // Infill
    mostInnerShells = sliceOperations->getMostInnerShells();
    infill = sliceOperations->generateInfill(mostInnerShells, erodedSlices, slicingParameterInputBoxes[6]->value(), slicingParameterInputBoxes[2]->value());

    // Support
    erodedSupportPerimeter = sliceOperations->generateErodedSupportPerimeter(allCompiledSlices, slicingParameterInputBoxes[2]->value(), widget->getSlicer()->getLayerHeight());
    supportInfill = sliceOperations->generateInfill(std::vector<Clipper2Lib::PathsD>(), erodedSupportPerimeter, slicingParameterInputBoxes[6]->value(), slicingParameterInputBoxes[2]->value());

    // Draw the first complete slice (contour + shells + infill)
    double maxSlicerHeight = allCompiledSlices.size() * widget->getSlicer()->getLayerHeight();
    double minSlicerHeight = widget->getSlicer()->getLayerHeight();
    if (minSlicerHeight <= maxSlicerHeight) {
        slicerHeightInputBox->setEnabled(true);
        slicerHeightInputBox->setValue(widget->getSlicer()->getLayerHeight());
        slicerHeightInputBox->setRange(minSlicerHeight, maxSlicerHeight);
        
		// Slider only accepts ints (so multiply by 100)
        slider->setRange(minSlicerHeight*100, maxSlicerHeight*100);
        slider->setValue(widget->getSlicer()->getLayerHeight()*100);
		slider->setTickInterval(slicingParameterInputBoxes[0]->value() * 100);
        slider->setSingleStep(slicingParameterInputBoxes[0]->value()*100);
		slider->setTickPosition(QSlider::TicksBothSides);
        slider->setEnabled(true);
        drawCompleteSlice(0);
    }
    progressBar->setValue(progressBar->value() + 10);
}

void MainWindow::setBedDimensions() {
	double width = bedWidthInput->text().toDouble();
	double depth = bedDepthInput->text().toDouble();
	widget->setPlateWidth(width);
	widget->setPlateDepth(depth);
	loadModel(modelFilePath);
}

void MainWindow::updateSpeedLabel() {
	float speed = slicingParameterInputBoxes[5]->value();
	labels[5]->setText("Speed multiplier (F" + QString::number(speed* 2200) + ")");
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

void MainWindow::generateGcode()
{
	// erodedSlices == omtrek, Shells == shells, Infill == infill
    int sliceAmount = erodedSlices.size();
	double maxXDistance = static_cast<double>(widget->getMesh()->getMaxXDistance());
	double maxYDistance = static_cast<double>(widget->getMesh()->getMaxYDistance());
    // Layer height controls: slicingParameterInputBoxes[0]
    // Nozzle diameter controls: slicingParameterInputBoxes[2]
    // Printbed temperature controls: slicingParameterInputBoxes[3]
    // Nozzle temperature controls: slicingParameterInputBoxes[4]
    // Speed multiplier controls: slicingParameterInputBoxes[5]
    // Filament diameter controls: slicingParameterInputBoxes[10]

	gcodeCreator->generateGCode(maxXDistance, maxYDistance, sliceAmount, erodedSlices, shells, infill, floors, roofs, erodedSupportPerimeter, supportInfill,
        "test", widget->getSlicer()->getLayerHeight(), slicingParameterInputBoxes[10]->value(), slicingParameterInputBoxes[3]->value(), slicingParameterInputBoxes[4]->value(),
        slicingParameterInputBoxes[2]->value(), slicingParameterInputBoxes[5]->value(), true);
	
}

void MainWindow::limitInfillDensity()
{
	slicingParameterInputBoxes[6]->setRange(slicingParameterInputBoxes[2]->value(), 100.0);
}
