#include "ComponentCollider.h"
#include "Application.h"
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentDynamicRigidBody.h"
#include "ComponentMesh.h"
#include "ModulePhysics.h"
#include "ModuleSceneManager.h"
#include "ModuleResourceManager.h"
#include "ModuleTimeManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleGui.h"
#include "ResourceShader.h"
#include "ResourceMesh.h"
#include "ComponentMeshRenderer.h"
#include "OpenGL.h"
#include "ResourceScene.h"
#include "ModuleThreading.h"
#include "ComponentCharacterController.h"

#include "PhysX_3.4/Include/PxPhysicsAPI.h"
#include "PhysX_3.4/Include/PxSimulationEventCallback.h"

#include "Imgui/imgui.h"
#include "Imgui/imgui_internal.h"

#include "Timer.h"

#include "mmgr/mmgr.h"

using namespace Broken;

ComponentCollider::ComponentCollider(GameObject* ContainerGO) : Component(ContainerGO, Component::ComponentType::Collider)
{
	name = "Collider";
	mesh = (ResourceMesh*)App->resources->CreateResource(Resource::ResourceType::MESH, "DefaultColliderMesh");
}

ComponentCollider::~ComponentCollider()
{
	mesh->Release();
}

void ComponentCollider::Update()
{
	if (editCollider) {
		UpdateCollider();
	}

	if (to_delete)
	{
		Delete();
		this->GetContainerGameObject()->RemoveComponent(this);
	}
}

void ComponentCollider::Enable()
{
	if (GetActor() != nullptr)
	{
		if (hasBeenDeactivated)
		{
			App->physics->mScene->addActor(*GetActor());
			hasBeenDeactivated = false;
		}
		//GetActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
	}
	active = true;
}

void ComponentCollider::Disable()
{
	if (GetActor() != nullptr)
	{
		//GetActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
		if (!hasBeenDeactivated)
		{
			App->physics->mScene->removeActor(*GetActor());
			hasBeenDeactivated = true;
		}
	}
	active = false;
}

void ComponentCollider::UpdateCollider() {
	CreateCollider((ComponentCollider::COLLIDER_TYPE)colliderType, true);
}

void ComponentCollider::DrawComponent()
{
	if (shape)
	{
		// --- Get shape's dimensions ---
		physx::PxGeometryHolder holder = shape->getGeometry();
		physx::PxGeometryType::Enum type = holder.getType();

		if (!mesh->IsInMemory())
		{
			// --- Draw shape according to type ---
			switch (type)
			{
			case physx::PxGeometryType::eSPHERE:
			{
				// --- Rebuild sphere ---
				App->scene_manager->CreateSphere(1, 25, 25, mesh);
				mesh->LoadToMemory();
			}
			break;
			case physx::PxGeometryType::eCAPSULE:
			{
				// --- Rebuild capsule ---
				App->scene_manager->CreateCapsule(radius, height, mesh);
				mesh->LoadToMemory();
			}
			break;
			case physx::PxGeometryType::eBOX:
			{
				// --- Rebuild box ---
				App->scene_manager->CreateCube(1, 1, 1, mesh);
				mesh->LoadToMemory();
			}
			break;
			default:
				break;
			}
		}
	}

	if (!dragged_mesh)
		current_mesh = mesh;
	else
		current_mesh = dragged_mesh;

	// --- Render shape ---
	if (current_mesh && current_mesh->IsInMemory() && current_mesh->vertices && current_mesh->Indices && draw && App->GetAppState() != AppState::PLAY)
	{
		RenderMeshFlags flags = wire;
		App->renderer3D->DrawMesh(globalMatrix * float4x4::FromQuat(dragged_rot), current_mesh, (ResourceMaterial*)App->resources->GetResource(App->resources->GetDefaultMaterialUID(), false), nullptr, flags, Color(125, 125, 125));
	}

}

