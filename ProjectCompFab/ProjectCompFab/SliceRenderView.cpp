#include "SliceRenderView.h"

SliceRenderView::SliceRenderView(QWidget* parent) : QOpenGLWidget(parent) {
}

void SliceRenderView::initializeGL() {
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void SliceRenderView::resizeGL(int w, int h) {
	glViewport(0, 0, w, h);
}

void SliceRenderView::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

SliceRenderView::~SliceRenderView() {
}