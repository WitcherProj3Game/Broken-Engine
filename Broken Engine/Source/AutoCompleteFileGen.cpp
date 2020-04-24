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
	SerializedFunction SetAnimationSpeed("SetAnimationSpeed", source.c_str());
	SerializedFunction SetCurrentAnimationSpeed("SetCurrentAnimationSpeed", source.c_str());
	SerializedFunction SetBlendTime("SetBlendTime", source.c_str());
	SerializedFunction CurrentAnimationEnded("CurrentAnimationEnded", source.c_str());


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
	SerializedFunction MakeElementInvisible("MakeElementInvisible", source.c_str());

	SerializedFunction SetUIBarPercentage("SetUIBarPercentage", source.c_str());
	SerializedFunction SetUICircularBarPercentage("SetUICircularBarPercentage", source.c_str());
	SerializedFunction SetText("SetText", source.c_str());
	SerializedFunction SetTextAndNumber("SetTextAndNumber", source.c_str());
	SerializedFunction SetTextNumber("SetTextNumber", source.c_str());


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
	SerializedFunction QuitGame("QuitGame", source.c_str());
	SerializedFunction Instantiate("Instantiate", source.c_str());

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
	SerializedFunction GetAreaCost("GetAreaCost", source.c_str());
	SerializedFunction SetAreaCost("SetAreaCost", source.c_str());
	SerializedFunction CalculatePath("CalculatePath", source.c_str());
	

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
	SerializedFunction KeyUp("KeyUp", source.c_str());
	SerializedFunction KeyRepeat("KeyRepeat", source.c_str());
	SerializedFunction KeyIdle("KeyIdle", source.c_str());

	SerializedFunction GetMouseButton("GetMouseButton", source.c_str());
	SerializedFunction MouseButtonState("MouseButtonState", source.c_str());

	SerializedFunction MouseButtonDown("MouseButtonDown", source.c_str());
	SerializedFunction MouseButtonUp("MouseButtonUp", source.c_str());
	SerializedFunction MouseButtonRepeat("MouseButtonRepeat", source.c_str());
	SerializedFunction MouseButtonIdle("MouseButtonIdle", source.c_str());

	SerializedFunction IsGamepadButton("IsGamepadButton", source.c_str());
	SerializedFunction GetControllerButton("GetControllerButton", source.c_str());
	SerializedFunction GetGamepadButtonState("GetGamepadButtonState", source.c_str());
	
	SerializedFunction IsJoystickAxis("IsJoystickAxis", source.c_str());
	SerializedFunction IsTriggerState("IsTriggerState", source.c_str());
	SerializedFunction GetControllerAxis("GetControllerAxis", source.c_str());
	SerializedFunction GetAxisState("GetAxisState", source.c_str());
	
	SerializedFunction GetAxisRealValue("GetAxisRealValue", source.c_str());
	SerializedFunction GetAxisValue("GetAxisValue", source.c_str());
	SerializedFunction ShakeController("ShakeController", source.c_str());
	SerializedFunction StopControllerShake("StopControllerShake", source.c_str());





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
	SerializedFunction RealDT("RealDT", source.c_str());
	SerializedFunction DT("DT", source.c_str());
	SerializedFunction GameTime("GameTime", source.c_str());
	SerializedFunction PauseGame("PauseGame", source.c_str());
	SerializedFunction ResumeGame("ResumeGame", source.c_str());
	SerializedFunction GetDebuggingPath("GetDebuggingPath", source.c_str());

		// Maths
	SerializedFunction CompareFloats("CompareFloats", source.c_str());
	SerializedFunction CompareDoubles("CompareDoubles", source.c_str());
	SerializedFunction RandomNumber("RandomNumber", source.c_str());
	SerializedFunction RandomNumberInRange("RandomNumberInRange", source.c_str());
	SerializedFunction RandomNumberList("RandomNumberList", source.c_str());


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
	SerializedFunction Translate("Translate", source.c_str());
	SerializedFunction SetPosition("SetPosition", source.c_str());

		// Rotation
	SerializedFunction GetRotation("GetRotation", source.c_str());
	SerializedFunction RotateObject("RotateObject", source.c_str());
	SerializedFunction SetObjectRotation("SetObjectRotation", source.c_str());
	SerializedFunction LookAt("LookAt", source.c_str());

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
	FindGameObject.description = "Returns the UID of the GameObject if it is found in the scene. This function searches using strings, only recomended to be called on Awake or Start since it consumes a lot of resources.";
	SerializedFunction GetMyUID("GetMyUID", source.c_str());
	GetMyUID.description = "Returns the UID of the gameObject that contains the script and that currently running the Awake, Start or Update functions.";
	SerializedFunction GetParent("GetParent", source.c_str());
	SerializedFunction GetGameObjectParent("GetGameObjectParent", source.c_str());
	SerializedFunction DestroyGameObject("DestroyGameObject", source.c_str());
	DestroyGameObject.description = "Deletes the gameObject with the passedUID.";
	SerializedFunction SetActiveGameObject("SetActiveGameObject", source.c_str());

	SerializedFunction GetMyLayer("GetMyLayer", source.c_str());
	SerializedFunction GetLayerByID("GetLayerByID", source.c_str());
	SerializedFunction GetComponent("GetComponent", source.c_str());
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
	SerializedFunction GetFrustumPlanesIntersection("GetFrustumPlanesIntersection", source.c_str());
	SerializedFunction WorldToScreen("WorldToScreen", source.c_str());
	SerializedFunction ScreenToWorld("ScreenToWorld", source.c_str());

	//PushBack all functions
	engine_functions.push_back(GetPositionInFrustum);
	engine_functions.push_back(GetFrustumPlanesIntersection);
	engine_functions.push_back(WorldToScreen);
	engine_functions.push_back(ScreenToWorld);
}

