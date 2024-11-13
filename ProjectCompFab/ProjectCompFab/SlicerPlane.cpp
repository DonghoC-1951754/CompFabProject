#include "SlicerPlane.h"
#include <algorithm>
#include <qDebug>

std::vector< std::vector<std::vector<glm::dvec3>>> SlicerPlane::slice(const Mesh* mesh, double slicerHeight) {
	orderedLineSegments.clear();
	std::vector<unsigned int> meshIndices = mesh->indices;
	std::vector<Vertex> meshVertices = mesh->vertices;
	lineSegments.clear();
	for (int i = 0; i < meshIndices.size(); i += 3) {
		std::vector<Vertex> currentTriangle;
		for (int j = 0; j < 3; j++) {
			currentTriangle.push_back(meshVertices[meshIndices[i + j]]);
		}
		calcLineSegments(currentTriangle, slicerHeight);
	}
	//auto test = getOrderedLineSegments();
	return getOrderedLineSegments();
}

void SlicerPlane::calcLineSegments(std::vector<Vertex> triangle, double slicerHeight) {
	std::vector<int> isAboveIndex;
	std::vector<int> isUnderIndex;
	for (int i = 0; i < 3; i++) {
		if (triangle[i].getPosition().y > slicerHeight) {
			isAboveIndex.push_back(i);
		}
		else if (triangle[i].getPosition().y < slicerHeight) {
			isUnderIndex.push_back(i);
		}
	}
	double xA, yA, zA, xB, yB, zB;
	if (isAboveIndex.size() == 2 || isUnderIndex.size() == 2) {
		// Vertex 1 and 2 are both above/under the slicer plane
		glm::dvec3 vertex1;
		glm::dvec3 vertex2;
		// Vertex 3 is the only vertex above/under the slicer plane
		glm::dvec3 vertex3;

		if (isAboveIndex.size() == 2) {
			vertex1 = triangle[isAboveIndex[0]].getPosition();
			vertex2 = triangle[isAboveIndex[1]].getPosition();
			vertex3 = triangle[isUnderIndex[0]].getPosition();
		}
		else {
			vertex1 = triangle[isUnderIndex[0]].getPosition();
			vertex2 = triangle[isUnderIndex[1]].getPosition();
			vertex3 = triangle[isAboveIndex[0]].getPosition();
		}
		xA = vertex1.x + (slicerHeight - vertex1.y) * (vertex3.x - vertex1.x) / (vertex3.y - vertex1.y);
		zA = vertex1.z + (slicerHeight - vertex1.y) * (vertex3.z - vertex1.z) / (vertex3.y - vertex1.y);
		yA = slicerHeight;

		xB = vertex2.x + (slicerHeight - vertex2.y) * (vertex3.x - vertex2.x) / (vertex3.y - vertex2.y);
		zB = vertex2.z + (slicerHeight - vertex2.y) * (vertex3.z - vertex2.z) / (vertex3.y - vertex2.y);
		yB = slicerHeight;
		std::vector<glm::dvec3> lineSegment;
		lineSegment.push_back(glm::dvec3(xA, yA, zA));
		lineSegment.push_back(glm::dvec3(xB, yB, zB));
		lineSegments.push_back(lineSegment);
	}
	
}

std::vector< std::vector<std::vector<glm::dvec3>>> SlicerPlane::getOrderedLineSegments() {
	auto unorderedLineSegments = lineSegments;
	orderedLineSegments.push_back(lineSegments[0]);
	unorderedLineSegments.erase(unorderedLineSegments.begin());

	while (unorderedLineSegments.size() > 0) {
		int i = 0;
		std::vector<glm::dvec3> tempLineSegment = orderedLineSegments.back();
		while (unorderedLineSegments.size() > 0 && i < unorderedLineSegments.size()) {
			double distance1 = glm::distance(tempLineSegment[1], unorderedLineSegments[i][0]);
			double distance2 = glm::distance(tempLineSegment[1], unorderedLineSegments[i][1]);
			if (distance1 < epsilon) {
				orderedLineSegments.push_back(unorderedLineSegments[i]);
				unorderedLineSegments.erase(unorderedLineSegments.begin() + i);
				break;
			}
			else if (distance2 < epsilon) {
				auto temp = unorderedLineSegments[i];
				std::reverse(temp.begin(), temp.end());
				orderedLineSegments.push_back(temp);
				unorderedLineSegments.erase(unorderedLineSegments.begin() + i);
				break;
			}
			++i;

		}
		if (i == unorderedLineSegments.size()) {
			polygonsOfOrderedLineSegments.push_back(orderedLineSegments);
			if (unorderedLineSegments.size() > 0) {
				orderedLineSegments.clear();
				orderedLineSegments.push_back(unorderedLineSegments[0]);
				unorderedLineSegments.erase(unorderedLineSegments.begin());
			}
		}
	}
	return polygonsOfOrderedLineSegments;
}