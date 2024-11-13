#pragma once
#include <QWidget>
#include <glm/glm.hpp>

class SliceRenderView;
class SliceWindow : public QWidget {
    Q_OBJECT

public:
    SliceWindow(QWidget* parent = nullptr);
    ~SliceWindow();
	void setSliceData(const std::vector< std::vector<std::vector<glm::dvec3>>> lineSegments);

private:
    SliceRenderView* sliceRenderWidget;
};

