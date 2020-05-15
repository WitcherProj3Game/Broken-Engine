#include "ResourceScene.h"
#include "Application.h"
#include "ModuleGui.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleSceneManager.h"
#include "ModuleEventManager.h"
#include "ModuleDetour.h"
#include "ModuleUI.h"
#include "ModuleScripting.h"
#include "ModulePhysics.h"
#include "ModuleRenderer3D.h"

#include "GameObject.h"

#include "ImporterScene.h"

#include "mmgr/mmgr.h"

using namespace Broken;
ResourceScene::ResourceScene(uint UID, const char* source_file) : Resource(Resource::ResourceType::SCENE, UID, source_file)
{
	extension = ".scene";
	resource_file = source_file;
	original_file = resource_file;

	previewTexID = App->gui->sceneTexID;

	octreeBox = AABB(float3(-100, -100, -100), float3(100, 100, 100));

}

ResourceScene::~ResourceScene()
{
	NoStaticGameObjects.clear();
	StaticGameObjects.clear();
}

bool ResourceScene::LoadInMemory()
{
	// --- Load scene game objects ---
	if (NoStaticGameObjects.size() == 0 && App->fs->Exists(resource_file.c_str()))
	{
		// --- Load Scene/model file ---
		json file = App->GetJLoader()->Load(resource_file.c_str());

		// --- Delete buffer data ---
		if (!file.is_null()) {

			//Load AABB of the scene octree
			bool treeSaved = false;

			//If there's no AABB saved, we'll store the indices of the static GOs here,
			//enclose all their AABBs to the octree, and them insert them into it
			std::vector<int> staticObjectsIndices;

			if (file["octreeBox"].is_null())
				octreeBox = AABB(float3(-100, -100, -100), float3(100, 100, 100));
			else
			{
				octreeBox = AABB(float3(file["octreeBox"]["minX"], file["octreeBox"]["minY"], file["octreeBox"]["minZ"]),
									float3(file["octreeBox"]["maxX"], file["octreeBox"]["maxY"], file["octreeBox"]["maxZ"]));

				App->scene_manager->tree.SetBoundaries(octreeBox);
				treeSaved = true;
			}

			std::vector<GameObject*> objects;

			// --- Iterate main nodes ---
			for (json::iterator it = file.begin(); it != file.end(); ++it)
			{
				// --- Retrieve GO's UID ---
				std::string uid = it.key().c_str();
				if (uid == "Navigation Data" || uid == "octreeBox" || uid == "SceneAmbientColor")
					continue;

				// --- Create a Game Object for each node ---
				GameObject* go = App->scene_manager->CreateEmptyGameObjectGivenUID(std::stoi(uid));

				// --- We are setting their parents later so we erase anything related to it ---
				go->index = -1;
				go->parent = nullptr;


				std::string name = file[it.key()]["Name"];
				go->SetName(name.c_str());

				if (!file[it.key()]["Active"].is_null())
					go->GetActive() = file[it.key()]["Active"];

				if (!file[it.key()]["Static"].is_null())
					go->Static = file[it.key()]["Static"];

				if (!file[it.key()]["Index"].is_null())
					go->index = file[it.key()]["Index"];

				if (!file[it.key()]["Navigation Static"].is_null())
					go->navigationStatic = file[it.key()]["Navigation Static"];

				if (!file[it.key()]["Navigation Area"].is_null())
					go->navigationArea = file[it.key()]["Navigation Area"];
				if (!file[it.key()]["PrefabChild"].is_null())
					go->is_prefab_child = file[it.key()]["PrefabChild"];

				if (!file[it.key()]["PrefabInstance"].is_null())
					go->is_prefab_instance = file[it.key()]["PrefabInstance"];



				if (!file[it.key()]["Model"].is_null())
				{
					Importer::ImportData IData(file[it.key()]["Model"].get<std::string>().c_str());
					go->model = (ResourceModel*)App->resources->ImportAssets(IData);
				}
				// --- Iterate components ---
				json components = file[it.key()]["Components"];

				for (json::iterator it2 = components.begin(); it2 != components.end(); ++it2) {
					// --- Determine ComponentType ---
					std::string type_string = it2.key();
					uint type_uint = std::stoi(type_string);
					Component::ComponentType type = (Component::ComponentType)type_uint;

					// --- and index ---
					int c_index = -1;
					json index = components[it2.key()]["index"];

					if (!index.is_null())
						c_index = index.get<uint>();

					Component* component = nullptr;

					// --- Create/Retrieve Component ---
					component = go->AddComponent(type, c_index);

					// --- Load Component Data ---
					if (component)
					{
						// NOTE: Commented this so components do not try to ask for a nonexistant go, we first create all gos and ask components to load later
						//component->Load(components[type_string]);

						// --- UID ---
						json c_UID = components[it2.key()]["UID"];

						if (!c_UID.is_null())
							component->SetUID(c_UID.get<uint>());
					}
				}

				objects.push_back(go);

				if (go->Static)
				{
					staticObjectsIndices.push_back(objects.size()-1); //store iterator
				}

				Event e;
				e.type = Event::EventType::GameObject_loaded;
				e.go = go;
				App->event_manager->PushEvent(e);
			}

			uint ite = 0;

			for (json::iterator it = file.begin(); it != file.end(); ++it)
			{
				// --- Iterate components ---
				json components = file[it.key()]["Components"];
				if (!components.is_null()) {
					std::vector<Component*>* go_components = &objects[ite]->GetComponents();

					for (json::iterator it2 = components.begin(); it2 != components.end(); ++it2)
					{
						// --- UID ---
						json c_UID = components[it2.key()]["UID"];
						int c_index = -1;
						json index = components[it2.key()]["index"];
						std::string type_string = it2.key();

						if (!index.is_null())
							c_index = index.get<uint>();

						go_components->at(c_index)->Load((components[type_string]));
					}
				}

				ite++;
			}

			//AKI CABESA AKI -- DIME HERMANO, DIME PACO
			if (!treeSaved)
			{
				//Enclose all the go AABBs into the box of the tree
				for (int i = 0; i < staticObjectsIndices.size(); i++) {
					if (!octreeBox.Intersects( objects[staticObjectsIndices[i]]->GetOBB().MinimalEnclosingAABB()))
						octreeBox.Enclose(objects[staticObjectsIndices[i]]->GetAABB());
				}

				App->scene_manager->tree.SetBoundaries(octreeBox);

			}

			for (int i = 0; i < staticObjectsIndices.size(); i++)
			{
				App->scene_manager->SetStatic(objects[staticObjectsIndices[i]], true, false);
			}

			///

			App->scene_manager->GetRootGO()->childs.clear();
			// --- Parent Game Objects / Build Hierarchy ---
			for (uint i = 0; i < objects.size(); ++i)
			{
				std::string parent_uid_string = file[std::to_string(objects[i]->GetUID())]["Parent"];
				int parent_uid = std::stoi(parent_uid_string);

				if (parent_uid > 0)
				{
					bool saved = false;
					for (uint j = 0; j < objects.size(); ++j) {
						if (parent_uid == objects[j]->GetUID()) {
							objects[j]->AddChildGO(objects[i], objects[i]->index);
							saved = true;
							break;
						}
					}
					// This is for compatibility with old scenes, yes, FML - Sergi
					if (!saved)
						App->scene_manager->GetRootGO()->AddChildGO(objects[i]);
				}
				else
					App->scene_manager->GetRootGO()->AddChildGO(objects[i], objects[i]->index);
			}
		}


		// Load navigation data
		json navigationdata = file["Navigation Data"];
		if (!navigationdata.is_null()) {
			App->detour->agentHeight = navigationdata["agentHeight"];
			App->detour->agentRadius = navigationdata["agentRadius"];
			App->detour->maxSlope = navigationdata["maxSlope"];
			App->detour->stepHeight = navigationdata["stepHeight"];
			App->detour->voxelSize = navigationdata["voxelSize"];

			App->detour->voxelHeight = navigationdata["voxelHeight"];
			App->detour->regionMinSize = navigationdata["regionMinSize"];
			App->detour->regionMergeSize = navigationdata["regionMergeSize"];
			App->detour->edgeMaxLen = navigationdata["edgeMaxLen"];
			App->detour->edgeMaxError = navigationdata["edgeMaxError"];
			App->detour->vertsPerPoly = navigationdata["vertsPerPoly"];
			App->detour->detailSampleDist = navigationdata["detailSampleDist"];
			App->detour->detailSampleMaxError = navigationdata["detailSampleMaxError"];
			App->detour->buildTiledMesh = navigationdata["buildTiledMesh"];


			if (!navigationdata["navMeshUID"].is_null())
				App->detour->loadNavMeshFile(navigationdata["navMeshUID"]);
			else
				App->detour->clearNavMesh();

			for (int i = 0; i < BE_DETOUR_TOTAL_AREAS; ++i) {
				std::string areaName = navigationdata["Areas"][i]["name"];
				sprintf_s(App->detour->areaNames[i], areaName.c_str());
				App->detour->areaCosts[i] = navigationdata["Areas"][i]["cost"];
			}
		}
		else {
			App->detour->setDefaultValues();
			App->detour->clearNavMesh();
		}


		// --- SCENE RENDERING DATA ---
		//Scene Color & Gamma Correction		
		m_SceneGammaCorrection = file.find("SceneGammaCorrection") == file.end() ? 1.0f : file["SceneGammaCorrection"].get<float>();
		float3 sceneColor = float3::one;
		if (file.find("SceneAmbientColor") != file.end())
		{
			json fileCol = file["SceneAmbientColor"];
			if (fileCol.find("R") != fileCol.end() && fileCol.find("G") != fileCol.end() && fileCol.find("B") != fileCol.end())
				sceneColor = float3(fileCol["R"].get<float>(), fileCol["G"].get<float>(), fileCol["B"].get<float>());
		}

		//m_SceneColor = sceneColor;
		App->renderer3D->SetAmbientColor(sceneColor);

		//Scene Blending
		bool autoBlend = file.find("SceneAutoBlend") == file.end() ? true : file["SceneAutoBlend"].get<bool>();
		BlendAutoFunction blendFunc = file.find("SceneBlendFunc") == file.end() ? BlendAutoFunction::STANDARD_INTERPOLATIVE : (BlendAutoFunction)file["SceneBlendFunc"].get<int>();
		BlendingEquations blendEq = file.find("SceneBlendEq") == file.end() ? BlendingEquations::ADD : (BlendingEquations)file["SceneBlendEq"].get<int>();
		BlendingTypes man_BlendSrc = file.find("SceneManualBlendFuncSrc") == file.end() ? BlendingTypes::SRC_ALPHA : (BlendingTypes)file["SceneManualBlendFuncSrc"].get<int>();
		BlendingTypes man_BlendDst = file.find("SceneManualBlendFuncDst") == file.end() ? BlendingTypes::ONE_MINUS_SRC_ALPHA : (BlendingTypes)file["SceneManualBlendFuncDst"].get<int>();

		App->renderer3D->SetRendererBlendingAutoFunction(blendFunc);
		App->renderer3D->SetRendererBlendingEquation(blendEq);
		App->renderer3D->SetRendererBlendingManualFunction(man_BlendSrc, man_BlendDst);
		App->renderer3D->m_AutomaticBlendingFunc = autoBlend;
	
		//Scene Skybox
		float skyboxExp = 1.0f;
		float3 skyboxColor = float3::one;
		float3 skyboxRot = float3::zero;

		if (file.find("Skybox") != file.end())
		{
			json fileSky = file["Skybox"];
			skyboxExp = fileSky.find("Exposure") == fileSky.end() ? 1.0f : fileSky["Exposure"].get<float>();

			if (fileSky.find("ColorTint") != fileSky.end())
			{
				json skyCol = file["ColorTint"];
				if (skyCol.find("R") != skyCol.end() && skyCol.find("G") != skyCol.end() && skyCol.find("B") != skyCol.end())
					skyboxColor = float3(skyCol["R"].get<float>(), skyCol["G"].get<float>(), skyCol["B"].get<float>());
			}

			if (fileSky.find("Rotation") != fileSky.end())
			{
				json skyRot = file["Rotation"];
				if (skyRot.find("X") != skyRot.end() && skyRot.find("Y") != skyRot.end() && skyRot.find("Z") != skyRot.end())
					skyboxRot = float3(skyRot["X"].get<float>(), skyRot["Y"].get<float>(), skyRot["Z"].get<float>());
			}
		}

		App->renderer3D->SetSkyboxExposure(skyboxExp);
		App->renderer3D->SetSkyboxColor(skyboxColor);
		App->renderer3D->SetSkyboxRotation(skyboxRot);

		//Scene Post-Pro
		bool useHDR = false;
		float HDRExp = 1.0f, PPGammaCorr = 1.0f;
		if (file.find("ScenePostProcessing") != file.end())
		{
			json filePP = file["ScenePostProcessing"];
			useHDR = filePP.find("HDRUsage") == filePP.end() ? 1.0f : filePP["HDRUsage"].get<bool>();
			HDRExp = filePP.find("HDRExposure") == filePP.end() ? 1.0f : filePP["HDRExposure"].get<float>();
			PPGammaCorr = filePP.find("PPGammaCorrection") == filePP.end() ? 1.0f : filePP["PPGammaCorrection"].get<float>();
		}

		App->renderer3D->m_UseHDR = useHDR;
		App->renderer3D->SetPostProHDRExposure(HDRExp);
		App->renderer3D->SetPostProGammaCorrection(PPGammaCorr);
	}

	return true;
}

