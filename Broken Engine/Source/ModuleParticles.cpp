#include "ModuleParticles.h"
#include "ModulePhysics.h"
#include "Application.h"
#include "ModuleInput.h"
#include "Particle.h"
#include "ModuleRenderer3D.h"
#include "ComponentCamera.h"

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

void ModuleParticles::DrawParticles(bool shadowsPass)
{

	// -- Frustum culling --
	Plane cameraPlanes[6];
	App->renderer3D->culling_camera->frustum.GetPlanes(cameraPlanes);

	std::map<float,Particle* >::iterator it = particlesToDraw.begin();
	while (it != particlesToDraw.end())
	{
		if ((*it).second->emitter != nullptr)
		{
			//Check if the particles are inside the frustum of the camera
			bool draw = true;
			for (int i = 0; i < 6; ++i)
			{
				//If the particles is on the positive side of one ore more planes, it's outside the frustum
				bool shadowsHandle = ((shadowsPass && (*it).second->emitter->m_CastShadows == false) || (!shadowsPass && (*it).second->emitter->m_OnlyShadows));
				if (cameraPlanes[i].IsOnPositiveSide((*it).second->position) || shadowsHandle)
				{
					draw = false;
					break;
				}
			}
			if (draw)
			{
				if (!shadowsPass && (*it).second->emitter)
				{
					(*it).second->emitter->SetEmitterBlending();
					if ((*it).second->emitter->particlesFaceCulling)
						glEnable(GL_CULL_FACE);
					else
						glDisable(GL_CULL_FACE);
				}

				(*it).second->Draw(shadowsPass);

				if (!shadowsPass)
					if ((*it).second->emitter->particlesFaceCulling == false)
						glEnable(GL_CULL_FACE);
			}
		}
		it++;
	}

	if (!shadowsPass)
		particlesToDraw.clear();
}