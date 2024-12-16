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
		glColor3f(1.0f, 0.0f, 0.5f);
		for (const auto& vertex : polygon) {
			glVertex2f(vertex.x, vertex.y);
		}
		glVertex2f(polygon[0].x, polygon[0].y);
		glEnd();
	}
	for (const auto& line : shells2D) {
		glBegin(GL_LINE_STRIP);
		glColor3f(0.4f, 1.0f, 0.0f);
		for (const auto& vertex : line) {
			glVertex2f(vertex.x, vertex.y);
		}
		glVertex2f(line[0].x, line[0].y);
		glEnd();
	}
	for (const auto& line : infill2D) {
		glBegin(GL_LINE_STRIP);
		glColor3f(0.0f, 0.2f, 1.0f);
		for (const auto& vertex : line) {
			glVertex2f(vertex.x, vertex.y);
		}
		glVertex2f(line[0].x, line[0].y);
		glEnd();
	}
	for (const auto& line : floorInfill2D) {
		glBegin(GL_LINE_STRIP);
		glColor3f(0.4f, 0.2f, 0.3f);
		for (const auto& vertex : line) {
			glVertex2f(vertex.x, vertex.y);
		}
		glVertex2f(line[0].x, line[0].y);
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

void SliceRenderView::setSliceDataClipper(Clipper2Lib::PathsD polygons)
{
	polygons2D.clear();
	for (const auto& polygon : polygons) {
		std::vector<glm::vec2> tempflattenedVertices;
		for (const auto& point : polygon) {
			tempflattenedVertices.push_back(glm::vec2(point.x, point.y));
		}
		polygons2D.push_back(tempflattenedVertices);
	}
	update();
}

void SliceRenderView::setSliceInfill(Clipper2Lib::PathsD infill)
{
	infill2D.clear();
	for (const auto& line : infill) {
		std::vector<glm::vec2> tempLine;
		for (const auto& point : line) {
			tempLine.push_back(glm::vec2(point.x, point.y));
		}
		infill2D.push_back(tempLine);
	}
	update();
}

void SliceRenderView::setSliceShells(std::vector<Clipper2Lib::PathsD> shells)
{
	shells2D.clear();
	for (const auto& shell : shells) {
		for (const auto& line : shell) {
			std::vector<glm::vec2> tempLine;
			for (const auto& point : line) {
				tempLine.push_back(glm::vec2(point.x, point.y));
			}
			shells2D.push_back(tempLine);
		}
	}
	update();
}

void SliceRenderView::setSliceFloorInfill(std::vector<Clipper2Lib::PathsD> floors)
{
	floorInfill2D.clear();
	for (const auto& floor : floors) {
		for (const auto& line : floor) {
			std::vector<glm::vec2> tempLine;
			for (const auto& point : line) {
				tempLine.push_back(glm::vec2(point.x, point.y));
			}
			floorInfill2D.push_back(tempLine);
		}
	}
	update();
}

void SliceRenderView::mousePressEvent(QMouseEvent* event) {
	lastMousePos = event->pos();
	if (event->button() == Qt::LeftButton) {
		panning = true;
		setCursor(Qt::ClosedHandCursor);
	}
}

void SliceRenderView::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		panning = false;
		setCursor(Qt::ArrowCursor);
	}
}

void SliceRenderView::mouseMoveEvent(QMouseEvent* event) {
	if (panning) {
		QPoint delta = event->pos() - lastMousePos;
		lastMousePos = event->pos();

		// Adjust panning speed and reverse the direction
		float adjustedPanSpeed = panSpeed / zoomLevel;

		// Reverse directions: horizontal (-delta.x), vertical (+delta.y)
		panOffset -= glm::vec2(delta.x() * adjustedPanSpeed, -delta.y() * adjustedPanSpeed);

		update();
	}
}

void SliceRenderView::wheelEvent(QWheelEvent* event) {
	int delta = event->angleDelta().y();

	// Logarithmic zoom for smooth zooming experience
	float scaleFactor = 1.0f + (zoomSpeed * delta / 120.0f);
	zoomLevel *= scaleFactor;

	update();
}

SliceRenderView::~SliceRenderView() {
}