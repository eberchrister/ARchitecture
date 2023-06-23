#include "ObjFormat.h"

#include <iostream>
#include <fstream>
#include <cstring>


OBJModel::OBJModel() {

}
OBJModel::~OBJModel() {

}


void OBJModel::LoadFromFile(const char* filename) {
	std::vector<Position> vertices;
	std::vector<Position> texcoords;
	std::vector<Normal> normals;
	std::vector<Face> faces;
	
	std::ifstream file(filename);

	std::cout << "Start opening file" << std::endl;
	std::cout << "filename: " << filename << std::endl;
	if (file) {
		std::cout << "Start reading file" << std::endl;
		std::string line;
		while (std::getline(file, line)) {
			std::cout << "Current line: " << line << std::endl;
			if (StartWith(line, "v ")) {
				{
					Position pos;
					sscanf_s(line.c_str(), "v %f %f %f", &pos.x, &pos.y, &pos.z);
					vertices.push_back(pos);
					mVertexData.push_back(pos);
				}
			}
			if (StartWith(line, "vt ")) {
				{	
					// use a different struct
					Position pos;
					sscanf_s(line.c_str(), "vt %f %f", &pos.x, &pos.y);
					texcoords.push_back(pos);
					mTextureData.push_back(pos);
				}
			}
			if (StartWith(line, "vn ")) {
				{
					Normal n;
					sscanf_s(line.c_str(), "vn %f %f %f", &n.x, &n.y, &n.z);
					normals.push_back(n);
					mNormalsData.push_back(n);
				}
			}
			if (StartWith(line, "f ")) {
				{
					Face f;
					// int v1, v2, v3;
					// int n1, n2, n3;
					// int t1, t2, t3;

					(void)sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &f.v1, &f.t1, &f.n1, &f.v2, &f.t2, &f.n2, &f.v3, &f.t3, &f.n3);
					faces.push_back(f);
					mFacesData.push_back(f);
				}
			}
		}
	}
	else {
		std::cout << "OBJ file loading failed" << std::endl;
	}

}

bool OBJModel::StartWith(std::string& line, const char* text) {
	size_t textLen = strlen(text);
	if (line.size() < textLen) {
		return false;
	}
	for (size_t i = 0; i < textLen; i++) {
		if (line[i] == text[i]) continue;
		else return false;
	}
	return true;
}

std::vector<OBJModel::Position> OBJModel::GetVertexData() {
	return mVertexData;
}

std::vector<OBJModel::Face> OBJModel::GetFacesData() {
	return mFacesData;
}

std::vector<OBJModel::Normal> OBJModel::GetNormalsData() {
	return mNormalsData;
}

std::vector<OBJModel::Position> OBJModel::GetTextureData() {
	return mTextureData;
}
