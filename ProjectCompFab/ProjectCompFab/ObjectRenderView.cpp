#include "ObjectRenderView.h"
#include "ObjectLoader.h"


ObjectRenderView::ObjectRenderView(QWidget* parent)
    : QOpenGLWidget(parent), cameraPos(0.0f, 0.0f, 100.0f), targetPos(0.0f, 0.0f, 0.0f), zoomFactor(1.0f)
{
	slicer = new SlicerPlane();
}

ObjectRenderView::~ObjectRenderView()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);
    glDeleteBuffers(1, &EBO1);
	delete mesh;
	mesh = nullptr;
    delete slicer;
	slicer = nullptr;
}

void ObjectRenderView::loadModel(const std::string& filename) {
    ObjectLoader loader;
    mesh = loader.loadSTL(filename);  // Load the model
}

void ObjectRenderView::resetRendering() {
    makeCurrent();
	setupMesh();
    update();
}

void ObjectRenderView::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    setupMesh();
	setupSlicer();

	// Compile the shader program
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/vertex_shader.glsl");
    shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/fragment_shader.glsl");
    shaderProgram.link();

	plateShader.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/printBedVert.glsl");
    plateShader.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/printBedFrag.glsl");
	plateShader.link();

    
}

void ObjectRenderView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Setup model, view and projection matrices
    QMatrix4x4 model, view, projection;
    model.setToIdentity();
    

    // position, where looking, leave be
    view.lookAt(cameraPos, targetPos, QVector3D(0, 1, 0));
    projection.perspective(45.0f, float(width()) / float(height()), 0.1f, 10000.0f);

    plateShader.bind();

    plateShader.setUniformValue("model", model);
    plateShader.setUniformValue("view", view);
    plateShader.setUniformValue("projection", projection);

    plateShader.release();

    drawPlate();

    shaderProgram.bind();
    shaderProgram.setUniformValue("model", model);
    shaderProgram.setUniformValue("view", view);
    shaderProgram.setUniformValue("projection", projection);
    renderSlicer();
    drawAxes();
    
    renderMesh();

    shaderProgram.release();

}

void ObjectRenderView::renderMesh() {
    // Set the cube color
    QMatrix4x4 model;
	model.scale(1.0f, 1.0f, -1.0f);
    shaderProgram.setUniformValue("cubeColor", QVector4D(1.0f, 0.5f, 0.0f, 1.0f));
    shaderProgram.setUniformValue("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0); // Unbind VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
}

void ObjectRenderView::renderSlicer() {
    slicer->setWidth(plateWidth);
    slicer->setDepth(plateDepth);
    //TODO update standardHeight in slicer.h

    QMatrix4x4 modelSlicer;
    modelSlicer.setToIdentity();
    modelSlicer.translate(-5.0f, 0.0f, 5.0f); // Move 5 units along the Z-axis
    shaderProgram.setUniformValue("cubeColor", QVector4D(0.071f, 0.42f, 1.0f, 0.4f));
    shaderProgram.setUniformValue("model", modelSlicer);

    glBindVertexArray(VAO1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);  // Adjust count based on your vertex data
    glBindVertexArray(0); // Unbind VAO
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
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
        float sensitivity = 0.3f;
        float damping = 0.9f;

        float angleX = delta.y() * sensitivity * damping;
        float angleY = -delta.x() * sensitivity * damping;
        static const float pitchLimit = 85.0f;

        QVector3D forward = (cameraPos - targetPos).normalized();
        QVector3D right = QVector3D::crossProduct(forward, QVector3D(0, 1, 0)).normalized();
        QVector3D up = QVector3D::crossProduct(right, forward).normalized();

        float currentPitch = qRadiansToDegrees(qAsin(forward.y()));
        float newPitch = qBound(-pitchLimit, currentPitch + angleX, pitchLimit);

        QQuaternion pitchRotation = QQuaternion::fromAxisAndAngle(right, newPitch - currentPitch);
        QQuaternion yawRotation = QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), angleY);

        QQuaternion totalRotation = yawRotation * pitchRotation;
        QVector3D direction = (cameraPos - targetPos).normalized();

        float distance = (cameraPos - targetPos).length();
        cameraPos = totalRotation.rotatedVector(direction * distance) + targetPos;
    }
    else if (panning) {
        float panSpeed = 0.1f;
        QVector3D right = QVector3D::crossProduct(cameraPos - targetPos, QVector3D(0, 1, 0)).normalized();
        QVector3D up = QVector3D::crossProduct(right, cameraPos - targetPos).normalized();

        targetPos += -delta.x() * panSpeed * right + delta.y() * panSpeed * up;
        cameraPos += -delta.x() * panSpeed * right + delta.y() * panSpeed * up;
    }

    update();  // Repaint with updated camera settings

    update();  // Repaint with updated camera settings
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
    QVector3D zAxis(0, 0, -100);

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

