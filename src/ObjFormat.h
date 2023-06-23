#include<vector>
#include<map>
#include<string>

class OBJModel
{
public:
	OBJModel();
	~OBJModel();

	void LoadFromFile(const char* filename);
	
	struct Position { float x, y, z; };
	struct Color { float r, g, b; };
	struct Normal { float x, y, z; };
	struct Face { int v1, t1, n1, v2, t2, n2, v3, t3, n3;};

	// void LoadMaterialFile(const char* filename);
	bool StartWith(std::string& line, const char* text);

	std::vector<Position> GetVertexData();
	std::vector<Face> GetFacesData();
	std::vector<Normal> GetNormalsData();
	std::vector<Position> GetTextureData();

	std::vector<Position> mVertexData;
	std::vector<Face> mFacesData;
	std::vector<Normal> mNormalsData;
	std::vector<Position> mTextureData;

};