void AutoCompleteFileGen::EmplacePhysicsFunctions()
{
	std::string source = "Physics";

	SerializedFunction GetAngularVelocity("GetAngularVelocity", source.c_str());
	SerializedFunction SetAngularVelocity("SetAngularVelocity", source.c_str());
	SerializedFunction GetLinearVelocity("GetLinearVelocity", source.c_str());
	SerializedFunction SetLinearVelocity("SetLinearVelocity", source.c_str());

	SerializedFunction AddTorque("AddTorque", source.c_str());
	SerializedFunction AddForce("AddForce", source.c_str());
	SerializedFunction UseGravity("UseGravity", source.c_str());
	SerializedFunction SetKinematic("SetKinematic", source.c_str());

	SerializedFunction OnTriggerEnter("OnTriggerEnter", source.c_str());
	SerializedFunction OnTriggerStay("OnTriggerStay", source.c_str());
	SerializedFunction OnTriggerExit("OnTriggerExit", source.c_str());

	SerializedFunction OnCollisionEnter("OnCollisionEnter", source.c_str());
	SerializedFunction OnCollisionStay("OnCollisionStay", source.c_str());
	SerializedFunction OnCollisionExit("OnCollisionExit", source.c_str());

	SerializedFunction Move("Move", source.c_str());
	SerializedFunction GetCharacterPosition("GetCharacterPosition", source.c_str());
	SerializedFunction SetCharacterPosition("SetCharacterPosition", source.c_str());
	SerializedFunction GetCharacterUpDirection("GetCharacterUpDirection", source.c_str());
	SerializedFunction SetCharacterUpDirection("SetCharacterUpDirection", source.c_str());

	SerializedFunction OverlapSphere("OverlapSphere", source.c_str());
	SerializedFunction Raycast("Raycast", source.c_str());


	//PushBack all functions
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
	SerializedFunction DeactivateParticlesEmission("DeactivateParticlesEmission", source.c_str());

	SerializedFunction PlayParticleEmitter("PlayParticleEmitter", source.c_str());
	SerializedFunction StopParticleEmitter("StopParticleEmitter", source.c_str());
	SerializedFunction SetEmissionRate("SetEmissionRate", source.c_str());
	SerializedFunction SetParticlesPerCreation("SetParticlesPerCreation", source.c_str());

	SerializedFunction SetOffsetPosition("SetOffsetPosition", source.c_str());
	SerializedFunction SetOffsetRotation("SetOffsetRotation", source.c_str());

	SerializedFunction SetExternalAcceleration("SetExternalAcceleration", source.c_str());
	SerializedFunction SetParticlesVelocity("SetParticlesVelocity", source.c_str());
	SerializedFunction SetRandomParticlesVelocity("SetRandomParticlesVelocity", source.c_str());

	SerializedFunction SetParticlesLooping("SetParticlesLooping", source.c_str());
	SerializedFunction SetParticlesDuration("SetParticlesDuration", source.c_str());
	SerializedFunction SetParticlesLifeTime("SetParticlesLifeTime", source.c_str());

	SerializedFunction SetParticlesScale("SetParticlesScale", source.c_str());
	SerializedFunction SetRandomParticlesScale("SetRandomParticlesScale", source.c_str());

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
	SerializedFunction PlayAudioEvent("PlayAudioEvent", source.c_str());
	SerializedFunction StopAudioEvent("StopAudioEvent", source.c_str());
	SerializedFunction PauseAudioEvent("PauseAudioEvent", source.c_str());
	SerializedFunction ResumeAudioEvent("ResumeAudioEvent", source.c_str());

	//PushBack all functions
	engine_functions.push_back(SetVolume);
	engine_functions.push_back(PlayAudioEvent);
	engine_functions.push_back(StopAudioEvent);
	engine_functions.push_back(PauseAudioEvent);
	engine_functions.push_back(ResumeAudioEvent);
}

//This function iterates the list of functions to deploy them in the snippets.json file to place in VSCode 
void AutoCompleteFileGen::GenerateAutoCompleteFile()
{
	GetAllFunctions();
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
	for (std::vector<SerializedFunction>::iterator it = engine_functions.begin(); it != engine_functions.end(); ++it)
	{
		body = (*it).name + "()";
		file[(*it).name.c_str()]["body"] = body.c_str();
		file[(*it).name.c_str()]["scope"] = (*it).scope.c_str();
		file[(*it).name.c_str()]["prefix"] = (*it).name.c_str();
		extended_description = (*it).description+ "Function belongs to: Scripting." + (*it).scope;
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

void AutoCompleteFileGen::GetAllFunctions()
{
	//luabridge::LuaRef table = luabridge::getGlobal(App->scripting->L, "Scripting");
	std::string file = App->scripting->GetScriptingBasePath() + "Lua_Globals/ACSystemGetter.lua";
	int compiled = luaL_dofile(App->scripting->L, file.c_str());

	/*if (compiled == LUA_OK)
	{
		luabridge::LuaRef ScriptGetTable = luabridge::getGlobal(App->scripting->L, "GetSystem");
		luabridge::LuaRef table(ScriptGetTable());
		const char* sys = "System";


		for (luabridge::Iterator it(table); !it.isNil(); ++it)
		{
			std::string test = (*it).first.tostring();
		}
	}*/

	/*ScriptingSystem system;
	luabridge::LuaRef table = luabridge::newTable(App->scripting->L);
	table(new ScriptingSystem);
	const char* sys = "System";


	for (luabridge::Iterator it(table); !it.isNil(); ++it)
	{
		std::string test = (*it).first.tostring();
	}*/
}
