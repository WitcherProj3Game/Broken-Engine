#include "Application.h"
#include "Imgui/imgui.h"
#include "GameObject.h"
#include "Timer.h"
#include "RandomGenerator.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"

#include "ComponentParticleEmitter.h"
#include "ComponentTransform.h"
#include "ComponentText.h"
#include "ComponentCamera.h"

#include "ModuleTimeManager.h"
#include "ModulePhysics.h"
#include "ModuleParticles.h"
#include "ModuleTextures.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleFileSystem.h"
#include "ModuleSceneManager.h"

#include "Particle.h"

#include "PhysX_3.4/Include/extensions/PxDefaultAllocator.h"
#include "PhysX_3.4/Include/extensions/PxDefaultErrorCallback.h"

#include "PhysX_3.4/Include/PxPhysicsAPI.h"

#include "mmgr/mmgr.h"


using namespace Broken;

ComponentParticleEmitter::ComponentParticleEmitter(GameObject* ContainerGO) :Component(ContainerGO, Component::ComponentType::ParticleEmitter)
{
	name = "Particle Emitter";
	Enable();

	App->particles->AddEmitter(this);

	particles.resize(maxParticles);

	for (int i = 0; i < maxParticles; ++i)
		particles[i] = new Particle();

	texture = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "DefaultTexture");
	App->renderer3D->particleEmitters.push_back(this);

	float4 whiteColor(1, 1, 1, 1);
	colors.push_back(whiteColor);

	pointsCurve.push_back(0.0f);
	pointsCurve.push_back(0.0f);
	pointsCurve.push_back(0.6f);
	pointsCurve.push_back(0.4f);
	pointsCurve.push_back(1.0f);
	pointsCurve.push_back(1.0f);
}

ComponentParticleEmitter::~ComponentParticleEmitter()
{
	drawingIndices.clear();

	App->particles->DeleteEmitter(this);

	for (int i = 0; i < maxParticles; ++i) {
		delete particles[i];
	}

	if (particleSystem && App->physics->mScene) {
		particleSystem->releaseParticles();
		App->physics->DeleteActor(particleSystem);
		indexPool->release();
		particles.clear();
	}

	texture->Release();


	for (std::vector<ComponentParticleEmitter*>::iterator it = App->renderer3D->particleEmitters.begin(); it != App->renderer3D->particleEmitters.end(); it++)
		if ((*it) == this) {
			App->renderer3D->particleEmitters.erase(it);
			break;
		}
}

void ComponentParticleEmitter::Update()
{
	/*if (auxCurve.size() != 0) {
		pointsCurve.resize(auxCurve.size());
		std::copy(auxCurve.begin(), auxCurve.end(), pointsCurve.begin());
	}*/

	if (!animation || !createdAnim) {
		if (particleMeshes.size() > 0) {
			for (int i = 0; i < particleMeshes.size(); ++i) {
				particleMeshes.at(i)->FreeMemory();
				delete particleMeshes.at(i);
			}
			particleMeshes.clear();
		}
		createdAnim = false;
	}

	if (animation && !createdAnim) {
		CreateAnimation(tileSize_X, tileSize_Y);
		createdAnim = true;
	}

	if (to_delete)
		this->GetContainerGameObject()->RemoveComponent(this);
}

void ComponentParticleEmitter::Enable()
{
	active = true;

	particleSystem = App->physics->mPhysics->createParticleSystem(maxParticles, perParticleRestOffset);
	particleSystem->setMaxMotionDistance(100);

	physx::PxFilterData filterData;
	filterData.word0 = (1 << GO->layer);
	filterData.word1 = App->physics->layer_list.at(GO->layer).LayerGroup;
	particleSystem->setSimulationFilterData(filterData);

	if (particleSystem)
		App->physics->AddParticleActor(particleSystem, GO);

	indexPool = physx::PxParticleExt::createIndexPool(maxParticles);

	particleSystem->setExternalAcceleration(externalAcceleration);
}

void ComponentParticleEmitter::Disable()
{

	App->physics->DeleteActor(particleSystem);

	active = false;
}

