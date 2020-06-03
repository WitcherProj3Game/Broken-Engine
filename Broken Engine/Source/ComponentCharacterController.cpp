#include "ComponentCharacterController.h"
#include "Application.h"
#include "GameObject.h"
#include "ModulePhysics.h"
#include "ModuleResourceManager.h"
#include "ResourceMesh.h"
#include "ResourceShader.h"
#include "ModuleSceneManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleScripting.h"
#include "ComponentTransform.h"
#include "ComponentScript.h"
#include "ModuleTimeManager.h"
#include "ModuleGui.h"
#include "ModuleInput.h"
#include "ScriptVar.h"
#include "PhysX_3.4/Include/characterkinematic/PxController.h"
#include "PhysX_3.4/Include/characterkinematic/PxCapsuleController.h"

#include "Imgui/imgui.h"

#include "mmgr/mmgr.h"

using namespace Broken;

ComponentCharacterController::ComponentCharacterController(GameObject* ContainerGO) : Component(ContainerGO, Component::ComponentType::CharacterController)
{
	float3 pos = GO->GetComponent<ComponentTransform>()->GetGlobalPosition();
	capsuleDesc.position = physx::PxExtendedVec3(pos.x, pos.y, pos.z);
	capsuleDesc.contactOffset = contactOffset;
	capsuleDesc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
	capsuleDesc.stepOffset = stepOffset;
	capsuleDesc.slopeLimit = cosf(DEGTORAD * slopeLimit);
	capsuleDesc.radius = radius;
	capsuleDesc.height = height;
	capsuleDesc.nonWalkableMode = physx::PxControllerNonWalkableMode::Enum::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
	capsuleDesc.upDirection = physx::PxVec3(0, 1, 0);
	capsuleDesc.material = App->physics->mMaterial;
	capsuleDesc.behaviorCallback = this;
	capsuleDesc.reportCallback = this;

	desc = &capsuleDesc;

	controller = App->physics->mControllerManager->createController(*desc);

	controller->getActor()->getShapes(&shape, 1);

	physx::PxFilterData filterData;
	filterData.word0 = (1 << GO->layer); // word0 = own ID
	filterData.word1 = App->physics->layer_list.at(GO->layer).LayerGroup;
	shape->setSimulationFilterData(filterData);

	App->physics->addActor(shape->getActor(), GO);
	//App->physics->addActor(controller->getActor(),GO);

	initialPosition = capsuleDesc.position;
	initialPosition2 = capsuleDesc.position;

	name = "Character Controller";
	mesh = (ResourceMesh*)App->resources->CreateResource(Resource::ResourceType::MESH, "DefaultCharacterController");
}

ComponentCharacterController::~ComponentCharacterController()
{
	mesh->Release();
	Delete();
}

void ComponentCharacterController::Enable()
{
	if (controller == nullptr)
	{
		controller = App->physics->mControllerManager->createController(*desc);

		controller->getActor()->getShapes(&shape, 1);

		physx::PxFilterData filterData;
		filterData.word0 = (1 << GO->layer); // word0 = own ID
		filterData.word1 = App->physics->layer_list.at(GO->layer).LayerGroup;
		shape->setSimulationFilterData(filterData);

		App->physics->addActor(shape->getActor(), GO);
		controller->setFootPosition(desc->position);
		hasBeenDeactivated = false;
	}
	//GetActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
	active = true;
}

void ComponentCharacterController::Disable()
{
	if (controller != nullptr) {
		if (shape)
		{
			if (shape->getActor() != nullptr)
			{
				//GetActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
				if (!hasBeenDeactivated )
				{
					desc->position = controller->getFootPosition();
					if(App->physics->actors.size() > 0)
					App->physics->actors.erase(shape->getActor());
				}
			}
		}
		shape = nullptr;
		controller->release();
		controller = nullptr;
		hasBeenDeactivated = true;
	}
	active = false;
}

