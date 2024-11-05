#pragma once
#include <vector>
class SlicerPlane
{
public:
	SlicerPlane() {};
	std::vector<float> getVertices() { return vertices; };
	std::vector<int> getIndices() { return indices; };
private:
	float width = 40.0f;
    std::vector<float> vertices{
        -width, 0.0f, -width,
		 width, 0.0f, -width,
		 width, 0.0f,  width,
        -width, 0.0f,  width,
    };
	std::vector<int> indices{
		0, 1, 2,
		2, 3, 0
	};
};