void ComponentParticleEmitter::UpdateParticles(float dt)
{
	int currentPlayTime = App->time->GetGameplayTimePassed() * 1000;

	// Create particle depending on the time
	if (emisionActive && App->GetAppState() == AppState::PLAY && !App->time->gamePaused) {
		if (App->time->GetGameplayTimePassed() * 1000 - spawnClock > emisionRate)
		{
			uint newParticlesAmount = (App->time->GetGameplayTimePassed() * 1000 - spawnClock) / emisionRate;
			CreateParticles(newParticlesAmount * particlesPerCreation);
		}

		if (emisionActive && !loop)
		{
			if ((App->time->GetGameplayTimePassed() * 1000) - emisionStart > duration)
				emisionActive = false;
		}
	}

	physx::PxFilterData filterData;
	filterData.word0 = (1 << GO->layer); // word0 = own ID
	filterData.word1 = App->physics->layer_list.at(GO->layer).LayerGroup; // word1 = ID mask to filter pairs that trigger a contact callback;
	particleSystem->setSimulationFilterData(filterData);

	//Update particles
//lock SDK buffers of *PxParticleSystem* ps for reading
	physx::PxParticleReadData* rd = particleSystem->lockParticleReadData();

	std::vector<physx::PxU32> indicesToErease;
	uint particlesToRelease = 0;

	// access particle data from physx::PxParticleReadData
	if (rd)
	{
		physx::PxStrideIterator<const physx::PxParticleFlags> flagsIt(rd->flagsBuffer);
		physx::PxStrideIterator<const physx::PxVec3> positionIt(rd->positionBuffer);

		for (unsigned i = 0; i < rd->validParticleRange; ++i, ++flagsIt, ++positionIt)
		{
			if (*flagsIt & physx::PxParticleFlag::eVALID)
			{
				//Check if particle should die
				if (App->time->GetGameplayTimePassed() * 1000 - particles[i]->spawnTime > particles[i]->lifeTime) {
					indicesToErease.push_back(i);
					particlesToRelease++;
					continue;
				}

				float diff_time = (App->time->GetGameplayTimePassed() * 1000 - particles[i]->spawnTime);
				if (diff_time > 0) {
					for (int i = 0; i < pointsCurve.size() / 2 - 1; ++i) {
						if (diff_time / particles[i]->lifeTime > pointsCurve[i * 2]) {
							float p1_time = pointsCurve[i * 2] * particles[i]->lifeTime;
							float p2_time = pointsCurve[i * 2 + 2] * particles[i]->lifeTime;
							float p1_value = pointsCurve[i * 2 + 1] * multiplier;
							float p2_value = pointsCurve[i * 2 + 3] * multiplier;
							float scope = (p2_value - p1_value) / (p2_time - p1_time);
							scaleOverTime = p1_value + scope * (diff_time - p1_time);
						}
					}
				}

				//Update particle position
				particles[i]->scale.x = scaleOverTime;
				particles[i]->scale.y = scaleOverTime;

				particles[i]->position = float3(positionIt->x, positionIt->y, positionIt->z);

				if (colorGradient && gradients.size() > 0)
				{
					if (particles[i]->currentGradient >= gradients.size())//Comment this and next line in case gradient widget is applyed
						particles[i]->currentGradient = gradients.size() - 1;
					particles[i]->color += gradients[particles[i]->currentGradient] * dt * 1000;

					if ((currentPlayTime - particles[i]->gradientTimer > colorDuration) && (particles[i]->currentGradient < gradients.size()-1))
					{
						particles[i]->currentGradient++;
						particles[i]->gradientTimer = currentPlayTime;
					}

				}

				if (particles[i]->scale.x < 0)
					particles[i]->scale.x = 0;

				if (particles[i]->scale.y < 0)
					particles[i]->scale.y = 0;

				//Choose Frame Animation
				if (animation && particleMeshes.size() > 0) {
					int time = App->time->GetGameplayTimePassed() * 1000 - particles[i]->spawnTime;
					int index = (particleMeshes.size() * time) / (particles[i]->lifeTime / cycles);
					particles[i]->plane = particleMeshes[(index + startFrame) % particleMeshes.size()];
				}
				else
					particles[i]->plane = App->scene_manager->plane;

				if(rotationOvertime1 != 0)
					particles[i]->rotation += particles[i]->rotationSpeed * DEGTORAD * dt;

			}
		}
		// return ownership of the buffers back to the SDK
		rd->unlock();
	}

	if (particlesToRelease > 0) {

		particleSystem->releaseParticles(particlesToRelease, physx::PxStrideIterator<physx::PxU32>(indicesToErease.data()));
		validParticles -= particlesToRelease;
		indexPool->freeIndices(particlesToRelease, physx::PxStrideIterator<physx::PxU32>(indicesToErease.data()));
	}

	SortParticles();
}

void ComponentParticleEmitter::SortParticles()
{
	physx::PxParticleReadData* rd = particleSystem->lockParticleReadData();
	if (rd)
	{
		physx::PxStrideIterator<const physx::PxParticleFlags> flagsIt(rd->flagsBuffer);

		for (unsigned i = 0; i < rd->validParticleRange; ++i, ++flagsIt)
		{
			if (*flagsIt & physx::PxParticleFlag::eVALID)
			{
				float distance = App->renderer3D->active_camera->frustum.Pos().Distance(particles[i]->position);
				drawingIndices[1.0f / distance] = i;
			}
		}

		// return ownership of the buffers back to the SDK
		rd->unlock();
	}
}

void ComponentParticleEmitter::DrawParticles()
{
	if (!active || drawingIndices.empty())
		return;

	std::map<float, int>::iterator it = drawingIndices.begin();
	while (it != drawingIndices.end())
	{
		int paco = (*it).second;
		particles[paco]->Draw();
		it++;
	}
	drawingIndices.clear();
}

void ComponentParticleEmitter::ChangeParticlesColor(float4 color)
{
	color /= 255.0f;

	for (int i = 0; i < maxParticles; ++i)
		particles[i]->color = color;
}

