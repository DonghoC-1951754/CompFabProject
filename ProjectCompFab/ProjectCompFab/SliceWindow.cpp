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
    setFixedSize(700, 500);  // Adjust size as needed
}

void SliceWindow::setSliceData(const std::vector<glm::vec3>& vertices) {
	//sliceRenderWidget->setSliceData(vertices);
}

SliceWindow::~SliceWindow() {}