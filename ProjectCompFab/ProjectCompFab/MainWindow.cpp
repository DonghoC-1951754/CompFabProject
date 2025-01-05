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
#include <QFormLayout>
#include <QMessageBox>

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

	bedWidth = 180;
	bedDepth = 180;
    createObjectRenderView();
    createBedDimensions();
    createSlicerHeightInput();
    createProgressBar();
    sliceButton = new QPushButton("Slice Model", sidePanel);
    sliceWindow = new SliceWindow();
    createSlicingParameterWidgets();

    gcodeButton = new QPushButton("Create GCode", sidePanel);
    gcodeButton->setDisabled(true);

    QHBoxLayout* sliceWindowHeaderLayout = new QHBoxLayout();
    QLabel* sliceWindowLabel = new QLabel("Slice Window", sidePanel);
    QPushButton* colorGuideButton = new QPushButton("Color Guide", sidePanel);

    // Add the label and button to the horizontal layout
    sliceWindowHeaderLayout->addWidget(sliceWindowLabel);
    sliceWindowHeaderLayout->addWidget(colorGuideButton);



    connect(slicerHeightInputBox, &QDoubleSpinBox::valueChanged, this, &MainWindow::changeSlicerHeight);
    connect(sliceButton, &QPushButton::clicked, this, &MainWindow::sliceModel);
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::openLoadModelDialog);
    connect(gcodeButton, &QPushButton::clicked, this, &MainWindow::openGCodeDialog);
    connect(slicingParameterInputBoxes[2], &QDoubleSpinBox::valueChanged, this, &MainWindow::limitInfillDensity);
    connect(colorGuideButton, &QPushButton::clicked, this, &MainWindow::showColorGuide);

    // Add widgets to the panel layout
    panelLayout->addWidget(label);
    panelLayout->addWidget(loadButton);
    panelLayout->addWidget(slicerHeightInputBox);
    panelLayout->addWidget(sliceButton);
    panelLayout->addWidget(progressBar);
    panelLayout->addWidget(progressText);
    panelLayout->addSpacing(15);
    panelLayout->addWidget(gcodeButton);
    panelLayout->addWidget(gridWidget);
    panelLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
    panelLayout->addLayout(sliceWindowHeaderLayout);
    panelLayout->addWidget(sliceWindow, Qt::AlignBottom);

    splitter->addWidget(widget);
    splitter->addWidget(slider);
    splitter->addWidget(sidePanel);

    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(splitter);

    sliceOperations = new SliceOperations();
}

void MainWindow::changeSlicerHeight(double height) {
    SlicerPlane* slicer = widget->getSlicer();
    double layerHeight = slicer->getLayerHeight();

    widget->setSlicerHeight(height);
    int stepNumber = std::round(height / layerHeight) - 1;
    qDebug() << "Step number: " << stepNumber << "height:" << height << "layerHeight:" << layerHeight;
    qDebug() << "Calc: " << (height / layerHeight) - 1;
    if (erodedSlices.size() > stepNumber) {
        slider->setValue(height * 100);
        drawCompleteSlice(stepNumber);
    }
}

void MainWindow::showColorGuide() {
    QMessageBox colorGuideBox;
    colorGuideBox.setWindowTitle("Color Guide");
    colorGuideBox.setText(
        "<p><span style='color:rgb(255, 255, 255);'>Black:</span> Perimeters</p>"
        "<p><span style='color:rgb(0, 255, 0);'>Green:</span> Shells</p>"
        "<p><span style='color:rgb(0, 0, 255);'>Blue:</span> Infill</p>"
        "<p><span style='color:rgb(255, 0, 0);'>Red:</span> Floors</p>"
        "<p><span style='color:rgb(255, 0, 255);'>Purple:</span> Roofs</p>"
        "<p><span style='color:rgb(128, 204, 255);'>Light Blue:</span> Support Perimeters and Support Infill</p>"
    );
    colorGuideBox.setIcon(QMessageBox::Information);
    colorGuideBox.exec();
}


