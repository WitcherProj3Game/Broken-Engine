#include "AutoCompleteFileGen.h"
#include "JSONLoader.h"
#include "ModuleFileSystem.h"
#include "Application.h"
#include "ModuleScripting.h"
#include "LuaIncludes.h"

#include"ScriptingSystem.h"

using namespace Broken;

AutoCompleteFileGen::AutoCompleteFileGen()
{
}

AutoCompleteFileGen::~AutoCompleteFileGen()
{
}

void AutoCompleteFileGen::EmplaceAnimationFunctions()
{
	std::string source = "Animations";

	SerializedFunction PlayAnimation("PlayAnimation", source.c_str());
	PlayAnimation.variables.push_back("name"); PlayAnimation.variables.push_back("speed"); PlayAnimation.variables.push_back("gameObject_UID");
	
	SerializedFunction SetAnimationSpeed("SetAnimationSpeed", source.c_str());
	SetAnimationSpeed.variables.push_back("name"); SetAnimationSpeed.variables.push_back("speed"); SetAnimationSpeed.variables.push_back("gameObject_UID");
	
	SerializedFunction SetCurrentAnimationSpeed("SetCurrentAnimationSpeed", source.c_str());
	SetCurrentAnimationSpeed.variables.push_back("speed"); SetCurrentAnimationSpeed.variables.push_back("gameObject_UID");
	
	SerializedFunction SetBlendTime("SetBlendTime", source.c_str());
	SetBlendTime.variables.push_back("blend_value"); SetBlendTime.variables.push_back("gameObject_UID");
	
	SerializedFunction CurrentAnimationEnded("CurrentAnimationEnded", source.c_str());
	CurrentAnimationEnded.variables.push_back("gameObject_UID");

	//Pushback all functions
	engine_functions.push_back(PlayAnimation);
	engine_functions.push_back(SetAnimationSpeed);
	engine_functions.push_back(SetCurrentAnimationSpeed);
	engine_functions.push_back(SetBlendTime);
	engine_functions.push_back(CurrentAnimationEnded);
}

void AutoCompleteFileGen::EmplaceUserInterfaceFunctions()
{
	std::string source = "UserInterface";

	SerializedFunction MakeElementVisible("MakeElementVisible", source.c_str());
	MakeElementVisible.variables.push_back("comp_type"); MakeElementVisible.variables.push_back("gameObject_UID");
	
	SerializedFunction MakeElementInvisible("MakeElementInvisible", source.c_str());
	MakeElementInvisible.variables.push_back("comp_type"); MakeElementInvisible.variables.push_back("gameObject_UID");


	SerializedFunction SetUIBarPercentage("SetUIBarPercentage", source.c_str());
	SetUIBarPercentage.variables.push_back("percentage"); SetUIBarPercentage.variables.push_back("gameObject_UID");
	
	SerializedFunction SetUICircularBarPercentage("SetUICircularBarPercentage", source.c_str());
	SetUICircularBarPercentage.variables.push_back("percentage"); SetUICircularBarPercentage.variables.push_back("gameObject_UID");
	
	SerializedFunction SetText("SetText", source.c_str());
	SetText.variables.push_back("UI_Text"); SetText.variables.push_back("gameObject_UID");
	
	SerializedFunction SetTextAndNumber("SetTextAndNumber", source.c_str());
	SetTextAndNumber.variables.push_back("UI_text"); SetTextAndNumber.variables.push_back("number"); SetTextAndNumber.variables.push_back("gameObject_UID");
	
	SerializedFunction SetTextNumber("SetTextNumber", source.c_str());
	SetTextNumber.variables.push_back("number"); SetTextNumber.variables.push_back("gameObject_UID");

	//PushBack All Functions
	engine_functions.push_back(MakeElementVisible);
	engine_functions.push_back(MakeElementInvisible);

	engine_functions.push_back(SetUIBarPercentage);
	engine_functions.push_back(SetUICircularBarPercentage);
	engine_functions.push_back(SetText);
	engine_functions.push_back(SetTextAndNumber);
	engine_functions.push_back(SetTextNumber);
}