void ComponentCollider::UpdateLocalMatrix() {

	ComponentDynamicRigidBody* dynamicRB = GO->GetComponent<ComponentDynamicRigidBody>();
	ComponentTransform* cTransform = GO->GetComponent<ComponentTransform>();

	if (!rigidStatic && (!dynamicRB || !dynamicRB->rigidBody))
		return;

	math::float4x4 gt = cTransform->GetGlobalTransform();

	//Render
	localMatrix.x = centerPosition.x + offset.x;
	localMatrix.y = centerPosition.y + offset.y;
	localMatrix.z = centerPosition.z + offset.z;

	localMatrix.scaleX = colliderSize.x * originalSize.x;
	localMatrix.scaleY = colliderSize.y * originalSize.y;
	localMatrix.scaleZ = colliderSize.z * originalSize.z;

	globalMatrix = gt * localMatrix;

	//PHYSX DEBUG
	float3 pos, scale;
	Quat rot;
	globalMatrix.Decompose(pos, rot, scale);

	float threshold = 0.1f;

	if ((math::Abs(scale.x - tmpScale.x) > threshold
		|| math::Abs(scale.y - tmpScale.y) > threshold
		|| math::Abs(scale.z - tmpScale.z) > threshold) && cTransform->updateValues)
	{
		editCollider = true;
		tmpScale = scale;
	}

	physx::PxVec3 posi(pos.x, pos.y, pos.z);
	physx::PxQuat quati(rot.x, rot.y, rot.z, rot.w);
	physx::PxTransform transform(posi, quati);


	if (dynamicRB == nullptr)
		rigidStatic->setGlobalPose(transform); //ON EDITOR
	else
	{
		if ((App->gui->isUsingGuizmo && App->GetAppState() != AppState::PLAY) || cTransform->updateValues || dynamicRB->is_kinematic) { //ON EDITOR

			dynamicRB->rigidBody->setGlobalPose(transform); 
			cTransform->updateValues = false;
		}
		if (dynamicRB->rigidBody != nullptr && !dynamicRB->is_kinematic && App->GetAppState() == AppState::PLAY) //ON GAME
		{
			UpdateTransformByRigidBody(dynamicRB, cTransform, &transform);
		}
	}
}

void ComponentCollider::UpdateTransformByRigidBody(ComponentDynamicRigidBody* RB, ComponentTransform* cTransform, physx::PxTransform* globalPos) {
	physx::PxTransform transform;
	if (!RB)
		return;

	if (globalPos && App->GetAppState() == AppState::PLAY && !toPlay && !App->time->gamePaused) {
		RB->rigidBody->setGlobalPose(*globalPos);
		toPlay = true;
	}


	transform = RB->rigidBody->getGlobalPose();
	float3 position = float3(
		(transform.p.x - globalMatrix.x) + cTransform->GetGlobalTransform().x,
		(transform.p.y - globalMatrix.y) + cTransform->GetGlobalTransform().y,
		(transform.p.z - globalMatrix.z) + cTransform->GetGlobalTransform().z);

	float4x4 new_transform = float4x4::FromTRS(
		position,
		Quat(transform.q.x, transform.q.y, transform.q.z, transform.q.w), 
		cTransform->GetGlobalTransform().Transposed().GetScale());

	cTransform->SetGlobalTransform(new_transform);
}

json ComponentCollider::Save() const
{
	//ENGINE_CONSOLE_LOG("Saved");
	json node;

	node["Active"] = this->active;

	int colliderType = 0;
	switch (type)
	{
	case COLLIDER_TYPE::NONE:
		colliderType = 0;
		break;
	case COLLIDER_TYPE::BOX:
		colliderType = 1;
		break;
	case COLLIDER_TYPE::SPHERE:
		colliderType = 2;
		break;
	case COLLIDER_TYPE::CAPSULE:
		colliderType = 3;
		break;
	case COLLIDER_TYPE::MESH:
		colliderType = 4;
		break;
	}

	node["colliderType"] = std::to_string(colliderType);
	node["isTrigger"] = std::to_string(isTrigger);
	node["isConvex"] = std::to_string(isConvex);

	node["localPositionx"] = std::to_string(centerPosition.x);
	node["localPositiony"] = std::to_string(centerPosition.y);
	node["localPositionz"] = std::to_string(centerPosition.z);

	node["originalScalex"] = std::to_string(originalSize.x);
	node["originalScaley"] = std::to_string(originalSize.y);
	node["originalScalez"] = std::to_string(originalSize.z);

	node["offsetx"] = std::to_string(offset.x);
	node["offsety"] = std::to_string(offset.y);
	node["offsetz"] = std::to_string(offset.z);

	node["globalPositionx"] = std::to_string(originalSize.x);
	node["globalPositiony"] = std::to_string(originalSize.y);
	node["globalPositionz"] = std::to_string(originalSize.z);

	node["localMatrixx"] = std::to_string(localMatrix.x);
	node["localMatrixy"] = std::to_string(localMatrix.y);
	node["localMatrixz"] = std::to_string(localMatrix.z);
	node["localMatrixw"] = std::to_string(localMatrix.w);

	node["globalMatrixx"] = std::to_string(globalMatrix.x);
	node["globalMatrixy"] = std::to_string(globalMatrix.y);
	node["globalMatrixz"] = std::to_string(globalMatrix.z);
	node["globalMatrixw"] = std::to_string(globalMatrix.w);

	node["scalex"] = std::to_string(colliderSize.x);
	node["scaley"] = std::to_string(colliderSize.y);
	node["scalez"] = std::to_string(colliderSize.z);

	node["radius"] = std::to_string(radius);

	node["height"] = std::to_string(height);

	node["lastIndex"] = std::to_string(lastIndex);

	node["colliderType"] = std::to_string(colliderType);

	node["tmpScalex"] = std::to_string(tmpScale.x);
	node["tmpScaley"] = std::to_string(tmpScale.y);
	node["tmpScalez"] = std::to_string(tmpScale.z);

	node["firstCreation"] = std::to_string(firstCreation);

	node["draggedUID"] = std::to_string(dragged_UID);

	node["draw"] = std::to_string(draw);

	if (hasBeenDeactivated)
		node["hasBeenDeactivated"] = std::to_string(1);
	else
		node["hasBeenDeactivated"] = std::to_string(0);


	return node;
}

