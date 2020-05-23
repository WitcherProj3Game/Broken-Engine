#ifndef MODULE_PARTICLES_H__
#define MODULE_PARTICLES_H__

#include "Module.h"
#include <queue>

BE_BEGIN_NAMESPACE

class ComponentParticleEmitter;
struct HigherPriority;

class BROKEN_API ModuleParticles : public Module
{
	friend class ComponentParticleEmitter;
public:
	ModuleParticles(bool start_enabled);
	~ModuleParticles();

	bool Init(json& config) override;
	bool Start() override;
	update_status Update(float dt) override;

	void AddEmitter(ComponentParticleEmitter* componentEmitter);
	void DeleteEmitter(ComponentParticleEmitter* componentEmitter);

	bool CleanUp() override;

public: 

	
	std::vector<ComponentParticleEmitter*> particleEmitters;
};

BE_END_NAMESPACE
#endif // _MODULE_PARTICLES_H__