void AutoCompleteFileGen::EmplaceSceneFunctions()
{
	std::string source = "Scene.lua";

	SerializedFunction LoadScene("LoadScene", source.c_str());
	LoadScene.variables.push_back("scene_UID");
	
	SerializedFunction QuitGame("QuitGame", source.c_str());
	
	SerializedFunction Instantiate("Instantiate", source.c_str());
	Instantiate.variables.push_back("resource_UID"); Instantiate.variables.push_back("x"); Instantiate.variables.push_back("y"); Instantiate.variables.push_back("z");
	Instantiate.variables.push_back("rot_x"); Instantiate.variables.push_back("rot_y"); Instantiate.variables.push_back("rot_z");

	//Pushback All Functions
	engine_functions.push_back(LoadScene);
	engine_functions.push_back(QuitGame);
	engine_functions.push_back(Instantiate);
}

void AutoCompleteFileGen::EmplaceNavigationFunctions()
{
	std::string source = "Navigation";

	SerializedFunction AllAreas("AllAreas", source.c_str());
	
	SerializedFunction GetAreaFromName("GetAreaFromName", source.c_str());
	GetAreaFromName.variables.push_back("name");
	
	SerializedFunction GetAreaCost("GetAreaCost", source.c_str());
	GetAreaCost.variables.push_back("areaIndex");
	
	SerializedFunction SetAreaCost("SetAreaCost", source.c_str());
	SetAreaCost.variables.push_back("areaIndex");
	
	SerializedFunction CalculatePath("CalculatePath", source.c_str());
	CalculatePath.variables.push_back("origin_x"); CalculatePath.variables.push_back("origin_y"); CalculatePath.variables.push_back("origin_z");
	CalculatePath.variables.push_back("dest_x"); CalculatePath.variables.push_back("dest_y"); CalculatePath.variables.push_back("dest_z"); CalculatePath.variables.push_back("areaMask");

	//Pushback All Functions
	engine_functions.push_back(AllAreas);
	engine_functions.push_back(GetAreaFromName);
	engine_functions.push_back(GetAreaCost);
	engine_functions.push_back(SetAreaCost);
	engine_functions.push_back(CalculatePath);
}

