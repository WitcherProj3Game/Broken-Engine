#include "ScriptingScenes.h"
#include "Application.h"
#include "ModuleScripting.h"
#include "ModuleSceneManager.h"
#include "ModuleResourceManager.h"
#include "GameObject.h"
#include "ImporterModel.h"
#include "ScriptData.h"
#include "ResourceScene.h"
#include "ResourcePrefab.h"
#include "ResourceModel.h"
#include "ComponentTransform.h"
#include "ModuleTimeManager.h"
#include "ModulePhysics.h"

using namespace Broken;
ScriptingScenes::ScriptingScenes() {}

ScriptingScenes::~ScriptingScenes() {}

void ScriptingScenes::LoadSceneFromScript(uint scene_UUID)
{
	ResourceScene* scene = (ResourceScene*)App->resources->GetResource(scene_UUID, false);
	App->time->Gametime_clock.Stop();
	App->scene_manager->SetActiveScene(scene);
	App->physics->physAccumulatedTime = 0.0f;//Reset Physics
	App->time->Gametime_clock.Start();
}

void ScriptingScenes::QuitGame()
{
	if (App->isGame)
		App->scripting->scripting_update = UPDATE_STOP;
	else 
	{
		App->GetAppState() = Broken::AppState::TO_EDITOR;
	}
}

uint ScriptingScenes::Instantiate(uint resource_UUID, float x, float y, float z, float alpha, float beta, float gamma)
{
	uint ret = 0;
	Resource* prefab = App->resources->GetResource(resource_UUID);

	if (prefab) {
		GameObject* go = App->resources->GetImporter<ImporterModel>()->InstanceOnCurrentScene(prefab->GetResourceFile(), nullptr);
		go->GetComponent<ComponentTransform>()->SetPosition(x, y, z);
		go->GetComponent<ComponentTransform>()->SetRotation({ alpha, beta, gamma });
		go->GetComponent<ComponentTransform>()->updateValues = true;
		go->TransformGlobal();
		go->GetComponent<ComponentTransform>()->updateValues = false;

		ret = go->GetUID();
	}

	return ret;
}