json ComponentParticleEmitter::Save() const
{
	json node;

	node["Active"] = this->active;

	node["positionX"] = std::to_string(emitterPosition.x);
	node["positionY"] = std::to_string(emitterPosition.y);
	node["positionZ"] = std::to_string(emitterPosition.z);

	node["rotationX"] = std::to_string(eulerRotation.x);
	node["rotationY"] = std::to_string(eulerRotation.y);
	node["rotationZ"] = std::to_string(eulerRotation.z);

	node["sizeX"] = std::to_string(size.x);
	node["sizeY"] = std::to_string(size.y);
	node["sizeZ"] = std::to_string(size.z);

	node["emisionRate"] = std::to_string(emisionRate);

	node["particlesPerCreation"] = std::to_string(particlesPerCreation);

	node["externalAccelerationX"] = std::to_string(externalAcceleration.x);
	node["externalAccelerationY"] = std::to_string(externalAcceleration.y);
	node["externalAccelerationZ"] = std::to_string(externalAcceleration.z);

	node["particlesVelocityX"] = std::to_string(particlesVelocity.x);
	node["particlesVelocityY"] = std::to_string(particlesVelocity.y);
	node["particlesVelocityZ"] = std::to_string(particlesVelocity.z);

	node["velocityRandomFactorX"] = std::to_string(velocityRandomFactor.x);
	node["velocityRandomFactorY"] = std::to_string(velocityRandomFactor.y);
	node["velocityRandomFactorZ"] = std::to_string(velocityRandomFactor.z);

	node["particlesLifeTime"] = std::to_string(particlesLifeTime);

	node["animation"] = std::to_string(animation);
	node["tiles_X"] = std::to_string(tileSize_X);
	node["tiles_Y"] = std::to_string(tileSize_Y);
	node["cycles"] = std::to_string(cycles);
	node["startFrame"] = std::to_string(startFrame);

	node["num_colors"] = std::to_string(colors.size());

	for (int i = 0; i < colors.size(); ++i) {
		node["colors"][i]["x"] = std::to_string(colors[i].x);
		node["colors"][i]["y"] = std::to_string(colors[i].y);
		node["colors"][i]["z"] = std::to_string(colors[i].z);
		node["colors"][i]["a"] = std::to_string(colors[i].w);
	}

	node["num_gradients"] = std::to_string(gradients.size());
	for (int i = 0; i < gradients.size(); ++i) {
		node["gradients"][i]["x"] = std::to_string(gradients[i].x);
		node["gradients"][i]["y"] = std::to_string(gradients[i].y);
		node["gradients"][i]["z"] = std::to_string(gradients[i].z);
		node["gradients"][i]["a"] = std::to_string(gradients[i].w);
	}

	node["grad_duration"] = std::to_string(colorDuration);

	node["GradientColor"] = colorGradient;

	node["Loop"] = loop;
	node["Duration"] = std::to_string(duration);

	node["particlesScaleX"] = std::to_string(particlesScale.x);
	node["particlesScaleY"] = std::to_string(particlesScale.y);

	node["particleScaleRandomFactor"] = std::to_string(particlesScaleRandomFactor);

	node["particleScaleOverTime"] = std::to_string(scaleOverTime);

	node["Resources"]["ResourceTexture"];

	if (texture)
		node["Resources"]["ResourceTexture"] = std::string(texture->GetResourceFile());

	node["separateAxis"] = std::to_string(separateAxis);
	node["rotationOvertime1"][0] = std::to_string(rotationOvertime1[0]);
	node["rotationOvertime1"][1] = std::to_string(rotationOvertime1[1]);
	node["rotationOvertime1"][2] = std::to_string(rotationOvertime1[2]);
	node["rotationOvertime2"][0] = std::to_string(rotationOvertime2[0]);
	node["rotationOvertime2"][1] = std::to_string(rotationOvertime2[1]);
	node["rotationOvertime2"][2] = std::to_string(rotationOvertime2[2]);
	node["constants"] = std::to_string(constants);

	return node;
}

