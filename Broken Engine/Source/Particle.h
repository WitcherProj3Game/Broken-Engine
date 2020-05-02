#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "Math.h"
#include "BrokenCore.h"

BE_BEGIN_NAMESPACE

class ResourceTexture;
class ResourceMesh;

class BROKEN_API Particle {
public:
	Particle();
	~Particle();

	void SetAnimation(ResourceMesh* mesh);

	void Draw();

public:


	float3 position = { 0,0,0 };
	float4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	uint lifeTime=1;
	uint spawnTime = 1;
	int currentGradient = 0;
	int gradientTimer = 0;
	float diameter=1;
	float3 rotation = { 0,0,0 };
	float3 scale = {1,1,1};
	float3 rotationSpeed = { 0,0,0 };
	float3 scaleSpeed = { 0,0,0 };

	ResourceTexture* texture = nullptr;
	ResourceMesh* plane = nullptr;
};

BE_END_NAMESPACE
#endif // __PARTICLE_H__