void ComponentCharacterController::Update()
{

	vel = physx::PxVec3(0);

	ComponentTransform* cTransform = GO->GetComponent<ComponentTransform>();

	float3 p;
	if (controller != nullptr)
	{
		p.x = controller->getActor()->getGlobalPose().p.x;
		p.y = controller->getActor()->getGlobalPose().p.y;
		p.z = controller->getActor()->getGlobalPose().p.z;


		//Move(velocity.x, velocity.z);


		if (cTransform->updateValues || App->gui->isUsingGuizmo)
		{
			cTransform->updateValues = false;
			float3 pos = cTransform->GetGlobalPosition();
			controller->setFootPosition(physx::PxExtendedVec3(pos.x, pos.y, pos.z));
		}

		if (gravity && App->GetAppState() == AppState::PLAY) {
			vel.y = App->physics->mScene->getGravity().y;
			Move();//Affect Gravity Always
		}


		physx::PxExtendedVec3 cctPosition = controller->getFootPosition();
		float3 cctPos(cctPosition.x, cctPosition.y, cctPosition.z);

		if (!creation)
		{
			float offset = radius + height * 0.5f + contactOffset;
			cctPos.y += offset;
			controller->setFootPosition(physx::PxExtendedVec3(controller->getFootPosition().x, controller->getFootPosition().y + offset, controller->getFootPosition().z));
			creation = true;
		}

		GO->GetComponent<ComponentTransform>()->SetPosition((float3)cctPos);

		if (to_delete)
		{
			Delete();
			this->GetContainerGameObject()->RemoveComponent(this);
		}
		else if(App->physics->actors[shape->getActor()] == nullptr){
			physx::PxFilterData filterData;
			filterData.word0 = (1 << GO->layer); // word0 = own ID
			filterData.word1 = App->physics->layer_list.at(GO->layer).LayerGroup;
			shape->setSimulationFilterData(filterData);

			App->physics->addActor(shape->getActor(),GO);
		}
	}
}

void ComponentCharacterController::Draw()
{
	if (!mesh->IsInMemory())
	{
		// --- Rebuild capsule ---
		App->scene_manager->CreateCapsule(radius * 2, height * 2, mesh);
		mesh->LoadToMemory();
	}

	// --- Render shape ---
	if (mesh && mesh->IsInMemory() && mesh->vertices && mesh->Indices && draw)
	{
		// --- Use default shader ---
		uint shaderID = App->renderer3D->defaultShader->ID;
		glUseProgram(shaderID);

		// --- Set uniforms ---
		GLint modelLoc = glGetUniformLocation(shaderID, "u_Model");

		float4x4 aux = GO->GetComponent<ComponentTransform>()->GetGlobalTransform();

		aux.y += controller->getPosition().y - controller->getFootPosition().y;

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, aux.Transposed().ptr());

		int vertexColorLocation = glGetUniformLocation(shaderID, "u_Color");
		glUniform4f(vertexColorLocation, 0.5f, 0.5f, 0.5f, 1.0f);

		int TextureSupportLocation = glGetUniformLocation(shaderID, "u_UseTextures");
		glUniform1i(TextureSupportLocation, 0);

		// --- Bind mesh's vao ---
		glBindVertexArray(mesh->VAO);

		// --- Activate wireframe mode ---
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// --- bind indices buffer and draw ---
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
		glDrawElements(GL_TRIANGLES, mesh->IndicesSize, GL_UNSIGNED_INT, NULL); // render primitives from array data

		// --- Unbind mesh's vao ---
		glBindVertexArray(0);

		// --- DeActivate wireframe mode ---
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// --- Set uniforms back to defaults ---
		glUniform1i(TextureSupportLocation, 0);
		glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void ComponentCharacterController::DrawComponent()
{
	if (controller) {
		if (!GetActive())
		{
			controller->getActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
			physx::PxShape* shape;
			controller->getActor()->getShapes(&shape, 1);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);

			if (!hasBeenDeactivated)
			{
				Delete();
				hasBeenDeactivated = true;
			}
		}

		else
		{
			physx::PxShape* shape;
			controller->getActor()->getShapes(&shape, 1);

			if (hasBeenDeactivated)
			{
				App->physics->addActor(shape->getActor(), GO);
				hasBeenDeactivated = false;
			}

			controller->getActor()->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
		}

		Draw();
	}
}
void ComponentCharacterController::SetVelocity(float velX, float velY, float velZ)
{
	vel.x = velX;
	vel.y = velY;
	vel.z = velZ;
	Move();
}