void AutoCompleteFileGen::EmplaceScriptingInputsFunctions()
{
	std::string source = "Input";

	SerializedFunction KeyDown("KeyDown", source.c_str());
	KeyDown.variables.push_back("key");
	
	SerializedFunction KeyUp("KeyUp", source.c_str());
	KeyUp.variables.push_back("key");
	
	SerializedFunction KeyRepeat("KeyRepeat", source.c_str());
	KeyRepeat.variables.push_back("key");
	
	SerializedFunction KeyIdle("KeyIdle", source.c_str());
	KeyIdle.variables.push_back("key");


	SerializedFunction GetMouseButton("GetMouseButton", source.c_str());
	GetMouseButton.variables.push_back("button");
	
	SerializedFunction MouseButtonState("MouseButtonState", source.c_str());
	MouseButtonState.variables.push_back("button");
	
	
	SerializedFunction MouseButtonDown("MouseButtonDown", source.c_str());
	MouseButtonDown.variables.push_back("button");
	
	SerializedFunction MouseButtonUp("MouseButtonUp", source.c_str());
	MouseButtonUp.variables.push_back("button");
	
	SerializedFunction MouseButtonRepeat("MouseButtonRepeat", source.c_str());
	MouseButtonRepeat.variables.push_back("button");
	
	SerializedFunction MouseButtonIdle("MouseButtonIdle", source.c_str());
	MouseButtonIdle.variables.push_back("button");
	
	
	SerializedFunction IsGamepadButton("IsGamepadButton", source.c_str());
	IsGamepadButton.variables.push_back("player_num"); IsGamepadButton.variables.push_back("button"); IsGamepadButton.variables.push_back("button_state");
	
	SerializedFunction GetControllerButton("GetControllerButton", source.c_str());
	GetControllerButton.variables.push_back("button");
	
	SerializedFunction GetGamepadButtonState("GetGamepadButtonState", source.c_str());
	GetGamepadButtonState.variables.push_back("state");
	
	
	SerializedFunction IsJoystickAxis("IsJoystickAxis", source.c_str());
	IsJoystickAxis.variables.push_back("player_num"); IsJoystickAxis.variables.push_back("joy_axis"); IsJoystickAxis.variables.push_back("axis_state");
	
	SerializedFunction IsTriggerState("IsTriggerState", source.c_str());
	IsTriggerState.variables.push_back("player_num"); IsTriggerState.variables.push_back("trigger"); IsTriggerState.variables.push_back("button_state");
	
	SerializedFunction GetControllerAxis("GetControllerAxis", source.c_str());
	GetControllerAxis.variables.push_back("axis_name");
	
	SerializedFunction GetAxisState("GetAxisState", source.c_str());
	GetAxisState.variables.push_back("state_name");
	
	
	SerializedFunction GetAxisRealValue("GetAxisRealValue", source.c_str());
	GetAxisRealValue.variables.push_back("player_num"); GetAxisRealValue.variables.push_back("joy_axis");
	
	SerializedFunction GetAxisValue("GetAxisValue", source.c_str());
	GetAxisValue.variables.push_back("player_num"); GetAxisValue.variables.push_back("joy_axis"); GetAxisValue.variables.push_back("threshold");
	
	SerializedFunction ShakeController("ShakeController", source.c_str());
	ShakeController.variables.push_back("player_num"); ShakeController.variables.push_back("intensity"); ShakeController.variables.push_back("milliseconds");
	
	SerializedFunction StopControllerShake("StopControllerShake", source.c_str());
	StopControllerShake.variables.push_back("player_num");




	engine_functions.push_back(KeyDown);
	engine_functions.push_back(KeyUp);
	engine_functions.push_back(KeyRepeat);
	engine_functions.push_back(KeyIdle);

	engine_functions.push_back(GetMouseButton);
	engine_functions.push_back(MouseButtonState);

	engine_functions.push_back(MouseButtonDown);
	engine_functions.push_back(MouseButtonUp);
	engine_functions.push_back(MouseButtonRepeat);
	engine_functions.push_back(MouseButtonIdle);

	engine_functions.push_back(IsGamepadButton);
	engine_functions.push_back(GetControllerButton);
	engine_functions.push_back(GetGamepadButtonState);

	engine_functions.push_back(IsJoystickAxis);
	engine_functions.push_back(IsTriggerState);
	engine_functions.push_back(GetControllerAxis);
	engine_functions.push_back(GetAxisState);

	engine_functions.push_back(GetAxisRealValue);
	engine_functions.push_back(GetAxisValue);
	engine_functions.push_back(ShakeController);
	engine_functions.push_back(StopControllerShake);
}


void AutoCompleteFileGen::EmplaceSystemFunctions()
{
	std::string source = "System";

	SerializedFunction LOG("LOG", source.c_str());
	LOG.variables.push_back("string");
	SerializedFunction RealDT("RealDT", source.c_str());
	SerializedFunction DT("DT", source.c_str());
	SerializedFunction GameTime("GameTime", source.c_str());
	SerializedFunction PauseGame("PauseGame", source.c_str());
	SerializedFunction ResumeGame("ResumeGame", source.c_str());
	SerializedFunction GetDebuggingPath("GetDebuggingPath", source.c_str());

		// Maths
	SerializedFunction CompareFloats("CompareFloats", source.c_str());
	CompareFloats.variables.push_back("float_a"); CompareFloats.variables.push_back("float_b");
	SerializedFunction CompareDoubles("CompareDoubles", source.c_str());
	CompareDoubles.variables.push_back("double_a"); CompareDoubles.variables.push_back("double_b");
	SerializedFunction RandomNumber("RandomNumber", source.c_str());
	SerializedFunction RandomNumberInRange("RandomNumberInRange", source.c_str());
	RandomNumberInRange.variables.push_back("min"); RandomNumberInRange.variables.push_back("max");
	SerializedFunction RandomNumberList("RandomNumberList", source.c_str());
	RandomNumberList.variables.push_back("listSize"); RandomNumberList.variables.push_back("min"); RandomNumberList.variables.push_back("max");

	//PushBack all functions
	engine_functions.push_back(LOG);
	engine_functions.push_back(RealDT);
	engine_functions.push_back(DT);
	engine_functions.push_back(GameTime);
	engine_functions.push_back(PauseGame);
	engine_functions.push_back(ResumeGame);
	engine_functions.push_back(GetDebuggingPath);
	engine_functions.push_back(CompareFloats);
	engine_functions.push_back(CompareDoubles);
	engine_functions.push_back(RandomNumber);
	engine_functions.push_back(RandomNumberInRange);
	engine_functions.push_back(RandomNumberList);
}