void ComponentParticleEmitter::Load(json& node)
{
	this->active = node["Active"].is_null() ? true : (bool)node["Active"];

	//load the strings
	std::string LpositionX = node["positionX"].is_null() ? "0" : node["positionX"];
	std::string LpositionY = node["positionY"].is_null() ? "0" : node["positionY"];
	std::string LpositionZ = node["positionZ"].is_null() ? "0" : node["positionZ"];

	std::string LrotationX = node["rotationX"].is_null() ? "0" : node["rotationX"];
	std::string LrotationY = node["rotationY"].is_null() ? "0" : node["rotationY"];
	std::string LrotationZ = node["rotationZ"].is_null() ? "0" : node["rotationZ"];

	std::string Lsizex = node["sizeX"].is_null() ? "0" : node["sizeX"];
	std::string Lsizey = node["sizeY"].is_null() ? "0" : node["sizeY"];
	std::string Lsizez = node["sizeZ"].is_null() ? "0" : node["sizeZ"];

	std::string LemisionRate = node["emisionRate"].is_null() ? "0" : node["emisionRate"]; // typo: emission

	std::string LparticlesPerCreation = node["particlesPerCreation"].is_null() ? "1" : node["particlesPerCreation"];

	std::string LexternalAccelerationx = node["externalAccelerationX"].is_null() ? "0" : node["externalAccelerationX"];
	std::string LexternalAccelerationy = node["externalAccelerationY"].is_null() ? "0" : node["externalAccelerationY"];
	std::string LexternalAccelerationz = node["externalAccelerationZ"].is_null() ? "0" : node["externalAccelerationZ"];

	std::string LparticlesVelocityx = node["particlesVelocityX"].is_null() ? "0" : node["particlesVelocityX"];
	std::string LparticlesVelocityy = node["particlesVelocityY"].is_null() ? "0" : node["particlesVelocityY"];
	std::string LparticlesVelocityz = node["particlesVelocityZ"].is_null() ? "0" : node["particlesVelocityZ"];

	std::string LvelocityRandomFactorx = node["velocityRandomFactorX"].is_null() ? "0" : node["velocityRandomFactorX"];
	std::string LvelocityRandomFactory = node["velocityRandomFactorY"].is_null() ? "0" : node["velocityRandomFactorY"];
	std::string LvelocityRandomFactorz = node["velocityRandomFactorZ"].is_null() ? "0" : node["velocityRandomFactorZ"];

	std::string LparticlesLifeTime = node["particlesLifeTime"].is_null() ? "0" : node["particlesLifeTime"];

	std::string LParticlesSize = node["particlesSize"].is_null() ? "0" : node["particlesSize"];

	std::string _animation = node["animation"].is_null() ? "0" : node["animation"];
	std::string _tiles_X = node["tiles_X"].is_null() ? "1" : node["tiles_X"];
	std::string _tiles_Y = node["tiles_Y"].is_null() ? "1" : node["tiles_Y"];
	std::string _cycles = node["cycles"].is_null() ? "1" : node["cycles"];
	std::string _startFrame = node["startFrame"].is_null() ? "0" : node["startFrame"];

	std::string LDuration = node["Duration"].is_null() ? "0" : node["Duration"];

	std::string LParticlesScaleX = node["particlesScaleX"].is_null() ? "1" : node["particlesScaleX"];
	std::string LParticlesScaleY = node["particlesScaleY"].is_null() ? "1" : node["particlesScaleY"];

	std::string LParticleScaleRandomFactor = node["particleScaleRandomFactor"].is_null() ? "1" : node["particleScaleRandomFactor"];

	std::string LScaleOverTime = node["particleScaleOverTime"].is_null() ? "0" : node["particleScaleOverTime"];

	std::string _num_colors = node["num_colors"].is_null() ? "0" : node["num_colors"];
	std::string _num_gradients = node["num_gradients"].is_null() ? "0" : node["num_gradients"];
	std::string _gradientDuration = node["grad_duration"].is_null() ? "0" : node["grad_duration"];


	std::string _separateAxis = node["separateAxis"].is_null() ? "0" : node["separateAxis"];
	std::string rotationOvertime1_X = node["rotationOvertime1"][0].is_null() ? "0" : node["rotationOvertime1"][0];
	std::string rotationOvertime1_Y = node["rotationOvertime1"][1].is_null() ? "0" : node["rotationOvertime1"][1];
	std::string rotationOvertime1_Z = node["rotationOvertime1"][2].is_null() ? "0" : node["rotationOvertime1"][2];
	std::string rotationOvertime2_X = node["rotationOvertime2"][0].is_null() ? "0" : node["rotationOvertime2"][0];
	std::string rotationOvertime2_Y = node["rotationOvertime2"][1].is_null() ? "0" : node["rotationOvertime2"][1];
	std::string rotationOvertime2_Z = node["rotationOvertime2"][2].is_null() ? "0" : node["rotationOvertime2"][2];
	std::string _constants = node["constants"].is_null() ? "0" : node["constants"];

	colorDuration = std::atoi(_gradientDuration.c_str());
	int num = std::stof(_num_colors);
	if (num != 0) {
		colors.pop_back();
		for (int i = 0; i < num; ++i) {
			std::string color_x = node["colors"][i]["x"].is_null() ? "255" : node["colors"][i]["x"];
			std::string color_y = node["colors"][i]["y"].is_null() ? "255" : node["colors"][i]["y"];
			std::string color_z = node["colors"][i]["z"].is_null() ? "255" : node["colors"][i]["z"];
			std::string color_a = node["colors"][i]["a"].is_null() ? "255" : node["colors"][i]["a"];
			float4 color = float4(std::stof(color_x), std::stof(color_y), std::stof(color_z), std::stof(color_a));
			colors.push_back(color);
		}
	}

	if (!node["GradientColor"].is_null())
		colorGradient = node["GradientColor"];
	else
		colorGradient = false;

	num = std::stof(_num_gradients);
	for (int i = 0; i < num; ++i) {
		std::string color_x = node["gradients"][i]["x"].is_null() ? "255" : node["gradients"][i]["x"];
		std::string color_y = node["gradients"][i]["y"].is_null() ? "255" : node["gradients"][i]["y"];
		std::string color_z = node["gradients"][i]["z"].is_null() ? "255" : node["gradients"][i]["z"];
		std::string color_a = node["gradients"][i]["a"].is_null() ? "255" : node["gradients"][i]["a"];
		float4 color = float4(std::stof(color_x), std::stof(color_y), std::stof(color_z), std::stof(color_a));
		gradients.push_back(color);
	}

	if (!node["Loop"].is_null())
		loop = node["Loop"];
	else
		loop = true;

	std::string path = node["Resources"]["ResourceTexture"].is_null() ? "0" : node["Resources"]["ResourceTexture"];
	App->fs->SplitFilePath(path.c_str(), nullptr, &path);
	path = path.substr(0, path.find_last_of("."));

	ResourceTexture* auxText = (ResourceTexture*)App->resources->GetResource(std::stoi(path));

	if (auxText != nullptr)
		texture = auxText;

	if (texture)
		texture->AddUser(GO);

	//Pass the strings to the needed dada types
	emitterPosition.x = std::stof(LpositionX);
	emitterPosition.y = std::stof(LpositionY);
	emitterPosition.z = std::stof(LpositionZ);

	eulerRotation.x = std::stof(LrotationX);
	eulerRotation.y = std::stof(LrotationY);
	eulerRotation.z = std::stof(LrotationZ);

	emitterRotation = Quat::FromEulerXYZ(eulerRotation.x * DEGTORAD, eulerRotation.y * DEGTORAD, eulerRotation.z * DEGTORAD);

	size.x = std::stof(Lsizex);
	size.y = std::stof(Lsizey);
	size.z = std::stof(Lsizez);

	emisionRate = std::stof(LemisionRate);

	particlesPerCreation = std::stoi(LparticlesPerCreation);

	externalAcceleration.x = std::stof(LexternalAccelerationx);
	externalAcceleration.y = std::stof(LexternalAccelerationy);
	externalAcceleration.z = std::stof(LexternalAccelerationz);
	particleSystem->setExternalAcceleration(externalAcceleration);

	particlesVelocity.x = std::stof(LparticlesVelocityx);
	particlesVelocity.y = std::stof(LparticlesVelocityy);
	particlesVelocity.z = std::stof(LparticlesVelocityz);

	velocityRandomFactor.x = std::stof(LvelocityRandomFactorx);
	velocityRandomFactor.y = std::stof(LvelocityRandomFactory);
	velocityRandomFactor.z = std::stof(LvelocityRandomFactorz);

	particlesLifeTime = std::stof(LparticlesLifeTime);

	duration = std::stoi(LDuration);

	particlesScale.x = std::stof(LParticlesScaleX);
	particlesScale.y = std::stof(LParticlesScaleY);

	particlesScaleRandomFactor = std::stof(LParticleScaleRandomFactor);

	scaleOverTime = std::stof(LScaleOverTime);

	animation = std::stof(_animation);
	tileSize_X = std::stof(_tiles_X);
	tileSize_Y = std::stof(_tiles_Y);
	cycles = std::stof(_cycles);
	startFrame = std::stof(_startFrame);

	separateAxis = std::stof(_separateAxis);
	rotationOvertime1[0] = std::stof(rotationOvertime1_X);
	rotationOvertime1[1] = std::stof(rotationOvertime1_Y);
	rotationOvertime1[2] = std::stof(rotationOvertime1_Z);
	rotationOvertime2[0] = std::stof(rotationOvertime2_X);
	rotationOvertime2[1] = std::stof(rotationOvertime2_Y);
	rotationOvertime2[2] = std::stof(rotationOvertime2_Z);
	constants = std::stof(_constants);

}

