#include "PhysxSimulationEvents.h"
#include "Application.h"
#include "ModulePhysics.h"
#include "GameObject.h"

using namespace Broken;

PhysxSimulationEvents::PhysxSimulationEvents(ModulePhysics* callback)
{
	this->callback = callback;
}

void PhysxSimulationEvents::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
{
	for (physx::PxU32 i = 0; i < nbPairs; ++i)
	{
		const physx::PxContactPair& cp = pairs[i];

		if (cp.events & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
		{
			GameObject* go1 = App->physics->actors[pairHeader.actors[0]];
			GameObject* go2 = App->physics->actors[pairHeader.actors[1]];

			go1->collisions.at(0) = go2;
			go2->collisions.at(0) = go1;
		}
	}
}

void PhysxSimulationEvents::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
{
	for (physx::PxU32 i = 0; i < count; ++i)
	{
		// ignore pairs when shapes have been deleted
		if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
			continue;

		GameObject* go1 = App->physics->actors[pairs->triggerActor];
		GameObject* go2 = App->physics->actors[pairs->otherActor];

		go1->collisions.at(1) = go2;
	}
}