void AutoCompleteFileGen::EmplaceTransformFunctions()
{
	std::string source = "Transform";

	SerializedFunction GetPosition("GetPosition", source.c_str());
	GetPosition.variables.push_back("gameObject_UID");
	SerializedFunction Translate("Translate", source.c_str());
	Translate.variables.push_back("x"); Translate.variables.push_back("y"); Translate.variables.push_back("z"); Translate.variables.push_back("gameObject_UID");
	SerializedFunction SetPosition("SetPosition", source.c_str());
	SetPosition.variables.push_back("x"); SetPosition.variables.push_back("y"); SetPosition.variables.push_back("z"); SetPosition.variables.push_back("gameObject_UID");
		// Rotation
	SerializedFunction GetRotation("GetRotation", source.c_str());
	GetRotation.variables.push_back("gameObject_UID");
	SerializedFunction RotateObject("RotateObject", source.c_str());
	RotateObject.variables.push_back("x"); RotateObject.variables.push_back("y"); RotateObject.variables.push_back("z"); RotateObject.variables.push_back("gameObject_UID");
	SerializedFunction SetObjectRotation("SetObjectRotation", source.c_str());
	SetObjectRotation.variables.push_back("x"); SetObjectRotation.variables.push_back("y"); SetObjectRotation.variables.push_back("z"); SetObjectRotation.variables.push_back("gameObject_UID");
	SerializedFunction LookAt("LookAt", source.c_str());
	LookAt.variables.push_back("spotX"); LookAt.variables.push_back("spotY"); LookAt.variables.push_back("spotZ"); LookAt.variables.push_back("gameObject_UID");

	engine_functions.push_back(GetPosition);
	engine_functions.push_back(Translate);
	engine_functions.push_back(SetPosition);
	engine_functions.push_back(GetRotation);
	engine_functions.push_back(RotateObject);
	engine_functions.push_back(SetObjectRotation);
	engine_functions.push_back(LookAt);
}

void AutoCompleteFileGen::EmplaceGameObjectFunctions()
{
	std::string source = "GameObject";

	SerializedFunction FindGameObject("FindGameObject", source.c_str());
	FindGameObject.variables.push_back("gameobject_name");
	FindGameObject.description = "Returns the UID of the GameObject if it is found in the scene. This function searches using strings, only recomended to be called on Awake or Start since it consumes a lot of resources.";
	SerializedFunction GetMyUID("GetMyUID", source.c_str());
	GetMyUID.description = "Returns the UID of the gameObject that contains the script and that currently running the Awake, Start or Update functions.";
	SerializedFunction GetParent("GetParent", source.c_str());
	SerializedFunction GetGameObjectParent("GetGameObjectParent", source.c_str());
	GetGameObjectParent.variables.push_back("gameObject_UID");
	SerializedFunction DestroyGameObject("DestroyGameObject", source.c_str());
	DestroyGameObject.description = "Deletes the gameObject with the passedUID.";
	DestroyGameObject.variables.push_back("gameObject_UID");
	SerializedFunction SetActiveGameObject("SetActiveGameObject", source.c_str());
	SetActiveGameObject.variables.push_back("bool_active"); SetActiveGameObject.variables.push_back("gameObject_UID");
	
	SerializedFunction GetMyLayer("GetMyLayer", source.c_str());
	SerializedFunction GetLayerByID("GetLayerByID", source.c_str());
	GetLayerByID.variables.push_back("UID");
	SerializedFunction GetComponent("GetComponent", source.c_str());
	GetComponent.variables.push_back("component_name"); GetComponent.variables.push_back("gameObject_UID");
	SerializedFunction GetScript("GetScript", source.c_str());
	GetScript.description = "Returns a reference to another script. The script returned is the one placed inside the gameObject with the UID you pass. ";


	//PushBack all functions
	engine_functions.push_back(FindGameObject);
	engine_functions.push_back(GetMyUID);
	engine_functions.push_back(GetParent);
	engine_functions.push_back(GetGameObjectParent);
	engine_functions.push_back(DestroyGameObject);
	engine_functions.push_back(SetActiveGameObject);
	engine_functions.push_back(GetMyLayer);

	engine_functions.push_back(GetLayerByID);
	engine_functions.push_back(GetComponent);
	engine_functions.push_back(GetScript);
}

