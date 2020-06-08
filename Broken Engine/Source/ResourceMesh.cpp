#include "ResourceMesh.h"
#include "Application.h"
#include "ModuleGui.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleThreading.h"

#include "ImporterMesh.h"

#include "OpenGL.h"

#include "mmgr/mmgr.h"

using namespace Broken;
ResourceMesh::ResourceMesh(uint UID, const char* source_file) : Resource(Resource::ResourceType::MESH, UID, source_file)
{
	extension = ".mesh";
	resource_file = MESHES_FOLDER + std::to_string(UID) + extension;

	previewTexID = App->gui->meshTexID;
}

ResourceMesh::~ResourceMesh() 
{
	//glDeleteTextures(1, (GLuint*)&previewTexID);
}

void ResourceMesh::CreateAABB() 
{
	aabb.SetNegativeInfinity();

	for (uint i = 0; i < VerticesSize; ++i)
		aabb.Enclose(float3(vertices[i].position));
}

void ResourceMesh::CreateOBB() {
	//obb.SetNegativeInfinity();

	std::vector<vec> vec_array;
	for (uint i = 0; i < VerticesSize; ++i)
		vec_array.push_back(vec(vertices[i].position));

	obb = OBB::OptimalEnclosingOBB(vec_array.data(), VerticesSize);


}

bool ResourceMesh::LoadInMemory()
{
	bool ret = true;

	
	if (App->fs->Exists(resource_file.c_str()))
	{
		// --- Load mesh data ---
		char* buffer = nullptr;
		App->fs->Load(resource_file.c_str(), &buffer);
		char* cursor = buffer;

		// amount of indices / vertices / normals / texture_coords
		uint ranges[3];
		uint bytes = sizeof(ranges);
		memcpy(ranges, cursor, bytes);
		bytes += ranges[0];

		IndicesSize = ranges[1];
		VerticesSize = ranges[2];

		vertices = new Vertex[VerticesSize];
		float* Vertices = new float[VerticesSize * 3];
		float* Normals = new float[VerticesSize * 3];
		unsigned char* Colors = new unsigned char[VerticesSize * 4];
		float* TexCoords = new float[VerticesSize * 2];

		float* Tangents = new float[VerticesSize * 3];
		float* BiTangents = new float[VerticesSize * 3];

		// --- Load indices ---
		cursor += bytes;
		bytes = sizeof(uint) * IndicesSize;
		Indices = new uint[IndicesSize];
		memcpy(Indices, cursor, bytes);

		// --- Load Vertices ---
		cursor += bytes;
		bytes = sizeof(float) * 3 * VerticesSize;
		memcpy(Vertices, cursor, bytes);

		// --- Load Normals ---
		cursor += bytes;
		bytes = sizeof(float) * 3 * VerticesSize;
		memcpy(Normals, cursor, bytes);

		// --- Load Colors ---
		cursor += bytes;
		bytes = sizeof(unsigned char) * 4 * VerticesSize;
		memcpy(Colors, cursor, bytes);

		// --- Load Texture Coords ---
		cursor += bytes;
		bytes = sizeof(float) * 2 * VerticesSize;
		memcpy(TexCoords, cursor, bytes);

		// --- Load Tangents ---
		cursor += bytes;
		bytes = sizeof(float) * 3 * VerticesSize;
		memcpy(Tangents, cursor, bytes);

		// --- Load BiTangents ---
		cursor += bytes;
		bytes = sizeof(float) * 3 * VerticesSize;
		memcpy(BiTangents, cursor, bytes);


		// --- Fill Vertex array ---

		for (uint i = 0; i < VerticesSize; ++i)
		{
			// --- Vertices ---
			vertices[i].position[0] = Vertices[i * 3];
			vertices[i].position[1] = Vertices[(i * 3) + 1];
			vertices[i].position[2] = Vertices[(i * 3) + 2];

			// --- Normals ---
			vertices[i].normal[0] = Normals[i * 3];
			vertices[i].normal[1] = Normals[(i * 3) + 1];
			vertices[i].normal[2] = Normals[(i * 3) + 2];

			// --- Colors ---
			vertices[i].color[0] = Colors[i * 4];
			vertices[i].color[1] = Colors[(i * 4) + 1];
			vertices[i].color[2] = Colors[(i * 4) + 2];
			vertices[i].color[3] = Colors[(i * 4) + 3];

			// --- Texture Coordinates ---
			vertices[i].texCoord[0] = TexCoords[i * 2];
			vertices[i].texCoord[1] = TexCoords[(i * 2) + 1];

			// --- Tangents ---
			vertices[i].tangent[0] = Tangents[i * 3];
			vertices[i].tangent[1] = Tangents[(i * 3) + 1];
			vertices[i].tangent[2] = Tangents[(i * 3) + 2];

			// --- Biitangents ---
			vertices[i].biTangent[0] = BiTangents[i * 3];
			vertices[i].biTangent[1] = BiTangents[(i * 3) + 1];
			vertices[i].biTangent[2] = BiTangents[(i * 3) + 2];
		}

		// --- Delete buffer data ---
		if (buffer)
		{
			delete[] buffer;
			buffer = nullptr;
			cursor = nullptr;
		}

		delete[] Vertices;
		delete[] Normals;
		delete[] Colors;
		delete[] TexCoords;
		delete[] Tangents;
		delete[] BiTangents;
	}

	CreateAABB();
	//CreateOBB();
	CreateVBO();
	CreateEBO();
	CreateVAO();

	return ret;
}

