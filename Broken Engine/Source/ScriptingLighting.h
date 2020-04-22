#ifndef __SCRIPTINGLIGHTING_H__
#define __SCRIPTINGLIGHTING_H__

#include "BrokenCore.h"

class lua_State;

namespace luabridge
{
	class LuaRef;
}

BE_BEGIN_NAMESPACE
class BROKEN_API ScriptingLighting
{
public:

	ScriptingLighting() {}
	~ScriptingLighting() {}

	// --- Setters ---
	void SetIntensity(float intensity, uint gameobject_UUID);
	void SetDistanceMultiplier(float distMult, uint gameobject_UUID);
	void SetType(int type, uint gameobject_UUID);
	void SetColor(float r, float g, float b, uint gameobject_UUID);
	void SetAttenuation(float K, float L, float Q, uint gameobject_UUID);
	void SetCutoff(float innerCutoff, float outerCutoff, uint gameobject_UUID);

	// --- Getters ---
	float GetIntensity(uint gameobject_UUID) const;
	float GetDistanceMultiplier(uint gameobject_UUID) const;
	int GetType(uint gameobject_UUID) const; //NONE = -1, DIR = 0, POINT = 1, SPOT = 2, MAX = 3
	luabridge::LuaRef GetColor(uint gameobject_UUID, lua_State* L) const;
	luabridge::LuaRef GetAttenuation(uint gameobject_UUID, lua_State* L) const;
	luabridge::LuaRef GetCutoff(uint gameobject_UUID, lua_State* L) const;
};
BE_END_NAMESPACE
#endif //__SCRIPTINGLIGHTING_H__