void ComponentCollider::Load(json& node)
{
	this->active = node["Active"].is_null() ? true : (bool)node["Active"];

	//ENGINE_CONSOLE_LOG("Load");
	//CreateCollider(COLLIDER_TYPE::NONE, true);

	std::string localPositionx = node["localPositionx"].is_null() ? "0" : node["localPositionx"];
	std::string localPositiony = node["localPositiony"].is_null() ? "0" : node["localPositiony"];
	std::string localPositionz = node["localPositionz"].is_null() ? "0" : node["localPositionz"];

	std::string originalScalex = node["originalScalex"].is_null() ? "0" : node["originalScalex"];
	std::string originalScaley = node["originalScaley"].is_null() ? "0" : node["originalScaley"];
	std::string originalScalez = node["originalScalez"].is_null() ? "0" : node["originalScalez"];

	std::string offsetx = node["offsetx"].is_null() ? "0" : node["offsetx"];
	std::string offsety = node["offsety"].is_null() ? "0" : node["offsety"];
	std::string offsetz = node["offsetz"].is_null() ? "0" : node["offsetz"];

	std::string localMatrixx = node["localMatrixx"].is_null() ? "0" : node["localMatrixx"];
	std::string localMatrixy = node["localMatrixy"].is_null() ? "0" : node["localMatrixy"];
	std::string localMatrixz = node["localMatrixz"].is_null() ? "0" : node["localMatrixz"];
	std::string localMatrixw = node["localMatrixw"].is_null() ? "0" : node["localMatrixw"];

	std::string globalMatrixx = node["globalMatrixx"].is_null() ? "0" : node["globalMatrixx"];
	std::string globalMatrixy = node["globalMatrixy"].is_null() ? "0" : node["globalMatrixy"];
	std::string globalMatrixz = node["globalMatrixz"].is_null() ? "0" : node["globalMatrixz"];
	std::string globalMatrixw = node["globalMatrixw"].is_null() ? "0" : node["globalMatrixw"];

	std::string scalex = node["scalex"].is_null() ? "0" : node["scalex"];
	std::string scaley = node["scaley"].is_null() ? "0" : node["scaley"];
	std::string scalez = node["scalez"].is_null() ? "0" : node["scalez"];

	std::string radius_ = node["radius"].is_null() ? "0" : node["radius"];

	std::string height_ = node["height"].is_null() ? "0" : node["height"];

	std::string lastIndex_ = node["lastIndex"].is_null() ? "0" : node["lastIndex"];

	std::string colliderType_ = node["colliderType"].is_null() ? "0" : node["colliderType"];

	std::string isTrigger_ = node["isTrigger"].is_null() ? "0" : node["isTrigger"];
	std::string isConvex_ = node["isConvex"].is_null() ? "0" : node["isConvex"];

	std::string tmpScalex = node["tmpScalex"].is_null() ? "0" : node["tmpScalex"];
	std::string tmpScaley = node["tmpScaley"].is_null() ? "0" : node["tmpScaley"];
	std::string tmpScalez = node["tmpScalez"].is_null() ? "0" : node["tmpScalez"];

	std::string firstCreation_ = node["firstCreation"].is_null() ? "0" : node["firstCreation"];

	std::string hasBeenDeactivated_ = node["hasBeenDeactivated"].is_null() ? "0" : node["hasBeenDeactivated"];

	std::string draggedUID_ = node["draggedUID"].is_null() ? "0" : node["draggedUID"];

	std::string draw_ = node["draw"].is_null() ? "0" : node["draw"];


	centerPosition = float3(std::stof(localPositionx), std::stof(localPositiony), std::stof(localPositionz));
	originalSize = float3(std::stof(originalScalex), std::stof(originalScaley), std::stof(originalScalez));
	colliderSize = float3(std::stof(scalex), std::stof(scaley), std::stof(scalez));
	offset = float3(std::stof(offsetx), std::stof(offsety), std::stof(offsetz));

	localMatrix.x = std::stof(localMatrixx);
	localMatrix.y = std::stof(localMatrixy);
	localMatrix.z = std::stof(localMatrixz);
	localMatrix.w = std::stof(localMatrixw);

	globalMatrix.x = std::stof(globalMatrixx);
	globalMatrix.y = std::stof(globalMatrixy);
	globalMatrix.z = std::stof(globalMatrixz);
	globalMatrix.w = std::stof(globalMatrixw);


	radius = std::stof(radius_);
	height = std::stof(height_);
	lastIndex = std::stoi(lastIndex_);
	colliderType = std::stoi(colliderType_);
	isTrigger = std::stoi(isTrigger_);
	isConvex = std::stoi(isConvex_);

	tmpScale = float3(std::stof(tmpScalex), std::stof(tmpScaley), std::stof(tmpScalez));

	firstCreation = false;

	draw = std::stof(draw_);

	dragged_UID = std::atoi(draggedUID_.c_str());

	toPlay = false;

	std::string colliderEnum = node["colliderType"];

	lastIndex = 0;

	int tmp = std::stoi(colliderEnum);
	colliderType = tmp;
	type = (ComponentCollider::COLLIDER_TYPE)colliderType;
	editCollider = true;
}