void ResourceScene::FreeMemory() {
	// --- Delete all scene game objects ---
	for (std::unordered_map<uint, GameObject*>::iterator it = NoStaticGameObjects.begin(); it != NoStaticGameObjects.end(); ++it)
	{
		delete (*it).second;
	}

	NoStaticGameObjects.clear();

	for (std::unordered_map<uint, GameObject*>::iterator it = StaticGameObjects.begin(); it != StaticGameObjects.end(); ++it)
	{
		delete (*it).second;
	}

	StaticGameObjects.clear();
	Event e;
	e.type = Event::EventType::Scene_unloaded;
	App->event_manager->PushEvent(e);

	// Note that this will be called once we load another scene, and the octree will be cleared right after this
}

void ResourceScene::DeactivateAllGameObjects()
{
	for (std::unordered_map<uint, GameObject*>::iterator it = NoStaticGameObjects.begin(); it != NoStaticGameObjects.end(); ++it)
	{
		(*it).second->GetActive() = false;
	}

	for (std::unordered_map<uint, GameObject*>::iterator it = StaticGameObjects.begin(); it != StaticGameObjects.end(); ++it)
	{
		(*it).second->GetActive() = false;
	}
}

void ResourceScene::ActivateAllGameObjects()
{
	for (std::unordered_map<uint, GameObject*>::iterator it = NoStaticGameObjects.begin(); it != NoStaticGameObjects.end(); ++it)
	{
		(*it).second->GetActive() = true;
	}

	for (std::unordered_map<uint, GameObject*>::iterator it = StaticGameObjects.begin(); it != StaticGameObjects.end(); ++it)
	{
		(*it).second->GetActive() = false;
	}
}

