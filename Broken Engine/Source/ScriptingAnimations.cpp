#include "ScriptingAnimations.h"

// -- Modules --
#include "Application.h"
#include "ModuleScripting.h"
#include "ModuleSceneManager.h"

// -- Components --
#include "GameObject.h"
#include "ComponentDynamicRigidBody.h"
#include "ComponentAnimation.h"

#include "ResourceScene.h"
#include "ScriptData.h"

using namespace Broken;
ScriptingAnimations::ScriptingAnimations() {}

ScriptingAnimations::~ScriptingAnimations() {}

void ScriptingAnimations::StartAnimation(const char* name, float speed, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go) 
	{
		ComponentAnimation* anim = go->GetComponent<ComponentAnimation>();

		if (anim)
			anim->PlayAnimation(name, speed);
		else
			ENGINE_CONSOLE_LOG("[Script]: (PlayAnimation) Animation component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (PlayAnimation) Game Object passed is NULL");
}

void ScriptingAnimations::SetCurrentAnimSpeed(float speed, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go) 
	{
		ComponentAnimation* anim = go->GetComponent<ComponentAnimation>();

		if (anim)
			anim->SetCurrentAnimationSpeed(speed);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetCurrentAnimationSpeed) Animation component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetCurrentAnimationSpeed) Game Object passed is NULL");

}

void ScriptingAnimations::SetAnimSpeed(const char* name, float speed, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go) 
	{
		ComponentAnimation* anim = go->GetComponent<ComponentAnimation>();

		if (anim)
			anim->SetAnimationSpeed(name, speed);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetAnimationSpeed) Animation component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetAnimationSpeed) Game Object passed is NULL");
	

}

void ScriptingAnimations::SetBlendTime(float value, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go) {
		ComponentAnimation* anim = go->GetComponent<ComponentAnimation>();

		if (anim)
			anim->ChangeBlendTime(value);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetBlendTime) Animation component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetBlendTime) Game Object passed is NULL");
	
	
}

int ScriptingAnimations::CurrentAnimEnded(uint gameobject_UUID)
{
	int ret = 0;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	ComponentAnimation* anim = nullptr;

	if (go) 
	{
		anim = go->GetComponent<ComponentAnimation>();

		if (anim)
			ret = (int)anim->CurrentAnimationEnded();
		else 
		{
			ENGINE_CONSOLE_LOG("[Script]: (CurrentAnimationEnded) Animation component is NULL");
			ret = -1;
		}
	}
	else
	{
		ENGINE_CONSOLE_LOG("[Script]: (CurrentAnimationEnded) Game Object passed is NULL");
		ret = -1;
	}

	return ret;
}

float ScriptingAnimations::GetCurrentFrame(uint gameobject_UUID) {
	float ret = -1;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);

	if (go) {
		ComponentAnimation* anim = go->GetComponent<ComponentAnimation>();

		if (anim)
			ret = anim->GetCurrentFrame();
		else
			ENGINE_CONSOLE_LOG("![Script]: (GetCurrentFrame) Animation component is NULL");
	}
	else 
		ENGINE_CONSOLE_LOG("![Script]: (GetCurrentFrame) Game Object passed is NULL");

	return ret;
}


void ScriptingAnimations::StopAnimation(uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentAnimation* anim = go->GetComponent<ComponentAnimation>();

		if (anim)
			anim->StopAnimation();
		else
			ENGINE_CONSOLE_LOG("![Script]: (StopAnimation) Animation component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (StopAnimation) Game Object passed is NULL");
}

void ScriptingAnimations::SetAnimPause(bool pause, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentAnimation* anim = go->GetComponent<ComponentAnimation>();
		if (anim)
			anim->SetAnimationPause(pause);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetAnimationPause) Animation component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetAnimationPause) Game Object passed is NULL");
}