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
	struct Face3 { int v1, t1, n1, v2, t2, n2, v3, t3, n3;};
	struct Face4 { int v1, t1, n1, v2, t2, n2, v3, t3, n3, v4, t4, n4; };

	// void LoadMaterialFile(const char* filename);
	bool StartWith(std::string& line, const char* text);

	std::vector<Position> GetVertexData();
	std::vector<Face4> GetFacesData();
	std::vector<Normal> GetNormalsData();
	std::vector<Position> GetTextureData();

	std::vector<Position> mVertexData;
	std::vector<Face4> mFacesData;
	std::vector<Normal> mNormalsData;
	std::vector<Position> mTextureData;

};