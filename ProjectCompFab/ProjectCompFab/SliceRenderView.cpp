#include "SliceRenderView.h"

SliceRenderView::SliceRenderView(QWidget* parent) : QOpenGLWidget(parent) {
}

void SliceRenderView::initializeGL() {
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void SliceRenderView::resizeGL(int w, int h) {
	glViewport(0, 0, w, h);

	float aspectRatio = static_cast<float>(w) / h;
	float zoomedWidth = 50 * zoomLevel * aspectRatio;
	float zoomedHeight = 50 * zoomLevel;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-zoomedWidth, zoomedWidth, -zoomedHeight, zoomedHeight, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void SliceRenderView::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();
	glScalef(zoomLevel, zoomLevel, 1.0f);
	glTranslatef(panOffset.x, panOffset.y, 0.0f);

	glColor3f(1.0f, 0.0f, 0.0f);

	for (const auto& polygon : polygons2D) {
		glBegin(GL_LINE_STRIP);
		for (const auto& vertex : polygon) {
			glVertex2f(vertex.x, vertex.y);
		}
		glEnd();
	}

	glPopMatrix();
}

void SliceRenderView::setSliceData(const std::vector< std::vector<std::vector<glm::dvec3>>> polygons) {
	polygons2D.clear();
	for (const auto& polygon : polygons) {
		std::vector<std::vector<glm::vec2>> tempLineSegments2D;
		std::vector<glm::vec2> tempflattenedVertices;
		for (const auto& line : polygon) {
			std::vector<glm::vec2> line2D;
			for (const auto& point : line) {
				line2D.push_back(glm::vec2(point.x, point.z));
			}
			tempLineSegments2D.push_back(line2D);
		}
		for (const auto& line : tempLineSegments2D) {
			tempflattenedVertices.insert(tempflattenedVertices.end(), line.begin(), line.end());
		}
		polygons2D.push_back(tempflattenedVertices);
	}
}

void SliceRenderView::mousePressEvent(QMouseEvent* event) {
	lastMousePos = event->pos();
	if (event->button() == Qt::MiddleButton) {
		panning = true;
		setCursor(Qt::ClosedHandCursor);
	}
}

void SliceRenderView::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::MiddleButton) {
		panning = false;
		setCursor(Qt::ArrowCursor);
	}
}

void SliceRenderView::mouseMoveEvent(QMouseEvent* event) {
	if (panning) {
		QPoint delta = event->pos() - lastMousePos;
		lastMousePos = event->pos();

		panOffset += glm::vec2(delta.x() * panSpeed, delta.y() * panSpeed);

		update();
	}
}

void SliceRenderView::wheelEvent(QWheelEvent* event) {
	int delta = event->angleDelta().y();

	zoomLevel += delta * zoomSpeed / 120;
	zoomLevel = qBound(minZoom, zoomLevel, maxZoom);

	update();
}

SliceRenderView::~SliceRenderView() {
}