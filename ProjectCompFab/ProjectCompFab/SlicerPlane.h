#pragma once
#include <vector>
class SlicerPlane
{
public:
	SlicerPlane() {};
	std::vector<float> getVertices() { return vertices; };
	std::vector<int> getIndices() { return indices; };
private:
    std::vector<float> vertices{
        -80.0f, 0.0f, -80.0f,
         80.0f, 0.0f, -80.0f,
         80.0f, 0.0f,  80.0f,
        -80.0f, 0.0f,  80.0f,
    };
	std::vector<int> indices{
		0, 1, 2,
		2, 3, 0
	};
};

