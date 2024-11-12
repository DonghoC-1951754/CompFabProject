#include "SlicerPlane.h"
#include <algorithm>

std::vector<std::vector<glm::vec3>> SlicerPlane::slice(const Mesh* mesh, double slicerHeight) {
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
		else {
			isUnderIndex.push_back(i);
		}
	}
	float xA, yA, zA, xB, yB, zB;
	if (isAboveIndex.size() == 2 || isUnderIndex.size() == 2) {
		// Vertex 1 and 2 are both above/under the slicer plane
		glm::vec3 vertex1;
		glm::vec3 vertex2;
		// Vertex 3 is the only vertex above/under the slicer plane
		glm::vec3 vertex3;

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
		std::vector<glm::vec3> lineSegment;
		std::vector<glm::vec3> pointSegment;
		if (xA == xB && zA == zB && yA == yB) {
			pointSegment.push_back(glm::vec3(xA, yA, zA));
			pointSegment.push_back(glm::vec3(xB, yB, zB));
			pointSegments.push_back(pointSegment);
		}
		else {
			lineSegment.push_back(glm::vec3(xA, yA, zA));
			lineSegment.push_back(glm::vec3(xB, yB, zB));
			lineSegments.push_back(lineSegment);
		}
	}
	
}

bool SlicerPlane::nextIsPointSegment(std::vector<glm::vec3> lineSegment) {
	auto endPoint = lineSegment[1];
	for (int i = 0; i < pointSegments.size(); i++) {
		if (glm::distance(endPoint, pointSegments[i][0]) < epsilon) {
			orderedLineSegments.push_back(pointSegments[i]);
			pointSegments.erase(pointSegments.begin() + i);
			return true;
		}
	}
	return false;
}

std::vector<std::vector<glm::vec3>> SlicerPlane::getOrderedLineSegments() {
	auto unorderedLineSegments = lineSegments;
	orderedLineSegments.push_back(lineSegments[0]);
	unorderedLineSegments.erase(unorderedLineSegments.begin());

	while (unorderedLineSegments.size() > 0) {
		int i = 0;
		std::vector<glm::vec3> tempLineSegment = orderedLineSegments.back();
		if (nextIsPointSegment(tempLineSegment)) {
			continue;
		}
		while (unorderedLineSegments.size() > 0) {
			float distance1 = glm::distance(tempLineSegment[1], unorderedLineSegments[i][0]);
			float distance2 = glm::distance(tempLineSegment[1], unorderedLineSegments[i][1]);
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
	}
	return orderedLineSegments;
}