void ComponentCharacterController::Move(float minDist)
{
	physx::PxFilterData filterData;
	filterData.word0 = App->physics->layer_list.at((int)GO->layer).LayerGroup; //layers that will collide

	physx::PxControllerFilters controllerFilter;
	controllerFilter.mFilterData = &filterData;
	if(controller)
	controller->move(vel, minDist, App->time->GetGameDt(), controllerFilter);
}

void ComponentCharacterController::Delete()
{
	if (controller)
	{
		if(shape && App->physics->actors.size()>0)
			App->physics->actors.erase(shape->getActor());
		controller->release();
		controller = nullptr;
	}
}

json ComponentCharacterController::Save() const
{
	//ENGINE_CONSOLE_LOG("Saved");

	json node;

	node["contactOffset"] = std::to_string(contactOffset);
	node["stepOffset"] = std::to_string(stepOffset);
	node["slopeLimit"] = std::to_string(slopeLimit);
	node["radius"] = std::to_string(radius);
	node["height"] = std::to_string(height);
	if (controller) {
		node["positionX"] = std::to_string(controller->getFootPosition().x);
		node["positionY"] = std::to_string(controller->getFootPosition().y);
		node["positionZ"] = std::to_string(controller->getFootPosition().z);
	}
	else {
		node["positionX"] = std::to_string(initialPosition2.x);
		node["positionY"] = std::to_string(initialPosition2.y);
		node["positionZ"] = std::to_string(initialPosition2.z);
	}
	node["draw"] = std::to_string(draw);

	if (controller) {
		if (controller->getNonWalkableMode() == physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING)
			node["nonWalkableMode"] = std::to_string(0);
	}
	else
		node["nonWalkableMode"] = std::to_string(1);

	node["gravity"] = std::to_string(gravity);

	return node;
}

void ComponentCharacterController::Load(json& node)
{
	//ENGINE_CONSOLE_LOG("Load");

	std::string contactOffset_ = node["contactOffset"].is_null() ? "0" : node["contactOffset"];
	std::string stepOffset_ = node["stepOffset"].is_null() ? "0" : node["stepOffset"];
	std::string slopeLimit_ = node["slopeLimit"].is_null() ? "0" : node["slopeLimit"];
	std::string radius_ = node["radius"].is_null() ? "0" : node["radius"];
	std::string height_ = node["height"].is_null() ? "0" : node["height"];
	std::string positionX = node["positionX"].is_null() ? "0" : node["positionX"];
	std::string positionY = node["positionY"].is_null() ? "0" : node["positionY"];
	std::string positionZ = node["positionZ"].is_null() ? "0" : node["positionZ"];
	std::string nonWalkableMode = node["nonWalkableMode"].is_null() ? "0" : node["nonWalkableMode"];
	std::string firstTime_ = node["firstTime"].is_null() ? "0" : node["firstTime"];
	std::string gravity_ = node["gravity"].is_null() ? "0" : node["gravity"];
	std::string draw_ = node["draw"].is_null() ? "0" : node["draw"];

	contactOffset = std::stof(contactOffset_);
	stepOffset = std::stof(stepOffset_);
	slopeLimit = std::stof(slopeLimit_);
	radius = std::stof(radius_);
	height = std::stof(height_);
	position.x = std::stof(positionX);
	position.y = std::stof(positionY);
	position.z = std::stof(positionZ);
	gravity = std::stof(gravity_);
	draw = std::stof(draw_);

	SetContactOffset(contactOffset);
	SetStepOffset(stepOffset);
	SetSlopeLimit(slopeLimit);
	SetRadius(radius);
	SetHeight(height);

	if (controller) {
		if (std::stof(nonWalkableMode) == 0)
		{
			controller->setNonWalkableMode(physx::PxControllerNonWalkableMode::Enum::ePREVENT_CLIMBING);
			sliding = false;
		}
		else
		{
			controller->setNonWalkableMode(physx::PxControllerNonWalkableMode::Enum::ePREVENT_CLIMBING_AND_FORCE_SLIDING);
			sliding = true;
		}
	}

	physx::PxExtendedVec3 pos = physx::PxExtendedVec3(position.x, position.y, position.z);
	initialPosition = pos;
	initialPosition2 = pos;

	creation = true;

	if(controller)
		controller->setFootPosition(physx::PxExtendedVec3(pos.x, pos.y, pos.z));
}

