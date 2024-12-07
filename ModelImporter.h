#pragma once


#include <iostream>
#include <vector>


class ModelImporter
{
private:
	std::vector<float> _vertVals;
	std::vector<float> _triangleVerts;
	std::vector<float> _textureCoords;
	std::vector<float> _stVals;
	std::vector<float> _normals;
	std::vector<float> _normVals;

public:
	ModelImporter();
	void parseOBJ(char* filePath);
	int getNumVertices();
	std::vector<float> getVertices();
	std::vector<float> getTextureCoordinates();
	std::vector<float> getNormals();

};


