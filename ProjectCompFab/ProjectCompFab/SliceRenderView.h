#pragma once
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <glm/glm.hpp>

class SliceRenderView : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
	Q_OBJECT
public:
    SliceRenderView(QWidget* parent = nullptr);
    ~SliceRenderView();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

};

