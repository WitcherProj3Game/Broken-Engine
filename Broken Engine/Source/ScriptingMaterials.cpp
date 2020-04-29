#include "ScriptingMaterials.h"

// -- Modules --
#include "Application.h"
#include "ModuleScripting.h"
#include "ModuleSceneManager.h"
#include "ModuleResourceManager.h"

// -- Components -
#include "GameObject.h"
#include "Components.h"
#include "ComponentMeshRenderer.h"
#include "ResourceMaterial.h"

#include "ScriptData.h"
#include "ResourceScene.h"


using namespace Broken;

// --------------------------- SETTERS ---------------------------
void ScriptingMaterials::SetTransparency(bool is_transparent, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				mat->has_transparencies = is_transparent;
			else
				ENGINE_CONSOLE_LOG("![Script]: (SetTransparency) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetTransparency) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetTransparency) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingMaterials::SetCulling(bool culling, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				mat->has_culling = culling;
			else
				ENGINE_CONSOLE_LOG("![Script]: (SetCulling) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetCulling) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetCulling) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingMaterials::SetShininess(float shininess, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				mat->m_Shininess = shininess;
			else
				ENGINE_CONSOLE_LOG("![Script]: (SetShininess) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetShininess) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetShininess) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingMaterials::SetTextureUsage(bool use_textures, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				mat->m_UseTexture = use_textures;
			else
				ENGINE_CONSOLE_LOG("![Script]: (SetTextureUsage) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetTextureUsage) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetTextureUsage) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingMaterials::SetAlpha(float alpha, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				mat->m_AmbientColor.w = alpha;
			else
				ENGINE_CONSOLE_LOG("![Script]: (SetAlpha) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetAlpha) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetAlpha) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingMaterials::SetColor(float r, float g, float b, float a, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
			{
				mat->m_AmbientColor = float4(r, g, b, 255.0f) / 255.0f;
				mat->m_AmbientColor.w = a;
			}
			else
				ENGINE_CONSOLE_LOG("![Script]: (SetColor) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetColor) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetColor) Could not find GameObject with UUID %d", gameobject_UUID);
}


// --------------------------- GETTERS ---------------------------
bool ScriptingMaterials::GetTransparency(uint gameobject_UUID) const
{
	bool ret = false;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				ret = mat->has_transparencies;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetTransparency) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetTransparency) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetTransparency) Could not find GameObject with UUID %d", gameobject_UUID);

	return ret;
}

bool ScriptingMaterials::GetCulling(uint gameobject_UUID) const
{
	bool ret = false;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				ret = mat->has_culling;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetCulling) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetCulling) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetCulling) Could not find GameObject with UUID %d", gameobject_UUID);

	return ret;
}

bool ScriptingMaterials::GetTextureUsage(uint gameobject_UUID) const
{
	bool ret = false;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				ret = mat->m_UseTexture;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetTextureUsage) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetTextureUsage) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetTextureUsage) Could not find GameObject with UUID %d", gameobject_UUID);

	return ret;
}

float ScriptingMaterials::GetShininess(uint gameobject_UUID) const
{
	float ret = -1.0f;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				ret = mat->m_Shininess;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetShininess) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetShininess) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetShininess) Could not find GameObject with UUID %d", gameobject_UUID);
	
	return ret;
}

float ScriptingMaterials::GetAlpha(uint gameobject_UUID) const
{
	float ret = -1.0f;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				ret = mat->m_AmbientColor.w;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetAlpha) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetAlpha) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetAlpha) Could not find GameObject with UUID %d", gameobject_UUID);

	return ret;
}

luabridge::LuaRef ScriptingMaterials::GetColor(uint gameobject_UUID, lua_State* L) const
{
	float4 color = float4(-1.0f, -1.0f, -1.0f, -1.0f);
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			ResourceMaterial* mat = mesh->material;
			if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
				color = mat->m_AmbientColor;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetColor) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetColor) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetColor) Could not find GameObject with UUID %d", gameobject_UUID);

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(color.x);
	table.append(color.y);
	table.append(color.z);
	table.append(color.w);

	return table;
}
