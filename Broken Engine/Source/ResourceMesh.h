#ifndef __RESOURCE_MESH__
#define __RESOURCE_MESH__

#include "Resource.h"
#include "MathGeoLib/include/Geometry/AABB.h"
#include "MathGeoLib/include/Geometry/OBB.h"
#include <mutex>

BE_BEGIN_NAMESPACE
struct BROKEN_API Vertex
{
	float position[3];
	float normal[3];
	float smoothNormal[3];
	unsigned char color[4];
	float texCoord[2];
	float tangent[3];
	float biTangent[3];
};

class BROKEN_API ResourceMesh : public Resource
{
public:

	ResourceMesh(uint UID, const char* source_file);
	~ResourceMesh();

	void CreateAABB();
	void CreateOBB();

	bool LoadInMemory() override;
	void FreeMemory() override;

	std::string previewTexPath;
	void CreateVAO();
	void CreateVBO();
	void CreateEBO();

	void CalculateSmoothNormals();
private:

public:

	AABB aabb;
	OBB obb;

	Vertex* vertices = nullptr;
	uint VerticesSize = 0;
	uint* Indices = nullptr;
	uint IndicesSize = 0;

	// --- New shader approach ---

	uint VBO = 0;	// Vertex Buffer Object
	uint EBO = 0;	// Element/Index Buffer Object

	uint VAO = 0;	// Vertex Array Object

	bool smoothNormals = false;

private:

	void OnOverwrite() override;
	void OnDelete() override;
	void Repath() override;

	void _CalculateFaceNormals(float from, float to, std::vector<std::vector<float3>> &wnormals);
	void _CalculateVertexSmooth(float from, float to, const std::vector<std::vector<float3>> &wnormals);
	std::mutex v_mutex;
};

BE_END_NAMESPACE
#endif // __RESOURCE_MESH__