void ComponentCollider::CreateInspectorNode()
{
	bool createAgain = false;

	if (ImGui::Button("Delete component"))
		to_delete = true;

	ImGui::SameLine();
	ImGui::Checkbox("Show", &draw);

	if (ImGui::Combo("Type", &colliderType, "NONE\0BOX\0SPHERE\0CAPSULE\0MESH\0\0"))
	{
		type = (ComponentCollider::COLLIDER_TYPE)colliderType;
		current_mesh = nullptr;
		colliderSize = float3::one;
		centerPosition = float3::zero;
		dragged_rot = Quat::identity;
		dragged_scale = float3::one;
		dragged_mesh = nullptr;
		editCollider = true;
		firstCreation = false;
		dragged_UID = 0;
	}

	if (type != ComponentCollider::COLLIDER_TYPE::MESH && type != ComponentCollider::COLLIDER_TYPE::NONE) {
		ImGui::Text("Is Trigger");
		ImGui::SameLine();
		if (ImGui::Checkbox("##T", &isTrigger))
			editCollider = true;

		float3* position = &centerPosition;

		ImGui::Text("Center");
		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

		ImGui::DragFloat("##PX", &position->x, 0.005f);

		ImGui::SameLine();

		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

		ImGui::DragFloat("##PY", &position->y, 0.005f);

		ImGui::SameLine();

		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

		ImGui::DragFloat("##PZ", &position->z, 0.005f);
	}

	switch (type)
	{
	case  ComponentCollider::COLLIDER_TYPE::SPHERE:
	{
		float prevRadius = radius;

		ImGui::Text("Radius");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##R", &radius, 0.005f);

		colliderSize.x = radius;
		colliderSize.y = radius;
		colliderSize.z = radius;

		if (prevRadius != radius || editCollider) {
			editCollider = true;
			colliderType = (int)COLLIDER_TYPE::SPHERE;
		}

		break;
	}
	case ComponentCollider::COLLIDER_TYPE::BOX:
	{
		float3 prevScale = colliderSize;
		ImGui::Text("Size");
		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

		ImGui::DragFloat("##SX", &colliderSize.x, 0.005f, 0.01f, 1000.0f);

		ImGui::SameLine();

		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

		ImGui::DragFloat("##SY", &colliderSize.y, 0.005f, 0.01f, 1000.0f);

		ImGui::SameLine();

		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

		ImGui::DragFloat("##SZ", &colliderSize.z, 0.005f, 0.01f, 1000.0f);

		if (prevScale.x != colliderSize.x || prevScale.y != colliderSize.y || prevScale.z != colliderSize.z || editCollider) {
			editCollider = true;
			colliderType = (int)COLLIDER_TYPE::BOX;
		}

		break;
	}
	case ComponentCollider::COLLIDER_TYPE::CAPSULE:
	{
		float prevRadius = radius;
		float prevheight = height;

		ImGui::Text("Radius");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##R", &radius, 0.005f);

		ImGui::Text("Height");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##H", &height, 0.005f);

		colliderSize.x = radius;
		colliderSize.y = height;
		colliderSize.z = radius;

		if (prevRadius != radius || prevheight != height || editCollider) {
			editCollider = true;
			colliderType = (int)COLLIDER_TYPE::CAPSULE;
		}
		break;
	}
	case ComponentCollider::COLLIDER_TYPE::MESH:
	{
		if (!firstCreation) {
			if (GO->HasComponent(ComponentType::Mesh))
				dragged_UID = GO->GetUID();
			firstCreation = true;
		}


		if (ImGui::Checkbox("Convex", &isConvex)) {
			editCollider = true;
			if (dragged_mesh) {
				if (triangle_mesh) {
					triangle_mesh = nullptr;
				}
				if (convex_mesh) {
					convex_mesh = nullptr;
				}
				dragged_mesh = nullptr;
			}
		}

		if (!isConvex) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			isTrigger = false;
		}

		ImGui::NewLine();
		ImGui::SameLine(50.f);
		if (ImGui::Checkbox("Trigger", &isTrigger)) {
			editCollider = true;
			if (dragged_mesh) {
				dragged_mesh = nullptr;
				if (convex_mesh) {
					convex_mesh->release();
					convex_mesh = nullptr;
				}
			}
		}

		if (!isConvex) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}

		ImGui::Text("Mesh");
		ImGui::SameLine();

		if (!dragged_mesh)
			ImGui::Button("Drag GO with Mesh", ImVec2(150, 20));
		else
			ImGui::Button(dragged_mesh->GetName(), ImVec2(150, 20));

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GO"))
			{
				uint UID = *(const uint*)payload->Data;
				GameObject* go = App->scene_manager->currentScene->GetGOWithUID(UID);
				if (go->HasComponent(ComponentType::Mesh)) {
					dragged_UID = UID;
					editCollider = true;
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		if (ImGui::Button("X")) {
			current_mesh = nullptr;
			colliderSize = float3::one;
			centerPosition = float3::zero;
			dragged_rot = Quat::identity;
			dragged_scale = float3::one;
			dragged_UID = 0;
			dragged_mesh = nullptr;
			if (convex_mesh) {
				convex_mesh->release();
				convex_mesh = nullptr;
			}
			if (triangle_mesh) {
				triangle_mesh->release();
				triangle_mesh = nullptr;
			}
			editCollider = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Update")) {
			editCollider = true;
		}
	}
	}
}

