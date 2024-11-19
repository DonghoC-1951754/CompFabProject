#include "SlicerPlane.h"
#include <algorithm>
#include <qDebug>

std::vector< std::vector<std::vector<glm::dvec3>>> SlicerPlane::slice(const Mesh* mesh, double slicerHeight) {
	polygonsOfOrderedLineSegments.clear();
	lineSegments.clear();
	std::vector<unsigned int> meshIndices = mesh->indices;
	std::vector<Vertex> meshVertices = mesh->vertices;
	for (int i = 0; i < meshIndices.size(); i += 3) {
		std::vector<Vertex> currentTriangle;
		for (int j = 0; j < 3; j++) {
			currentTriangle.push_back(meshVertices[meshIndices[i + j]]);
		}
		calcLineSegments(currentTriangle, slicerHeight);
	}
	fillPolygonWithOrderedSegments();
	return polygonsOfOrderedLineSegments;
}

void SlicerPlane::setContours(std::vector<std::vector<std::vector<glm::dvec3>>> polygons)
{
	contours.clear();
	for (const auto& polygon : polygons) {
		Clipper2Lib::PathD currentContour;
		auto flattenedPolygon = getFlattenedPolygon(polygon);
		for (const auto& vertex : flattenedPolygon) {
			currentContour.push_back(Clipper2Lib::PointD(vertex.x, vertex.z));
		}
		contours.push_back(currentContour);
	}
}

Clipper2Lib::PathsD SlicerPlane::compilePolygons()
{
	Clipper2Lib::ClipperD clipper;
	clipper.AddSubject(contours);
	Clipper2Lib::PathsD unionResult;
	clipper.Execute(Clipper2Lib::ClipType::Union, Clipper2Lib::FillRule::EvenOdd, unionResult);
	clipper.Clear();
	return unionResult;
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

void SlicerPlane::fillPolygonWithOrderedSegments() {
	std::vector<std::vector<glm::dvec3>> unorderedLineSegments = lineSegments;
	while (unorderedLineSegments.size() > 0) {
		std::vector<std::vector<glm::dvec3>> singleOrderedPolygon;
		singleOrderedPolygon = getSingleOrderedPolygon(unorderedLineSegments);
		polygonsOfOrderedLineSegments.push_back(singleOrderedPolygon);
	}
}

std::vector<std::vector<glm::dvec3>> SlicerPlane::getSingleOrderedPolygon(std::vector<std::vector<glm::dvec3>>& remainingUnorderedLineSegments) {
	std::vector<std::vector<glm::dvec3>> orderedPolygon;
	// Add the first line segment to the ordered polygon and remove it from remainingUnorderedLineSegments
	orderedPolygon.push_back(remainingUnorderedLineSegments[0]);
	remainingUnorderedLineSegments.erase(remainingUnorderedLineSegments.begin());

	while (remainingUnorderedLineSegments.size() > 0) {
		int i = 0;
		std::vector<glm::dvec3> latestOrderedSegment = orderedPolygon.back();
		while (i < remainingUnorderedLineSegments.size()) {
			double distance1 = glm::distance(latestOrderedSegment[1], remainingUnorderedLineSegments[i][0]);
			double distance2 = glm::distance(latestOrderedSegment[1], remainingUnorderedLineSegments[i][1]);
			if (distance1 < epsilon) {
				orderedPolygon.push_back(remainingUnorderedLineSegments[i]);
				remainingUnorderedLineSegments.erase(remainingUnorderedLineSegments.begin() + i);
				break;
			}
			else if (distance2 < epsilon) {
				auto temp = remainingUnorderedLineSegments[i];
				std::reverse(temp.begin(), temp.end());
				orderedPolygon.push_back(temp);
				remainingUnorderedLineSegments.erase(remainingUnorderedLineSegments.begin() + i);
				break;
			}
			else if (i == remainingUnorderedLineSegments.size() - 1) {
				return orderedPolygon;
			}
			++i;
		}
	}
	return orderedPolygon;
}

std::vector<glm::dvec3> SlicerPlane::getFlattenedPolygon(std::vector<std::vector<glm::dvec3>> singlePolygon)
{
	//std::vector<std::vector<glm::dvec3>> tempLineSegments2D;
	std::vector<glm::dvec3> flattenedPolygon;
	std::vector<glm::dvec3> lastLine;
	for (const auto& line : singlePolygon) {
		flattenedPolygon.push_back(line[0]);
		lastLine = line;
	}
	flattenedPolygon.push_back(lastLine[1]);
	return flattenedPolygon;
}
