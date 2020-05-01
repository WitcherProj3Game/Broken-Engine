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
	const char* GetMaterialNameByUUID(const uint mat_UUID) const;
	int GetMaterialUUIDByName(const char* mat_name) const;

	// --- Set Shader ---
	//SetShader (through name and uuid)
	//SetUniform (through uniform name & type)
	// --- Get Shader ---
	//GetShader (name and uuid)
	//GetUniform (through uniform name & type)

};
BE_END_NAMESPACE

#endif //__SCRIPTINGMATERIALS_H__