void ComponentCollider::GetMesh() {

	if (dragged_UID != 0) {
		GameObject* go = App->scene_manager->currentScene->GetGOWithUID(dragged_UID);
		if (go) {
			if (go->HasComponent(ComponentType::Mesh)) {
				dragged_mesh = go->GetComponent<ComponentMesh>()->resource_mesh;
				dragged_scale = GO->GetComponent<ComponentTransform>()->GetGlobalTransform().GetScale();
				if (go != GO) {
					centerPosition = go->GetComponent<ComponentTransform>()->GetPosition();
					dragged_rot = go->GetComponent<ComponentTransform>()->GetQuaternionRotation();
					colliderSize = dragged_scale;
					localMesh = true;
				}
			}
		}
	}
}


void ComponentCollider::CreateCollider(ComponentCollider::COLLIDER_TYPE type, bool createAgain) {
	if (shape != nullptr && (lastIndex != (int)type || createAgain)) {
		shape->release();
		shape = nullptr;
		if (GO->GetComponent<ComponentDynamicRigidBody>() != nullptr)
		{
			if (GO->GetComponent<ComponentDynamicRigidBody>()->rigidBody != nullptr) {
				App->physics->DeleteActor(GO->GetComponent<ComponentDynamicRigidBody>()->rigidBody);
				GO->GetComponent<ComponentDynamicRigidBody>()->rigidBody = nullptr;
			}
			if (createAgain && rigidStatic) {
				App->physics->DeleteActor(rigidStatic);
				rigidStatic = nullptr;
			}
		}
		else {
			App->physics->DeleteActor(rigidStatic);
			rigidStatic = nullptr;
		}

		// --- Make sure to always enter here or else the mesh's data won't be released!!! ---
		if (mesh && mesh->IsInMemory())
		{
			mesh->Release();
		}
	}

	if (lastIndex == (int)type && !createAgain) {
		return;
	}

	physx::PxTransform localTransform(physx::PxVec3(centerPosition.x, centerPosition.y, centerPosition.z));
	ComponentTransform* transform = GO->GetComponent<ComponentTransform>();
	float3 tScale = transform->GetScale();

	float3 pos, scale;
	Quat quat;
	transform->GetGlobalTransform().Decompose(pos, quat, scale);

	switch (type) {
	case ComponentCollider::COLLIDER_TYPE::BOX: {

		float3 center;

		physx::PxBoxGeometry boxGeometry;
		Quat q = transform->rotation;
		Quat qInverse = q;
		if (!firstCreation)
		{
			qInverse.InverseAndNormalize();
			Quat tmp = q * qInverse;
			transform->SetRotation(tmp);

			GO->UpdateAABB();

			if (GO->HasComponent(ComponentType::Mesh))
				originalSize = GO->GetOBB().Size().Div(scale);
			else
				originalSize = float3::one;

			center = GO->GetAABB().CenterPoint();

		}

		boxGeometry = physx::PxBoxGeometry(physx::PxVec3(originalSize.x * scale.x * colliderSize.x * 0.5, originalSize.y * scale.y * colliderSize.y * 0.5, originalSize.z * scale.z * colliderSize.z * 0.5));

		physx::PxTransform position(physx::PxVec3(center.x, center.y, center.z));

		shape = App->physics->mPhysics->createShape(boxGeometry, *App->physics->mMaterial);

		CreateRigidbody(boxGeometry, position);

		if (!firstCreation)
		{
			transform->SetRotation(q); //RESET TO ORIGNAL ROTATION

			GO->UpdateAABB();
			firstCreation = true;

			//center = GO->GetOBB().CenterPoint();
			float3 dir = center-transform->GetGlobalPosition();
			float3 dir2 = quat.Inverted().Mul(dir); // rotate it
			offset = (dir2.Div(scale));

			offset.Mul(scale);
		}

		lastIndex = (int)ComponentCollider::COLLIDER_TYPE::BOX;
		break;
	}
	case ComponentCollider::COLLIDER_TYPE::SPHERE: {
		physx::PxSphereGeometry SphereGeometry(radius);
		shape = App->physics->mPhysics->createShape(SphereGeometry, *App->physics->mMaterial);
		shape->setGeometry(SphereGeometry);

		physx::PxTransform transform(transform->position.x, transform->position.y, transform->position.z);
		CreateRigidbody(SphereGeometry, transform);

		lastIndex = (int)ComponentCollider::COLLIDER_TYPE::SPHERE;
		break;
	}
	case ComponentCollider::COLLIDER_TYPE::CAPSULE: {
		physx::PxCapsuleGeometry CapsuleGeometry(radius, height);
		shape = App->physics->mPhysics->createShape(CapsuleGeometry, *App->physics->mMaterial);
		physx::PxTransform relativePose(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1)));
		shape->setLocalPose(relativePose);

		physx::PxTransform transform(transform->position.x, transform->position.y, transform->position.z);
		CreateRigidbody(CapsuleGeometry, transform);

		lastIndex = (int)ComponentCollider::COLLIDER_TYPE::CAPSULE;
		break;
	}
	case ComponentCollider::COLLIDER_TYPE::MESH: {
		if (!firstCreation) {
			if (GO->HasComponent(ComponentType::Mesh)) {
				dragged_UID = GO->GetUID();
			}
			firstCreation = true;
		}
		GetMesh();
		if (dragged_mesh) {
			physx::PxTransform position(GO->GetAABB().CenterPoint().x, GO->GetAABB().CenterPoint().y, GO->GetAABB().CenterPoint().z);
			Quat aux = dragged_rot;
			float3 rot_scale = aux.Inverted().Mul(dragged_scale).Abs();
			if (localMesh)
				colliderSize = rot_scale;
			physx::PxMeshScale mesh_scale(physx::PxVec3(rot_scale.x, rot_scale.y, rot_scale.z), physx::PxQuat(dragged_rot.x, dragged_rot.y, dragged_rot.z, dragged_rot.w));
			//physx::PxMeshScale mesh_scale(physx::PxVec3(dragged_scale.x, dragged_scale.y, dragged_scale.z), physx::PxQuat(physx::PxIDENTITY()));

			if (isConvex) {//Convex Collider
				if (!App->physics->cooked_convex[dragged_mesh]) {//Cook Mesh
					physx::PxVec3* vertices = new physx::PxVec3[dragged_mesh->VerticesSize];
					for (int j = 0; j < dragged_mesh->VerticesSize; ++j) {
						vertices[j].x = dragged_mesh->vertices[j].position[0];
						vertices[j].y = dragged_mesh->vertices[j].position[1];
						vertices[j].z = dragged_mesh->vertices[j].position[2];
					}

					physx::PxConvexMeshDesc convexDesc;
					convexDesc.points.count = dragged_mesh->VerticesSize;
					convexDesc.points.stride = sizeof(physx::PxVec3);
					convexDesc.points.data = vertices;

					///--------------------------------COOKING-------------------------------------------
					//convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;

					//physx::PxDefaultMemoryOutputStream outStream;
					//if (!App->physics->mCooking->cookConvexMesh(convexDesc, outStream)) {
					//	return ENGINE_CONSOLE_LOG("| Error cooking vertices on Mesh Collider");
					//}

					//// Create the mesh from a stream.
					//physx::PxDefaultMemoryInputData inStream(outStream.getData(), outStream.getSize());
					//convex_mesh = App->physics->mPhysics->createConvexMesh(inStream);
					///--------------------------------RUNTIME-------------------------------------------
					convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX | physx::PxConvexFlag::eDISABLE_MESH_VALIDATION | physx::PxConvexFlag::eFAST_INERTIA_COMPUTATION;
					convex_mesh = App->physics->mCooking->createConvexMesh(convexDesc, App->physics->mPhysics->getPhysicsInsertionCallback());
					///-----------------------------------------------------------------------------------

					App->physics->cooked_convex.insert(std::pair<ResourceMesh*, physx::PxConvexMesh*>(dragged_mesh, nullptr));
					App->physics->cooked_convex[dragged_mesh] = convex_mesh;

					delete[] vertices;
				}
				else {
					convex_mesh = (physx::PxConvexMesh*)App->physics->cooked_convex[dragged_mesh];
				}
				physx::PxConvexMeshGeometry geometry(convex_mesh, mesh_scale);
				shape = App->physics->mPhysics->createShape(geometry, *App->physics->mMaterial);

				CreateRigidbody(geometry, position);
			}
			else { //Mesh Collider
				if (!App->physics->cooked_meshes[dragged_mesh]) {
					physx::PxVec3* vertices = new physx::PxVec3[dragged_mesh->VerticesSize];
					for (int j = 0; j < dragged_mesh->VerticesSize; ++j) {
						vertices[j].x = dragged_mesh->vertices[j].position[0];
						vertices[j].y = dragged_mesh->vertices[j].position[1];
						vertices[j].z = dragged_mesh->vertices[j].position[2];
					}
					physx::PxU16* indices = new physx::PxU16[dragged_mesh->IndicesSize];
					for (uint i = 0; i < dragged_mesh->IndicesSize; ++i) {
						indices[i] = dragged_mesh->Indices[i];
					}

					physx::PxTriangleMeshDesc meshDesc;
					meshDesc.points.count = dragged_mesh->VerticesSize;
					meshDesc.points.stride = sizeof(physx::PxVec3);
					meshDesc.points.data = vertices;

					meshDesc.triangles.count = dragged_mesh->IndicesSize / 3;
					meshDesc.triangles.stride = 3 * sizeof(physx::PxU16);
					meshDesc.triangles.data = indices;

					meshDesc.flags = physx::PxMeshFlag::e16_BIT_INDICES;
					///--------------------------------COOKING-------------------------------------------
					/*physx::PxDefaultMemoryOutputStream writeBuffer;
					if (!App->physics->mCooking->cookTriangleMesh(meshDesc, writeBuffer)) {
						return ENGINE_AND_SYSTEM_CONSOLE_LOG("| Could not create Mesh Collider");
					}
					physx::PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
					triangle_mesh = App->physics->mPhysics->createTriangleMesh(readBuffer);*/
					///--------------------------------RUNTIME--------------------------------------------
					physx::PxTolerancesScale scale;
					physx::PxCookingParams params(scale);
					params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
					params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
					params.meshCookingHint = physx::PxMeshCookingHint::eCOOKING_PERFORMANCE;
					App->physics->mCooking->setParams(params);
					triangle_mesh = App->physics->mCooking->createTriangleMesh(meshDesc, App->physics->mPhysics->getPhysicsInsertionCallback());
					///----------------------------------------------------------------------------------------------------------

					App->physics->cooked_meshes.insert(std::pair<ResourceMesh*, physx::PxBase*>(dragged_mesh, nullptr));
					App->physics->cooked_meshes[dragged_mesh] = triangle_mesh;

					delete[] vertices;
					delete[] indices;
				}
				else {
					triangle_mesh = (physx::PxTriangleMesh*)App->physics->cooked_meshes[dragged_mesh];
					//triangle_mesh->acquireReference();
				}
				physx::PxTriangleMeshGeometry geometry(triangle_mesh, mesh_scale);
				shape = App->physics->mPhysics->createShape(geometry, *App->physics->mMaterial);
				CreateRigidbody(geometry, position);
			}

		}
		break;
	}
	case ComponentCollider::COLLIDER_TYPE::NONE: {
		lastIndex = -1;
		break;
	}
	}
	editCollider = false;
}


