#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ProjectCompFab.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include "ObjectLoader.h"
//#include "Shader.h"
#include <QOpenGLShaderProgram>
#include <QOpenGLExtraFunctions>

class ObjectRenderView : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    ObjectRenderView(QWidget *parent = nullptr);
    void loadModel(const std::string& filename);
    ~ObjectRenderView();

protected:

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    Mesh* mesh;
    GLuint VBO, VAO, EBO;
    void setupMesh();
    QOpenGLShaderProgram shaderProgram;
    Ui::ProjectCompFabClass ui;
    //Shader shader;
};
