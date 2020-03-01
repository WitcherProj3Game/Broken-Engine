#include "Application.h"
#include "ComponentDynamicRigidBody.h"
#include "ComponentCollider.h"
#include "GameObject.h"
#include "ModulePhysics.h"

#include "Imgui/imgui.h"

#include "mmgr/mmgr.h"


ComponentDynamicRigidBody::ComponentDynamicRigidBody(GameObject* ContainerGO) : Component(ContainerGO, Component::ComponentType::DynamicRigidBody)
{
	/*if (!(rigidBody = ContainerGO->GetComponent<ComponentDynamicRigidBody>()->rigidBody))
		rigidBody = physx::PxCreateDynamic(*App->physics->mPhysics, PxTransform(), PxBoxGeometry(), *App->physics->mMaterial, 0.0f);*/
	if (rigidBody != nullptr) {
		SetMass(mass);
		SetDensity(density);
		UseGravity(use_gravity);
		SetKinematic(is_kinematic);
		/*SetLinearVelocity(linear_vel);
		SetAngularVelocity(angular_vel);
		SetLinearDamping(linear_damping);
		SetAngularDamping(angular_damping);*/
		FeezePosition_X(freezePosition_X);
		FeezePosition_Y(freezePosition_Y);
		FeezePosition_Z(freezePosition_Z);
		FreezeRotation_X(freezeRotation_X);
		FreezeRotation_Y(freezeRotation_Y);
		FreezeRotation_Z(freezeRotation_Z);
	}
}

ComponentDynamicRigidBody::~ComponentDynamicRigidBody()
{

}

json ComponentDynamicRigidBody::Save() const
{
	json node;

	node["mass"] = std::to_string(mass);
	node["density"] = std::to_string(density);
	node["use_gravity"] = std::to_string((int)use_gravity);
	node["is_kinematic"] = std::to_string((int)is_kinematic);

	node["freezePosition_X"] = std::to_string((int)freezePosition_X);
	node["freezePosition_Y"] = std::to_string((int)freezePosition_Y);
	node["freezePosition_Z"] = std::to_string((int)freezePosition_Z);

	node["freezeRotation_X"] = std::to_string((int)freezeRotation_X);
	node["freezeRotation_Y"] = std::to_string((int)freezeRotation_Y);
	node["freezeRotation_Z"] = std::to_string((int)freezeRotation_Z);

	node["linear_velx"] = std::to_string(linear_vel.x);
	node["linear_vely"] = std::to_string(linear_vel.y);
	node["linear_velz"] = std::to_string(linear_vel.z);

	node["angular_velx"] = std::to_string(angular_vel.x);
	node["angular_vely"] = std::to_string(angular_vel.y);
	node["angular_velz"] = std::to_string(angular_vel.z);

	node["linear_damping"] = std::to_string(linear_damping);

	node["angular_damping"] = std::to_string(angular_damping);

	return node;
}

void ComponentDynamicRigidBody::Load(json& node)
{
	std::string mass_ = node["mass"];
	std::string density_ = node["density"];
	std::string use_gravity_ = node["use_gravity"];
	std::string is_kinematic_ = node["is_kinematic"];

	std::string freezePosition_X_ = node["freezePosition_X"];
	std::string freezePosition_Y_ = node["freezePosition_Y"];
	std::string freezePosition_Z_ = node["freezePosition_Z"];

	std::string freezeRotation_X_ = node["freezeRotation_X"];
	std::string freezeRotation_Y_ = node["freezeRotation_Y"];
	std::string freezeRotation_Z_ = node["freezeRotation_Z"];

	std::string linear_velx = node["linear_velx"];
	std::string linear_vely = node["linear_vely"];
	std::string linear_velz = node["linear_velz"];

	std::string angular_velx = node["angular_velx"];
	std::string angular_vely = node["angular_vely"];
	std::string angular_velz = node["angular_velz"];

	std::string linear_damping_ = node["linear_damping"];

	std::string angular_damping_ = node["angular_damping"];

	mass = std::stof(mass_);
	density = std::stof(density_);
	use_gravity = std::stoi(use_gravity_);
	is_kinematic = std::stoi(is_kinematic_);

	freezePosition_X = std::stoi(freezePosition_X_);
	freezePosition_Y = std::stoi(freezePosition_Y_);
	freezePosition_Z = std::stoi(freezePosition_Z_);

	freezeRotation_X = std::stoi(freezeRotation_X_);
	freezeRotation_Y = std::stoi(freezeRotation_Y_);
	freezeRotation_Z = std::stoi(freezeRotation_Z_);

	linear_vel = float3(std::stof(linear_velx), std::stof(linear_vely), std::stof(linear_velz));

	angular_vel = float3(std::stof(angular_velx), std::stof(angular_vely), std::stof(angular_velz));

	linear_damping = std::stoi(linear_damping_);

	angular_damping = std::stoi(angular_damping_);


}