void ComponentCharacterController::CreateInspectorNode()
{
	if (ImGui::Button("Delete component"))
		to_delete = true;

	ImGui::Checkbox("Show", &draw);

	if (ImGui::DragFloat("Radius", &radius, 0.005f))
	{
		if (mesh && mesh->IsInMemory())
			mesh->Release();

		SetRadius(radius);
	}

	if (ImGui::DragFloat("Height", &height, 0.005f))
	{
		if (mesh && mesh->IsInMemory())
			mesh->Release();

		SetHeight(height);
	}

	if (ImGui::DragFloat("contactOffset", &contactOffset, 0.005f))
	{
		if (mesh && mesh->IsInMemory())
			mesh->Release();

		SetContactOffset(contactOffset);
	}

	if (ImGui::DragFloat("stepOffset", &stepOffset, 0.005f))
	{
		if (mesh && mesh->IsInMemory())
			mesh->Release();

		SetStepOffset(stepOffset);
	}

	if (ImGui::DragFloat("slopeLimit", &slopeLimit, 0.005f))
	{
		if (mesh && mesh->IsInMemory())
			mesh->Release();

		SetSlopeLimit(slopeLimit);
	}


	if (ImGui::Checkbox("No Slide", &sliding))
	{
		if (sliding && controller)
			controller->setNonWalkableMode(physx::PxControllerNonWalkableMode::Enum::ePREVENT_CLIMBING_AND_FORCE_SLIDING);

		else
			controller->setNonWalkableMode(physx::PxControllerNonWalkableMode::Enum::ePREVENT_CLIMBING);
	}

	ImGui::Checkbox("Gravity", &gravity);
}

void ComponentCharacterController::SetContactOffset(float offset)
{
	if(controller)
	controller->setContactOffset(offset);
}

void ComponentCharacterController::SetStepOffset(float offset)
{
	if (controller)
	controller->setStepOffset(offset);
}

void ComponentCharacterController::SetSlopeLimit(float limit)
{
	if (controller)
	static_cast<physx::PxCapsuleController*>(controller)->setSlopeLimit(cosf(DEGTORAD * limit));
}

void ComponentCharacterController::SetRadius(float radius)
{
	if (controller)
	static_cast<physx::PxCapsuleController*>(controller)->setRadius(radius);
}

void ComponentCharacterController::SetHeight(float height)
{
	if (controller)
	static_cast<physx::PxCapsuleController*>(controller)->resize(height);
	//static_cast<physx::PxCapsuleController*>(controller)->setHeight(height);
}

