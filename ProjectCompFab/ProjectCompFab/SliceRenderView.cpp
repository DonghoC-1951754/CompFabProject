#include "SliceRenderView.h"

SliceRenderView::SliceRenderView(QWidget* parent) : QOpenGLWidget(parent) {
}

void SliceRenderView::initializeGL() {
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shaderProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, "./shaders/slice_shaders/vertex_shader.glsl");
	shaderProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, "./shaders/slice_shaders/fragment_shader.glsl");
	shaderProgram.link();

	vao.create();
	vbo.create();

	setupSlice();
}

void SliceRenderView::resizeGL(int w, int h) {
	glViewport(0, 0, w, h);
}

void SliceRenderView::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shaderProgram.bind();
	vao.bind();

	QMatrix4x4 model, view, projection;
	model.setToIdentity();

	// position, where looking, leave be
	view.lookAt(QVector3D(0, 0, 200), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
	projection.perspective(45.0f, float(width()) / float(height()), 0.1f, 1000.0f);

	shaderProgram.setUniformValue("model", model);
	shaderProgram.setUniformValue("view", view);
	shaderProgram.setUniformValue("projection", projection);

	int offset = 0;
	for (const auto& line : lineSegments2D) {
		// Render each line segment in sequence
		glDrawArrays(GL_LINE_STRIP, offset, static_cast<int>(line.size()));
		offset += static_cast<int>(line.size());
	}

	vao.release();
	shaderProgram.release();
}

void SliceRenderView::setSliceData(const std::vector<std::vector<glm::vec3>> lineSegments) {
	lineSegments2D.clear();
	for (const auto& line : lineSegments) {
		std::vector<glm::vec2> line2D;
		for (const auto& point : line) {
			line2D.push_back(glm::vec2(point.x, point.z));
		}
		lineSegments2D.push_back(line2D);
	}
	
}

void SliceRenderView::setupSlice() {
	flattenedVertices.clear();
	for (const auto& line : lineSegments2D) {
		flattenedVertices.insert(flattenedVertices.end(), line.begin(), line.end());
	}

	vao.bind();
	vbo.bind();
	vbo.allocate(flattenedVertices.data(), static_cast<int>(flattenedVertices.size() * sizeof(glm::vec2)));

	shaderProgram.bind();
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
	glEnableVertexAttribArray(0);

	vbo.release();
	vao.release();
}

SliceRenderView::~SliceRenderView() {
}