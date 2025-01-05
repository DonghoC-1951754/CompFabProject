#include "ObjectRenderView.h"
#include "ObjectLoader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // For glm::perspective, glm::lookAt, etc.
#include <glm/gtc/type_ptr.hpp>  



ObjectRenderView::ObjectRenderView(QWidget* parent)
    : QOpenGLWidget(parent), cameraPos(0.0f, 10.0f, 100.0f), targetPos(90.0f, 10.0f, -90.0f), zoomFactor(1.0f)
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
	loader.setPlateWidth(plateWidth);
	loader.setPlateDepth(plateDepth);
    mesh = loader.loadSTL(filename);  // Load the model
}

void ObjectRenderView::resetRendering() {
    makeCurrent();
	//setupMesh();
    update();
}

void ObjectRenderView::initializeGL() {
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    initializeOpenGLFunctions();
   
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set light properties
    GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);

    GLfloat no_light[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, no_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, no_light);

    // Enable smooth shading
    glShadeModel(GL_SMOOTH);

    setupSlicer();
    setupPlate();

    // Slicing plane shader
    slicerProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/slicingPlaneVert.glsl");
    slicerProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/slicingPlaneFrag.glsl");
    slicerProgram.link();

    // Build plate shader
    plateShader.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/printBedVert.glsl");
    plateShader.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/printBedFrag.glsl");
    plateShader.link();

    
}

void ObjectRenderView::paintGL() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Clear buffers

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), float(width()) / float(height()), 0.1f, 10000.0f);

    glm::mat4 view = glm::lookAt(
        glm::vec3(cameraPos.x(), cameraPos.y(), cameraPos.z()), // Camera position (eye)
        glm::vec3(targetPos.x(), targetPos.y(), targetPos.z()), // Look-at position (center)
        glm::vec3(0.0f, 1.0f, 0.0f)  // Up vector
    );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMultMatrixf(glm::value_ptr(projection));  // Apply projection matrix

    // Load the view matrix into OpenGL (camera transformations)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMultMatrixf(glm::value_ptr(view));  // Apply view matrix

    // Apply transformations and render the model
    glPushMatrix();
    //glTranslatef(0.0f, 0.0f, -5.0f);  // Move model into view
    glScalef(1.0f, 1.0f, -1.0f);
    renderMesh();  // Replace with your actual mesh rendering code
    glPopMatrix();

    drawAxes();

    

    QMatrix4x4 modelShader, viewShader, projectionShader;
    modelShader.setToIdentity();

    // position, where looking, leave be
    viewShader.lookAt(cameraPos, targetPos, QVector3D(0, 1, 0));
    projectionShader.perspective(45.0f, float(width()) / float(height()), 0.1f, 10000.0f);

    //view.lookAt(cameraPos, targetPos, QVector3D(0, 1, 0));
    plateShader.bind();
    plateShader.setUniformValue("model", modelShader);
    plateShader.setUniformValue("view", viewShader);
    plateShader.setUniformValue("projection", projectionShader);

    plateShader.release();

    drawPlate();

    slicerProgram.bind();
    modelShader.setToIdentity();
    slicerProgram.setUniformValue("model", modelShader);
    slicerProgram.setUniformValue("view", viewShader);
    slicerProgram.setUniformValue("projection", projectionShader);
    slicerProgram.setUniformValue("planeColor", QVector4D(0.1f, 0.0f, 0.3f, 0.5f));  // Red color

    renderSlicer();
}

void ObjectRenderView::renderMesh() {

    //glBegin(GL_LINES);
    //glColor3f(0.0f, 0.0f, 1.0f); // Blue color for normals

    auto indices = mesh->indices;
    auto vertices = mesh->vertices;

    //for (unsigned int i = 0; i < indices.size(); i++) {
    //    const Vertex& vertex = vertices[indices[i]];
    //    glVertex3f(vertex.getPosition().x, vertex.getPosition().y, vertex.getPosition().z);
    //    glVertex3f(vertex.getPosition().x + vertex.getNormal().x, vertex.getPosition().y + vertex.getNormal().y, vertex.getPosition().z + vertex.getNormal().z);
    //}

    //glEnd();  // End normal lines rendering

	glEnable(GL_LIGHTING);

    GLfloat light_position[] = { cameraPos.x(), cameraPos.y(), -cameraPos.z(), 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Set material properties
    GLfloat material_diffuse[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_diffuse);

    GLfloat no_material[] = { 0.5f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, no_material);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, no_material);

    glBegin(GL_TRIANGLES);

    for (unsigned int i = 0; i < indices.size(); i++) {
        const Vertex& vertex = vertices[indices[i]];

        // Set the normal for the lighting calculation
        glNormal3f(vertex.getNormal().x, vertex.getNormal().y, vertex.getNormal().z);

        // Set the color or material properties (you can customize these for your object)
        glColor3f(1.0f, 0.0f, 0.0f);  // Red color for the object

        // Render the vertex
        glVertex3f(vertex.getPosition().x, vertex.getPosition().y, vertex.getPosition().z);
    }

    glEnd();  // End triangle rendering

    glFlush();
	glDisable(GL_LIGHTING);

}

