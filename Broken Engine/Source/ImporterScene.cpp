#include "ImporterScene.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"
#include "ModuleSceneManager.h"
#include "ModuleDetour.h"

#include "ResourceScene.h"
#include "GameObject.h"
#include "ImporterMeta.h"
#include "ResourceMeta.h"
#include "ResourceNavMesh.h"

#include "mmgr/mmgr.h"

using namespace Broken;
ImporterScene::ImporterScene() : Importer(Importer::ImporterType::Scene)
{

}

ImporterScene::~ImporterScene()
{

}

// MYTODO: Give some use to return type (bool) in all functions (if load fails log...)

Resource* ImporterScene::Import(ImportData& IData) const
{
	// --- Meta was deleted, just trigger a load with a new uid ---
	Resource* scene = Load(IData.path);

	return scene;
}

Resource* ImporterScene::Load(const char* path) const
{
	ResourceScene* scene = nullptr;

	// --- Load Scene file ---
	if (path) 
	{
		ImporterMeta* IMeta = App->resources->GetImporter<ImporterMeta>();
		ResourceMeta* meta = (ResourceMeta*)IMeta->Load(path);

		if (meta) 
		{
			scene = App->resources->scenes.find(meta->GetUID()) != App->resources->scenes.end() ? App->resources->scenes.find(meta->GetUID())->second : (ResourceScene*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SCENE, meta->GetOriginalFile(), meta->GetUID());
		}
		else 
		{
			scene = (ResourceScene*)App->resources->CreateResource(Resource::ResourceType::SCENE, path);
		}

	}

	return scene;
}

void ImporterScene::SaveSceneToFile(ResourceScene* scene) const
{
	// --- Save Scene/Model to file ---

	json file;

	for (std::unordered_map<uint, GameObject*>::iterator it = scene->NoStaticGameObjects.begin(); it != scene->NoStaticGameObjects.end(); ++it)
	{
		std::string string_uid = std::to_string((*it).second->GetUID());
		// --- Create GO Structure ---
		file[string_uid];
		file[string_uid]["Name"] = (*it).second->GetName();
		file[string_uid]["Active"] = (*it).second->GetActive();
		file[string_uid]["Static"] = (*it).second->Static;
		file[string_uid]["Index"] = (*it).second->index;
		file[string_uid]["Navigation Static"] = (*it).second->navigationStatic;
		file[string_uid]["Navigation Area"] = (*it).second->navigationArea;
		file[string_uid]["Index"] = (*it).second->index;

		if ((*it).second->parent != App->scene_manager->GetRootGO())
			file[string_uid]["Parent"] = std::to_string((*it).second->parent->GetUID());
		else
			file[string_uid]["Parent"] = "-1";


		file[string_uid]["Components"];

		for (uint i = 0; i < (*it).second->GetComponents().size(); ++i)
		{
			// --- Save Components to file ---
			file[string_uid]["Components"][std::to_string((uint)(*it).second->GetComponents()[i]->GetType())] = (*it).second->GetComponents()[i]->Save();
			file[string_uid]["Components"][std::to_string((uint)(*it).second->GetComponents()[i]->GetType())]["index"] = i;
			file[string_uid]["Components"][std::to_string((uint)(*it).second->GetComponents()[i]->GetType())]["UID"] = (*it).second->GetComponents()[i]->GetUID();
		}
	}

	for (std::unordered_map<uint, GameObject*>::iterator it = scene->StaticGameObjects.begin(); it != scene->StaticGameObjects.end(); ++it)
	{
		std::string string_uid = std::to_string((*it).second->GetUID());

		// --- Create GO Structure ---
		file[string_uid];
		file[string_uid]["Name"] = (*it).second->GetName();
		file[string_uid]["Active"] = (*it).second->GetActive();
		file[string_uid]["Static"] = (*it).second->Static;
		file[string_uid]["Navigation Static"] = (*it).second->navigationStatic;
		file[string_uid]["Navigation Area"] = (*it).second->navigationArea;
		file[string_uid]["Index"] = (*it).second->index;

		if ((*it).second->parent != App->scene_manager->GetRootGO())
			file[string_uid]["Parent"] = std::to_string((*it).second->parent->GetUID());
		else
			file[string_uid]["Parent"] = "-1";


		file[string_uid]["Components"];

		for (uint i = 0; i < (*it).second->GetComponents().size(); ++i)
		{
			// --- Save Components to file ---
			file[string_uid]["Components"][std::to_string((uint)(*it).second->GetComponents()[i]->GetType())] = (*it).second->GetComponents()[i]->Save();
			file[string_uid]["Components"][std::to_string((uint)(*it).second->GetComponents()[i]->GetType())]["index"] = i;
		}

	}

	if (scene == App->scene_manager->currentScene || scene == App->scene_manager->temporalScene) {
		json navdata = file["Navigation Data"];
		// --- Navigation Data --
		file["Navigation Data"]["agentRadius"] = App->detour->agentRadius;
		file["Navigation Data"]["agentHeight"] = App->detour->agentHeight;
		file["Navigation Data"]["maxSlope"] = App->detour->maxSlope;
		file["Navigation Data"]["stepHeight"] = App->detour->stepHeight;
		file["Navigation Data"]["voxelSize"] = App->detour->voxelSize;

		file["Navigation Data"]["voxelHeight"] = App->detour->voxelHeight;
		file["Navigation Data"]["regionMinSize"] = App->detour->regionMinSize;
		file["Navigation Data"]["regionMergeSize"] = App->detour->regionMergeSize;
		file["Navigation Data"]["edgeMaxLen"] = App->detour->edgeMaxLen;
		file["Navigation Data"]["edgeMaxError"] = App->detour->edgeMaxError;
		file["Navigation Data"]["vertsPerPoly"] = App->detour->vertsPerPoly;
		file["Navigation Data"]["detailSampleDist"] = App->detour->detailSampleDist;
		file["Navigation Data"]["detailSampleMaxError"] = App->detour->detailSampleMaxError;
		file["Navigation Data"]["buildTiledMesh"] = App->detour->buildTiledMesh;

		for (int i = 0; i < BE_DETOUR_TOTAL_AREAS; ++i) {
			file["Navigation Data"]["Areas"][i]["name"] = App->detour->areaNames[i];
			file["Navigation Data"]["Areas"][i]["cost"] = App->detour->areaCosts[i];
		}

		const ResourceNavMesh* c_navmesh = App->detour->getNavMeshResource();
		if (c_navmesh != nullptr)
			file["Navigation Data"]["navMeshUID"] = c_navmesh->GetUID();
	}

	// --- Serialize JSON to string ---
	std::string data;
	App->GetJLoader()->Serialize(file, data);

	// --- Finally Save to file ---
	char* buffer = (char*)data.data();
	uint size = data.length();

	App->fs->Save(scene->GetResourceFile(), buffer, size);
	scene->SetOriginalFile(scene->GetResourceFile());

	// --- Create meta ---
	ImporterMeta* IMeta = App->resources->GetImporter<ImporterMeta>();
	ResourceMeta* meta = (ResourceMeta*)App->resources->CreateResourceGivenUID(Resource::ResourceType::META, scene->GetResourceFile(), scene->GetUID());

	if (meta)
	{
		meta->Date = App->fs->GetLastModificationTime(scene->GetOriginalFile());
		IMeta->Save(meta);
	}
	else
		ENGINE_CONSOLE_LOG("|[error]: Could not load meta from: %s", scene->GetResourceFile());
}