void ObjectRenderView::setupSlicer() {
	slicer->setWidth(plateWidth);
	slicer->setDepth(plateDepth);
    //TODO update standardHeight in slicer.h
    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    glGenBuffers(1, &EBO1);

    glBindVertexArray(VAO1);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, slicer->getVertices().size() * sizeof(float), slicer->getVertices().data(), GL_STATIC_DRAW);

    // Generate and bind the EBO
   
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, slicer->getIndices().size() * sizeof(int), slicer->getIndices().data(), GL_STATIC_DRAW);

    // Define the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0); // Position
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    glBindVertexArray(0); // Unbind VAO
}


std::vector<Clipper2Lib::PathsD> ObjectRenderView::getAllSlices() {
    double meshLowestPoint = mesh->getLowestPoint();
    double meshHighestPoint = mesh->getHighestPoint();
    // Edge case: lift slicer plane height by 0.00000001
    double currentHeight = meshLowestPoint + 0.00000001;
    double layerHeight = slicer->getLayerHeight();
    currentHeight += layerHeight;
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

void ObjectRenderView::drawPlate() {
    plateShader.bind();

    // Define the vertices of the plate (18cm x 18cm), positioned flat at height 0
    GLfloat plateVertices[] = {
        // Positions          // No need for colors as they are generated in the shader
        0.0f, 0.0f, 0.0f,       // Bottom-left corner (0, 0)
        plateWidth, 0.0f, 0.0f,  // Bottom-right corner (18, 0)
        plateWidth, 0.0f, -plateDepth,  // Top-right corner (18, -18)
        0.0f, 0.0f, -plateDepth   // Top-left corner (0, -18)
    };

    // Define the indices for the square (two triangles)
    GLuint plateIndices[] = {
        0, 1, 2,   // First triangle (Bottom-left, Bottom-right, Top-right)
        0, 2, 3    // Second triangle (Bottom-left, Top-right, Top-left)
    };

    // Create and bind VAO, VBO, and EBO for the plate
    GLuint plateVAO, plateVBO, plateEBO;
    glGenVertexArrays(1, &plateVAO);
    glGenBuffers(1, &plateVBO);
    glGenBuffers(1, &plateEBO);

    glBindVertexArray(plateVAO);

    glBindBuffer(GL_ARRAY_BUFFER, plateVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plateVertices), plateVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plateEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plateIndices), plateIndices, GL_STATIC_DRAW);

    // Set the vertex attribute pointer for positions (no colors needed)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    glBindVertexArray(0); // Unbind VAO

    // Create transformation matrices
    QMatrix4x4 modelBed;
    modelBed.setToIdentity();
    plateShader.setUniformValue("model", modelBed);
	plateShader.setUniformValue("plateWidth", static_cast<float>(plateWidth));
    plateShader.setUniformValue("plateDepth", static_cast<float>(plateDepth));


    // Draw the plate
    glBindVertexArray(plateVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // 6 indices for 2 triangles
    glBindVertexArray(0); // Unbind VAO

    // Clean up
    glDeleteVertexArrays(1, &plateVAO);
    glDeleteBuffers(1, &plateVBO);
    glDeleteBuffers(1, &plateEBO);

    plateShader.release();
}


void ObjectRenderView::setPlateWidth(double width) {
	plateWidth = width;
	update();
}

void ObjectRenderView::setPlateDepth(double depth) {
	plateDepth = depth;
	update();
}

double ObjectRenderView::getPlateWidth() {
	return plateWidth;
}

double ObjectRenderView::getPlateDepth() {
	return plateDepth;
}

void ObjectRenderView::setLayerHeight(double depth) {
    layerHeight = depth;
    update();
}

double ObjectRenderView::getLayerHeight() {
    return layerHeight;
}