#include "Application.h"
#include "ComponentDynamicRigidBody.h"
#include "ComponentCollider.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "ModulePhysics.h"

#include "Imgui/imgui.h"

#include "mmgr/mmgr.h"

using namespace Broken;

ComponentDynamicRigidBody::ComponentDynamicRigidBody(GameObject* ContainerGO) : Component(ContainerGO, Component::ComponentType::DynamicRigidBody)
{
	name = "Dynamic RigidBody";


	if (rigidBody != nullptr)
	{
		SetMass(mass);
		SetDensity(density);
		UseGravity(use_gravity);
		SetKinematic(is_kinematic);
		SetLinearVelocity(linear_vel);
		SetAngularVelocity(angular_vel);
		SetLinearDamping(linear_damping);
		SetAngularDamping(angular_damping);
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
	App->physics->DeleteActor(rigidBody);
}

void ComponentDynamicRigidBody::Update()
{
	setRBValues();

	UpdateRBValues();

	if (to_delete)
		this->GetContainerGameObject()->RemoveComponent(this);
}

json ComponentDynamicRigidBody::Save() const
{
	json node;

	node["Active"] = this->active;

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
	this->active = node.contains("Active") ? (bool)node["Active"] : true;

	std::string mass_ = node.contains("mass") ? node["mass"] : "0";
	std::string density_ = node.contains("density") ? node["density"] : "0";
	std::string use_gravity_ = node.contains("use_gravity") ? node["use_gravity"] : "0";
	std::string is_kinematic_ = node.contains("is_kinematic") ? node["is_kinematic"] : "0";

	std::string freezePosition_X_ = node.contains("freezePosition_X") ? node["freezePosition_X"] : "0";
	std::string freezePosition_Y_ = node.contains("freezePosition_Y") ? node["freezePosition_Y"] : "0";
	std::string freezePosition_Z_ = node.contains("freezePosition_Z") ? node["freezePosition_Z"] : "0";

	std::string freezeRotation_X_ = node.contains("freezeRotation_X") ? node["freezeRotation_X"] : "0";
	std::string freezeRotation_Y_ = node.contains("freezeRotation_Y") ? node["freezeRotation_Y"] : "0";
	std::string freezeRotation_Z_ = node.contains("freezeRotation_Z") ? node["freezeRotation_Z"] : "0";

	std::string linear_velx = node.contains("linear_velx") ? node["linear_velx"] : "0";
	std::string linear_vely = node.contains("linear_vely") ? node["linear_vely"] : "0";
	std::string linear_velz = node.contains("linear_velz") ? node["linear_velz"] : "0";

	std::string angular_velx = node.contains("angular_velx") ? node["angular_velx"] : "0";
	std::string angular_vely = node.contains("angular_vely") ? node["angular_vely"] : "0";
	std::string angular_velz = node.contains("angular_velz") ? node["angular_velz"] : "0";

	std::string linear_damping_ = node.contains("linear_damping") ? node["linear_damping"] : "0";

	std::string angular_damping_ = node.contains("angular_damping") ? node["angular_damping"] : "0";

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

	setRBValues();
	update = true;
}

void ComponentDynamicRigidBody::CreateInspectorNode()
{
	if (ImGui::Button("Delete component"))
		to_delete = true;

	ImGui::Text("Mass:"); ImGui::SameLine();
	if (ImGui::DragFloat("##M", &mass, 1.0f, 0.0f, 100000.0f))
	{
		update = true;
	}

	ImGui::Text("Density:"); ImGui::SameLine();
	if (ImGui::DragFloat("##D", &density, 1.0f, 0.0f, 100000.0f))
	{
		update = true;
	}

	ImGui::Text("Gravity:"); ImGui::SameLine();
	if (ImGui::Checkbox("##G", &use_gravity))
	{
		update = true;
	}

	ImGui::Text("Kinematic:"); ImGui::SameLine();
	if (ImGui::Checkbox("##K", &is_kinematic))
	{
		update = true;
	}

	ImGui::Text("Linear Velocity:"); ImGui::SameLine(); ImGui::PushItemWidth(50);
	if (ImGui::DragFloat("##LVX", &linear_vel.x))
	{
		update = true;
	}

	ImGui::SameLine();
	if (ImGui::DragFloat("##LVY", &linear_vel.y))
	{
		update = true;
	}

	ImGui::SameLine();

	if (ImGui::DragFloat("##LVZ", &linear_vel.z))
	{
		update = true;
	}

	ImGui::PopItemWidth();

	ImGui::Text("Angular Velocity:"); ImGui::SameLine(); ImGui::PushItemWidth(50);

	if (ImGui::DragFloat("##AVX", &angular_vel.x))
	{
		update = true;
	}

	ImGui::SameLine();

	if (ImGui::DragFloat("##AVY", &angular_vel.y))
	{
		update = true;
	}

	ImGui::SameLine();
	if (ImGui::DragFloat("##AVZ", &angular_vel.z))
	{
		update = true;
	}

	ImGui::PopItemWidth();

	ImGui::Text("Linear Damping:"); ImGui::SameLine();

	if (ImGui::DragFloat("##LD", &linear_damping))
	{
		update = true;
	}

	ImGui::Text("Angular Damping:"); ImGui::SameLine();

	if (ImGui::DragFloat("##AD", &angular_damping))
	{
		update = true;
	}


	if (ImGui::TreeNode("Constraints"))
	{
		ImGui::Text("Freeze Position"); ImGui::SameLine();
		if (ImGui::Checkbox("##FPX", &freezePosition_X))
			update = true;
		ImGui::SameLine();
		if (ImGui::Checkbox("##FPY", &freezePosition_Y))
			update = true;
		ImGui::SameLine();
		if (ImGui::Checkbox("##FPZ", &freezePosition_Z))
			update = true;

		ImGui::Text("Freeze Rotation"); ImGui::SameLine();
		if (ImGui::Checkbox("##FRX", &freezeRotation_X))
			update = true;
		ImGui::SameLine();
		if (ImGui::Checkbox("##FRY", &freezeRotation_Y))
			update = true;
		ImGui::SameLine();
		if (ImGui::Checkbox("##FRZ", &freezeRotation_Z))
			update = true;
		ImGui::TreePop();
	}

	StaticToDynamicRigidBody();
}

void ComponentDynamicRigidBody::StaticToDynamicRigidBody()
{
	ComponentCollider* collider = GO->GetComponent<ComponentCollider>();
	if (collider != nullptr && rigidBody == nullptr)
	{
		collider->CreateCollider(collider->type, true);
		update = true;
	}
}

void ComponentDynamicRigidBody::setRBValues() {
	if (rigidBody != nullptr)
	{
		if (linear_vel.x != 0.0f || linear_vel.y != 0.0f || linear_vel.z != 0.0f)
			SetLinearVelocity(linear_vel);

		if (angular_vel.x != 0.0f || angular_vel.y != 0.0f || angular_vel.z != 0.0f)
			SetAngularVelocity(angular_vel);
	}
}

void ComponentDynamicRigidBody::UpdateRBValues() {
	if (rigidBody != nullptr && update) {
		SetMass(mass);
		SetDensity(density);
		UseGravity(use_gravity);
		SetKinematic(is_kinematic);
		SetLinearVelocity(linear_vel);
		SetAngularVelocity(angular_vel);
		SetLinearDamping(linear_damping);
		SetAngularDamping(angular_damping);
		FeezePosition_X(freezePosition_X);
		FeezePosition_Y(freezePosition_Y);
		FeezePosition_Z(freezePosition_Z);
		FreezeRotation_X(freezeRotation_X);
		FreezeRotation_Y(freezeRotation_Y);
		FreezeRotation_Z(freezeRotation_Z);
		//rigidBody->setGlobalPose(rigidBody->getGlobalPose());
		update = false;
	}
}
