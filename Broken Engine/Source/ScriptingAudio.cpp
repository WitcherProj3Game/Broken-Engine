#include "ScriptingAudio.h"

// -- Modules --
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScripting.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"
#include "ModuleSceneManager.h"

// -- Components --
#include "GameObject.h"
#include "ComponentParticleEmitter.h"
#include "ComponentDynamicRigidBody.h"
#include "ComponentCollider.h"
#include "ComponentAnimation.h"
#include "ComponentCamera.h"
#include "ComponentAudioSource.h"
#include "ResourceScene.h"



#include "../Game/Assets/Sounds/Wwise_IDs.h"
#include "ScriptData.h"

using namespace Broken;

void ScriptingAudio::SetVolume(float volume, uint UID)
{
	GameObject* GO = App->scene_manager->currentScene->GetGOWithUID(UID);
	if (GO)
	{
		ComponentAudioSource* sound = GO->GetComponent<ComponentAudioSource>();
		if (sound)
			sound->SetVolume(volume);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetVolume) Sound Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetVolume) GameObject with UID %d does not exist", UID);
}

void ScriptingAudio::PlayAudioEventGO(std::string event, uint UID)
{
	GameObject* GO = App->scene_manager->currentScene->GetGOWithUID(UID);
	if (GO)
	{
		ComponentAudioSource* sound = GO->GetComponent<ComponentAudioSource>();
		if (sound)
		{
			uint EventId = App->audio->EventMap[event];
			sound->SetID(EventId);
			sound->wwiseGO->PlayEvent(EventId);
			sound->isPlaying = true;
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (PlayAudioEventGO) Component AudioSource does not exist");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (StopAudioEventGO) GameObject with UID %d does not exist", UID);
}

void ScriptingAudio::StopAudioEventGO(std::string event, uint UID)
{
	GameObject* GO = App->scene_manager->currentScene->GetGOWithUID(UID);
	if (GO)
	{
		ComponentAudioSource* sound = GO->GetComponent<ComponentAudioSource>();
		if (sound)
		{
			uint EventId = App->audio->EventMap[event];
			sound->SetID(EventId);
			sound->wwiseGO->StopEvent(EventId);
			sound->isPlaying = false;
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (StopAudioEventGO) Component AudioSource does not exist");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (StopAudioEventGO) GameObject with UID %d does not exist", UID);
}

void ScriptingAudio::PauseAudioEventGO(std::string event, uint UID)
{
	GameObject* GO = App->scene_manager->currentScene->GetGOWithUID(UID);
	if (GO)
	{
		ComponentAudioSource* sound = GO->GetComponent<ComponentAudioSource>();
		if (sound)
		{
			uint EventId = App->audio->EventMap[event];
			sound->SetID(EventId);
			sound->wwiseGO->PauseEvent(EventId);
			sound->isPlaying = false;
			sound->isPaused = true;
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (PauseAudioEventGO) Component AudioSource does not exist");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (PauseAudioEventGO) GameObject with UID %d does not exist", UID);
}

void ScriptingAudio::ResumeAudioEventGO(std::string event,uint UID)
{
	GameObject* GO = App->scene_manager->currentScene->GetGOWithUID(UID);
	if (GO)
	{
		ComponentAudioSource* sound = GO->GetComponent<ComponentAudioSource>();
		if (sound)
		{
			uint EventId = App->audio->EventMap[event];
			sound->SetID(EventId);
			sound->wwiseGO->ResumeEvent(EventId);
			sound->isPlaying = true;
			sound->isPaused = false;
		}
		ENGINE_CONSOLE_LOG("![Script]: (ResumeAudioEventGO) Component AudioSource does not exist");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (ResumeAudioEventGO) GameObject with UID %d does not exist", UID);
}

void ScriptingAudio::PlayAudioEvent(std::string event)
{
	ComponentAudioSource* sound = App->scripting->current_script->my_component->GetContainerGameObject()->GetComponent<ComponentAudioSource>();
	if (sound)
	{
		uint EventId = App->audio->EventMap[event];
		sound->SetID(EventId);
		sound->wwiseGO->PlayEvent(EventId);
		sound->isPlaying = true;
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (PlayAudioEvent) Sound Emmiter component is NULL");
}

void ScriptingAudio::StopAudioEvent(std::string event)
{
	ComponentAudioSource* sound = App->scripting->current_script->my_component->GetContainerGameObject()->GetComponent<ComponentAudioSource>();
	if (sound)
	{
		uint EventId = App->audio->EventMap[event];
		sound->SetID(EventId);
		sound->wwiseGO->StopEvent(EventId);
		sound->isPlaying = false;
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (StopAudioEvent) Sound Emmiter component is NULL");
}

void ScriptingAudio::PauseAudioEvent(std::string event)
{
	ComponentAudioSource* sound = App->scripting->current_script->my_component->GetContainerGameObject()->GetComponent<ComponentAudioSource>();
	if (sound)
	{
		uint EventId = App->audio->EventMap[event];
		sound->SetID(EventId);
		sound->wwiseGO->PauseEvent(EventId);
		sound->isPlaying = false;
		sound->isPaused = true;
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (PauseAudioEvent) Sound Emmiter component is NULL");
}

void ScriptingAudio::ResumeAudioEvent(std::string event)
{
	ComponentAudioSource* sound = App->scripting->current_script->my_component->GetContainerGameObject()->GetComponent<ComponentAudioSource>();
	if (sound)
	{
		uint EventId = App->audio->EventMap[event];
		sound->SetID(EventId);
		sound->wwiseGO->ResumeEvent(EventId);
		sound->isPlaying = true;
		sound->isPaused = false;
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (ResumeAudioEvent) Sound Emmiter component is NULL");
}

void ScriptingAudio::SetAudioTrigger(std::string trigger)
{
	ComponentAudioSource* sound = App->scripting->current_script->my_component->GetContainerGameObject()->GetComponent<ComponentAudioSource>();
	if (sound)
	{
		uint wwisegoid = sound->wwiseGO->id;
		App->audio->SetAudioTrigger(wwisegoid, trigger);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetAudioTrigger) Sound Emmiter component is NULL");
}