void ComponentDynamicRigidBody::CreateInspectorNode()
{
	ImGui::Checkbox("##Dynamic RigidBody", &GetActive()); ImGui::SameLine(); ImGui::Text("Dynamic RigidBody");

	ImGui::Text("Mass:"); ImGui::SameLine(); 
	if (ImGui::DragFloat("##M", &mass,1.0f, 0.0f, 100000.0f)) 
		SetMass(mass);

	ImGui::Text("Density:"); ImGui::SameLine(); 
	if (ImGui::DragFloat("##D", &density, 1.0f, 0.0f, 100000.0f))
		SetDensity(density);

	ImGui::Text("Gravity:"); ImGui::SameLine(); 
	if (ImGui::Checkbox("##G", &use_gravity))
		UseGravity(use_gravity);

	ImGui::Text("Kinematic:"); ImGui::SameLine(); 
	if (ImGui::Checkbox("##K", &is_kinematic))
		SetKinematic(is_kinematic);

	ImGui::Text("Linear Velocity:"); ImGui::SameLine(); ImGui::PushItemWidth(50); 
	if (ImGui::DragFloat("##LVX", &linear_vel.x))
		SetLinearVelocity(linear_vel);

		ImGui::SameLine(); 
	if (ImGui::DragFloat("##LVY", &linear_vel.y))
		SetLinearVelocity(linear_vel);

	ImGui::SameLine(); 
	
	if(ImGui::DragFloat("##LVZ", &linear_vel.z))
		SetLinearVelocity(linear_vel);
			
	ImGui::PopItemWidth();

	ImGui::Text("Angular Velocity:"); ImGui::SameLine(); ImGui::PushItemWidth(50); 
	
	if (ImGui::DragFloat("##AVX", &angular_vel.x))
		SetAngularVelocity(angular_vel);
	
	ImGui::SameLine(); 
	
	if (ImGui::DragFloat("##AVY", &angular_vel.y))
		SetAngularVelocity(angular_vel);
	
	ImGui::SameLine(); 
	if (ImGui::DragFloat("##AVZ", &angular_vel.z))
		SetAngularVelocity(angular_vel);
	
	ImGui::PopItemWidth();
	
	ImGui::Text("Linear Damping:"); ImGui::SameLine(); 
	
	if (ImGui::DragFloat("##LD", &linear_damping))
		SetLinearDamping(linear_damping);

	ImGui::Text("Angular Damping:"); ImGui::SameLine(); 
	
	if (ImGui::DragFloat("##AD", &angular_damping))
		SetAngularDamping(angular_damping);


	if (ImGui::TreeNode("Constraints"))
	{
		ImGui::Text("Freeze Position"); ImGui::SameLine(); ImGui::Checkbox("##FPX", &freezePosition_X); ImGui::SameLine(); ImGui::Checkbox("##FPY", &freezePosition_Y); ImGui::SameLine(); ImGui::Checkbox("##FPZ", &freezePosition_Z);
		ImGui::Text("Freeze Rotation"); ImGui::SameLine(); ImGui::Checkbox("##FRX", &freezeRotation_X); ImGui::SameLine(); ImGui::Checkbox("##FRY", &freezeRotation_Y); ImGui::SameLine(); ImGui::Checkbox("##FRZ", &freezeRotation_Z);
		ImGui::TreePop();
	}

	if (rigidBody != nullptr) {
		FeezePosition_X(freezePosition_X);
		FeezePosition_Y(freezePosition_Y);
		FeezePosition_Z(freezePosition_Z);
		FreezeRotation_X(freezeRotation_X);
		FreezeRotation_Y(freezeRotation_Y);
		FreezeRotation_Z(freezeRotation_Z);
	}

	StaticToDynamicRigidBody();
}

void ComponentDynamicRigidBody::StaticToDynamicRigidBody()
{
	ComponentCollider* collider = GO->GetComponent<ComponentCollider>();
	if (collider != nullptr && rigidBody == nullptr)
	{
		collider->CreateCollider(collider->type, true);
	}
}