void AutoCompleteFileGen::EmplaceCameraFunctions()
{
	std::string source = "Camera";

	SerializedFunction GetPositionInFrustum("GetPositionInFrustum", source.c_str());
	GetPositionInFrustum.variables.push_back("x"); GetPositionInFrustum.variables.push_back("y"); GetPositionInFrustum.variables.push_back("z"); GetPositionInFrustum.variables.push_back("FOV_ratio1"); GetPositionInFrustum.variables.push_back("FOV_ratio2");
	
	SerializedFunction GetFrustumPlanesIntersection("GetFrustumPlanesIntersection", source.c_str());
	GetFrustumPlanesIntersection.variables.push_back("x"); GetFrustumPlanesIntersection.variables.push_back("y"); GetFrustumPlanesIntersection.variables.push_back("z"); GetFrustumPlanesIntersection.variables.push_back("FOV_ratio");
	
	SerializedFunction WorldToScreen("WorldToScreen", source.c_str());
	WorldToScreen.variables.push_back("x"); WorldToScreen.variables.push_back("y"); WorldToScreen.variables.push_back("z");
	
	SerializedFunction ScreenToWorld("ScreenToWorld", source.c_str());
	ScreenToWorld.variables.push_back("x"); ScreenToWorld.variables.push_back("y"); ScreenToWorld.variables.push_back("distance");

	//PushBack all functions
	engine_functions.push_back(GetPositionInFrustum);
	engine_functions.push_back(GetFrustumPlanesIntersection);
	engine_functions.push_back(WorldToScreen);
	engine_functions.push_back(ScreenToWorld);
}

