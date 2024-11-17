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
    void resetRendering();

	double getSlicerHeight() { return slicerHeight; };
    void setSlicerHeight(double height) { slicerHeight = height; update(); };

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

	double slicerHeight = 0.0;
    /*void setSlicerHeight(double height) { slicerHeight = height; };*/
	//double getSlicerHeight() { return slicerHeight; };

public slots:
    void setSliderSlicerHeight(double value) {
        slicerHeight = value;
        update();
    }
    std::vector< std::vector<std::vector<glm::dvec3>>> sliceMesh() {
        slicerHeight += 0.00000001;
		return slicer->slice(mesh, slicerHeight);
	}

    std::vector<Clipper2Lib::PathsD> getAllSlices() {
        double meshLowestPoint = mesh->getLowestPoint();
		double meshHighestPoint = mesh->getHighestPoint();
        // Edge case: lift slicer plane height by 0.00000001
		double currentHeight = meshLowestPoint + 0.00000001;
        double layerHeight = slicer->getLayerHeight();

		std::vector<Clipper2Lib::PathsD> allCompiledSlices;

        while (currentHeight < meshHighestPoint) {
			auto rawSlice = slicer->slice(mesh, currentHeight);
			slicer->setContours(rawSlice);
			Clipper2Lib::PathsD compiledSlice = slicer->compilePolygons();
			allCompiledSlices.push_back(compiledSlice);
            currentHeight += layerHeight;
        }
		return allCompiledSlices;
    }

	void changeLayerHeight(double value) {
		slicer->setLayerHeight(value);
        
	}
};