void ResourceMesh::FreeMemory()
{
	
	glDeleteBuffers(1, (GLuint*)&VBO);

	glDeleteBuffers(1, (GLuint*)&EBO);
	glDeleteVertexArrays(1, (GLuint*)&VAO);

	if (vertices)
	{
		delete[] vertices;
		vertices = nullptr;
	}

	if (Indices)
	{
		delete[] Indices;
		Indices = nullptr;
	}

	VBO = 0;
	EBO = 0;
	VAO = 0;
}

void ResourceMesh::CreateVBO()
{
	if (vertices != nullptr)
	{
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * VerticesSize, vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else
		ENGINE_CONSOLE_LOG("|[error]: Could not create VBO, null vertices");
}

void ResourceMesh::CreateEBO()
{
	if (Indices != nullptr)
	{
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * IndicesSize, Indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
	else
		ENGINE_CONSOLE_LOG("|[error]: Could not create EBO, null indices");
}

void ResourceMesh::CreateVAO()
{
	// --- Create a Vertex Array Object ---
	glGenVertexArrays(1, &VAO);
	// --- Bind it ---
	glBindVertexArray(VAO);

	// Bind the VBO 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// --- Set all vertex attribute pointers ---

	// --- Vertex Position ---
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glEnableVertexAttribArray(0);

	// --- Vertex Normal ---
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(1);

	// --- Vertex Color ---
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(offsetof(Vertex, color)));
	glEnableVertexAttribArray(2);

	// --- Vertex Texture coordinates ---
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
	glEnableVertexAttribArray(3);

	// --- Vertex Tangents ---
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tangent)));
	glEnableVertexAttribArray(4);

	// --- Vertex Bitangents ---
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, biTangent)));
	glEnableVertexAttribArray(5);

	// --- Vertex Smooth Normals ---
	glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, smoothNormal)));
	glEnableVertexAttribArray(6);

	// --- Unbind VAO and VBO ---
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ResourceMesh::OnOverwrite()
{
	// Since mesh is not a standalone resource (which means it is always owned by a model) the model is in charge
	// of overwriting it (see ResourceModel OnOverwrite for details)
	NotifyUsers(ResourceNotificationType::Overwrite);
}

void ResourceMesh::OnDelete() 
{
	NotifyUsers(ResourceNotificationType::Deletion);

	FreeMemory();

	if(App->fs->Exists(resource_file.c_str()))
		App->fs->Remove(resource_file.c_str());
	App->fs->Remove(previewTexPath.c_str());

	App->resources->RemoveResourceFromFolder(this);
	App->resources->ONResourceDestroyed(this);
}

void ResourceMesh::Repath() 
{
	ImporterMesh* IMesh = App->resources->GetImporter<ImporterMesh>();
	IMesh->Save(this);
}

