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

// -- Others --
#include "ResourceMaterial.h"
#include "ResourceShader.h"
#include "ImporterMaterial.h"

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
// ----------------------------------------------------------------------- SETTERS (by Mat) --------------------------------------------------------------
void ScriptingMaterials::SetMaterialTransparency(bool is_transparent, const char* material_name)
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->has_transparencies = is_transparent;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTransparent) Material is default or null");
}

void ScriptingMaterials::SetMaterialCulling(bool culling, const char* material_name)
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->has_culling = culling;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialCulling) Material is default or null");
}

void ScriptingMaterials::SetMaterialShininess(float shininess, const char* material_name)
{
	if (shininess < -2.0f || shininess > 500.0f)
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialShininess) Shininess is not between [-2.0, 500.0]");

	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_Shininess = shininess;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialShininess) Material is default or null");
}

void ScriptingMaterials::SetMaterialTextureUsage(bool use_textures, const char* material_name)
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_UseTexture = use_textures;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialTextureUsage) Material is default or null");
}

void ScriptingMaterials::SetMaterialAlpha(float alpha, const char* material_name)
{
	if (alpha > 1.0f || alpha < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialAlpha) Alpha has to be between [0.0, 1.0]");
		return;
	}

	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_AmbientColor.w = alpha;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialAlpha) Material is default or null");
}

void ScriptingMaterials::SetMaterialColor(float r, float g, float b, float a, const char* material_name)
{
	if (r > 255.0f || g > 255.0f || b > 255.0f || a > 1.0f || r < 0.0f || g < 0.0f || b < 0.0f || a < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialColor) Color has to be between [0.0, 255.0] and alpha between [0.0, 1.0]");
		return;
	}

	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_AmbientColor = float4(r, g, b, a);
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetMaterialColor) Material is default or null");
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------- GETTERS (by Mat) --------------------------------------------------------------
bool ScriptingMaterials::GetMaterialTransparency(const char* material_name) const
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat)
		return mat->has_transparencies;
	else
		ENGINE_AND_SYSTEM_CONSOLE_LOG("![Script]: (GetMaterialTransparency) Material is default or null");

	return false;
}

bool ScriptingMaterials::GetMaterialCulling(const char* material_name) const
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat)
		return mat->has_culling;
	else
		ENGINE_AND_SYSTEM_CONSOLE_LOG("![Script]: (GetMaterialCulling) Material is default or null");

	return false;
}

bool ScriptingMaterials::GetMaterialTextureUsage(const char* material_name) const
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat)
		return mat->m_UseTexture;
	else
		ENGINE_AND_SYSTEM_CONSOLE_LOG("![Script]: (GetMaterialTextureUsage) Material is default or null");

	return false;
}

float ScriptingMaterials::GetMaterialShininess(const char* material_name) const
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat)
		return mat->m_Shininess;
	else
		ENGINE_AND_SYSTEM_CONSOLE_LOG("![Script]: (GetMaterialShininess) Material is default or null");

	return -1.0f;
}

float ScriptingMaterials::GetMaterialAlpha(const char* material_name) const
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat)
		return mat->m_AmbientColor.w;
	else
		ENGINE_AND_SYSTEM_CONSOLE_LOG("![Script]: (GetMaterialAlpha) Material is default or null");

	return -1.0f;
}

luabridge::LuaRef ScriptingMaterials::GetMaterialColor(const char* material_name, lua_State* L) const
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	float4 color = float4(-1.0f, -1.0f, -1.0f, -1.0f);

	if (mat)
		color = mat->m_AmbientColor;
	else
		ENGINE_AND_SYSTEM_CONSOLE_LOG("![Script]: (GetMaterialColor) Material is default or null");

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(color.x);
	table.append(color.y);
	table.append(color.z);
	table.append(color.w);

	return table;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------- SETTERS -----------------------------------------------------------------------
void ScriptingMaterials::SetTransparency(bool is_transparent, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetObjectMaterialTransparent)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->has_transparencies = is_transparent;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetObjectMaterialTransparent) Mesh material is default or null");

}

void ScriptingMaterials::SetCulling(bool culling, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetObjectMaterialCulling)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->has_culling = culling;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetObjectMaterialCulling) Mesh material is default or null");
}

void ScriptingMaterials::SetShininess(float shininess, uint gameobject_UUID)
{
	if (shininess < -2.0f || shininess > 500.0f)
		ENGINE_CONSOLE_LOG("![Script]: (SetObjectMaterialShininess) Shininess is not between [-2.0, 500.0]");

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetObjectMaterialShininess)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_Shininess = shininess;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetObjectMaterialShininess) Mesh material is default or null");
}

