#pragma once
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <glm/glm.hpp>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include "clipper2/clipper.h"

class SliceRenderView : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
    SliceRenderView(QWidget* parent = nullptr);
    ~SliceRenderView();
	void setSliceData(const std::vector< std::vector<std::vector<glm::dvec3>>> polygons);
    void setSliceDataClipper(Clipper2Lib::PathsD polygons);
	void setSliceInfill(Clipper2Lib::PathsD infill);
	void setSliceShells(Clipper2Lib::PathsD shells);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
    std::vector<std::vector<glm::vec2>> polygons2D;
	std::vector<std::vector<glm::vec2>> infill2D;
	std::vector<std::vector<glm::vec2>> shells2D;

    bool panning = false;
    QPoint lastMousePos;
    float panSpeed = 0.1f;
    glm::vec2 panOffset = glm::vec2(0.0f);

    float zoomLevel = 1.0f;                // Track the current zoom level
    float minZoom = 0.1f;                  // Minimum zoom level
    float maxZoom = 10.0f;                 // Maximum zoom level
    float zoomSpeed = 0.1f;
};

