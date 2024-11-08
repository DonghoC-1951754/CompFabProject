#pragma once
#include "ui_ProjectCompFab.h"
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "ObjectLoader.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>
#include <QMouseEvent>
#include <QWheelEvent>
#include "SlicerPlane.h"


class ObjectRenderView : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    ObjectRenderView(QWidget *parent = nullptr);
    void loadModel(const std::string& filename);
    ~ObjectRenderView();
    SlicerPlane* getSlicer() { return slicer; };

protected:

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // Event handlers for camera control
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    Mesh* mesh;
    SlicerPlane* slicer;
    
    GLuint VBO, VAO, EBO, VBO1, VAO1, EBO1;
    void setupMesh();
    void setupSlicer();
    void renderMesh();
    void renderSlicer();
    glm::vec3 findMidpoint();
    QOpenGLShaderProgram shaderProgram;
    Ui::ProjectCompFabClass ui;

    void drawAxes();

    QVector3D cameraPos;
    QVector3D targetPos;
    float zoomFactor;

    // Mouse control variables
    QPoint lastMousePos;
    bool rotating = false; // True when right mouse button is held for rotation
    bool panning = false;  // True when middle mouse button is held for panning

	float slicerHeight = 0.0f;
    void setSlicerHeight(float height) { slicerHeight = height; };
	float getSlicerHeight() { return slicerHeight; };

public slots:
    void setSliderSlicerHeight(float value) {
        slicerHeight = value;
        update(); // Refresh the OpenGL widget to reflect changes
    }
    std::vector<std::vector<glm::vec3>> sliceMesh() {
		return slicer->slice(mesh, slicerHeight);
		//update(); // Refresh the OpenGL widget to reflect changes
	}
};