void ComponentParticleEmitter::CreateInspectorNode()
{
	ImGui::Text("Loop");
	ImGui::SameLine();

	if (ImGui::Checkbox("##PELoop", &loop)) {
		if (loop)
			emisionActive = true;
	}

	ImGui::Text("Duration");
	ImGui::SameLine();
	if (ImGui::DragInt("##PEDuration", &duration))
		Play();

	if (ImGui::Button("Delete component"))
		to_delete = true;


	//Emitter position
	ImGui::Text("Position");

	ImGui::Text("X");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

	ImGui::DragFloat("##SPositionX", &emitterPosition.x, 0.05f);

	ImGui::SameLine();

	ImGui::Text("Y");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

	ImGui::DragFloat("##SPositionY", &emitterPosition.y, 0.05f);

	ImGui::SameLine();

	ImGui::Text("Z");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

	ImGui::DragFloat("##SPositionZ", &emitterPosition.z, 0.05f);

	//Emitter rotation
	ImGui::Text("Rotation");

	float3 rotation = eulerRotation;
	bool rotationUpdated = false;

	ImGui::Text("X");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

	if (ImGui::DragFloat("##SRotationX", &rotation.x, 0.15f, -10000.0f, 10000.0f))
		rotationUpdated = true;

	ImGui::SameLine();

	ImGui::Text("Y");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

	if (ImGui::DragFloat("##SRotationY", &rotation.y, 0.15f, -10000.0f, 10000.0f))
		rotationUpdated = true;

	ImGui::SameLine();

	ImGui::Text("Z");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

	if (ImGui::DragFloat("##SRotationZ", &rotation.z, 0.15f, -10000.0f, 10000.0f))
		rotationUpdated = true;

	if (rotationUpdated) {
		float3 difference = (rotation - eulerRotation) * DEGTORAD;
		Quat quatrot = Quat::FromEulerXYZ(difference.x, difference.y, difference.z);

		emitterRotation = Quat::FromEulerXYZ(rotation.x * DEGTORAD, rotation.y * DEGTORAD, rotation.z * DEGTORAD);
		eulerRotation = rotation;
	}

	//Emitter size
	ImGui::Text("Emitter size");

	ImGui::Text("X");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

	ImGui::DragFloat("##SEmitterX", &size.x, 0.05f, 0.0f, 100.0f);

	ImGui::SameLine();

	ImGui::Text("Y");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

	ImGui::DragFloat("##SEmitterY", &size.y, 0.05f, 0.0f, 100.0f);

	ImGui::SameLine();

	ImGui::Text("Z");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);

	ImGui::DragFloat("##SEmitterZ", &size.z, 0.05f, 0.0f, 100.0f);

	//External forces
	ImGui::Text("External forces ");
	bool forceChanged = false;
	//X
	ImGui::Text("X");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
	if (ImGui::DragFloat("##SExternalforcesX", &externalAcceleration.x, 0.05f, -50.0f, 50.0f))
		forceChanged = true;

	ImGui::SameLine();
	//Y
	ImGui::Text("Y");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
	if (ImGui::DragFloat("##SExternalforcesY", &externalAcceleration.y, 0.05f, -50.0f, 50.0f))
		forceChanged = true;
	//Z
	ImGui::SameLine();
	ImGui::Text("Z");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
	if (ImGui::DragFloat("##SExternalforcesZ", &externalAcceleration.z, 0.05f, -50.0f, 50.0f))
		forceChanged = true;

	if (forceChanged)
		particleSystem->setExternalAcceleration(externalAcceleration);


	//Emision rate
	ImGui::Text("Emision rate (ms)");
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.3f);
	ImGui::DragFloat("##SEmision rate", &emisionRate, 1.0f, 1.0f, 100000.0f);

	//Emision rate
	ImGui::Text("Particles to create");
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.3f);
	ImGui::DragInt("##SParticlespercreation", &particlesPerCreation, 1.0f, 1.0f, 500.0f);

	//Particles lifetime
	ImGui::Text("Particles lifetime (ms)");
	if (ImGui::DragInt("##SParticlesLifetime", &particlesLifeTime, 3.0f, 0.0f, 10000.0f))
	{
		UpdateAllGradients();
	}

	int maxParticles = particlesPerCreation/emisionRate * particlesLifeTime;
	ImGui::Text("Total particles alive: %d", maxParticles);

	ImGui::Separator();

	if (ImGui::TreeNode("Direction & velocity"))
	{
		//Particles velocity
		ImGui::Text("Particles velocity");
		//X
		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SVelocityX", &particlesVelocity.x, 0.05f, -100.0f, 100.0f);

		ImGui::SameLine();
		//Y
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SVelocityY", &particlesVelocity.y, 0.05f, -100.0f, 100.0f);
		//Z
		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SVelocityZ", &particlesVelocity.z, 0.05f, -100.0f, 100.0f);

		//Random velocity factor
		ImGui::Text("Velocity random factor");
		//X
		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SRandomVelocityX", &velocityRandomFactor.x, 0.05f, 0.0f, 100.0f);

		ImGui::SameLine();
		//Y
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SRandomVelocityY", &velocityRandomFactor.y, 0.05f, 0.0f, 100.0f);
		//Z
		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SRandomVelocityZ", &velocityRandomFactor.z, 0.05f, 0.0f, 100.0f);

		ImGui::TreePop();
	}

	ImGui::Separator();

	if (ImGui::TreeNode("Color over Lifetime"))
	{
		////Particles Color
		int delete_color = -1;
		for (int i = 0; i < colors.size(); ++i)
		{
			std::string label = "##PEParticle Color";
			label.append(std::to_string(i));
			if (ImGui::ColorEdit4(label.data(), (float*)&colors[i], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
			{
				UpdateAllGradients();
			}
			if (colors.size() > 1) {
				colorGradient = true;
				ImGui::SameLine();
				ImGui::PushID(std::to_string(i).c_str());
				if (ImGui::Button("x")) {
					delete_color = i;
				}
				ImGui::PopID();
			}
			else {
				colorGradient = false;
			}
		}

		if (delete_color != -1) {
			int i = 0;
			std::vector<float4>::iterator it = colors.begin();
			while (it != colors.end()) {
				if (i == delete_color) {
					it = colors.erase(it);
					std::vector<float4>::iterator g_it = gradients.begin();
					if (i != 0)
						std::advance(g_it, i - 1);
					gradients.erase(g_it);
					UpdateAllGradients();
				}
				else {
					++it;
				}
				i++;
			}
		}

		if (ImGui::Button("Add color"))
		{
			uint index = colors.size() - 1;
			colors.push_back(colors[index]); //Start the new color with tha same the last one had
			colorDuration = particlesLifeTime / (gradients.size() + 1);

			//Update the gradients
			float4 newGradient = (colors[index + 1] - colors[index]) / colorDuration;
			gradients.push_back(newGradient);
			UpdateAllGradients();
		}

		ImGui::TreePop();
	}

	ImGui::Separator();

	if (ImGui::TreeNode("Rotation over Lifetime"))
	{
		ImGui::Text("Separate Axis");
		ImGui::SameLine();
		ImGui::Checkbox("##separateaxis", &separateAxis);
		ImGui::Text("Ang. Vel:");
		ImGui::SameLine();
		if (!separateAxis) {
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragInt("##Z2", &rotationOvertime1[2]);
			if (constants) {
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::DragInt("##Z2", &rotationOvertime2[2]);
			}
			ImGui::SameLine();
			if (ImGui::SmallButton("v"))
				ImGui::OpenPopup("Component options");
		}
		else {
			int cursor = ImGui::GetCursorPosX();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragInt("##X0", &rotationOvertime1[0], 1, -1000, 1000);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragInt("##Y0", &rotationOvertime1[1], 1, -1000, 1000);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragInt("##Z0", &rotationOvertime1[2], 1, -1000, 1000);
			ImGui::SameLine();
			if (ImGui::SmallButton("v"))
				ImGui::OpenPopup("Component options");
			if (constants) {
				ImGui::SetCursorPosX(cursor);
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::DragInt("##X1", &rotationOvertime2[0], 1, -1000, 1000);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::DragInt("##Y1", &rotationOvertime2[1], 1, -1000, 1000);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::DragInt("##Z1", &rotationOvertime2[2], 1, -1000, 1000);
			}
		}

		if (ImGui::BeginPopup("Component options"))
		{
			if (ImGui::MenuItem("Constant", "", !constants))
			{
				constants = false;
			}
			if (ImGui::MenuItem("Random Between two Constants", "", constants))
			{
				constants = true;
			}
			ImGui::EndPopup();
		}

		ImGui::TreePop();
	}

	ImGui::Separator();

	if (ImGui::TreeNode("Animation"))
	{
		int tmpX = tileSize_X;
		int tmpY = tileSize_Y;
		ImGui::Checkbox("Animation", &animation);
		ImGui::Text("Tiles:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragInt("X", &tileSize_X, 1, 1, texture->Texture_width);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragInt("Y", &tileSize_Y, 1, 1, texture->Texture_height);
		ImGui::Text("Start Frame:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragInt("##sframe", &startFrame, 1, 0, (tileSize_X * tileSize_Y) - 1);
		ImGui::Text("Cycles:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragInt("##cycle", &cycles, 1, 1, 100);

		if (tmpX != tileSize_X || tmpY != tileSize_Y && animation) {
			createdAnim = false;
		}

		ImGui::TreePop();
	}

	ImGui::Separator();

	if (ImGui::TreeNode("Renderer"))
	{
		//Scale
		ImGui::Text("Scale");

		//X
		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SParticlesScaleX", &particlesScale.x, 0.05f, 0.1f, 50.0f);

		ImGui::SameLine();
		//Y
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SParticlesScaleY", &particlesScale.y, 0.05f, 0.1f, 50.0f);

		//Scale random factor
		ImGui::Text("Scale random factor");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SParticlesRandomScaleX", &particlesScaleRandomFactor, 0.05f, 1.0f, 50.0f);

		//Scale over time
		ImGui::Text("Scale over time");
		ImGui::DragFloat("##SParticlesScaleOverTime", &scaleOverTime, 0.005f, -50.0f, 50.0f);

		// Image
		ImGui::Text("Image");
		ImGui::SameLine();

		if (texture == nullptr)
			ImGui::Image((ImTextureID)App->textures->GetDefaultTextureID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0)); //default texture
		else
			ImGui::Image((ImTextureID)texture->GetTexID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0)); //loaded texture

		//drag and drop
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource"))
			{
				uint UID = *(const uint*)payload->Data;
				Resource* resource = App->resources->GetResource(UID, false);

				if (resource && resource->GetType() == Resource::ResourceType::TEXTURE)
				{
					if (texture)
						texture->Release();

					texture = (ResourceTexture*)App->resources->GetResource(UID);
				}
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::ColorEdit4("##PEParticle Color", (float*)&colors[0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("Start Color");

		ImGui::TreePop();
	}

	ImGui::Separator();

	int new_count = pointsCurve.size() / 2;
	float* data = &pointsCurve[0];
	
	ImGui::CurveEditor("##Curve Editor", data, pointsCurve.size() / 2, multiplier, ImVec2((int)ImGui::GetWindowWidth() - 50,150), SHOW_GRID | NO_TANGENTS, &new_count);
	if (new_count * 2 != pointsCurve.size()) {
		std::vector<float> auxCurve = { data, data + new_count * 2 };
		pointsCurve.resize(auxCurve.size());
		std::copy(auxCurve.begin(), auxCurve.end(), pointsCurve.begin());
	}
}

double ComponentParticleEmitter::GetRandomValue(double min, double max) //EREASE IN THE FUTURE
{
	return App->RandomNumberGenerator.GetDoubleRNinRange(min, max);
}

void ComponentParticleEmitter::CreateParticles(uint particlesAmount)
{
	uint particlesToCreate = particlesAmount;

	if (validParticles < maxParticles)
	{
		Quat rotation = GO->GetComponent<ComponentTransform>()->rotation * emitterRotation;

		if (particlesToCreate > maxParticles - validParticles)
			particlesToCreate = maxParticles - validParticles;

		validParticles += particlesToCreate;
		spawnClock = App->time->GetGameplayTimePassed() * 1000;

		physx::PxParticleCreationData creationData;

		//Create 1 particle each time
		creationData.numParticles = particlesToCreate;
		physx::PxU32* index = new physx::PxU32[particlesToCreate];

		const physx::PxStrideIterator<physx::PxU32> indexBuffer(index);

		indexPool->allocateIndices(particlesToCreate, indexBuffer);

		float3 globalPosition = GO->GetComponent<ComponentTransform>()->GetGlobalPosition();

		physx::PxVec3* positionBuffer = new physx::PxVec3[particlesToCreate];
		physx::PxVec3* velocityBuffer = new physx::PxVec3[particlesToCreate];

		for (int i = 0; i < particlesToCreate; ++i) {

			//Set velocity of the new particles
			physx::PxVec3 velocity((particlesVelocity.x + GetRandomValue(-velocityRandomFactor.x, velocityRandomFactor.x)),
				particlesVelocity.y + GetRandomValue(-velocityRandomFactor.y, velocityRandomFactor.y),
				particlesVelocity.z + GetRandomValue(-velocityRandomFactor.z, velocityRandomFactor.z));

			Quat velocityQuat = Quat(velocity.x, velocity.y, velocity.z, 0);

			velocityQuat = rotation * velocityQuat * rotation.Conjugated();

			velocityBuffer[i] = physx::PxVec3(velocityQuat.x, velocityQuat.y, velocityQuat.z);

			//Set position of the new particles
			physx::PxVec3 position(GetRandomValue(-size.x, size.x) + emitterPosition.x,
				+GetRandomValue(-size.y, size.y) + emitterPosition.y,
				+GetRandomValue(-size.z, size.z) + emitterPosition.z);


			Quat positionQuat = Quat(position.x, position.y, position.z, 0);
			positionQuat = rotation * positionQuat * rotation.Conjugated();
			positionBuffer[i] = physx::PxVec3(positionQuat.x + globalPosition.x, positionQuat.y + globalPosition.y, positionQuat.z + globalPosition.z);

			particles[index[i]]->lifeTime = particlesLifeTime;
			particles[index[i]]->spawnTime = spawnClock;
			particles[index[i]]->color = colors[0];
			particles[index[i]]->texture = texture;
			particles[index[i]]->gradientTimer = spawnClock;
			particles[index[i]]->currentGradient = 0;

			//Set scale
			float randomScaleValue = GetRandomValue(1, particlesScaleRandomFactor);
			particles[index[i]]->scale.x = particlesScale.x * randomScaleValue;
			particles[index[i]]->scale.y = particlesScale.y * randomScaleValue;

			//Set Rotation
			particles[index[i]]->rotation = float3::zero;
			float3 rot1 = float3::zero;
			float3 rot2 = float3::zero;
			if (separateAxis) {
				rot1 = float3((float)rotationOvertime1[0], (float)rotationOvertime1[1], (float)rotationOvertime1[2]);
				rot2 = float3((float)rotationOvertime2[0], (float)rotationOvertime2[1], (float)rotationOvertime2[2]);
			}
			else {
				rot1 = float3(0, 0, (float)rotationOvertime1[2]);
				rot2 = float3(0, 0, (float)rotationOvertime2[2]);
			}
			if (constants) {
				if(rot1.x <= rot2.x)
					particles[index[i]]->rotationSpeed.x = GetRandomValue(rot1.x, rot2.x);
				else
					particles[index[i]]->rotationSpeed.x = GetRandomValue(rot2.x, rot1.x);
				if (rot1.y <= rot2.y)
					particles[index[i]]->rotationSpeed.y = GetRandomValue(rot1.y, rot2.y);
				else
					particles[index[i]]->rotationSpeed.y = GetRandomValue(rot2.y, rot1.y);
				if (rot1.z <= rot2.z)
					particles[index[i]]->rotationSpeed.z = GetRandomValue(rot1.z, rot2.z);
				else
					particles[index[i]]->rotationSpeed.z = GetRandomValue(rot2.z, rot1.z);
			}
			else {
				particles[index[i]]->rotationSpeed = rot1;
			}

		}

		creationData.indexBuffer = indexBuffer;
		creationData.positionBuffer = physx::PxStrideIterator<const physx::PxVec3>(positionBuffer);
		creationData.velocityBuffer = physx::PxStrideIterator<const physx::PxVec3>(velocityBuffer);

		bool succes = particleSystem->createParticles(creationData);

		delete[] index;
		delete[] positionBuffer;
		delete[] velocityBuffer;
	}
}

void ComponentParticleEmitter::CreateAnimation(uint w, uint h) {
	int width = texture->Texture_width / w;
	int height = texture->Texture_height / h;

	for (int j = h - 1; j >= 0; --j) {
		for (int i = 0; i < w; ++i) {
			//Create new Frame
			ResourceMesh* plane = new ResourceMesh(App->GetRandom().Int(), "ParticleMesh");
			App->scene_manager->CreatePlane(1, 1, 1, plane);

			//Set Texture Coords
			plane->vertices[0].texCoord[0] = i * width / (float)texture->Texture_width;
			plane->vertices[0].texCoord[1] = j * height / (float)texture->Texture_height;
			plane->vertices[2].texCoord[0] = ((i * width) + width) / (float)texture->Texture_width;
			plane->vertices[2].texCoord[1] = j * height / (float)texture->Texture_height;
			plane->vertices[1].texCoord[0] = i * width / (float)texture->Texture_width;
			plane->vertices[1].texCoord[1] = ((j * height) + height) / (float)texture->Texture_height;
			plane->vertices[3].texCoord[0] = ((i * width) + width) / (float)texture->Texture_width;
			plane->vertices[3].texCoord[1] = ((j * height) + height) / (float)texture->Texture_height;

			//Update Buffer
			plane->LoadInMemory();
			particleMeshes.push_back(plane);
		}
	}
}

void ComponentParticleEmitter::UpdateAllGradients()
{
	if (gradients.size() > 0) {
		colorDuration = particlesLifeTime / gradients.size();
		for (int i = 0; i < colors.size(); ++i) {
			if (i == 0) //If we change the first color, only 1 gradient is affected
			{
				gradients[0] = (colors[1] - colors[0]) / colorDuration;
			}
			else if (i == colors.size() - 1) //If we changed the last color, only 1 gradient is affected
			{
				gradients[i - 1] = (colors[i] - colors[i - 1]) / colorDuration;
			}
			else //Else, 2 gradients are afected
			{
				gradients[i - 1] = (colors[i] - colors[i - 1]) / colorDuration;
				gradients[i] = (colors[i + 1] - colors[i]) / colorDuration;
			}
		}
	}
}

void ComponentParticleEmitter::Play()
{
	emisionActive = true;
	emisionStart = App->time->GetGameplayTimePassed() * 1000;
	spawnClock = emisionStart;
}

void ComponentParticleEmitter::Stop()
{
	emisionActive = false;
}

void ComponentParticleEmitter::SetLooping(bool active)
{
	loop = active;
}

void ComponentParticleEmitter::SetEmisionRate(float ms)
{
	emisionRate = ms;
}

void ComponentParticleEmitter::SetParticlesPerCreation(int particlesAmount)
{
	particlesPerCreation = particlesAmount;
}

void ComponentParticleEmitter::SetExternalAcceleration(float x, float y, float z)
{
	particleSystem->setExternalAcceleration(physx::PxVec3(x, y, z));
}

void ComponentParticleEmitter::SetParticlesVelocity(float x, float y, float z)
{
	particlesVelocity = physx::PxVec3(x, y, z);
}

void ComponentParticleEmitter::SetVelocityRF(float x, float y, float z)
{
	velocityRandomFactor = physx::PxVec3(x, y, z);
}

void ComponentParticleEmitter::SetDuration(int duration)
{
	duration = duration;
}

void ComponentParticleEmitter::SetLifeTime(int ms)
{
	particlesLifeTime = ms;

	colorDuration = particlesLifeTime / gradients.size();

	//Update gradients if we have to
	if (colors.size() > 1)
	{
		UpdateAllGradients();
	}
}

void ComponentParticleEmitter::SetParticlesScale(float x, float y)
{
	particlesScale.x = x;
	particlesScale.y = y;
}

void ComponentParticleEmitter::SetParticlesScaleRF(float randomFactor)
{
	particlesScaleRandomFactor = randomFactor;
}

void ComponentParticleEmitter::UpdateActorLayer(const int* layerMask)
{
	App->physics->UpdateParticleActorLayer(particleSystem, (LayerMask*)layerMask);
}

void ComponentParticleEmitter::SetOffsetPosition(float x, float y, float z)
{
	emitterPosition = float3(x, y, z);
}

void ComponentParticleEmitter::SetOffsetRotation(float x, float y, float z)
{
	float3 rotation(x, y, z);
	float3 difference = (rotation- eulerRotation) * DEGTORAD;
	Quat quatrot = Quat::FromEulerXYZ(difference.x, difference.y, difference.z);


	//emitterRotation = emitterRotation * quatrot;
	emitterRotation = Quat::FromEulerXYZ(rotation.x * DEGTORAD, rotation.y * DEGTORAD, rotation.z * DEGTORAD);
	eulerRotation = rotation;
}