template <class Geometry>
void ComponentCollider::CreateRigidbody(Geometry geometry, physx::PxTransform position) {
	if (!HasDynamicRigidBody(geometry, position))
	{
		if (rigidStatic)
			App->physics->DeleteActor(rigidStatic);

		physx::PxFilterData filterData;
		filterData.word0 = (1 << GO->layer); // word0 = own ID
		filterData.word1 = App->physics->layer_list.at(GO->layer).LayerGroup; // word1 = ID mask to filter pairs that trigger a contact callback;

		if (isTrigger) {
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		}
		else {
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		}

		shape->setSimulationFilterData(filterData);
		shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setQueryFilterData(filterData);

		rigidStatic = PxCreateStatic(*App->physics->mPhysics, position, *shape);

		App->physics->addActor(rigidStatic, GO);

	}
}

void ComponentCollider::Delete()
{
	if (shape)
	{
		shape->release();
		shape = nullptr;
	}

	if (GO->GetComponent<ComponentDynamicRigidBody>() != nullptr)
	{
		if (GO->GetComponent<ComponentDynamicRigidBody>()->rigidBody != nullptr) {
			App->physics->DeleteActor(GO->GetComponent<ComponentDynamicRigidBody>()->rigidBody);
			GO->GetComponent<ComponentDynamicRigidBody>()->rigidBody = nullptr;
		}
	}
	if (rigidStatic) {
		App->physics->DeleteActor(rigidStatic);
		rigidStatic = nullptr;
	}
}

