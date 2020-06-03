#include "ScriptingGameobject.h"
#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleInput.h"
#include "ModuleScripting.h"
#include "ModuleSceneManager.h"
#include "ResourceScene.h"
#include "GameObject.h"
#include "Components.h"
#include "ComponentScript.h"
#include "ScriptData.h"
#include "ModuleEventManager.h"

using namespace Broken;
ScriptingGameobject::ScriptingGameobject() {}

ScriptingGameobject::~ScriptingGameobject() {}

// Returns the UID of the GameObject if it is found
uint ScriptingGameobject::FindGameObject(const char* go_name)
{
	uint ret = 0;

	GameObject* go = App->scene_manager->currentScene->GetGOWithName(go_name);

	if (go != nullptr)
	{
		ret = go->GetUID();
	}
	else
	{
		ENGINE_CONSOLE_LOG("(SCRIPTING) Alert! Gameobject %s was not found! 0 will be returned", go_name);
	}
	return ret;
}

luabridge::LuaRef ScriptingGameobject::GetGOChilds(uint gameobject_UUID, lua_State* L) const
{
	luabridge::LuaRef table(L, luabridge::newTable(L));
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go != nullptr)
	{
		if (go->childs.size() > 0)
		{
			for (int i = 0; i < go->childs.size(); ++i)
			{
				uint sonID = go->childs[i]->GetUID();
				if (sonID != 0)
					table.append(sonID);
				else
					ENGINE_CONSOLE_LOG("![Script]: (GetGOChilds) Gameobject %d has a child that does not exist!", gameobject_UUID);
			}
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetGOChilds) Gameobject %d has no childs!", gameobject_UUID);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetGOChilds) Gameobject with UID %d does not exist!", gameobject_UUID);

	return table;
}

uint ScriptingGameobject::FindChildGameObject(const char* go_name)
{
	uint ret = 0;

	GameObject* go = App->scripting->current_script->my_component->GetContainerGameObject();

	if (go != nullptr)
	{
		for (int i = 0; go->childs.size(); ++i) {
			if (!strcmp(go->childs.at(i)->GetName(),go_name)) {
				ret = go->childs.at(i)->GetUID();
				break;
			}
		}

		if (ret == 0)
			ENGINE_CONSOLE_LOG("![Script]: (FindChildGameObject) Gameobject %s was not found! 0 will be returned", go_name);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (FindChildGameObject) Gameobject %s was not found! 0 will be returned", go_name);
	
	return ret;
}

uint ScriptingGameobject::FindChildGameObjectFromGO(const char* go_name, uint gameobject_UUID) {
	uint ret = 0;

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go != nullptr) {
		for (int i = 0; go->childs.size(); ++i) {
			if (!strcmp(go->childs.at(i)->GetName(), go_name)) {
				ret = go->childs.at(i)->GetUID();
				break;
			}
		}

		if (ret == 0)
			ENGINE_CONSOLE_LOG("![Script]: (FindChildGameObjectFromGO) Gameobject %s was not found in GO with UID %d! 0 will be returned", go_name, gameobject_UUID);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (FindChildGameObjectFromGO) Gameobject with UID %d does not exist!", gameobject_UUID);

	return ret;
}

uint ScriptingGameobject::GetMyUID()
{
	uint ret = 0;

	if(App->scripting->current_script != nullptr)
		ret = App->scripting->current_script->my_component->GetContainerGameObject()->GetUID();

	return ret;
}

uint ScriptingGameobject::GetScriptGOParent()
{
	uint ret = 0;
	GameObject* go = App->scripting->current_script->my_component->GetContainerGameObject()->parent;

	if (go && go->GetName() != App->scene_manager->GetRootGO()->GetName())
		ret = go->GetUID();
	else
		ENGINE_CONSOLE_LOG("(SCRIPTING) Alert! This Gameobject has no parent! 0 will be returned");

	return ret;
}

uint ScriptingGameobject::GetGOParentFromUID(uint gameobject_UUID)
{
	uint ret = 0;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go && go->GetName() != App->scene_manager->GetRootGO()->GetName())
	{
		if (go->parent && go->parent->GetName() != App->scene_manager->GetRootGO()->GetName())
			ret = go->parent->GetUID();
		else
			ENGINE_CONSOLE_LOG("(SCRIPTING) Alert! Gameobject with %d UUID has no parent! 0 will be returned", gameobject_UUID);
	}
	else
		ENGINE_CONSOLE_LOG("(SCRIPTING) Alert! Gameobject with %d UUID was not found! 0 will be returned", gameobject_UUID);

	return ret;
}