void ScriptingMaterials::SetTextureUsage(bool use_textures, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetObjectMaterialTextureUsage)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_UseTexture = use_textures;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetObjectMaterialTextureUsage) Mesh material is default or null");
}

void ScriptingMaterials::SetAlpha(float alpha, uint gameobject_UUID)
{
	if (alpha > 1.0f || alpha < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetObjectMaterialAlpha) Alpha has to be between [0.0, 1.0]");
		return;
	}

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetObjectMaterialAlpha)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		mat->m_AmbientColor.w = alpha;
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetObjectMaterialAlpha) Mesh material is default or null");
}

void ScriptingMaterials::SetColor(float r, float g, float b, float a, uint gameobject_UUID)
{
	if (r > 255.0f || g > 255.0f || b > 255.0f || a > 1.0f || r < 0.0f || g < 0.0f || b < 0.0f || a < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetObjectMaterialColor) Color has to be between [0.0, 255.0] and alpha between [0.0, 1.0]");
		return;
	}

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetObjectMaterialColor)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
	{
		mat->m_AmbientColor = float4(r, g, b, 255.0f) / 255.0f;
		mat->m_AmbientColor.w = a;
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetObjectMaterialColor) Mesh material is default or null");
}


// -------------------------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------- GETTERS -----------------------------------------------------------------------
bool ScriptingMaterials::GetTransparency(uint gameobject_UUID) const
{
	bool ret = false;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetObjectMaterialTransparency)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->has_transparencies;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetObjectMaterialTransparency) Mesh material is default or null");

	return ret;
}

bool ScriptingMaterials::GetCulling(uint gameobject_UUID) const
{
	bool ret = false;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetObjectMaterialCulling)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->has_culling;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetObjectMaterialCulling) Mesh material is default or null");

	return ret;
}

bool ScriptingMaterials::GetTextureUsage(uint gameobject_UUID) const
{
	bool ret = false;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetObjectMaterialTextureUsage)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->m_UseTexture;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetObjectMaterialTextureUsage) Mesh material is default or null");

	return ret;
}

float ScriptingMaterials::GetShininess(uint gameobject_UUID) const
{
	float ret = -1.0f;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetObjectMaterialShininess)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->m_Shininess;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetObjectMaterialShininess) Mesh material is default or null");

	return ret;
}

float ScriptingMaterials::GetAlpha(uint gameobject_UUID) const
{
	float ret = -1.0f;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetObjectMaterialAlpha)", gameobject_UUID, &mesh) == false)
		return ret;

	ResourceMaterial* mat = mesh->material;
	if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
		ret = mat->m_AmbientColor.w;
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetObjectMaterialAlpha) Mesh material is default or null");

	return ret;
}

luabridge::LuaRef ScriptingMaterials::GetColor(uint gameobject_UUID, lua_State* L) const
{
	ComponentMeshRenderer* mesh = nullptr;
	float4 color = float4(-1.0f, -1.0f, -1.0f, -1.0f);

	if (FailSafeCheck("(GetObjectMaterialColor)", gameobject_UUID, &mesh) == true)
	{
		ResourceMaterial* mat = mesh->material;
		if (mat && mat->GetUID() != App->resources->GetDefaultMaterialUID())
			color = mat->m_AmbientColor;
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetObjectMaterialColor) Mesh material is default or null");
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

	ResourceMaterial* new_mat = App->resources->GetMaterialByName(mat_name);
	if (new_mat != nullptr)
		mesh->material = new_mat;
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetMaterialByName) Couldn't Find Material: '%s' ... Or was null", mat_name);
}

void ScriptingMaterials::SetMaterialByUUID(uint mat_UUID, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetMaterialByUID)", gameobject_UUID, &mesh) == false)
		return;

	ResourceMaterial* new_mat = App->resources->GetMaterialByUUID(mat_UUID);
	if (new_mat != nullptr)
		mesh->material = new_mat;
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetMaterialByUID) Couldn't Find Material with UUID %d", mat_UUID);
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------- MATERIAL GETTERS -------------------------------------------------------------------
const char* ScriptingMaterials::GetCurrentMaterialName(uint gameobject_UUID) const
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetCurrentMatName)", gameobject_UUID, &mesh) == false)
		return "null";

	if (mesh->material != nullptr)
		return mesh->material->GetName();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetCurrentMatName) Mesh Material is null");

	return "null";
}