void ObjectRenderView::renderSlicer() {
	
	slicer->setPlaneSize(plateWidth, plateDepth);
    slicer->setStandardHeight(slicerHeight);

    QMatrix4x4 model;
    model.setToIdentity();
    model.translate(-5.0f, 0.0f, 5.0f);

    slicerProgram.setUniformValue("model", model);
    slicerProgram.setUniformValue("planeColor", QVector4D(0.1f, 0.0f, 0.3f, 0.5f));  // Red color

	auto vertices = slicer->getVertices();
    glBegin(GL_QUADS);
        glVertex3f(vertices[0], vertices[1], vertices[2]);  // Move further along the Z-axis
        glVertex3f(vertices[3], vertices[4], vertices[5]);
        glVertex3f(vertices[6], vertices[7], vertices[8]);
        glVertex3f(vertices[9], vertices[10], vertices[11]);
    glEnd();
    slicerProgram.release();
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
    else if (event->button() == Qt::LeftButton) {
        panning = true;
    }
}

// Mouse Release Event
void ObjectRenderView::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        rotating = false;
    }
    else if (event->button() == Qt::LeftButton) {
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

        targetPos += -delta.x() * panSpeed * right - delta.y() * panSpeed * up;
        cameraPos += -delta.x() * panSpeed * right - delta.y() * panSpeed * up;
    }

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



void ObjectRenderView::drawAxes() {
    // Save the current matrix state
    glPushMatrix();

    // Define axis vertices
    QVector3D origin(0, 0, 0);
    QVector3D xAxis(100, 0, 0);
    QVector3D yAxis(0, 100, 0);
    QVector3D zAxis(0, 0, -100);

    // Draw X axis in red
    glColor3f(1.0f, 0.0f, 0.0f);  // Red color for X axis
    glBegin(GL_LINES);
    glVertex3f(origin.x(), origin.y(), origin.z());
    glVertex3f(xAxis.x(), xAxis.y(), xAxis.z());
    glEnd();

    // Draw Y axis in green
    glColor3f(0.0f, 1.0f, 0.0f);  // Green color for Y axis
    glBegin(GL_LINES);
    glVertex3f(origin.x(), origin.y(), origin.z());
    glVertex3f(yAxis.x(), yAxis.y(), yAxis.z());
    glEnd();

    // Draw Z axis in blue
    glColor3f(0.0f, 0.0f, 1.0f);  // Blue color for Z axis
    glBegin(GL_LINES);
    glVertex3f(origin.x(), origin.y(), origin.z());
    glVertex3f(zAxis.x(), zAxis.y(), zAxis.z());
    glEnd();

    // Restore the matrix state
    glPopMatrix();
}


std::vector<Clipper2Lib::PathsD> ObjectRenderView::getAllSlices() {
    double meshLowestPoint = mesh->getLowestY();
    double meshHighestPoint = mesh->getHighestY();
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

void ObjectRenderView::setupPlate() {
    updatePlateVertices();

    glGenVertexArrays(1, &plateVAO);
    glGenBuffers(1, &plateVBO);
    glGenBuffers(1, &plateEBO);

    glBindVertexArray(plateVAO);

    glBindBuffer(GL_ARRAY_BUFFER, plateVBO);
    glBufferData(GL_ARRAY_BUFFER, plateVertices.size() * sizeof(GLfloat), plateVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plateEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, plateIndices.size() * sizeof(GLuint), plateIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ObjectRenderView::setupSlicer() {
    slicer->setWidth(plateWidth);
    slicer->setDepth(plateDepth);
    //TODO update standardHeight in slicer.h
    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    glGenBuffers(1, &EBO1);

    glBindVertexArray(VAO1);

    auto vertices = slicer->getVertices();

    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Generate and bind the EBO

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertices.size() * sizeof(int), vertices.data(), GL_STATIC_DRAW);

    // Define the vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0); // Position
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
    glBindVertexArray(0); // Unbind VAO

}

void ObjectRenderView::drawPlate() {
    plateShader.bind();
    if (plateSizeChanged) {
        updatePlateVertices(); // Update vertices only if the plate is marked dirty
        plateSizeChanged = false;   // Reset the flag
    }
    QMatrix4x4 modelBed;
    modelBed.setToIdentity();
    plateShader.setUniformValue("model", modelBed);
    plateShader.setUniformValue("plateWidth", static_cast<float>(plateWidth));
    plateShader.setUniformValue("plateDepth", static_cast<float>(plateDepth));
    // Re-upload the updated vertices to the GPU

    glBindVertexArray(plateVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    plateShader.release();
}

void ObjectRenderView::updatePlateVertices() {
    plateVertices[3] = plateWidth;
    plateVertices[6] = plateWidth;
    plateVertices[8] = -plateDepth;
    plateVertices[11] = -plateDepth;
    glBindBuffer(GL_ARRAY_BUFFER, plateVBO);
    glBufferData(GL_ARRAY_BUFFER, plateVertices.size() * sizeof(GLfloat), plateVertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind the buffer
}