void AutoCompleteFileGen::EmplacePhysicsFunctions()
{
	std::string source = "Physics";

	SerializedFunction GetMass("GetMass", source.c_str());
	GetMass.variables.push_back("gameObject_UID");

	SerializedFunction SetMass("SetMass", source.c_str());
	SetMass.variables.push_back("mass"); SetMass.variables.push_back("gameObject_UID");



	SerializedFunction GetAngularVelocity("GetAngularVelocity", source.c_str());
	GetAngularVelocity.variables.push_back("gameObject_UID");
	
	SerializedFunction SetAngularVelocity("SetAngularVelocity", source.c_str());
	SetAngularVelocity.variables.push_back("x"); SetAngularVelocity.variables.push_back("y"); SetAngularVelocity.variables.push_back("z"); SetAngularVelocity.variables.push_back("gameObject_UID");
	
	SerializedFunction GetLinearVelocity("GetLinearVelocity", source.c_str());
	GetLinearVelocity.variables.push_back("gameObject_UID");
	
	SerializedFunction SetLinearVelocity("SetLinearVelocity", source.c_str());
	SetLinearVelocity.variables.push_back("x"); SetLinearVelocity.variables.push_back("y"); SetLinearVelocity.variables.push_back("z"); SetLinearVelocity.variables.push_back("gameObject_UID");
	
	
	SerializedFunction AddTorque("AddTorque", source.c_str());
	AddTorque.variables.push_back("forceX"); AddTorque.variables.push_back("forceY"); AddTorque.variables.push_back("forceZ"); AddTorque.variables.push_back("force_mode"); AddTorque.variables.push_back("gameObject_UID");
	
	SerializedFunction AddForce("AddForce", source.c_str());
	AddForce.variables.push_back("forceX"); AddForce.variables.push_back("forceY"); AddForce.variables.push_back("forceZ"); AddForce.variables.push_back("force_mode"); AddForce.variables.push_back("gameObject_UID");
	
	SerializedFunction UseGravity("UseGravity", source.c_str());
	UseGravity.variables.push_back("bool_enable"); UseGravity.variables.push_back("gameObject_UID");
	
	SerializedFunction SetKinematic("SetKinematic", source.c_str());
	SetKinematic.variables.push_back("bool_enable"); SetKinematic.variables.push_back("gameObject_UID");
	

	SerializedFunction OnTriggerEnter("OnTriggerEnter", source.c_str());
	OnTriggerEnter.variables.push_back("gameObject_UID");
	
	SerializedFunction OnTriggerStay("OnTriggerStay", source.c_str());
	OnTriggerStay.variables.push_back("gameObject_UID");
	
	SerializedFunction OnTriggerExit("OnTriggerExit", source.c_str());
	OnTriggerExit.variables.push_back("gameObject_UID");


	SerializedFunction OnCollisionEnter("OnCollisionEnter", source.c_str());
	OnCollisionEnter.variables.push_back("gameObject_UID");
	
	SerializedFunction OnCollisionStay("OnCollisionStay", source.c_str());
	OnCollisionStay.variables.push_back("gameObject_UID");
	
	SerializedFunction OnCollisionExit("OnCollisionExit", source.c_str());
	OnCollisionExit.variables.push_back("gameObject_UID");


	SerializedFunction Move("Move", source.c_str());
	Move.variables.push_back("vel_x"); Move.variables.push_back("vel_z"); Move.variables.push_back("gameObject_UID");
	
	SerializedFunction GetCharacterPosition("GetCharacterPosition", source.c_str());
	GetCharacterPosition.variables.push_back("gameObject_UID");
	
	SerializedFunction SetCharacterPosition("SetCharacterPosition", source.c_str());
	SetCharacterPosition.variables.push_back("x"); SetCharacterPosition.variables.push_back("y"); SetCharacterPosition.variables.push_back("z"); SetCharacterPosition.variables.push_back("gameObject_UID");
	
	SerializedFunction GetCharacterUpDirection("GetCharacterUpDirection", source.c_str());
	GetCharacterUpDirection.variables.push_back("gameObject_UID");
	
	SerializedFunction SetCharacterUpDirection("SetCharacterUpDirection", source.c_str());
	SetCharacterUpDirection.variables.push_back("rot_x"); SetCharacterUpDirection.variables.push_back("rot_y"); SetCharacterUpDirection.variables.push_back("rot_z"); SetCharacterUpDirection.variables.push_back("gameObject_UID");
	
	
	SerializedFunction OverlapSphere("OverlapSphere", source.c_str());
	OverlapSphere.variables.push_back("pos_x"); OverlapSphere.variables.push_back("pos_y"); OverlapSphere.variables.push_back("pos_z"); OverlapSphere.variables.push_back("radius"); OverlapSphere.variables.push_back("layer");
	
	SerializedFunction Raycast("Raycast", source.c_str());
	Raycast.variables.push_back("origin_x"); Raycast.variables.push_back("origin_y"); Raycast.variables.push_back("origin_z");
	Raycast.variables.push_back("direction_x"); Raycast.variables.push_back("direction_y"); Raycast.variables.push_back("direction_z_z");
	Raycast.variables.push_back("maxDistance"); Raycast.variables.push_back("layer"); Raycast.variables.push_back("bool_hitTriggers");

	//PushBack all functions
	engine_functions.push_back(GetMass);
	engine_functions.push_back(SetMass);

	engine_functions.push_back(GetAngularVelocity);
	engine_functions.push_back(SetAngularVelocity);
	engine_functions.push_back(GetLinearVelocity);
	engine_functions.push_back(SetLinearVelocity);

	engine_functions.push_back(AddTorque);
	engine_functions.push_back(AddForce);
	engine_functions.push_back(UseGravity);
	engine_functions.push_back(SetKinematic);

	engine_functions.push_back(OnTriggerEnter);
	engine_functions.push_back(OnTriggerStay);
	engine_functions.push_back(OnTriggerStay);
	engine_functions.push_back(OnTriggerExit);

	engine_functions.push_back(OnCollisionEnter);
	engine_functions.push_back(OnCollisionStay);
	engine_functions.push_back(OnCollisionExit);

	engine_functions.push_back(Move);
	engine_functions.push_back(GetCharacterPosition);
	engine_functions.push_back(SetCharacterPosition);
	engine_functions.push_back(GetCharacterUpDirection);
	engine_functions.push_back(SetCharacterUpDirection);

	engine_functions.push_back(OverlapSphere);
	engine_functions.push_back(Raycast);
}

