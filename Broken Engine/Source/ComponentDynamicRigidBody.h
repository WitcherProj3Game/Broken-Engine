#ifndef _COMPONENT_RIGIDBODY_H_
#define _COMPONENT_RIGIDBODY_H_

#include "Module.h"
#include "Globals.h"
#include "GameObject.h"

#include "PhysX_3.4/Include/PxPhysicsAPI.h"

using namespace physx;

class ComponentDynamicRigidBody : public Component
{
public:
	ComponentDynamicRigidBody(GameObject* ContainerGO);
	virtual ~ComponentDynamicRigidBody();

	inline PxReal GetMass() { return rigidBody->getMass(); }
	inline void SetMass(PxReal mass) { rigidBody->setMass(mass); }
	inline void SetDensity(PxReal density) { PxRigidBodyExt::updateMassAndInertia(*rigidBody, density); }

	inline void UseGravity(bool enable) { rigidBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !enable); }
	inline void SetKinematic(bool enable) { rigidBody->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, enable); }

	//Lineal
	inline float3 GetLinearVelocity() { return float3(rigidBody->getLinearVelocity().x, rigidBody->getLinearVelocity().y, rigidBody->getLinearVelocity().z); }
	inline float3 GetAngularVelocity() { return float3(rigidBody->getAngularVelocity().x, rigidBody->getAngularVelocity().y, rigidBody->getAngularVelocity().z); }
	inline void SetLinearVelocity(float3 vel) { rigidBody->setLinearVelocity(PxVec3(vel.x, vel.y, vel.z)); }
	inline void SetAngularVelocity(float3 vel) { rigidBody->setAngularVelocity(PxVec3(vel.x, vel.y, vel.z)); }

	inline void SetAngularDamping(PxReal value) { rigidBody->setAngularDamping(value); }
	inline void SetLinearDamping(PxReal value) { rigidBody->setLinearDamping(value); }
	inline PxReal GetAngularDamping() { rigidBody->getAngularDamping(); }
	inline PxReal GetLinearDamping() { rigidBody->getLinearDamping(); }

	//Acceleration
	inline void AddForce(PxVec3 force, PxForceMode::Enum mode) { rigidBody->addForce(force, mode); }
	inline void AddTorque(PxVec3 force, PxForceMode::Enum mode) { rigidBody->addTorque(force, mode); }

	inline void FeezePosition_X(bool enable) { rigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_X, enable); }
	inline void FeezePosition_Y(bool enable) { rigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, enable); }
	inline void FeezePosition_Z(bool enable) { rigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, enable); }
	inline void FreezeRotation_X(bool enable) { rigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, enable); }
	inline void FreezeRotation_Y(bool enable) { rigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, enable); }
	inline void FreezeRotation_Z(bool enable) { rigidBody->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, enable); }

	static inline Component::ComponentType GetType() { return Component::ComponentType::DynamicRigidBody; };

	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;
	void CreateInspectorNode() override;

private:
	void StaticToDynamicRigidBody();

public:
	physx::PxRigidDynamic* rigidBody = nullptr;

public:
	float mass = 0.f;
	float density = 1.f;
	bool use_gravity = true;
	bool is_kinematic = false;;
	float3 linear_vel = float3::zero;
	float3 angular_vel = float3::zero;
	float linear_damping = 0.f;
	float angular_damping = 0.f;
	bool freezePosition_X = false, freezePosition_Y = false, freezePosition_Z = false;
	bool freezeRotation_X = false, freezeRotation_Y = false, freezeRotation_Z = false;

};
#endif