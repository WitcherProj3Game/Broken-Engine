#ifndef __SCRIPTINGSYSTEM_H__
#define __SCRIPTINGSYSTEM_H__

#include "BrokenCore.h"

class lua_State;

namespace luabridge {
	class LuaRef;
}

BE_BEGIN_NAMESPACE
class BROKEN_API ScriptingSystem {
public:
	ScriptingSystem();
	~ScriptingSystem();

public:

	void LogFromLua(const char* string);
	float GetRealDT() const;
	float GetDT() const;
	float GameTime();
	void PauseGame();
	void ResumeGame();
	const char* GetDebuggingPath();

	// Maths
	int FloatNumsAreEqual(float a, float b);
	int DoubleNumsAreEqual(double a, double b);
	double RandomNumber();
	double RandomNumberInRange(double min, double max);
	luabridge::LuaRef RandomNumberList(int listSize, double min, double max, lua_State* L);

	//Lerp Functions
	float MathFloatLerp(float a, float b, float time) const;
	float MathFloatInvLerp(float a, float b, float x) const;
	luabridge::LuaRef MathFloat2Lerp(float ax, float ay, float bx, float by, float time, lua_State* L) const;
	luabridge::LuaRef MathFloat3Lerp(float ax, float ay, float az, float bx, float by, float bz, float time, lua_State* L) const;
	//luabridge::LuaRef MathFloat4Lerp(float ax, float ay, float az, float aw, float bx, float by, float bz, float bw, float time, lua_State* L) const;
	//luabridge::LuaRef MathQuatLerp(float ax, float ay, float az, float aw, float bx, float by, float bz, float bw, float time, lua_State* L) const;
	//luabridge::LuaRef MathQuatSlerp(float ax, float ay, float az, float aw, float bx, float by, float bz, float bw, float time, lua_State* L) const;
};
BE_END_NAMESPACE
#endif // __SCRIPTINGSYSTEM_H__