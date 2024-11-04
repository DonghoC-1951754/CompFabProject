#include "ObjectRenderView.h"
#include "ObjectLoader.h"


ObjectRenderView::ObjectRenderView(QWidget* parent)
    : QOpenGLWidget(parent)
{
}

void ObjectRenderView::loadModel(const std::string& filename) {
    ObjectLoader loader;
    mesh = loader.loadSTL(filename);  // Load the model
}

void ObjectRenderView::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

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
    view.lookAt(QVector3D(50, 80, 150), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    projection.perspective(45.0f, float(width()) / float(height()), 0.1f, 150.0f);

    shaderProgram.setUniformValue("model", model);
    shaderProgram.setUniformValue("view", view);
    shaderProgram.setUniformValue("projection", projection);

    // Set the cube color
    shaderProgram.setUniformValue("cubeColor", QVector4D(1.0f, 0.5f, 0.0f, 1.0f)); // Orange color

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    shaderProgram.release();
}

void ObjectRenderView::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
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

    glBindVertexArray(0); // Unbind the VAO
}

ObjectRenderView::~ObjectRenderView()
{}
