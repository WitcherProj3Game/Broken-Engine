#include "ScriptingParticles.h"

// -- Modules --
#include "Application.h"
#include "ModuleScripting.h"
#include "ModuleSceneManager.h"
#include "ModuleResourceManager.h"

// -- Components --
#include "GameObject.h"
#include "ComponentParticleEmitter.h"
#include "ComponentDynamicRigidBody.h"

// --- Others ---
#include "ResourceScene.h"
#include "ScriptData.h"

using namespace Broken;
ScriptingParticles::ScriptingParticles() {}

ScriptingParticles::~ScriptingParticles() {}

// PARTICLES ----------------------------------------------------------
void ScriptingParticles::ActivateParticleEmitter(uint gameobject_UUID) const
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();

		if (emitter && !emitter->IsEnabled())
		{
			emitter->Enable();
			ENGINE_CONSOLE_LOG("[Script]: (ActivateParticlesEmission) Particles Emission Enabled");
		}
		else if (emitter && emitter->IsEnabled())
			ENGINE_CONSOLE_LOG("[Script]: (ActivateParticlesEmission) Particle Emmiter component already Enabled");
		else
			ENGINE_CONSOLE_LOG("[Script]: (ActivateParticlesEmission) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (ActivateParticlesEmission) Alert! This Gameobject with %d UUID was not found!", gameobject_UUID);
}

void ScriptingParticles::DeactivateParticleEmitter(uint gameobject_UUID) const
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();

		if (emitter && emitter->IsEnabled())
		{
			emitter->Disable();
			ENGINE_CONSOLE_LOG("[Script]: (DeactivateParticlesEmission) Particles Emission Disabled");
		}
		else if (emitter && !emitter->IsEnabled())
			ENGINE_CONSOLE_LOG("[Script]: (DeactivateParticlesEmission) Particle Emmiter component already Disabled");
		else
			ENGINE_CONSOLE_LOG("[Script]: (DeactivateParticlesEmission) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (DeactivateParticlesEmission) Alert! This Gameobject with %d UUID was not found!", gameobject_UUID);
}

void ScriptingParticles::PlayParticleEmitter(uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->Play();
		else
			ENGINE_CONSOLE_LOG("[Script]: (PlayParticleEmitter) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (PlayParticleEmitter) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::StopParticleEmitter(uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->Stop();
		else
			ENGINE_CONSOLE_LOG("[Script]: (StopParticleEmitter) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (StopParticleEmitter) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetEmissionRateFromScript(float ms, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetEmisionRate(ms);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetEmissionRate) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetEmissionRate) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticlesPerCreationFromScript(int particlesAmount, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetParticlesPerCreation(particlesAmount);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetParticlesPerCreation) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticlesPerCreation) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetOffsetPosition(float x, float y, float z, uint gameobject_UUID) 
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetOffsetPosition(x,y,z);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetOffsetPosition) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetOffsetPosition) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetOffsetRotation(float x, float y, float z, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetOffsetRotation(x,y,z);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetOffsetRotation) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetOffsetRotation) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticleAcceleration(float x, float y, float z, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetExternalAcceleration(x, y, z);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetExternalAcceleration) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetExternalAcceleration) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticleVelocityFromScript(float x, float y, float z, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetParticlesVelocity(x, y, z);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetParticlesVelocity) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticlesVelocity) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetRandomParticleVelocity(float x, float y, float z, uint gameobject_UUID)
{	
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetVelocityRF(x, y, z);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetRandomParticlesVelocity) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetRandomParticlesVelocity) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticleLooping(bool active, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetLooping(active);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetParticlesLooping) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticlesLooping) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticleDuration(int duration, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetDuration(duration);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetParticlesDuration) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticlesDuration) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticleLifeTime(int ms, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetLifeTime(ms);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetParticlesLifeTime) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticlesLifeTime) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticleScaleFromScript(float x, float y, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetParticlesScale(x, y);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetParticlesScale) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticlesScale) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetRandomParticleScale(float randomFactor, uint gameobject_UUID)
{	
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetParticlesScaleRF(randomFactor);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetRandomParticlesScale) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetRandomParticlesScale) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetScaleOverTime(float scale, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetScaleOverTime(scale);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetScaleOverTime) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetScaleOverTime) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetTextureByUUID(uint texture_UUID, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetTexture(texture_UUID);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetTextureByUID) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetTextureByUID) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetTextureByName(const char* texture_name, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
		{
			const std::pair<uint, ResourceTexture*> res_texture = App->resources->GetTextureResourceByName(texture_name);
			if (res_texture.second)
				emitter->SetTexture(res_texture.first);
			else
				ENGINE_CONSOLE_LOG("[Script]: (SetTextureByName) Texture passed was NULL!");
		}
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetTextureByName) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetTextureByName) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticlesRotationOverTime(int rotationOverTime, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetParticlesRotationOverTime(rotationOverTime);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetParticlesRotationOverTime) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticlesRotationOverTime) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticlesRandomRotationOverTime(int randomRotation, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetParticlesRandomRotationOverTime(randomRotation);
		else
			ENGINE_CONSOLE_LOG("[Script]:(SetParticlesRandomRotationOverTime)  Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticlesRandomRotationOverTime) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticles3DRotationOverTime(int rotationOverTimeX, int rotationOverTimeY, int rotationOverTimeZ, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetParticles3DRotationOverTime(rotationOverTimeX, rotationOverTimeY, rotationOverTimeZ);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetParticles3DRotationOverTime) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticles3DRotationOverTime) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::SetParticles3DRandomRotationOverTime(int rotationOverTimeX, int rotationOverTimeY, int rotationOverTimeZ, uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->SetParticles3DRandomRotationOverTime(rotationOverTimeX, rotationOverTimeY, rotationOverTimeZ);
		else
			ENGINE_CONSOLE_LOG("[Script]: (SetParticles3DRandomRotationOverTime) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (SetParticles3DRandomRotationOverTime) GameObject with UUID %d could not be found!", gameobject_UUID);
}

void ScriptingParticles::RemoveParticlesRandomRotation(uint gameobject_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		ComponentParticleEmitter* emitter = go->GetComponent<ComponentParticleEmitter>();
		if (emitter)
			emitter->RemoveParticlesRandomRotation();
		else
			ENGINE_CONSOLE_LOG("[Script]: (RemoveParticlesRandomRotation) Particle Emmiter component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("[Script]: (RemoveParticlesRandomRotation) GameObject with UUID %d could not be found!", gameobject_UUID);
}