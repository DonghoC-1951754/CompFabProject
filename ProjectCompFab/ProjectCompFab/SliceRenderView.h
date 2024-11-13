#pragma once
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <glm/glm.hpp>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

class SliceRenderView : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
    SliceRenderView(QWidget* parent = nullptr);
    ~SliceRenderView();
	void setSliceData(const std::vector< std::vector<std::vector<glm::dvec3>>> polygons);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
	//std::vector<std::vector<glm::vec2>> lineSegments2D;
    //std::vector<glm::vec2> flattenedVertices;
    std::vector<std::vector<glm::vec2>> polygons2D;
    QOpenGLBuffer vbo;
    QOpenGLVertexArrayObject vao;
    QOpenGLShaderProgram shaderProgram;
};