// Created for on-play temporal scene
void ResourceScene::CopyInto(ResourceScene* target)
{
	if (target)
	{
		// --- Delete all scene game objects ---
		for (std::unordered_map<uint, GameObject*>::iterator it = NoStaticGameObjects.begin(); it != NoStaticGameObjects.end(); ++it)
		{
			target->NoStaticGameObjects[(*it).first] = (*it).second;
		}

		for (std::unordered_map<uint, GameObject*>::iterator it = StaticGameObjects.begin(); it != StaticGameObjects.end(); ++it)
		{
			target->StaticGameObjects[(*it).first] = (*it).second;
		}
	}
}

GameObject* ResourceScene::GetGOWithName(const char* GO_name)
{
	GameObject* ret = nullptr;

	std::string GO_stringname = GO_name;

	for (std::unordered_map<uint, GameObject*>::iterator it = NoStaticGameObjects.begin(); it != NoStaticGameObjects.end(); ++it)
	{
		std::string name = (*it).second->GetName();
		if (name.compare(GO_stringname) == 0)
		{
			ret = (*it).second;
			return ret;
		}
	}

	for (std::unordered_map<uint, GameObject*>::iterator it = StaticGameObjects.begin(); it != StaticGameObjects.end(); ++it)
	{
		std::string name = (*it).second->GetName();
		if (name.compare(GO_stringname) == 0)
		{
			ret = (*it).second;
			return ret;
		}
	}

	return ret;
}

