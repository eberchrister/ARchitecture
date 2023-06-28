#include "ObjFormat.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>

OBJModel::OBJModel() {

}
OBJModel::~OBJModel() {

}


void OBJModel::LoadFromFile(const char* filename) {
	std::vector<cv::Point3f> vertices;
	std::vector<Position> texcoords;
	std::vector<Normal> normals;
	std::vector<std::vector<Face>> faces;
	
	std::ifstream file(filename);

	std::cout << "Start opening file" << std::endl;
	std::cout << "filename: " << filename << std::endl;
	if (file) {
		std::cout << "Start reading file" << std::endl;
		std::string line;
		while (std::getline(file, line)) {
			//std::cout << "Current line: " << line << std::endl;
			if (StartWith(line, "v ")) {
				{
					cv::Point3f pos;
					sscanf_s(line.c_str(), "v %f %f %f", &pos.x, &pos.y, &pos.z);
					vertices.push_back(pos);
					mVertexData.push_back(pos);
				}
			}
			if (StartWith(line, "vt ")) {
				{	
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
					std::istringstream iss(line);
					std::string token;
					std::vector<Face> line_faces;
					int i = 0;
					while (std::getline(iss, token, ' ')){
						Face face;
						(void)sscanf_s(token.c_str(), "%d/%d/%d", &face.v, &face.t, &face.n);

						// the first element is the line f-descriptor
						if (i != 0) {
							line_faces.push_back(face);
						}
						i++;
					}
					
					faces.push_back(line_faces);
					mFacesData.push_back(line_faces);
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

std::vector<cv::Point3f> OBJModel::GetVertexData() {
	return mVertexData;
}

std::vector<std::vector<OBJModel::Face>> OBJModel::GetFacesData() {
	return mFacesData;
}

std::vector<OBJModel::Normal> OBJModel::GetNormalsData() {
	return mNormalsData;
}

std::vector<OBJModel::Position> OBJModel::GetTextureData() {
	return mTextureData;
}