void AutoCompleteFileGen::EmplaceParticlesFunctions()
{
	std::string source = "Particles";

	SerializedFunction ActivateParticlesEmission("ActivateParticlesEmission", source.c_str());
	ActivateParticlesEmission.variables.push_back("gameObject_UID");

	SerializedFunction DeactivateParticlesEmission("DeactivateParticlesEmission", source.c_str());
	DeactivateParticlesEmission.variables.push_back("gameObject_UID");


	SerializedFunction PlayParticleEmitter("PlayParticleEmitter", source.c_str());
	PlayParticleEmitter.variables.push_back("gameObject_UID");

	SerializedFunction StopParticleEmitter("StopParticleEmitter", source.c_str());
	PlayParticleEmitter.variables.push_back("gameObject_UID");

	SerializedFunction SetEmissionRate("SetEmissionRate", source.c_str());
	SetEmissionRate.variables.push_back("ms"); SetEmissionRate.variables.push_back("gameObject_UID");

	SerializedFunction SetParticlesPerCreation("SetParticlesPerCreation", source.c_str());
	SetParticlesPerCreation.variables.push_back("particles_amount"); SetParticlesPerCreation.variables.push_back("gameObject_UID");


	SerializedFunction SetOffsetPosition("SetOffsetPosition", source.c_str());
	SetOffsetPosition.variables.push_back("x"); SetOffsetPosition.variables.push_back("y"); SetOffsetPosition.variables.push_back("z"); SetOffsetPosition.variables.push_back("gameObject_UID");
	
	SerializedFunction SetOffsetRotation("SetOffsetRotation", source.c_str());
	SetOffsetRotation.variables.push_back("x"); SetOffsetRotation.variables.push_back("y"); SetOffsetRotation.variables.push_back("z"); SetOffsetRotation.variables.push_back("gameObject_UID");


	SerializedFunction SetExternalAcceleration("SetExternalAcceleration", source.c_str());
	SetExternalAcceleration.variables.push_back("x"); SetExternalAcceleration.variables.push_back("y"); SetExternalAcceleration.variables.push_back("z"); SetExternalAcceleration.variables.push_back("gameObject_UID");
	
	SerializedFunction SetParticlesVelocity("SetParticlesVelocity", source.c_str());
	SetParticlesVelocity.variables.push_back("x"); SetParticlesVelocity.variables.push_back("y"); SetParticlesVelocity.variables.push_back("z"); SetParticlesVelocity.variables.push_back("gameObject_UID");
	
	SerializedFunction SetRandomParticlesVelocity("SetRandomParticlesVelocity", source.c_str());
	SetRandomParticlesVelocity.variables.push_back("x"); SetRandomParticlesVelocity.variables.push_back("y"); SetRandomParticlesVelocity.variables.push_back("z"); SetRandomParticlesVelocity.variables.push_back("gameObject_UID");

	
	SerializedFunction SetParticlesLooping("SetParticlesLooping", source.c_str());
	SetParticlesLooping.variables.push_back("bool_active"); SetParticlesLooping.variables.push_back("gameObject_UID");
	
	SerializedFunction SetParticlesDuration("SetParticlesDuration", source.c_str());
	SetParticlesDuration.variables.push_back("duration"); SetParticlesDuration.variables.push_back("gameObject_UID");
	
	SerializedFunction SetParticlesLifeTime("SetParticlesLifeTime", source.c_str());
	SetParticlesLifeTime.variables.push_back("ms"); SetParticlesLifeTime.variables.push_back("gameObject_UID");

	
	SerializedFunction SetParticlesScale("SetParticlesScale", source.c_str());
	SetParticlesScale.variables.push_back("x"); SetParticlesScale.variables.push_back("y"); SetParticlesScale.variables.push_back("gameObject_UID");
	
	SerializedFunction SetRandomParticlesScale("SetRandomParticlesScale", source.c_str());
	SetRandomParticlesScale.variables.push_back("randomFactor"); SetRandomParticlesScale.variables.push_back("gameObject_UID");


	//PushBack all functions
	engine_functions.push_back(ActivateParticlesEmission);
	engine_functions.push_back(DeactivateParticlesEmission);

	engine_functions.push_back(PlayParticleEmitter);
	engine_functions.push_back(StopParticleEmitter);
	engine_functions.push_back(SetEmissionRate);
	engine_functions.push_back(SetParticlesPerCreation);

	engine_functions.push_back(SetOffsetPosition);
	engine_functions.push_back(SetOffsetRotation);

	engine_functions.push_back(SetExternalAcceleration);
	engine_functions.push_back(SetParticlesVelocity);
	engine_functions.push_back(SetRandomParticlesVelocity);

	engine_functions.push_back(SetParticlesLooping);
	engine_functions.push_back(SetParticlesDuration);
	engine_functions.push_back(SetParticlesLifeTime);

	engine_functions.push_back(SetParticlesScale);
	engine_functions.push_back(SetRandomParticlesScale);
}

