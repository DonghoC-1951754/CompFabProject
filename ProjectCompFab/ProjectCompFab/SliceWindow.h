#pragma once
#include <QWidget>
#include <glm/glm.hpp>
#include "clipper2/clipper.h"

class SliceRenderView;
class SliceWindow : public QWidget {
    Q_OBJECT

public:
    SliceWindow(QWidget* parent = nullptr);
    ~SliceWindow();
	void setSliceData(const std::vector< std::vector<std::vector<glm::dvec3>>> lineSegments);
	void setSLiceDataClipper(Clipper2Lib::PathsD polygons);
	void setSliceInfill(Clipper2Lib::PathsD infill);

private:
    SliceRenderView* sliceRenderWidget;
};