int ScriptingMaterials::GetCurrentMaterialUUID(uint gameobject_UUID) const
{
	int ret = -1;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetCurrentMatUID)", gameobject_UUID, &mesh) == false)
		return ret;

	if (mesh->material != nullptr)
		ret = mesh->material->GetUID();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetCurrentMatUID) Mesh Material is null");

	return ret;
}

const char* ScriptingMaterials::GetMaterialNameByUUID(uint mat_UUID) const
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByUUID(mat_UUID);
	if (new_mat != nullptr)
		return new_mat->GetName();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetMaterialNameFromUID) Couldn't Find Material with UUID %d", mat_UUID);

	return "null";
}

int ScriptingMaterials::GetMaterialUUIDByName(const char* mat_name) const
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByName(mat_name);
	if (new_mat != nullptr)
		return new_mat->GetUID();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetMaterialUIDFromName) Couldn't Find Material: '%s' ... Or was null", mat_name);

	return -1;
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------- SHADER SETTERS --------------------------------------------------------------------
void ScriptingMaterials::SetShaderByName(const char* shader_name, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetShaderByName)", gameobject_UUID, &mesh) == false)
		return;

	if (mesh->material->GetUID() == App->resources->GetDefaultMaterialUID())
	{
		ENGINE_CONSOLE_LOG("[Script]: (SetShaderByName) Object has Default Material and cannot be modified");
		return;
	}

	ResourceShader* new_shader = App->resources->GetShaderByName(shader_name);
	if (new_shader != nullptr)
	{
		mesh->material->shader = new_shader;
		mesh->material->shader->GetAllUniforms(mesh->material->uniforms);
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetShaderByName) Couldn't Find Shader: '%s' ... Or was null", shader_name);
}

void ScriptingMaterials::SetShaderByUUID(uint shader_UUID, uint gameobject_UUID)
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(SetShaderByUID)", gameobject_UUID, &mesh) == false)
		return;

	if (mesh->material->GetUID() == App->resources->GetDefaultMaterialUID())
	{
		ENGINE_CONSOLE_LOG("[Script]: (SetShaderByUID) Object has Default Material and cannot be modified");
		return;
	}

	ResourceShader* new_shader = App->resources->GetShaderByUUID(shader_UUID);
	if (new_shader != nullptr)
	{
		mesh->material->shader = new_shader;
		mesh->material->shader->GetAllUniforms(mesh->material->uniforms);
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetShaderByUID) Couldn't Find Shader with UUID %d", shader_UUID);
}

void ScriptingMaterials::SetShaderToMaterial(const char* shader_name, const char* material_name)
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByName(material_name);
	
	if (new_mat == nullptr || new_mat->GetUID() == App->resources->GetDefaultMaterialUID())
	{
		ENGINE_CONSOLE_LOG("[Script]: (SetShaderToMaterial) Material passed (%s) is null or Default (cannot be modified)", material_name);
		return;
	}	
	
	ResourceShader* new_shader = App->resources->GetShaderByName(shader_name);
	if (new_shader)
	{
		new_mat->shader = new_shader;
		new_mat->shader->GetAllUniforms(new_mat->uniforms);
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetShaderToMaterial) Shader Passed is null (%s)", shader_name);
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------- SHADER GETTERS --------------------------------------------------------------------
const char* ScriptingMaterials::GetCurrentShaderName(uint gameobject_UUID) const
{
	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetCurrentShaderName)", gameobject_UUID, &mesh) == false)
		return "null";

	if (mesh->material != nullptr)
	{
		if (mesh->material->shader != nullptr)
			return mesh->material->shader->GetName();
		else
			ENGINE_CONSOLE_LOG("[Script]: (GetCurrentShaderName) Material Shader is null");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetCurrentShaderName) Mesh Material is null");


	return "null";
}

int ScriptingMaterials::GetCurrentShaderUUID(uint gameobject_UUID) const
{
	int ret = -1;

	ComponentMeshRenderer* mesh = nullptr;
	if (FailSafeCheck("(GetCurrentShaderUID)", gameobject_UUID, &mesh) == false)
		return ret;

	if (mesh->material != nullptr)
	{
		if (mesh->material->shader != nullptr)
			return mesh->material->shader->GetUID();
		else
			ENGINE_CONSOLE_LOG("[Script]: (GetCurrentShaderName) Material Shader is null");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetCurrentShaderUID) Mesh Material is null");

	return ret;
}

const char* ScriptingMaterials::GetShaderNameByUUID(uint shader_UUID) const
{
	ResourceShader* new_shader = App->resources->GetShaderByUUID(shader_UUID);
	if (new_shader != nullptr)
		return new_shader->GetName();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetShaderNameFromUID) Couldn't Find Shader with UUID %d", shader_UUID);

	return "null";
}