MainWindow::~MainWindow() {
    delete widget;
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

void MainWindow::sliderValueChanged(int value)
{
    int stepSize = slider->singleStep();
    if (value % stepSize != 0) {
        if ((value % stepSize) < (stepSize / 2)) {
            slider->setValue(value - (value % stepSize));
            slicerHeightInputBox->setValue(slider->value() / 100.0);
        }
        else {
            slider->setValue(value + (stepSize - (value % stepSize)));
            slicerHeightInputBox->setValue(slider->value() / 100.0);
        }
    }
    else {
        slicerHeightInputBox->setValue(slider->value() / 100.0);
    }

}

void MainWindow::openLoadModelDialog() {
    QFileDialog dialog(this);
    dialog.setDirectory("./resources");
    QString filePath = dialog.getOpenFileName(this, "Open File", "", "STL Files (*.stl)");
    if (!filePath.isEmpty()) {
		modelFilePath = filePath.toStdString();
        loadModel(filePath.toStdString());
    }
    else {
        qDebug() << "Empty filepath!";
    }
}

void MainWindow::openGCodeDialog() {
    // Create the dialog window
    QDialog* gcodeDialog = new QDialog(this);
    gcodeDialog->setWindowTitle("GCode Settings");

    QVBoxLayout* dialogLayout = new QVBoxLayout(gcodeDialog);

    // Filename input
    filenameInput = new QLineEdit(gcodeDialog);
    filenameInput->setPlaceholderText("Enter GCode filename");
    dialogLayout->addWidget(new QLabel("Filename:"));
    dialogLayout->addWidget(filenameInput);

    // Create form layout for slicing parameters
    QFormLayout* formLayout = new QFormLayout();
    printbedTempLabel = new QLabel("Printbed Temperature:");
    nozzleTempLabel = new QLabel("Nozzle Temperature:");
    speedMultiplierLabel = new QLabel("Speed Multiplier (F3000):");
	retractionLabel = new QLabel("Enable Retraction:");
	primeLabel = new QLabel("Enable Priming:");
	speedCalcLabel = new QLabel("Short Segment Speed Reduction:");

    // Printbed temperature input
    printbedTempInput = new QDoubleSpinBox(gcodeDialog);
    printbedTempInput->setDecimals(0);
    printbedTempInput->setRange(20, 150);
    printbedTempInput->setSingleStep(1);
    printbedTempInput->setValue(60);
    formLayout->addRow(printbedTempLabel, printbedTempInput);

    // Nozzle temperature input
    nozzleTempInput = new QDoubleSpinBox(gcodeDialog);
    nozzleTempInput->setDecimals(0);
    nozzleTempInput->setRange(25, 300);
    nozzleTempInput->setSingleStep(1);
    nozzleTempInput->setValue(200);
    formLayout->addRow(nozzleTempLabel, nozzleTempInput);

    // Speed multiplier input
    speedMultiplierInput = new QDoubleSpinBox(gcodeDialog);
    speedMultiplierInput->setDecimals(1);
    speedMultiplierInput->setRange(0.1, 5);
    speedMultiplierInput->setSingleStep(0.1);
    speedMultiplierInput->setValue(1);
    formLayout->addRow(speedMultiplierLabel, speedMultiplierInput);
    connect(speedMultiplierInput, &QDoubleSpinBox::valueChanged, this, &MainWindow::updateSpeedLabel);

	// Retraction checkbox
	enableRetraction = new QCheckBox(gcodeDialog);
	enableRetraction->setChecked(true);
	formLayout->addRow(retractionLabel, enableRetraction);

	// Prime checkbox
	enablePrime = new QCheckBox(gcodeDialog);
	enablePrime->setChecked(true);
	formLayout->addRow(primeLabel, enablePrime);

    // Speed reduction checkbox
    enableSpeedCalc = new QCheckBox(gcodeDialog);
    enableSpeedCalc->setChecked(false);
    formLayout->addRow(speedCalcLabel, enableSpeedCalc);

    dialogLayout->addLayout(formLayout);

    // Add "Generate" and "Cancel" buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* generateButton = new QPushButton("Generate", gcodeDialog);
    QPushButton* cancelButton = new QPushButton("Cancel", gcodeDialog);
    buttonLayout->addWidget(generateButton);
    buttonLayout->addWidget(cancelButton);
    dialogLayout->addLayout(buttonLayout);

    // Connect buttons
    connect(generateButton, &QPushButton::clicked, [=]() {
        QString filename = filenameInput->text();
        if (filename.isEmpty()) {
            QMessageBox::warning(gcodeDialog, "Error", "Please enter a filename.");
            return;
        }

        // Get the values from the input boxes
        gCodeFileName = filename;
        printBedTemp = printbedTempInput->value();
        nozzleTemp = nozzleTempInput->value();
        speedMultiplier = speedMultiplierInput->value();
		retractionToggle = enableRetraction->isChecked();
		primeToggle = enablePrime->isChecked();
		speedToggle = enableSpeedCalc->isChecked();

        // Call GCode generation function
        generateGcode();

        gcodeDialog->accept();
        });

    connect(cancelButton, &QPushButton::clicked, gcodeDialog, &QDialog::reject);

    // Show the dialog
    gcodeDialog->exec();
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
    progressText->setText("Progress: not sliced yet...");
}

void MainWindow::createSlicingParameterWidgets()
{
    gridWidget = new QWidget;
    QGridLayout* gridLayout = new QGridLayout(gridWidget);

    labels.push_back(new QLabel(QString("Layer height").arg(1)));
    labels.push_back(new QLabel(QString("Number of shells").arg(2)));
    labels.push_back(new QLabel(QString("Nozzle diameter").arg(3)));
    labels.push_back(new QLabel(QString("Infill density (%)").arg(4)));
    labels.push_back(new QLabel(QString("Enable supports").arg(5)));
    labels.push_back(new QLabel(QString("Floor amount").arg(6)));
    labels.push_back(new QLabel(QString("Roof amount").arg(7)));
    labels.push_back(new QLabel(QString("filament diameter").arg(8)));

    for (int row = 0; row < labels.size(); ++row) {
        slicingParameterInputBoxes.push_back(new QDoubleSpinBox);
        if (row != 4) {
            gridLayout->addWidget(labels[row], row, 0);
            gridLayout->addWidget(slicingParameterInputBoxes[row], row, 1);
        }
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

    // Infill density controls
    slicingParameterInputBoxes[3]->setDecimals(1);
    slicingParameterInputBoxes[3]->setValue(10.0);
    slicingParameterInputBoxes[3]->setRange(0.0, 100.0);
    slicingParameterInputBoxes[3]->setSingleStep(1);

    // Floor controls
    slicingParameterInputBoxes[5]->setDecimals(0);
    slicingParameterInputBoxes[5]->setSingleStep(1);
    slicingParameterInputBoxes[5]->setValue(2);

    // Roof controls
    slicingParameterInputBoxes[6]->setDecimals(0);
    slicingParameterInputBoxes[6]->setSingleStep(1);
    slicingParameterInputBoxes[6]->setValue(2);

    // Filament diameter controls
    slicingParameterInputBoxes[7]->setDecimals(2);
    slicingParameterInputBoxes[7]->setRange(0, 5.0);
    slicingParameterInputBoxes[7]->setSingleStep(0.05);
    slicingParameterInputBoxes[7]->setValue(1.75);

    gridLayout->addWidget(labels[4], 8, 0);
    enableSupport = new QCheckBox();
    gridLayout->addWidget(enableSupport, 8, 1);


    gridWidget->setLayout(gridLayout);
}

void MainWindow::createBedDimensions() {
    bedDimensionsMainLayout = new QVBoxLayout();

    bedLabel = new QLabel("Bed Dimensions (Width x Depth):", sidePanel);
    bedDimensionsMainLayout->addWidget(bedLabel);

    bedDimensionsLayout = new QHBoxLayout();

    bedWidthInput = new QDoubleSpinBox(sidePanel);
    bedWidthInput->setDecimals(0);
    bedWidthInput->setRange(50, 1000);
    bedWidthInput->setSingleStep(1);
    bedWidthInput->setValue(180);
    bedWidthInput->setMinimumWidth(80);
    bedDimensionsLayout->addWidget(bedWidthInput);

    widthUnitLabel = new QLabel("mm", sidePanel);
    bedDimensionsLayout->addWidget(widthUnitLabel);

    xLabel = new QLabel("x", sidePanel);
    bedDimensionsLayout->addWidget(xLabel);

    bedDepthInput = new QDoubleSpinBox(sidePanel);
    bedDepthInput->setDecimals(0);
    bedDepthInput->setRange(50, 1000);
    bedDepthInput->setSingleStep(1);
    bedDepthInput->setValue(180);
    bedDepthInput->setMinimumWidth(80);
    bedDimensionsLayout->addWidget(bedDepthInput);



    setDimLayout = new QHBoxLayout();
    setDimButton = new QPushButton("Dimensions are: " + bedWidthInput->text() + " x " + bedDepthInput->text(), sidePanel);
    setDimButton->setDisabled(true);
    setDimLayout->addWidget(setDimButton);
    connect(setDimButton, &QPushButton::clicked, this, &MainWindow::setBedDimensions);
    connect(setDimButton, &QPushButton::clicked, this, &MainWindow::updateBedText);
    connect(bedWidthInput, &QDoubleSpinBox::textChanged, this, &MainWindow::updateBedText);
    connect(bedDepthInput, &QDoubleSpinBox::textChanged, this, &MainWindow::updateBedText);



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
    progressText = new QLabel("Progress: not sliced yet...");
}

void MainWindow::drawCompleteSlice(int index)
{
    sliceWindow->setSLiceDataClipper(erodedSlices[index]);
    sliceWindow->setSliceShells(shells[index]);
    sliceWindow->setSliceInfill(infill[index]);
    sliceWindow->setSliceFloorInfill(floors[index]);
    sliceWindow->setSliceRoofInfill(roofs[index]);
    if (enableSupport->isChecked()) {
        sliceWindow->setBasicSupportPerimeter(erodedSupportPerimeter[index]);
        sliceWindow->setBasicSupportInfill(supportInfill[index]);
    }
}

void MainWindow::calculateSlices()
{
    // Layer height controls slicingParameterInputBoxes[0]
    // Shell amount controls slicingParameterInputBoxes[1]
    // Nozzle diameter controls slicingParameterInputBoxes[2]
    // Infill density controls slicingParameterInputBoxes[3]
    // Floor controls slicingParameterInputBoxes[5]
    // Roof controls slicingParameterInputBoxes[6]
    // Filament diameter controls slicingParameterInputBoxes[7]
    progressText->setText("Progress: calculating perimeters...");
    auto allCompiledSlices = widget->getAllSlices();
    sliceAmount = allCompiledSlices.size();
    progressBar->setValue(progressBar->value() + 10);
    // Contour
    progressText->setText("Progress: eroding perimeters...");
    erodedSlices = sliceOperations->erodeSlicesForGCode(allCompiledSlices, slicingParameterInputBoxes[2]->value());
    progressBar->setValue(progressBar->value() + 10);
    // Shell
    progressText->setText("Progress: generating shells...");
    shells = sliceOperations->addShells(erodedSlices, slicingParameterInputBoxes[1]->value(), slicingParameterInputBoxes[2]->value());


    // Floor
    progressText->setText("Progress: generating floors and roofs...");
    progressBar->setValue(progressBar->value() + 20);
    floors = sliceOperations->generateRoofsAndFloorsInfill(erodedSlices, slicingParameterInputBoxes[5]->value(), true, slicingParameterInputBoxes[2]->value());
    // Roof
    roofs = sliceOperations->generateRoofsAndFloorsInfill(erodedSlices, slicingParameterInputBoxes[6]->value(), false, slicingParameterInputBoxes[2]->value());

    // Infill
    progressText->setText("Progress: generating sparse infill...");
    progressBar->setValue(progressBar->value() + 20);
    mostInnerShells = sliceOperations->getMostInnerShells();
    infill = sliceOperations->generateInfill(mostInnerShells, erodedSlices, slicingParameterInputBoxes[3]->value(), slicingParameterInputBoxes[2]->value());

    // Support
    progressText->setText("Progress: generating support...");
    progressBar->setValue(progressBar->value() + 20);
    if (enableSupport->isChecked()) {
        erodedSupportPerimeter = sliceOperations->generateErodedSupportPerimeter(allCompiledSlices, slicingParameterInputBoxes[2]->value(), widget->getSlicer()->getLayerHeight());
        supportInfill = sliceOperations->generateBasicSupportInfill(erodedSupportPerimeter, slicingParameterInputBoxes[3]->value(), slicingParameterInputBoxes[2]->value());
    }


    // Slicing done
    progressText->setText("Progress: done!");
    progressBar->setValue(progressBar->value() + 20);

    // Draw the first complete slice (contour + shells + infill)
    double maxSlicerHeight = allCompiledSlices.size() * widget->getSlicer()->getLayerHeight();
    double minSlicerHeight = widget->getSlicer()->getLayerHeight();
    if (minSlicerHeight <= maxSlicerHeight) {
        slicerHeightInputBox->setEnabled(true);
        slicerHeightInputBox->setValue(widget->getSlicer()->getLayerHeight());
        slicerHeightInputBox->setRange(minSlicerHeight, maxSlicerHeight);

        // Slider only accepts ints (so multiply by 100)
        slider->setRange(minSlicerHeight * 100, maxSlicerHeight * 100);
        slider->setValue(widget->getSlicer()->getLayerHeight() * 100);
        //slider->setTickInterval(slicingParameterInputBoxes[0]->value() * 100);
        slider->setSingleStep(slicingParameterInputBoxes[0]->value() * 100);
        slider->setPageStep(slicingParameterInputBoxes[0]->value() * 100);
        connect(slider, &QSlider::valueChanged, this, &MainWindow::sliderValueChanged);


        slider->setEnabled(true);
        drawCompleteSlice(0);
    }
}

void MainWindow::setBedDimensions() {
    bedWidth = bedWidthInput->text().toDouble();
    bedDepth = bedDepthInput->text().toDouble();
    widget->setPlateWidth(bedWidth);
    widget->setPlateDepth(bedDepth);
    loadModel(modelFilePath);
}

void MainWindow::updateSpeedLabel() {
    float speed = speedMultiplierInput->value();
    speedMultiplierLabel->setText("Speed Multiplier (F" + QString::number(speed * 3000) + ")");
    qDebug() << "Speed: " << speed;
}

void MainWindow::updateBedText() {
	double bedWidthCurrentInput = bedWidthInput->text().toDouble();
	double bedDepthCurrentInput = bedDepthInput->text().toDouble();
    setDimButton->setDisabled(true);
    setDimButton->setText("Dimensions are: " + QString::number(bedWidth) + " x " + QString::number(bedDepth));
    if (!(widget->getPlateWidth() == bedWidthCurrentInput && widget->getPlateDepth() == bedDepthCurrentInput)) {
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

    gcodeCreator = new GcodeCreator(maxXDistance, maxYDistance, bedWidth, bedDepth, sliceAmount, slicingParameterInputBoxes[7]->value(),
        printBedTemp, nozzleTemp, slicingParameterInputBoxes[2]->value(), speedMultiplier,
		widget->getSlicer()->getLayerHeight(), primeToggle, enableSupport->isChecked(), retractionToggle, speedToggle,
        erodedSlices, shells, infill, floors, roofs, erodedSupportPerimeter, supportInfill);

    gcodeCreator->generateGCode(gCodeFileName.toStdString());

    delete gcodeCreator;
}

void MainWindow::limitInfillDensity()
{
    slicingParameterInputBoxes[3]->setRange(slicingParameterInputBoxes[2]->value(), 100.0);
}
