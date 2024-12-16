#include "SliceWindow.h"
#include <QVBoxLayout>
#include "SliceRenderView.h"



SliceWindow::SliceWindow(QWidget* parent)
    : QWidget(parent) {

    // Set up the layout
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Create the OpenGL widget
    sliceRenderWidget = new SliceRenderView();
    sliceRenderWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Add the OpenGL widget to the layout
    layout->addWidget(sliceRenderWidget);

    // Set the layout for the OpenGL window
    setLayout(layout);

    // Set a fixed size for the window (optional)
    setFixedSize(300, 300);  // Adjust size as needed
}

void SliceWindow::setSliceData(const std::vector< std::vector<std::vector<glm::dvec3>>> lineSegments) {
	sliceRenderWidget->setSliceData(lineSegments);
    sliceRenderWidget->update();
}

void SliceWindow::setSLiceDataClipper(Clipper2Lib::PathsD polygons)
{
	sliceRenderWidget->setSliceDataClipper(polygons);
}

void SliceWindow::setSliceInfill(Clipper2Lib::PathsD infill)
{
	sliceRenderWidget->setSliceInfill(infill);
}

void SliceWindow::setSliceShells(std::vector<Clipper2Lib::PathsD> shells)
{
	sliceRenderWidget->setSliceShells(shells);
}

void SliceWindow::setSliceFloorInfill(std::vector<Clipper2Lib::PathsD> floors)
{
	sliceRenderWidget->setSliceFloorInfill(floors);
}

void SliceWindow::setSliceRoofInfill(std::vector<Clipper2Lib::PathsD> roofs)
{
    sliceRenderWidget->setSliceRoofInfill(roofs);
}

SliceWindow::~SliceWindow() {}