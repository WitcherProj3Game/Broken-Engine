#include "ScriptingSystem.h"
#include "Application.h"
#include "ModuleTimeManager.h"
#include "ModuleScripting.h"
#include "ModuleGui.h"
#include "RandomGenerator.h"
#include "ScriptData.h"
#include "Math.h"

using namespace Broken;
ScriptingSystem::ScriptingSystem() {}
		 
ScriptingSystem::~ScriptingSystem() {}

//Function that Lua will be able to call as LOG
void ScriptingSystem::LogFromLua(const char* string) {
	ENGINE_CONSOLE_LOG("[Script]: %s", string);
}

float ScriptingSystem::GetRealDT() const {
	return App->time->GetRealTimeDt();
}

float ScriptingSystem::GetDT() const {
	return App->time->GetGameDt();
}

float ScriptingSystem::GameTime()
{
	return App->time->GetGameplayTimePassed();
}

void ScriptingSystem::PauseGame()
{
	App->time->gamePaused = true;
}

void ScriptingSystem::ResumeGame()
{
	App->time->gamePaused = false;
}

int ScriptingSystem::IsGamePaused()
{
	return App->time->gamePaused;
}

const char* ScriptingSystem::GetDebuggingPath()
{
	return App->scripting->debug_path.c_str();
}

// MATHS -------------------------------------------------------------
int ScriptingSystem::FloatNumsAreEqual(float a, float b)
{
	return (fabs(a - b) < std::numeric_limits<float>::epsilon());
}

int ScriptingSystem::DoubleNumsAreEqual(double a, double b)
{
	return (fabs(a - b) < std::numeric_limits<double>::epsilon());
}

double ScriptingSystem::RandomNumber()
{
	return App->RandomNumberGenerator.GetDoubleRN();
}

double ScriptingSystem::RandomNumberInRange(double min, double max)
{
	return App->RandomNumberGenerator.GetDoubleRNinRange(min, max);
}

luabridge::LuaRef ScriptingSystem::RandomNumberList(int listSize, double min, double max, lua_State* L)
{
	std::vector<double> numbers = App->RandomNumberGenerator.GetDoubleRNVecinRange(listSize, min, max);

	luabridge::LuaRef ret = 0;
	luabridge::LuaRef uniforms(L, luabridge::newTable(L));
	for (int i = 0; i < numbers.size(); ++i)
	{
		uniforms.append(numbers[i]);
	}

	ret = uniforms;

	return ret;
}


// ------------------------------------------------ LERPS/SLERP ------------------------------------------------
float ScriptingSystem::MathFloatLerp(float a, float b, float time) const
{
	float ret = -1.0f;

	if (time < 0.0f || time > 1.0f)
		ENGINE_CONSOLE_LOG("![Script]: (MathFloatLerp) Lerp Time value must be between [0, 1]!");
	else
		ret = math::Lerp(a, b, time);

	return ret;
}

float ScriptingSystem::MathFloatInvLerp(float a, float b, float x) const
{
	float ret = -1.0f;

	if(Abs(b - a) < 1e-5f && Abs(b - a) > -1e-5f)
		ENGINE_CONSOLE_LOG("![Script]: (MathFloatInverseLerp) (A-B = 0), Cannot perform division");
	else
		ret = math::InvLerp(a, b, x);

	return ret;
}

luabridge::LuaRef ScriptingSystem::MathFloat2Lerp(float ax, float ay, float bx, float by, float time, lua_State* L) const
{
	float2 ret = float2(-1.0f, -1.0f);
	if (time < 0.0f || time > 1.0f)
		ENGINE_CONSOLE_LOG("![Script]: (MathFloat2Lerp) Lerp Time value must be between [0, 1]!");
	else
	{
		float2 a = float2(ax, ay);
		float2 b = float2(bx, by);
		ret = math::Lerp(a, b, time);
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(ret.x);
	table.append(ret.y);
	return table;
}

luabridge::LuaRef ScriptingSystem::MathFloat3Lerp(float ax, float ay, float az, float bx, float by, float bz, float time, lua_State* L) const
{
	float3 ret = float3(-1.0f, -1.0f, -1.0f);
	if (time < 0.0f || time > 1.0f)
		ENGINE_CONSOLE_LOG("![Script]: (MathFloat3Lerp) Lerp Time value must be between [0, 1]!");
	else
	{
		float3 a = float3(ax, ay, az);
		float3 b = float3(bx, by, bz);
		ret = math::Lerp(a, b, time);
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(ret.x);
	table.append(ret.y);
	table.append(ret.z);
	return table;
}

/*luabridge::LuaRef ScriptingSystem::MathFloat4Lerp(float ax, float ay, float az, float aw, float bx, float by, float bz, float bw, float time, lua_State* L) const
{
	float4 ret = float4(-1.0f, -1.0f, -1.0f, -1.0f);
	if (time < 0.0f || time > 1.0f)
		ENGINE_CONSOLE_LOG("![Script]: (MathFloat4Lerp) Lerp Time value must be between [0, 1]!");
	else
	{
		float4 a = float4(ax, ay, az, aw);
		float4 b = float4(bx, by, bz, bw);
		ret = math::Lerp(a, b, time);
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(ret.x);
	table.append(ret.y);
	table.append(ret.z);
	table.append(ret.w);
	return table;
}

luabridge::LuaRef ScriptingSystem::MathQuatLerp(float ax, float ay, float az, float aw, float bx, float by, float bz, float bw, float time, lua_State* L) const
{
	Quat ret = Quat::identity;
	if (time < 0.0f || time > 1.0f)
		ENGINE_CONSOLE_LOG("![Script]: (MathQuatLerp) Lerp Time value must be between [0, 1]!");
	else
	{
		Quat a = Quat(ax, ay, az, aw);
		Quat b = Quat(bx, by, bz, bw);
		ret = math::Lerp(a, b, time);
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(ret.x);
	table.append(ret.y);
	table.append(ret.z);
	table.append(ret.w);
	return table;
}

luabridge::LuaRef ScriptingSystem::MathQuatSlerp(float ax, float ay, float az, float aw, float bx, float by, float bz, float bw, float time, lua_State* L) const
{
	Quat ret = Quat::identity;
	if (time < 0.0f || time > 1.0f)
		ENGINE_CONSOLE_LOG("![Script]: (MathQuatSlerp) Lerp Time value must be between [0, 1]!");
	else
	{
		Quat a = Quat(ax, ay, az, aw);
		Quat b = Quat(bx, by, bz, bw);
		ret = math::Slerp(a, b, time);
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(ret.x);
	table.append(ret.y);
	table.append(ret.z);
	table.append(ret.w);
	return table;
}*/