void ResourceMesh::CalculateSmoothNormals()
{
	std::vector<std::vector<float3>> wnormals;
	wnormals.resize(VerticesSize);

	float tricount = IndicesSize / 3; //Each triangle uses up 3 indices
	uint nthreads = App->threading->GetConcurrentThreads();
	int tris_per_batch = int(tricount / nthreads);

	// We send the triangle batches to the other threads to calculate it parallel
	for (int i = 0; i < nthreads - 1; ++i)
		App->threading->ADDTASK(this, ResourceMesh::_CalculateFaceNormals, tris_per_batch * i, tris_per_batch * (i + 1), wnormals);
	App->threading->ADDTASK(this, ResourceMesh::_CalculateFaceNormals, tris_per_batch * (nthreads - 1), tricount, wnormals);

	App->threading->FinishProcessing(); // We wait for the triangle normals to be all calculated

	// We calculate the amount of vertices per batch
	int vertices_per_batch = int(VerticesSize / nthreads);

	// We send the vertices batches to calculate
	for (int i = 0; i < nthreads - 1; ++i)
		App->threading->ADDTASK(this, ResourceMesh::_CalculateVertexSmooth, vertices_per_batch * i, vertices_per_batch * (i + 1), wnormals);
	App->threading->ADDTASK(this, ResourceMesh::_CalculateVertexSmooth, vertices_per_batch * (nthreads - 1), VerticesSize, wnormals);

	App->threading->FinishProcessing(); // We wait for all the smooth normals to be calculated
	smoothNormals = true;
}

void ResourceMesh::_CalculateFaceNormals(float from, float to, std::vector<std::vector<float3>> &wnormals)
{
	for (int f = from; f < to; f++)
	{
		// p1, p2 and p3 are the points in the face (f)
		float3 p1, p2, p3;
		p1 = {vertices[Indices[f * 3]].normal[0], vertices[Indices[f * 3]].normal[1], vertices[Indices[f * 3]].normal[2]};
		p2 = {vertices[Indices[f * 3 + 1]].normal[0], vertices[Indices[f * 3 + 1]].normal[1], vertices[Indices[f * 3 + 1]].normal[2]};
		p3 = {vertices[Indices[f * 3 + 2]].normal[0], vertices[Indices[f * 3 + 2]].normal[1], vertices[Indices[f * 3 + 2]].normal[2]};

		// calculate facet normal of the triangle using cross product;
		// both components are "normalized" against a common point chosen as the base
		float3 n = (p2 - p1).Cross(p3 - p1); // p1 is the 'base' here

		// get the angle between the two other points for each point;
		// the starting point will be the 'base' and the two adjacent points will be normalized against it
		float a1 = (p2 - p1).AngleBetween(p3 - p1); // p1 is the 'base' here
		float a2 = (p3 - p2).AngleBetween(p1 - p2); // p2 is the 'base' here
		float a3 = (p1 - p3).AngleBetween(p2 - p3); // p3 is the 'base' here

		// normalize the initial facet normals if you want to ignore surface area
		n.Normalize();

		v_mutex.lock();
		// store the weighted normal in an structured array
		wnormals[Indices[f * 3]].push_back(n * a1);
		wnormals[Indices[f * 3 + 1]].push_back(n * a2);
		wnormals[Indices[f * 3 + 2]].push_back(n * a3);
		v_mutex.unlock();
	}
}

void ResourceMesh::_CalculateVertexSmooth(float from, float to, const std::vector<std::vector<float3>> &wnormals)
{
	for (int v = from; v < to; v++)
	{
		float3 N;

		// run through the normals in each vertex's array and interpolate them
		// vertex(v) here fetches the data of the vertex at index 'v'
		for (int n = 0; n < wnormals[v].size(); v++)
		{
			N += wnormals[v].at(n);
		}

		// normalize the final normal
		N.Normalize();
		vertices[v].smoothNormal[0] = N.x;
		vertices[v].smoothNormal[1] = N.y;
		vertices[v].smoothNormal[2] = N.z;
	}
}

