#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "Math.h"
#include "BrokenCore.h"

BE_BEGIN_NAMESPACE

class ResourceTexture;
class ResourceMesh;
class ComponentParticleEmitter;

class BROKEN_API Particle {
public:
	Particle();
	~Particle();

	void Draw(bool shadowsPass);

public:

	float3 position = { 0,0,0 };
	float4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	uint lifeTime=1;
	uint spawnTime = 1;
	int currentGradient = 0;
	int gradientTimer = 0;
	float diameter=1;
	float3 rotation = { 0,0,0 };
	float3 scale = float3::one;
	float3 rotationSpeed = { 0,0,0 };
	float3 scaleSpeed = { 0,0,0 };
	float3 emitterSpawnPosition = { 0,0,0 };
	float distanceToCam = 0.0f;

	Quat intialRotation = Quat::identity;

	int startFrame = 0;

	bool h_billboard = false;
	bool v_billboard = false;
	bool cam_billboard = true;

	ResourceTexture* texture = nullptr;
	ResourceMesh* particle_mesh = nullptr;

	//Rendering Stuff
	bool scene_colorAffected = true;
	bool light_Affected = true;
	bool receive_shadows = true;

	ComponentParticleEmitter* emitter = nullptr;
};

BE_END_NAMESPACE
#endif // __PARTICLE_H__