//Return the GameObject with this UID
GameObject* ResourceScene::GetGOWithUID(uint UID)
{
	GameObject* ret_go = nullptr;
	std::unordered_map<uint, GameObject*>::iterator it;
	it = NoStaticGameObjects.find(UID);
	if (it != NoStaticGameObjects.end())
		ret_go = it->second;
	if (ret_go == nullptr) {
		it = StaticGameObjects.find(UID);
		if (it != StaticGameObjects.end())
			ret_go = it->second;
	}
	return ret_go;
}

void ResourceScene::OnOverwrite() {
	// No reason to overwrite scene
}

void ResourceScene::OnDelete()
{

	if (this->GetUID() == App->scene_manager->currentScene->GetUID()
		&& (this->GetUID() != App->scene_manager->defaultScene->GetUID()))
	{
		App->scene_manager->SetActiveScene(App->scene_manager->defaultScene);
	}

	FreeMemory();

	if (this->GetUID() == App->scene_manager->defaultScene->GetUID())
	{
		App->physics->DeleteActors();

		// --- Reset octree ---
		App->scene_manager->tree.SetBoundaries(AABB(float3(-100, -100, -100), float3(100, 100, 100)));

		// --- Release current scene ---
		Release();
		App->scripting->CleanUpInstances();

		// --- Clear root ---
		App->scene_manager->GetRootGO()->childs.clear();

		App->ui_system->Clear();
	}

	App->fs->Remove(resource_file.c_str());

	App->resources->RemoveResourceFromFolder(this);

	if (this->GetUID() != App->scene_manager->defaultScene->GetUID())
		App->resources->ONResourceDestroyed(this);
	else
		LoadToMemory();
}
