#include "ScriptingLighting.h"

// -- Modules --
#include "Application.h"
#include "ModuleScripting.h"
#include "ModuleSceneManager.h"

// -- Components -
#include "GameObject.h"
#include "Components.h"
#include "ComponentLight.h"

#include "ScriptData.h"
#include "ResourceScene.h"

// -- Utilities --
//#include "TranslatorUtilities.h"

using namespace Broken;

// --- Setters ---
void ScriptingLighting::SetIntensity(float intensity, uint gameobject_UUID)
{
	if (intensity < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetLightIntensity) Light Intensity cannot be negative");
		return;
	}

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			light->SetLightIntensity(intensity);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetLightIntensity) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetLightIntensity) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingLighting::SetDistanceMultiplier(float distMult, uint gameobject_UUID)
{
	if (distMult < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetLightIntensity) Light Dist. Multiplier cannot be negative");
		return;
	}

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			light->SetLightDistanceMultiplier(distMult);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetDistMultiplier) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetDistMultiplier) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingLighting::SetType(int type, uint gameobject_UUID)
{
	if (type <= -1 || type >= 3)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetLightType) Error! Light Type Parameter Invalid!");
		return;
	}

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			light->SetLightType((LightType)type);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetLightType) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetLightType) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingLighting::SetColor(float r, float g, float b, uint gameobject_UUID)
{
	if (r > 255.0f || g > 255.0f || b > 255.0f || r < 0.0f || g < 0.0f || b < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetLightColor) Color has to be between [0.0, 255.0]");
		return;
	}

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			light->SetLightColor(float3(r, g, b)/255.0f);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetLightColor) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetLightColor) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingLighting::SetAttenuation(float K, float L, float Q, uint gameobject_UUID)
{
	if (K < 0.0f || L < 0.0f || Q < 0.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (SetLightAttenuation) Light Attenuation cannot be negative");
		return;
	}

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			light->SetLightAttenuationFactors(K, L, Q);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetLightAttenuation) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetLightAttenuation) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingLighting::SetCutoff(float innerCutoff, float outerCutoff, uint gameobject_UUID)
{
	if (innerCutoff < 0.0f || outerCutoff < 0.0f || innerCutoff > 360.0f || outerCutoff > 360.0f)
	{
		ENGINE_CONSOLE_LOG("![Script]: (GetLightCutoff) Light Cutoffs cannot be negative or bigger than 360º");
		return;
	}

	if ((innerCutoff > (outerCutoff - 0.01f)) || (outerCutoff < (innerCutoff + 0.01f)))
	{
		ENGINE_CONSOLE_LOG("![Script]: (GetLightCutoff) Inner Cuttoff >= Outer Cuttoff or Outer Cuttoff <= Inner Cuttoff. That's wrong");
		return;
	}

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			light->SetLightInOutCutoff(innerCutoff, outerCutoff);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetLightCutoff) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetLightCutoff) Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingLighting::SetShadowerLight(uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			App->renderer3D->SetShadowerLight(light);
		else
			ENGINE_CONSOLE_LOG("![Script]: () Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: () Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingLighting::SetLightShadowsIntensity(float intensity, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			light->m_ShadowsIntensity = intensity;
		else
			ENGINE_CONSOLE_LOG("![Script]: () Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: () Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingLighting::SetLightShadowsFrustumSize(float x, float y, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			light->SetLightShadowsFrustumSize(x, y);
		else
			ENGINE_CONSOLE_LOG("![Script]: () Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: () Could not find GameObject with UUID %d", gameobject_UUID);
}

void ScriptingLighting::SetLightShadowsFrustumPlanes(float nearp, float farp, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			light->SetLightShadowsFrustumPlanes(nearp, farp);
		else
			ENGINE_CONSOLE_LOG("![Script]: () Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: () Could not find GameObject with UUID %d", gameobject_UUID);
}

// --- Getters ---
float ScriptingLighting::GetIntensity(uint gameobject_UUID) const
{
	float ret = 0.0f;

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			ret = light->GetLightIntensity();
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetLightIntensity) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetLightIntensity) Could not find GameObject with UUID %d", gameobject_UUID);

	return ret;
}

float ScriptingLighting::GetDistanceMultiplier(uint gameobject_UUID) const
{
	float ret = 0.0f;

	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			ret = light->GetLightDistanceMultiplier();
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetDistMultiplier) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetDistMultiplier) Could not find GameObject with UUID %d", gameobject_UUID);

	return ret;
}

int ScriptingLighting::GetType(uint gameobject_UUID) const
{
	int ret = -1;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
		{
			ret = (int)light->GetLightType();
			if(ret <= -1 || ret >= 3)
				ENGINE_CONSOLE_LOG("![Script]: (GetLightType) WARNING! Light Type Invalid");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetLightType) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetLightType) Could not find GameObject with UUID %d", gameobject_UUID);

	return ret;
}

luabridge::LuaRef ScriptingLighting::GetColor(uint gameobject_UUID, lua_State* L) const
{
	float3 color = float3(0.0f);
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			color = light->GetLightColor() * 255.0f;
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetLightColor) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetLightColor) Could not find GameObject with UUID %d", gameobject_UUID);

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(color.x);
	table.append(color.y);
	table.append(color.z);

	return table;
}

luabridge::LuaRef ScriptingLighting::GetAttenuation(uint gameobject_UUID, lua_State* L) const
{
	float3 att = float3(0.0f);
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			att = light->GetLightAttenuationKLQ();
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetLightAttenuation) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetLightAttenuation) Could not find GameObject with UUID %d", gameobject_UUID);

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(att.x);
	table.append(att.y);
	table.append(att.z);

	return table;
}

luabridge::LuaRef ScriptingLighting::GetCutoff(uint gameobject_UUID, lua_State* L) const
{
	float2 cutoff = float2(0.0f);
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go)
	{
		ComponentLight* light = go->GetComponent<ComponentLight>();
		if (light)
			cutoff = light->GetLightInOutCutoff();
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetLightCutoff) Light Component is null");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetLightCutoff) Could not find GameObject with UUID %d", gameobject_UUID);

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(cutoff.x);
	table.append(cutoff.y);

	return table;
}
