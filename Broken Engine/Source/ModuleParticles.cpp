#include "ModuleParticles.h"
#include "ModulePhysics.h"
#include "Application.h"
#include "ModuleInput.h"

#include "ComponentParticleEmitter.h"

#include "Optick/include/optick.h"

#include "mmgr/mmgr.h"

using namespace Broken;

ModuleParticles::ModuleParticles(bool start_enabled)
{
	name = "Particles";
}

ModuleParticles::~ModuleParticles()
{}

bool ModuleParticles::Init(json& config)
{
	return true;
}

bool ModuleParticles::Start()
{

	return true;
}

update_status ModuleParticles::Update(float dt)
{
	OPTICK_CATEGORY("Particles Update", Optick::Category::VFX);
	
	for (int i = 0; i < particleEmitters.size(); ++i)
		if (particleEmitters[i]->IsEnabled())
			particleEmitters[i]->UpdateParticles(dt);

	return UPDATE_CONTINUE;
}

void ModuleParticles::AddEmitter(ComponentParticleEmitter* componentEmitter)
{
	particleEmitters.push_back(componentEmitter);
}

void ModuleParticles::DeleteEmitter(ComponentParticleEmitter* componentEmitter)
{
	for (std::vector<ComponentParticleEmitter*>::iterator it = particleEmitters.begin();it != particleEmitters.end() ; it++) {
		if (*(it) == componentEmitter) {
			particleEmitters.erase(it);
			break;
		}
	}
}

bool ModuleParticles::CleanUp()
{

	for (int i = 0; i < particleEmitters.size(); ++i)
		particleEmitters[i]->particleSystem->releaseParticles();

	return true;
}