int ScriptingMaterials::GetShaderUUIDByName(const char* shader_name) const
{
	ResourceShader* new_shader = App->resources->GetShaderByName(shader_name);
	if (new_shader != nullptr)
		return new_shader->GetUID();
	else
		ENGINE_CONSOLE_LOG("[Script]: (GetShaderUIDFromName) Couldn't Find Shader: '%s' ... Or was null", shader_name);

	return -1;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------- SHADER UNIFORMS -------------------------------------------------------------------
void ScriptingMaterials::SetUniformInt(const char* material_name, const char* unif_name, int value)
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByName(material_name);
	if (new_mat == nullptr || new_mat->GetUID() == App->resources->GetDefaultMaterialUID())
	{
		ENGINE_CONSOLE_LOG("[Script]: (SetUniformInt) Material passed (%s) is null or Default (cannot be modified)", material_name);
		return;
	}

	for (uint i = 0; i < new_mat->uniforms.size(); ++i)
	{
		if (new_mat->uniforms[i]->name == unif_name)
		{
			new_mat->uniforms[i]->value.intU = value;
			new_mat->UpdateUniforms();
			App->resources->GetImporter<ImporterMaterial>()->Save(new_mat);
		}
	}
}

void ScriptingMaterials::SetUniformFloat(const char* material_name, const char* unif_name, float value)
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByName(material_name);
	if (new_mat == nullptr || new_mat->GetUID() == App->resources->GetDefaultMaterialUID())
	{
		ENGINE_CONSOLE_LOG("[Script]: (SetUniformFloat) Material passed (%s) is null or Default (cannot be modified)", material_name);
		return;
	}

	for (uint i = 0; i < new_mat->uniforms.size(); ++i)
	{
		if (new_mat->uniforms[i]->name == unif_name)
		{
			new_mat->uniforms[i]->value.floatU = value;
			new_mat->UpdateUniforms();
			App->resources->GetImporter<ImporterMaterial>()->Save(new_mat);
		}
	}
}

void ScriptingMaterials::SetUniformVec2(const char* material_name, const char* unif_name, float x, float y)
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByName(material_name);
	if (new_mat == nullptr || new_mat->GetUID() == App->resources->GetDefaultMaterialUID())
	{
		ENGINE_CONSOLE_LOG("[Script]: (SetUniformVec2) Material passed (%s) is null or Default (cannot be modified)", material_name);
		return;
	}

	for (uint i = 0; i < new_mat->uniforms.size(); ++i)
	{
		if (new_mat->uniforms[i]->name == unif_name)
		{
			new_mat->uniforms[i]->value.vec2U = float2(x, y);
			new_mat->UpdateUniforms();
			App->resources->GetImporter<ImporterMaterial>()->Save(new_mat);
		}
	}
}

void ScriptingMaterials::SetUniformVec3(const char* material_name, const char* unif_name, float x, float y, float z)
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByName(material_name);
	if (new_mat == nullptr || new_mat->GetUID() == App->resources->GetDefaultMaterialUID())
	{
		ENGINE_CONSOLE_LOG("[Script]: (SetUniformVec3) Material passed (%s) is null or Default (cannot be modified)", material_name);
		return;
	}

	for (uint i = 0; i < new_mat->uniforms.size(); ++i)
	{
		if (new_mat->uniforms[i]->name == unif_name)
		{
			new_mat->uniforms[i]->value.vec3U = float3(x, y, z);
			new_mat->UpdateUniforms();
			App->resources->GetImporter<ImporterMaterial>()->Save(new_mat);
		}
	}
}

void ScriptingMaterials::SetUniformVec4(const char* material_name, const char* unif_name, float x, float y, float z, float w)
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByName(material_name);
	if (new_mat == nullptr || new_mat->GetUID() == App->resources->GetDefaultMaterialUID())
	{
		ENGINE_CONSOLE_LOG("[Script]: (SetUniformVec4) Material passed (%s) is null or Default (cannot be modified)", material_name);
		return;
	}
	
	for (uint i = 0; i < new_mat->uniforms.size(); ++i)
	{
		if (new_mat->uniforms[i]->name == unif_name)
		{
			new_mat->uniforms[i]->value.vec4U = float4(x, y, z, w);
			new_mat->UpdateUniforms();
			App->resources->GetImporter<ImporterMaterial>()->Save(new_mat);
		}
	}
}