void AutoCompleteFileGen::EmplaceAudioFunctions()
{
	std::string source = "Audio";

	SerializedFunction SetVolume("SetVolume", source.c_str());
	SetVolume.variables.push_back("volume");
	
	SerializedFunction PlayAudioEvent("PlayAudioEvent", source.c_str());
	PlayAudioEvent.variables.push_back("event");
	
	SerializedFunction StopAudioEvent("StopAudioEvent", source.c_str());
	StopAudioEvent.variables.push_back("event");
	
	SerializedFunction PauseAudioEvent("PauseAudioEvent", source.c_str());
	PauseAudioEvent.variables.push_back("event");
	
	SerializedFunction ResumeAudioEvent("ResumeAudioEvent", source.c_str());
	ResumeAudioEvent.variables.push_back("event");

	//PushBack all functions
	engine_functions.push_back(SetVolume);
	engine_functions.push_back(PlayAudioEvent);
	engine_functions.push_back(StopAudioEvent);
	engine_functions.push_back(PauseAudioEvent);
	engine_functions.push_back(ResumeAudioEvent);
}

//This function iterates the list of functions to deploy them in the snippets.json file to place in VSCode 
void AutoCompleteFileGen::GenerateAutoCompleteFile(bool variables_entered)
{
	// Include all functions by executing the separated functions
	EmplaceSystemFunctions();
	EmplaceTransformFunctions();
	EmplaceGameObjectFunctions();
	EmplaceCameraFunctions();
	EmplacePhysicsFunctions();
	EmplaceParticlesFunctions();
	EmplaceAudioFunctions();
	EmplaceAnimationFunctions();
	EmplaceUserInterfaceFunctions();
	EmplaceSceneFunctions();
	EmplaceNavigationFunctions();
	EmplaceScriptingInputsFunctions();

	//Start the process
	nlohmann::json file;

	//Iterate all functions into the .json file
	std::string body;
	std::string extended_description;
	std::string vars_description;
	for (std::vector<SerializedFunction>::iterator it = engine_functions.begin(); it != engine_functions.end(); ++it)
	{
		body = (*it).name + "("; // Here we stipulate what will be pasted into the code when we hit enter, we will iterate variables and finally close the ()
		
		if (variables_entered == true)
		{
			for (int i = 0; i < (*it).variables.size(); ++i)
			{
				body += "$" + std::to_string(i);
				body += (*it).variables[i].c_str();

				if ((*it).variables.size() - 1 > i) //If we aren't the last member of variables
					body += ", ";
			}
		}

		body +=  ")";

		file[(*it).name.c_str()]["body"] = body.c_str();
		file[(*it).name.c_str()]["scope"] = (*it).scope.c_str();
		file[(*it).name.c_str()]["prefix"] = (*it).name.c_str();

		if (variables_entered == true)
		{
			extended_description = "Scripting." + (*it).scope + " \\\n" + (*it).description;
		}
		else
		{
			vars_description = (*it).name.c_str();
			vars_description += "(";

			for (int i = 0; i < (*it).variables.size(); ++i)
			{
				vars_description += (*it).variables[i];
				if ((*it).variables.size() - 1 > i) //If we aren't the last member of variables
					vars_description += ", ";
			}
			vars_description += ")";

			extended_description = "Scripting." + (*it).scope + " \\\n" + (*it).description + " \\\n" + vars_description.c_str();
		}

		file[(*it).name.c_str()]["description"] = extended_description.c_str();
	}

	// --- Serialize JSON to string ---
	std::string data;
	App->GetJLoader()->Serialize(file, data);

	// --- Finally Save to file ---
	char* buffer = (char*)data.data();
	uint size = data.length();

	std::string file_path = "snippets.json";
	App->fs->Save(file_path.c_str(), buffer, size);
}