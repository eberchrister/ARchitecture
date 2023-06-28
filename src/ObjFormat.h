#include<vector>
#include<map>
#include<string>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>

class OBJModel
{
public:
	OBJModel();
	~OBJModel();

	void LoadFromFile(const char* filename);
	
	struct Position { float x, y, z; };
	struct Color { float r, g, b; };
	struct Normal { float x, y, z; };
	struct Face { int v, t, n; };

	// void LoadMaterialFile(const char* filename);
	bool StartWith(std::string& line, const char* text);

	std::vector<cv::Point3f> GetVertexData();
	std::vector<std::vector<Face>> GetFacesData();
	std::vector<Normal> GetNormalsData();
	std::vector<Position> GetTextureData();

	std::vector<cv::Point3f> mVertexData;
	std::vector<std::vector<Face>> mFacesData;
	std::vector<Normal> mNormalsData;
	std::vector<Position> mTextureData;

};