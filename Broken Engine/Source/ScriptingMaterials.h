#ifndef __SCRIPTINGMATERIALS_H__
#define __SCRIPTINGMATERIALS_H__

#include "BrokenCore.h"

class lua_State;

namespace luabridge
{
	class LuaRef;
}

BE_BEGIN_NAMESPACE
class BROKEN_API ScriptingMaterials
{
public:

	ScriptingMaterials() {}
	~ScriptingMaterials() {}

	// --- Standard Setters ---
	void SetTransparency(bool is_transparent, uint gameobject_UUID);
	void SetCulling(bool culling, uint gameobject_UUID);
	void SetShininess(float shininess, uint gameobject_UUID);
	void SetTextureUsage(bool use_textures, uint gameobject_UUID);
	void SetAlpha(float alpha, uint gameobject_UUID);
	void SetColor(float r, float g, float b, float a, uint gameobject_UUID);

	// --- Standard Getters ---
	bool GetTransparency(uint gameobject_UUID) const;
	bool GetCulling(uint gameobject_UUID) const;
	bool GetTextureUsage(uint gameobject_UUID) const;
	float GetShininess(uint gameobject_UUID) const;
	float GetAlpha(uint gameobject_UUID) const;
	luabridge::LuaRef GetColor(uint gameobject_UUID, lua_State* L) const;

	// --- Set Material ---
	void SetMaterialByName(const char* mat_name, uint gameobject_UUID);
	void SetMaterialByUUID(uint mat_UUID, uint gameobject_UUID);

	// --- Get Materials ---
	const char* GetCurrentMaterialName(uint gameobject_UUID) const;
	int GetCurrentMaterialUUID(uint gameobject_UUID) const;
	const char* GetMaterialNameByUUID(uint mat_UUID) const;
	int GetMaterialUUIDByName(const char* mat_name) const;

	// --- Set Shader ---
	void SetShaderByName(const char* shader_name, uint gameobject_UUID);
	void SetShaderByUUID(uint shader_UUID, uint gameobject_UUID);
	void SetShaderToMaterial(const char* shader_name, const char* material_name);

	// --- Get Shader ---
	const char* GetCurrentShaderName(uint gameobject_UUID) const;
	int GetCurrentShaderUUID(uint gameobject_UUID) const;
	const char* GetShaderNameByUUID(uint shader_UUID) const;
	int GetShaderUUIDByName(const char* shader_name) const;

	// --- Uniforms ---
	void SetUniformInt(const char* material_name, const char* unif_name, int value);
	void SetUniformFloat(const char* material_name, const char* unif_name, float value);
	void SetUniformVec2(const char* material_name, const char* unif_name, float x, float y);
	void SetUniformVec3(const char* material_name, const char* unif_name, float x, float y, float z);
	void SetUniformVec4(const char* material_name, const char* unif_name, float x, float y, float z, float w);
	void SetUniformBool(const char* material_name, const char* unif_name, bool value);

	// --- Uniform Getters ---
	int GetUniformInt(const char* material_name, const char* unif_name) const;
	float GetUniformFloat(const char* material_name, const char* unif_name) const;
	luabridge::LuaRef GetUniformVec2(const char* material_name, const char* unif_name, lua_State* L) const;
	luabridge::LuaRef GetUniformVec3(const char* material_name, const char* unif_name, lua_State* L) const;
	luabridge::LuaRef GetUniformVec4(const char* material_name, const char* unif_name, lua_State* L) const;
};
BE_END_NAMESPACE

#endif //__SCRIPTINGMATERIALS_H__