physx::PxControllerBehaviorFlags ComponentCharacterController::getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor)
{
	if (shape.getFlags() & physx::PxShapeFlag::eTRIGGER_SHAPE)
	{
		GameObject* go = App->physics->actors[(physx::PxRigidActor*) & actor];
		if (go)
		{
			go->collisions.at(Collision_Type::ONTRIGGER_ENTER) = GO;
			GO->collisions.at(Collision_Type::ONTRIGGER_ENTER) = go;

			ComponentScript* script = go->GetComponent<ComponentScript>();
			ComponentScript* script2 = GO->GetComponent<ComponentScript>();
			ScriptFunc function;
			function.name = "OnTriggerEnter";

			if (script) {
				App->scripting->CallbackScriptFunction(script, function);
				ENGINE_CONSOLE_LOG("DETECTED ON TRIGGER: SCRIPT 1");
			}
			if (script2) {
				App->scripting->CallbackScriptFunction(script2, function);
				ENGINE_CONSOLE_LOG("DETECTED ON TRIGGER: SCRIPT 2");
			}
		}
	}
	else
	{
		GameObject* go = App->physics->actors[(physx::PxRigidActor*) & actor];
		if (go)
		{
			GO->collisions.at(Collision_Type::ONCOLLISION_ENTER) = go;
			go->collisions.at(Collision_Type::ONCOLLISION_ENTER) = GO;

			ComponentScript* script = go->GetComponent<ComponentScript>();
			ComponentScript* script2 = GO->GetComponent<ComponentScript>();

			ScriptFunc function;
			function.name = "OnCollisionEnter";

			if (script) {
				App->scripting->CallbackScriptFunction(script, function);
			}
			if (script2) {
				App->scripting->CallbackScriptFunction(script2, function);
			}
		}
	}

	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

physx::PxControllerBehaviorFlags ComponentCharacterController::getBehaviorFlags(const physx::PxController& controller)
{

	physx::PxShape* shape1;
	this->controller->getActor()->getShapes(&shape1, 1);

	physx::PxShape* shape;
	controller.getActor()->getShapes(&shape, 1);


	if (shape1 == nullptr || shape == nullptr)
		physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;

	physx::PxRigidActor* a = this->controller->getActor();
	physx::PxRigidActor* b = controller.getActor();

	if (shape->getFlags() & physx::PxShapeFlag::eTRIGGER_SHAPE || shape1->getFlags() & physx::PxShapeFlag::eTRIGGER_SHAPE)
	{
		GameObject* go = App->physics->actors[shape->getActor()];
		if (go)
		{
			GO->collisions.at(Collision_Type::ONTRIGGER_ENTER) = go;
			go->collisions.at(Collision_Type::ONTRIGGER_ENTER) = GO;

			ComponentScript* script = go->GetComponent<ComponentScript>();
			ComponentScript* script2 = GO->GetComponent<ComponentScript>();
			ScriptFunc function;
			function.name = "OnTriggerEnter";

			if (script) {
				App->scripting->CallbackScriptFunction(script, function);
				ENGINE_CONSOLE_LOG("DETECTED ON TRIGGER: SCRIPT 1");
			}
			if (script2) {
				App->scripting->CallbackScriptFunction(script2, function);
				ENGINE_CONSOLE_LOG("DETECTED ON TRIGGER: SCRIPT 2");
			}
		}
	}
	else
	{
		GameObject* go = App->physics->actors[shape->getActor()];
		if (go)
		{
			GO->collisions.at(Collision_Type::ONCOLLISION_ENTER) = go;
			go->collisions.at(Collision_Type::ONCOLLISION_ENTER) = GO;

			ComponentScript* script = go->GetComponent<ComponentScript>();
			ComponentScript* script2 = GO->GetComponent<ComponentScript>();

			ScriptFunc function;
			function.name = "OnCollisionEnter";
			if (script) {
				App->scripting->CallbackScriptFunction(script, function);
			}
			if (script2) {
				App->scripting->CallbackScriptFunction(script2, function);
			}
		}
	}

	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

physx::PxControllerBehaviorFlags ComponentCharacterController::getBehaviorFlags(const physx::PxObstacle& obstacle)
{
	return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;
}

void ComponentCharacterController::onShapeHit(const physx::PxControllerShapeHit& hit)
{
	defaultCCTInteraction(hit);
}

void ComponentCharacterController::defaultCCTInteraction(const physx::PxControllerShapeHit& hit)
{
	physx::PxRigidDynamic* actor = nullptr;
	physx::PxActor* act = nullptr;
	act = hit.shape->getActor();
	if (act) {
		if (act->getConcreteType() == physx::PxConcreteType::eRIGID_DYNAMIC)
			actor = (physx::PxRigidDynamic*)hit.shape->getActor();
	}
	if (actor)
	{
		if (actor->getRigidBodyFlags() & physx::PxRigidBodyFlag::eKINEMATIC)
			return;

		if (0)
		{
			const physx::PxVec3 p = actor->getGlobalPose().p + hit.dir * 10.0f;

			physx::PxShape* shape;
			actor->getShapes(&shape, 1);
			physx::PxRaycastHit newHit;
			physx::PxU32 n = physx::PxShapeExt::raycast(*shape, *shape->getActor(), p, -hit.dir, 20.0f, physx::PxHitFlag::ePOSITION, 1, &newHit);
			if (n)
			{
				// We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
				// useless stress on the solver. It would be possible to enable/disable vertical pushes on
				// particular objects, if the gameplay requires it.
				const physx::PxVec3 upVector = hit.controller->getUpDirection();
				const physx::PxF32 dp = hit.dir.dot(upVector);
				if (fabsf(dp) < 1e-3f)
				{
					const physx::PxTransform globalPose = actor->getGlobalPose();
					const physx::PxVec3 localPos = globalPose.transformInv(newHit.position);
					addForceAtLocalPos(*actor, hit.dir * hit.length * 1000.0f, localPos, physx::PxForceMode::eACCELERATION);
				}
			}
		}

		// We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
		// useless stress on the solver. It would be possible to enable/disable vertical pushes on
		// particular objects, if the gameplay requires it.
		const physx::PxVec3 upVector = hit.controller->getUpDirection();
		const physx::PxF32 dp = hit.dir.dot(upVector);
		if (fabsf(dp) < 1e-3f)
		{
			const physx::PxTransform globalPose = actor->getGlobalPose();
			const physx::PxVec3 localPos = globalPose.transformInv(toVec3(hit.worldPos));
			physx::PxActor* act1 = nullptr;
			physx::PxRigidDynamic* actor1 = nullptr;
			act1 = hit.shape->getActor();
			if (act1) {
				if (act1->getConcreteType() == physx::PxConcreteType::eRIGID_DYNAMIC)
					actor1 = (physx::PxRigidDynamic*)hit.shape->getActor();
			}
			addForceAtLocalPos(*actor, hit.dir * hit.length * 1000, localPos, physx::PxForceMode::eACCELERATION);

		}
	}
}

void ComponentCharacterController::addForceAtLocalPos(physx::PxRigidBody& body, const physx::PxVec3& force, const physx::PxVec3& pos, physx::PxForceMode::Enum mode, bool wakeup)
{
	//transform pos to world space
	const physx::PxVec3 globalForcePos = body.getGlobalPose().transform(pos);

	addForceAtPosInternal(body, force, globalForcePos, mode, wakeup);
}

void ComponentCharacterController::addForceAtPosInternal(physx::PxRigidBody& body, const physx::PxVec3& force, const physx::PxVec3& pos, physx::PxForceMode::Enum mode, bool wakeup)
{
	const physx::PxTransform globalPose = body.getGlobalPose();
	const physx::PxVec3 centerOfMass = globalPose.transform(body.getCMassLocalPose().p);

	const physx::PxVec3 torque = (pos - centerOfMass).cross(force);
	body.addForce(force, mode, wakeup);
	body.addTorque(torque, mode, wakeup);
}