template <class Geometry>
bool ComponentCollider::HasDynamicRigidBody(Geometry geometry, physx::PxTransform transform)
{
	ComponentDynamicRigidBody* dynamicRB = GO->GetComponent<ComponentDynamicRigidBody>();

	if (dynamicRB != nullptr)
	{

		float3 position, scale = float3::zero;
		Quat rot = Quat::identity;

		globalMatrix.Decompose(position, rot, scale);

		if (rigidStatic) {
			App->physics->DeleteActor(rigidStatic);
		}
		if (dynamicRB->rigidBody) {
			App->physics->DeleteActor(dynamicRB->rigidBody);
		}

		shape = App->physics->mPhysics->createShape(geometry, *App->physics->mMaterial);

		if (isTrigger) {
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
		}
		else {
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		}

		physx::PxFilterData filterData;
		filterData.word0 = (1 << GO->layer); // word0 = own ID
		filterData.word1 = App->physics->layer_list.at(GO->layer).LayerGroup; // word1 = ID mask to filter pairs that trigger a contact callback;

		shape->setSimulationFilterData(filterData);
		shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setQueryFilterData(filterData);

		dynamicRB->rigidBody = PxCreateDynamic(*App->physics->mPhysics, transform, *shape, 1.0f);
		dynamicRB->update = true;
		dynamicRB->UpdateRBValues();

		dynamicRB->rigidBody->setGlobalPose(physx::PxTransform(position.x, position.y, position.z, physx::PxQuat(rot.x, rot.y, rot.z, rot.w)));

		App->physics->addActor(dynamicRB->rigidBody, GO);

		return true;
	}
	return false;
}

physx::PxRigidActor* ComponentCollider::GetActor() {
	ComponentDynamicRigidBody* dynamicRB = GO->GetComponent<ComponentDynamicRigidBody>();

	if (dynamicRB != nullptr)
		return dynamicRB->rigidBody;
	else
		return rigidStatic;
}

void ComponentCollider::UpdateActorLayer(const int* layerMask) {
	ComponentDynamicRigidBody* dynamicRB = GO->GetComponent<ComponentDynamicRigidBody>();
	ComponentCharacterController* controller = GO->GetComponent<ComponentCharacterController>();
	if (controller) {
		App->physics->UpdateActorLayer(dynamicRB->rigidBody, (LayerMask*)layerMask);
	}else if (dynamicRB != nullptr)
		App->physics->UpdateActorLayer(dynamicRB->rigidBody, (LayerMask*)layerMask);
	else
		App->physics->UpdateActorLayer(rigidStatic, (LayerMask*)layerMask);
}
