#include "ModuleMeshImporter.h"
#include "ModuleTextures.h"
#include "ComponentMesh.h"
#include "OpenGL.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"
#include "Assimp/include/cfileio.h"

#pragma comment (lib, "Assimp/libx86/assimp.lib")

#include "mmgr/mmgr.h"

void MyAssimpCallback(const char * msg, char * userData)
{
	LOG("[Assimp]: %s", msg);
}

ModuleMeshImporter::ModuleMeshImporter(bool start_enabled) : Module(start_enabled)
{
}

ModuleMeshImporter::~ModuleMeshImporter()
{
}

bool ModuleMeshImporter::Init(json file)
{
	// --- Stream LOG messages to MyAssimpCallback, that sends them to console ---
	struct aiLogStream stream;
	stream.callback = MyAssimpCallback;
	aiAttachLogStream(&stream);

	return true;
}

bool ModuleMeshImporter::Start()
{
	LoadFBX("Assets/BakerHouse.fbx");

	return true;
}


bool ModuleMeshImporter::CleanUp()
{
	// --- Detach assimp log stream ---
	aiDetachAllLogStreams();

	// -- Release all buffer data and own stored data ---
	for (uint i = 0; i < meshes.size(); ++i)
	{
		delete meshes[i];
	}


	return true;
}

bool ModuleMeshImporter::LoadFBX(const char* path)
{
	// --- Import scene from path ---
	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);

	// --- Get Directory from filename ---
	std::string directory = path;
	App->fs->GetDirectoryFromPath(directory);

	// --- Query Scene first Material to get Texture name. If found, create a texture and return ID  ---
	uint TextureID = 0;
	GetTextureIDFromSceneMaterial(*scene, TextureID, directory);

	if (scene != nullptr && scene->HasMeshes())
	{
		// --- Use scene->mNumMeshes to iterate on scene->mMeshes array ---

		for (uint i = 0; i < scene->mNumMeshes; ++i)
		{
			// --- Create new Resource mesh to store current scene mesh data ---
			ComponentMesh* new_mesh = new ComponentMesh;
			meshes.push_back(new_mesh);

			// --- Get Scene mesh number i ---
			aiMesh* mesh = scene->mMeshes[i];

			// --- Import mesh data (fill new_mesh)---
			new_mesh->ImportMesh(mesh,TextureID);
		}

		// --- Free scene ---
		aiReleaseImport(scene);

	}
	else
		LOG("|[error]: Error loading scene %s", path);


	return true;
}


void ModuleMeshImporter::Draw()
{
	// --- Activate wireframe mode ---

	if (App->renderer3D->wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	for (uint i = 0; i < meshes.size(); ++i)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, meshes[i]->TextureID);
		glActiveTexture(GL_TEXTURE0);

		glBindBuffer(GL_ARRAY_BUFFER, meshes[i]->TextureCoordsID);
		glTexCoordPointer(2, GL_FLOAT, 0, NULL);
		
		// --- Draw mesh ---
		glEnableClientState(GL_VERTEX_ARRAY); // enable client-side capability

		glBindBuffer(GL_ARRAY_BUFFER, meshes[i]->VerticesID); // start using created buffer (vertices)
		glVertexPointer(3, GL_FLOAT, 0, NULL); // Use selected buffer as vertices 

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[i]->IndicesID); // start using created buffer (indices)
		glDrawElements(GL_TRIANGLES, meshes[i]->IndicesSize, GL_UNSIGNED_INT, NULL); // render primitives from array data

		glBindBuffer(GL_ARRAY_BUFFER, 0); // Stop using buffer (vertices)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Stop using buffer (indices)

		glDisableClientState(GL_VERTEX_ARRAY); // disable client-side capability

		// ----        ----

		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// --- Draw Vertex Normals ---
		//if (meshes[i]->Normals)
		//{
		//	glBegin(GL_LINES);
		//	glLineWidth(1.0f);
		//	uint Normal_length = 1;

		//	glColor4f(0.0f, 0.5f, 0.5f, 1.0f);

		//	for (uint j = 0; j < meshes[i]->VerticesSize; ++j)
		//	{
		//		glVertex3f(meshes[i]->Vertices[j].x, meshes[i]->Vertices[j].y, meshes[i]->Vertices[j].z);
		//		glVertex3f(meshes[i]->Vertices[j].x + meshes[i]->Normals[j].x*Normal_length, meshes[i]->Vertices[j].y + meshes[i]->Normals[j].y*Normal_length, meshes[i]->Vertices[j].z + meshes[i]->Normals[j].z*Normal_length);
		//	}

		//	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		//	glEnd();

		//}

		// --- Draw Face Normals 

		glBegin(GL_LINES);
		glLineWidth(1.0f);
		uint Normal_length = 1;

		glColor4f(0.0f, 0.5f, 0.5f, 1.0f);
		Triangle face;

		for (uint j = 0; j < meshes[i]->VerticesSize/3; ++j)
		{
			face.a = meshes[i]->Vertices[(j*3)+2];
			face.b = meshes[i]->Vertices[(j*3) + 1];
			face.c = meshes[i]->Vertices[(j*3) ];

			float3 face_center = face.Centroid();
			float3 face_normal = face.NormalCW();

			glVertex3f(face_center.x, face_center.y, face_center.z);
			glVertex3f(face_center.x + face_normal.x*Normal_length, face_center.y + face_normal.y*Normal_length, face_center.z + face_normal.z*Normal_length);
		}

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		glEnd();



	}

	// --- DeActivate wireframe mode ---
	if (App->renderer3D->wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

uint ModuleMeshImporter::GetNumMeshes() const
{
	return meshes.size();
}

void ModuleMeshImporter::GetTextureIDFromSceneMaterial(const aiScene & scene, uint & texture_ID, std::string & directory)
{
	if (scene.HasMaterials())
	{
		// --- Get scene's first material ---
		aiMaterial* material = scene.mMaterials[0];

		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString Texture_path;

			// --- Specify type of texture to retrieve (in this case DIFFUSE/ALBEDO)---
			material->GetTexture(aiTextureType_DIFFUSE, 0, &Texture_path);

			// --- Build whole path to texture file ---
			directory.append(Texture_path.C_Str());

			// --- If we find the texture file, load it ---
			if (App->fs->Exists(directory.data()))
				texture_ID = App->textures->CreateTextureFromFile(directory.data());

		}
	}
}

