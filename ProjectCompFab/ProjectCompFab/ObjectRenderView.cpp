#include "ObjectRenderView.h"
#include "ObjectLoader.h"


ObjectRenderView::ObjectRenderView(QWidget* parent)
    : QOpenGLWidget(parent), cameraPos(0.0f, 0.0f, 100.0f), targetPos(0.0f, 0.0f, 0.0f), zoomFactor(1.0f)
{
}

void ObjectRenderView::loadModel(const std::string& filename) {
    ObjectLoader loader;
    mesh = loader.loadSTL(filename);  // Load the model
}

void ObjectRenderView::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    setupMesh();

	// Compile the shader program
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/vertex_shader.glsl");
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/fragment_shader.glsl");
    shaderProgram.link();
    
}

void ObjectRenderView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaderProgram.bind();

    // Setup model, view and projection matrices
    QMatrix4x4 model, view, projection;
    model.setToIdentity();

    // position, where looking, leave be
    view.lookAt(cameraPos, targetPos, QVector3D(0, 1, 0));
    projection.perspective(45.0f, float(width()) / float(height()), 0.1f, 1000.0f);

    shaderProgram.setUniformValue("model", model);
    shaderProgram.setUniformValue("view", view);
    shaderProgram.setUniformValue("projection", projection);

    drawAxes();

    // Set the cube color
    shaderProgram.setUniformValue("cubeColor", QVector4D(1.0f, 0.5f, 0.0f, 1.0f));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    shaderProgram.release();
}

void ObjectRenderView::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

// Mouse Press Event for Rotation and Panning
void ObjectRenderView::mousePressEvent(QMouseEvent* event) {
    lastMousePos = event->pos();
    if (event->button() == Qt::RightButton) {
        rotating = true;
    }
    else if (event->button() == Qt::MiddleButton) {
        panning = true;
    }
}

// Mouse Release Event
void ObjectRenderView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        rotating = false;
    }
    else if (event->button() == Qt::MiddleButton) {
        panning = false;
    }
}

// Mouse Move Event for Camera Rotation and Panning
void ObjectRenderView::mouseMoveEvent(QMouseEvent* event) {
    QPoint delta = event->pos() - lastMousePos;
    lastMousePos = event->pos();

    if (rotating) {
        float angleX = delta.y() * 0.5f; // Rotate up/down
        float angleY = delta.x() * 0.5f; // Rotate left/right

        QMatrix4x4 rotation;
        rotation.rotate(angleX, 1, 0, 0);
        rotation.rotate(angleY, 0, 1, 0);

        QVector3D direction = cameraPos - targetPos; // Direction vector
        cameraPos = rotation.map(direction) + targetPos; // Apply rotation and translate
    }
    else if (panning) {
        float panSpeed = 0.1f;
        QVector3D right = QVector3D::crossProduct(cameraPos - targetPos, QVector3D(0, 1, 0)).normalized();
        QVector3D up = QVector3D::crossProduct(right, cameraPos - targetPos).normalized();

        targetPos += -delta.x() * panSpeed * right + delta.y() * panSpeed * up;
        cameraPos += -delta.x() * panSpeed * right + delta.y() * panSpeed * up;
    }

    update(); // Repaint with new camera settings
}

// Wheel Event for Zooming
void ObjectRenderView::wheelEvent(QWheelEvent* event) {
    float zoomSpeed = 5.0f; // Change this value to control zoom speed
    float zoomAmount = event->angleDelta().y() * zoomSpeed / 120; // 120 = standard scroll delta

    // Calculate the direction from the camera to the target
    QVector3D direction = (cameraPos - targetPos).normalized();

    // Move the camera along the direction vector based on the zoom amount
    cameraPos -= direction * zoomAmount; // Move camera closer or further from the target

    // Optional: Clamp the zoom to prevent zooming too far in or out
    float minZoom = 1.0f; // Minimum distance from the target
    float maxZoom = 300.0f; // Maximum distance from the target
    float distance = (cameraPos - targetPos).length();

    if (distance < minZoom) {
        cameraPos = targetPos + direction * minZoom; // Clamp to min zoom
    }
    else if (distance > maxZoom) {
        cameraPos = targetPos + direction * maxZoom; // Clamp to max zoom
    }

    update(); // Repaint with updated zoom
}

void ObjectRenderView::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO); // Generate the Element Buffer Object

    glBindVertexArray(VAO);

    // Bind the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);

    // Bind the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), &mesh->indices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::offsetPosition());
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)Vertex::offsetNormal());
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TextureCoords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void ObjectRenderView::drawAxes() {
    // Set up a simple line shader (if not already in the shader program)
    QMatrix4x4 model;
    model.setToIdentity();

    shaderProgram.setUniformValue("model", model);

    // Define axis vertices
    QVector3D origin(0, 0, 0);
    QVector3D xAxis(100, 0, 0);
    QVector3D yAxis(0, 100, 0);
    QVector3D zAxis(0, 0, 100);

    // Draw X axis in red
    shaderProgram.setUniformValue("cubeColor", QVector4D(1.0f, 0.0f, 0.0f, 1.0f));  // Red color
    glBegin(GL_LINES);
    glVertex3f(origin.x(), origin.y(), origin.z());
    glVertex3f(xAxis.x(), xAxis.y(), xAxis.z());
    glEnd();

    // Draw Y axis in green
    shaderProgram.setUniformValue("cubeColor", QVector4D(0.0f, 1.0f, 0.0f, 1.0f));  // Green color
    glBegin(GL_LINES);
    glVertex3f(origin.x(), origin.y(), origin.z());
    glVertex3f(yAxis.x(), yAxis.y(), yAxis.z());
    glEnd();

    // Draw Z axis in blue
    shaderProgram.setUniformValue("cubeColor", QVector4D(0.0f, 0.0f, 1.0f, 1.0f));  // Blue color
    glBegin(GL_LINES);
    glVertex3f(origin.x(), origin.y(), origin.z());
    glVertex3f(zAxis.x(), zAxis.y(), zAxis.z());
    glEnd();
}

ObjectRenderView::~ObjectRenderView()
{}
