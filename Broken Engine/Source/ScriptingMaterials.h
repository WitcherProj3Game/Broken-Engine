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


	//Set & Get Material and FindMaterialByName (through UUID?)
	//void SetMaterialByName(const char* mat_name);
	//void SetMaterialByUUID(uint mat_UUID);
	//void GetCurrentMaterialName(); //Not a void
	//void GetCurrentMaterialUUID(); //Not a void
	//void GetMaterialUUIDByName(); //Not a void

	// --- Setters ---
	void SetTransparency(bool is_transparent, uint gameobject_UUID);
	void SetCulling(bool culling, uint gameobject_UUID);
	void SetShininess(float shininess, uint gameobject_UUID);
	void SetTextureUsage(bool use_textures, uint gameobject_UUID);
	void SetAlpha(float alpha, uint gameobject_UUID);
	void SetColor(float r, float g, float b, float a, uint gameobject_UUID);

	//SetShader (through name and uuid)
	//SetUniform (through uniform name & type)

	// --- Getters ---
	bool GetTransparency(uint gameobject_UUID) const;
	bool GetCulling(uint gameobject_UUID) const;
	bool GetTextureUsage(uint gameobject_UUID) const;
	float GetShininess(uint gameobject_UUID) const;
	float GetAlpha(uint gameobject_UUID) const;
	luabridge::LuaRef GetColor(uint gameobject_UUID, lua_State* L) const;

	//GetShader (name and uuid)
	//GetUniform (through uniform name & type)

};
BE_END_NAMESPACE

#endif //__SCRIPTINGMATERIALS_H__