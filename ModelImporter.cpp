#include "ModelImporter.h"
#include <sstream>
#include <fstream>
#include <istream>
#include <string>

using namespace std;

ModelImporter::ModelImporter()
{

}

void ModelImporter::parseOBJ(char* filePath)
{
	float x = 0.f, y = 0.f, z = 0.f;
	string content;
	ifstream fileStream(filePath, std::ios::in);
	if (!fileStream.is_open()) {
		cout << "null " << filePath;
		return;
	}
	string line = "";
	while (getline(fileStream, line))
	{
		if (line.compare(0, 2, "v ") == 0)    //注意v后面有空格
		{
			std::stringstream ss(line.erase(0, 1));
			ss >> x >> y >> z;
			_vertVals.push_back(x);
			_vertVals.push_back(y);
			_vertVals.push_back(z);
		}
		if (line.compare(0, 2, "vt") == 0)
		{
			std::stringstream ss(line.erase(0, 2));
			ss >> x >> y;
			_stVals.push_back(x);
			_stVals.push_back(y);
		}
		if (line.compare(0, 2, "vn") == 0)
		{
			std::stringstream ss(line.erase(0, 2));
			ss >> x >> y >> z;
			_normVals.push_back(x);
			_normVals.push_back(y);
			_normVals.push_back(z);
		}
		if (line.compare(0, 1, "f") == 0)  //原书有误
		{
			string oneCorner, v, t, n;
			std::stringstream ss(line.erase(0, 2));
			while (getline(ss, oneCorner, ' '))
			{
				stringstream oneCornerSS(oneCorner);
				getline(oneCornerSS, v, '/');
				int vertRef = (stoi(v) - 1) * 3;
				_triangleVerts.push_back(_vertVals[vertRef]);
				_triangleVerts.push_back(_vertVals[vertRef + 1]);
				_triangleVerts.push_back(_vertVals[vertRef + 2]);
				if (getline(oneCornerSS, t, '/')) {
					int tcRef = (stoi(t) - 1) * 2;
					_textureCoords.push_back(_stVals[tcRef]);
					_textureCoords.push_back(_stVals[tcRef + 1]);
				}
				if (getline(oneCornerSS, n, '/')) {
					int normRef = (stoi(n) - 1) * 3;
					_normals.push_back(_normVals[normRef]);
					_normals.push_back(_normVals[normRef + 1]);
					_normals.push_back(_normVals[normRef + 2]);
				}
			}
		}
	}
	fileStream.close();
}

int ModelImporter::getNumVertices()
{
	return (_triangleVerts.size() / 3);
}

std::vector<float> ModelImporter::getVertices()
{
	return _triangleVerts;
}

std::vector<float> ModelImporter::getTextureCoordinates()
{
	return _textureCoords;
}

std::vector<float> ModelImporter::getNormals()
{
	return _normals;
}

