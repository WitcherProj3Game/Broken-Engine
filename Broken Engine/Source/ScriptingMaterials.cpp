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
				ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTransparent) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTransparent) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTransparent) Could not find GameObject with UUID %d", gameobject_UUID);
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
				ENGINE_CONSOLE_LOG("![Script]: (SetMaterialCulling) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetMaterialCulling) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialCulling) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingMaterials::SetShininess(float shininess, uint gameobject_UUID)
{
	if (shininess < 1.0f || shininess > 500.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialShininess) Shininess has to be between [1.0, 500.0]");
		return;
	}

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
				ENGINE_CONSOLE_LOG("![Script]: (SetMaterialShininess) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetMaterialShininess) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialShininess) Could not find GameObject with UUID %d", gameobject_UUID);
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
				ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTextureUsage) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTextureUsage) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTextureUsage) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingMaterials::SetAlpha(float alpha, uint gameobject_UUID)
{
	if (alpha > 1.0f || alpha < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetAlpha) Alpha has to be between [0.0, 1.0]");
		return;
	}

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
				ENGINE_CONSOLE_LOG("![Script]: (SetMaterialAlpha) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetMaterialAlpha) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialAlpha) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingMaterials::SetColor(float r, float g, float b, float a, uint gameobject_UUID)
{
	if (r > 255.0f || g > 255.0f || b > 255.0f || a > 1.0f || r < 0.0f || g < 0.0f || b < 0.0f || a < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialColor) Color has to be between [0.0, 255.0] and alpha between [0.0, 1.0]");
		return;
	}

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
				ENGINE_CONSOLE_LOG("![Script]: (SetMaterialColor) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetMaterialColor) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialColor) Could not find GameObject with UUID %d", gameobject_UUID);
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
				ENGINE_CONSOLE_LOG("![Script]: (GetMaterialTransparency) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetMaterialTransparency) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetMaterialTransparency) Could not find GameObject with UUID %d", gameobject_UUID);

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
				ENGINE_CONSOLE_LOG("![Script]: (GetMaterialCulling) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetMaterialCulling) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetMaterialCulling) Could not find GameObject with UUID %d", gameobject_UUID);

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
				ENGINE_CONSOLE_LOG("![Script]: (GetMaterialShininess) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetMaterialShininess) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetMaterialShininess) Could not find GameObject with UUID %d", gameobject_UUID);
	
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
				ENGINE_CONSOLE_LOG("![Script]: (GetMaterialAlpha) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetMaterialAlpha) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetMaterialAlpha) Could not find GameObject with UUID %d", gameobject_UUID);

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
				ENGINE_CONSOLE_LOG("![Script]: (GetMaterialColor) Mesh material is default or null");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetMaterialColor) Game Object has no mesh (or null)");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetMaterialColor) Could not find GameObject with UUID %d", gameobject_UUID);

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(color.x);
	table.append(color.y);
	table.append(color.z);
	table.append(color.w);

	return table;
}
