#pragma once
#include <QWidget>
#include <glm/glm.hpp>

class SliceRenderView;
class SliceWindow : public QWidget {
    Q_OBJECT

public:
    SliceWindow(QWidget* parent = nullptr);
    ~SliceWindow();
	void setSliceData(const std::vector<std::vector<glm::vec3>> lineSegments);

private:
    SliceRenderView* sliceRenderWidget;
};