void ScriptingMaterials::SetUniformBool(const char* material_name, const char* unif_name, bool value)
{
	ResourceMaterial* new_mat = App->resources->GetMaterialByName(material_name);
	if (new_mat == nullptr || new_mat->GetUID() == App->resources->GetDefaultMaterialUID())
	{
		ENGINE_CONSOLE_LOG("[Script]: (SetUniformBool) Material passed (%s) is null or Default (cannot be modified)", material_name);
		return;
	}

	new_mat->shader->setBool(unif_name, value);
}


// ------ UNIFORM GETTERS ------
int ScriptingMaterials::GetUniformInt(const char* material_name, const char* unif_name) const
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat == nullptr)
	{
		ENGINE_CONSOLE_LOG("[Script]: (GetUniformInt) Material passed (%s) is null", material_name);
		return -1;
	}

	for (uint i = 0; i < mat->uniforms.size(); ++i)
		if (mat->uniforms[i]->name == unif_name)
			return mat->uniforms[i]->value.intU;

	ENGINE_CONSOLE_LOG("[Script]: (GetUniformInt) Couldn't Find Uniform '%s' for Material '%s'", unif_name, material_name);
	return -1;
}

float ScriptingMaterials::GetUniformFloat(const char* material_name, const char* unif_name) const
{
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	if (mat == nullptr)
	{
		ENGINE_CONSOLE_LOG("[Script]: (GetUniformFloat) Material passed (%s) is null", material_name);
		return -1.0f;
	}

	for (uint i = 0; i < mat->uniforms.size(); ++i)
		if (mat->uniforms[i]->name == unif_name)
			return mat->uniforms[i]->value.floatU;

	ENGINE_CONSOLE_LOG("[Script]: (GetUniformFloat) Couldn't Find Uniform '%s' for Material '%s'", unif_name, material_name);
	return -1.0f;
}

luabridge::LuaRef ScriptingMaterials::GetUniformVec2(const char* material_name, const char* unif_name, lua_State* L) const
{
	float2 vec = float2(-1.0f, -1.0f);
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);

	if (mat == nullptr)
		ENGINE_CONSOLE_LOG("[Script]: (GetUniformVec2) Material passed (%s) is null", material_name);
	else
	{
		for (uint i = 0; i < mat->uniforms.size(); ++i)
			if (mat->uniforms[i]->name == unif_name)
				vec = mat->uniforms[i]->value.vec2U;

		if (vec.Equals(float2(-1.0f, -1.0f)))
			ENGINE_CONSOLE_LOG("[Script]: (GetUniformVec2) Couldn't Find Uniform '%s' for Material '%s'", unif_name, material_name);
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(vec.x);
	table.append(vec.y);
	return table;
}

luabridge::LuaRef ScriptingMaterials::GetUniformVec3(const char* material_name, const char* unif_name, lua_State* L) const
{
	float3 vec = float3(-1.0f, -1.0f, -1.0f);
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);
	
	if (mat == nullptr)
		ENGINE_CONSOLE_LOG("[Script]: (GetUniformVec3) Material passed (%s) is null", material_name);
	else
	{
		for (uint i = 0; i < mat->uniforms.size(); ++i)
			if (mat->uniforms[i]->name == unif_name)
				vec = mat->uniforms[i]->value.vec3U;

		if (vec.Equals(float3(-1.0f, -1.0f, -1.0f)))
			ENGINE_CONSOLE_LOG("[Script]: (GetUniformVec3) Couldn't Find Uniform '%s' for Material '%s'", unif_name, material_name);
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(vec.x);
	table.append(vec.y);
	table.append(vec.z);
	return table;
}

luabridge::LuaRef ScriptingMaterials::GetUniformVec4(const char* material_name, const char* unif_name, lua_State* L) const
{
	float4 vec = float4(-1.0f, -1.0f, -1.0f, -1.0f);
	ResourceMaterial* mat = App->resources->GetMaterialByName(material_name);

	if (mat == nullptr)
		ENGINE_CONSOLE_LOG("[Script]: (GetUniformVec4) Material passed (%s) is null", material_name);
	else
	{
		for (uint i = 0; i < mat->uniforms.size(); ++i)
			if (mat->uniforms[i]->name == unif_name)
				vec = mat->uniforms[i]->value.vec4U;

		if (vec.Equals(float4(-1.0f, -1.0f, -1.0f, -1.0f)))
			ENGINE_CONSOLE_LOG("[Script]: (GetUniformVec4) Couldn't Find Uniform '%s' for Material '%s'", unif_name, material_name);
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(vec.x);
	table.append(vec.y);
	table.append(vec.z);
	table.append(vec.w);
	return table;
}