void ScriptingGameobject::DestroyGOFromScript(uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
		App->scene_manager->DestroyGameObject(go);
	else
		ENGINE_CONSOLE_LOG("(SCRIPTING) Alert! Gameobject with %d UUID does not exist!", gameobject_UUID);
}

void ScriptingGameobject::SetActiveGameObject(bool active, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go) {
		if (active)
			go->Enable();
		else
			go->Disable();
	}
	else
		ENGINE_CONSOLE_LOG("(SCRIPTING) Alert! Gameobject with %d UUID does not exist!", gameobject_UUID);
}
bool ScriptingGameobject::IsActiveGameObject(uint gameobject_UUID) {
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	bool ret = false;

	if (go)
		ret = go->GetActive();
	else
		ENGINE_CONSOLE_LOG("![Script]: (IsActiveGameObject): Gameobject with %d UUID does not exist!", gameobject_UUID);

	return ret;
}
uint ScriptingGameobject::GetComponentFromGO(const char* component_name, uint gameobject_UUID)
{
	uint ret = 0;
	GameObject* go = nullptr;
	go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go != nullptr)
	{
		Component* comp = nullptr;
		std::string name = component_name;

		if (name.compare("Animation") == 0)
			comp = (Component*)go->GetComponent<ComponentAnimation>();
		else if (name.compare("Audio") == 0)
			comp = (Component*)go->GetComponent<ComponentAudioSource>();
		else if (name.compare("Bone") == 0)
			comp = (Component*)go->GetComponent<ComponentBone>();
		else if (name.compare("Camera") == 0)
			comp = (Component*)go->GetComponent<ComponentCamera>();
		else if (name.compare("Collider") == 0)
			comp = (Component*)go->GetComponent<ComponentCollider>();
		else if (name.compare("Rigidbody") == 0)
			comp = (Component*)go->GetComponent<ComponentDynamicRigidBody>();
		else if (name.compare("Mesh") == 0)
			comp = (Component*)go->GetComponent<ComponentMesh>();
		else if (name.compare("Emitter") == 0)
			comp = (Component*)go->GetComponent<ComponentParticleEmitter>();
		else if (name.compare("Text") == 0)
			comp = (Component*)go->GetComponent<ComponentText>();
		else if (name.compare("Bar") == 0)
			comp = (Component*)go->GetComponent<ComponentProgressBar>();
		else if (name.compare("Button") == 0)
			comp = (Component*)go->GetComponent<ComponentButton>();
		else if (name.compare("Image") == 0)
			comp = (Component*)go->GetComponent<ComponentImage>();
		else if (name.compare("Canvas") == 0)
			comp = (Component*)go->GetComponent<ComponentCanvas>();

		if (comp != nullptr)
		{
			ret = comp->GetUID();
		}
		else
		{
			ENGINE_CONSOLE_LOG("(SCRIPTING) Alert! Component %s was not found inside Gameobject with UUID %d! 0 will be returned", component_name, gameobject_UUID);
		}
	}
	else
	{
		ENGINE_CONSOLE_LOG("(SCRIPTING) Alert! Gameobject with UUID %d was not found! 0 will be returned", gameobject_UUID);
	}

	return ret;
}

luabridge::LuaRef ScriptingGameobject::GetScript(uint gameobject_UUID, lua_State* L)
{
	luabridge::LuaRef ret = 0;

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go != nullptr)
	{
		ComponentScript* component_script = go->GetComponent<ComponentScript>();
		if (component_script != nullptr) {
			ScriptInstance* script = App->scripting->GetScriptInstanceFromComponent(component_script);

			ret = script->my_table_class;
		}
	}
	return ret;
}

int ScriptingGameobject::GetMyLayer()
{
	GameObject* body = App->scripting->current_script->my_component->GetContainerGameObject();
	if (body) {
		return body->GetLayer();
	}
	return -1;
}

int ScriptingGameobject::GetLayerByID(uint UID)
{
	GameObject* body = App->scene_manager->currentScene->GetGOWithUID(UID);
	if (body) {
		return body->GetLayer();
	}
	return -1;
}
