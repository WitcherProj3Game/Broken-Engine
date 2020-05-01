#include "ScriptingMaterials.h"

// -- Modules --
#include "Application.h"
#include "ModuleScripting.h"
#include "ModuleSceneManager.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"

// -- Components -
#include "GameObject.h"
#include "Components.h"
#include "ComponentMeshRenderer.h"
#include "ResourceMaterial.h"

#include "ScriptData.h"
#include "ResourceScene.h"


using namespace Broken;

bool FailSafeCheck(const char* function_name, uint gameobject_UUID, ComponentMeshRenderer** ret_mesh)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentMeshRenderer* mesh = go->GetComponent<ComponentMeshRenderer>();
		if (mesh)
		{
			*ret_mesh = mesh;
			return true;
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: %s Game Object has no mesh (or null)", function_name);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: %s Could not find GameObject with UUID %d", function_name, gameobject_UUID);

	return false;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------- SETTERS -----------------------------------------------------------------------
void ScriptingMaterials::SetTransparency(bool is_transparent, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialTransparent)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->has_transparencies = is_transparent;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTransparent) Mesh material is default or null");

}

void ScriptingMaterials::SetCulling(bool culling, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialCulling)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->has_culling = culling;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialCulling) Mesh material is default or null");
}

void ScriptingMaterials::SetShininess(float shininess, uint gameobject_UUID)
{
	if (shininess < 1.0f || shininess > 500.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialShininess) Shininess has to be between [1.0, 500.0]");
		return;
	}

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialShininess)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_Shininess = shininess;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialShininess) Mesh material is default or null");
}

void ScriptingMaterials::SetTextureUsage(bool use_textures, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialTextureUsage)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_UseTexture = use_textures;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTextureUsage) Mesh material is default or null");
}

void ScriptingMaterials::SetAlpha(float alpha, uint gameobject_UUID)
{
	if (alpha > 1.0f || alpha < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialAlpha) Alpha has to be between [0.0, 1.0]");
		return;
	}

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialAlpha)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_AmbientColor.w = alpha;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialAlpha) Mesh material is default or null");
}

void ScriptingMaterials::SetColor(float r, float g, float b, float a, uint gameobject_UUID)
{
	if (r > 255.0f || g > 255.0f || b > 255.0f || a > 1.0f || r < 0.0f || g < 0.0f || b < 0.0f || a < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialColor) Color has to be between [0.0, 255.0] and alpha between [0.0, 1.0]");
		return;
	}

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialColor)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
	{
		mat->m_AmbientColor = float4(r, g, b, 255.0f) / 255.0f;
		mat->m_AmbientColor.w = a;
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialColor) Mesh material is default or null");
}


// -------------------------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------- GETTERS -----------------------------------------------------------------------
bool ScriptingMaterials::GetTransparency(uint gameobject_UUID) const
{
	bool ret = false;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetMaterialTransparency)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->has_transparencies;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetMaterialTransparency) Mesh material is default or null");

	return ret;
}

bool ScriptingMaterials::GetCulling(uint gameobject_UUID) const
{
	bool ret = false;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetMaterialCulling)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->has_culling;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetMaterialCulling) Mesh material is default or null");

	return ret;
}

bool ScriptingMaterials::GetTextureUsage(uint gameobject_UUID) const
{
	bool ret = false;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetTextureUsage)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->m_UseTexture;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetTextureUsage) Mesh material is default or null");

	return ret;
}

float ScriptingMaterials::GetShininess(uint gameobject_UUID) const
{
	float ret = -1.0f;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetMaterialShininess)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->m_Shininess;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetMaterialShininess) Mesh material is default or null");

	return ret;
}

float ScriptingMaterials::GetAlpha(uint gameobject_UUID) const
{
	float ret = -1.0f;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetMaterialAlpha)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->m_AmbientColor.w;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetMaterialAlpha) Mesh material is default or null");

	return ret;
}

luabridge::LuaRef ScriptingMaterials::GetColor(uint gameobject_UUID, lua_State* L) const
{
	ComponentMeshRenderer* mesh = nullptr;
	float4 color = float4(-1.0f, -1.0f, -1.0f, -1.0f);

	if (FailSafeCheck("(GetMaterialColor)", gameobject_UUID, &mesh) == true)
	{
		ResourceMaterial* mat = mesh->material;
		if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
			color = mat->m_AmbientColor;
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetMaterialColor) Mesh material is default or null");
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(color.x);
	table.append(color.y);
	table.append(color.z);
	table.append(color.w);

	return table;
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------- MATERIAL SETTERS -------------------------------------------------------------------
void ScriptingMaterials::SetMaterialByName(const char* mat_name, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialByName)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* new_mat = App->resources->GetMaterialByName(mat_name).second;
	if (new_mat != nullptr)
		mesh->material = new_mat;
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetMaterialByName) Couldn't Find Material: '%s' ... Or was null", mat_name);
}

void ScriptingMaterials::SetMaterialByUUID(uint mat_UUID, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialByUUID)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* new_mat = App->resources->GetMaterialByUUID(mat_UUID).second;
	if (new_mat != nullptr)
		mesh->material = new_mat;
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetMaterialByUUID) Couldn't Find Material with UUID %d", mat_UUID);
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------- MATERIAL GETTERS -------------------------------------------------------------------
const char* ScriptingMaterials::GetCurrentMaterialName(uint gameobject_UUID) const
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialByName)", gameobject_UUID, &mesh) == false)
		return "null";

	if (mesh->material != nullptr)
		return mesh->material->GetName();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetCurrentMatUUID) Mesh Material is null");

	return "null";
}

int ScriptingMaterials::GetCurrentMaterialUUID(uint gameobject_UUID) const
{
	int ret = -1;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetMaterialAlpha)", gameobject_UUID, &mesh) == false)
		return ret;

	if (mesh->material != nullptr)
		ret = mesh->material->GetUID();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetCurrentMatUUID) Mesh Material is null");

	return ret;
}

const char* ScriptingMaterials::GetMaterialNameByUUID(uint mat_UUID) const
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByUUID(mat_UUID).second;
	if (new_mat != nullptr)
		return new_mat->GetName();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetMaterialName) Couldn't Find Material with UUID %d", mat_UUID);

	return "null";
}

int ScriptingMaterials::GetMaterialUUIDByName(const char* mat_name) const
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByName(mat_name).second;
	if (new_mat != nullptr)
		return new_mat->GetUID();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetMaterialUUID) Couldn't Find Material: '%s' ... Or was null", mat_name);

	return -1;
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------- SHADER SETTERS --------------------------------------------------------------------
void ScriptingMaterials::SetShaderByName(const char* shader_name, uint gameobject_UUID)
{

}

void ScriptingMaterials::SetShaderByUUID(uint shader_UUID, uint gameobject_UUID)
{

}

void ScriptingMaterials::SetShaderToMaterial(const char* shader_name, const char* material_name)
{

}


// --------------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------- SHADER GETTERS --------------------------------------------------------------------
const char* ScriptingMaterials::GetCurrentShaderName(uint gameobject_UUID) const
{
	return "null";
}

int ScriptingMaterials::GetCurrentShaderUUID(uint gameobject_UUID) const
{
	return 0;
}

const char* ScriptingMaterials::GetShaderNameByUUID(uint mat_UUID) const
{
	return "null";
}

int ScriptingMaterials::GetShaderUUIDByName(const char* mat_name) const
{
	return 0;
}