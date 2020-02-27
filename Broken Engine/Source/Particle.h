#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "PhysX_3.4/Include/PxPhysicsAPI.h"

#include "Globals.h" 

#include "Math.h"

using namespace physx;

class Particle {
public:
	Particle();
	~Particle();

	void Draw();

public:

	float3 position;

	uint lifeTime;
	uint spawnTime = 0;
	float diameter;
};

#endif // __PARTICLE_H__