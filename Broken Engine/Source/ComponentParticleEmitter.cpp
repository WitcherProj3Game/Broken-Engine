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
#include "CurveEditor.h"

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

	if (scaleCurve == nullptr) {
		scaleCurve = new CurveEditor("##scale", LINEAR);
		scaleCurve->Init();
		curves.push_back(scaleCurve);
	}
	if (rotateCurve == nullptr) {
		rotateCurve = new CurveEditor("##rotation", LINEAR);
		rotateCurve->Init();
		curves.push_back(rotateCurve);
	}
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


	for (std::vector<ComponentParticleEmitter*>::iterator it = App->renderer3D->particleEmitters.begin(); it != App->renderer3D->particleEmitters.end(); it++){
		if ((*it) == this) {
			App->renderer3D->particleEmitters.erase(it);
			break;
		}
	}

	for (int i = 0; i < curves.size(); ++i) {
		delete curves[i];
	}
}

void ComponentParticleEmitter::Update()
{

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
	active = true;

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
		if (currentPlayTime - spawnClock > emisionRate)
		{
			uint newParticlesAmount = ((currentPlayTime - spawnClock) / emisionRate) * particlesPerCreation;

			if (!firstEmision)
				CreateParticles(newParticlesAmount);
			else
			{
				if (newParticlesAmount > particlesPerCreation)
					CreateParticles(particlesPerCreation);
				else
					CreateParticles(newParticlesAmount);

				firstEmision = false;
			}
		}

		if (emisionActive && !loop)
		{
			if ((currentPlayTime) - emisionStart > duration)
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
				if (currentPlayTime - particles[i]->spawnTime > particles[i]->lifeTime) {
					indicesToErease.push_back(i);
					particlesToRelease++;
					continue;
				}

				float diff_time = (App->time->GetGameplayTimePassed() * 1000 - particles[i]->spawnTime);

				if (scaleconstants == 2) {
					scaleOverTime = scaleCurve->GetCurrentValue(diff_time, particles[i]->lifeTime);
					particles[i]->scale.x = scaleOverTime;
					particles[i]->scale.y = scaleOverTime;
				}

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
					int time = currentPlayTime - particles[i]->spawnTime;
					int index = (particleMeshes.size() * time) / (particles[i]->lifeTime / cycles);
					particles[i]->plane = particleMeshes[(index + startFrame) % particleMeshes.size()];
				}
				else
					particles[i]->plane = App->scene_manager->plane;

				if (rotationconstants == 2) {
					particles[i]->rotationSpeed.z = rotateCurve->GetCurrentValue(diff_time, particles[i]->lifeTime);
				}

				if (rotationOvertime1 != 0)
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

	Plane cameraPlanes[6];
	App->renderer3D->culling_camera->frustum.GetPlanes(cameraPlanes);

	std::map<float, int>::iterator it = drawingIndices.begin();
	while (it != drawingIndices.end())
	{
		int paco = (*it).second;

		//Check if the particles are inside the frustum of the camera
		bool draw = true;
		for (int i = 0; i < 6; ++i)
		{
			//If the particles is on the positive side of one ore more planes, it's outside the frustum
			if (cameraPlanes[i].IsOnPositiveSide(particles[paco]->position))
			{
				draw = false;
				break;
			}
		}
		if (draw)
			particles[paco]->Draw();
		it++;
	}
	drawingIndices.clear();
}

void ComponentParticleEmitter::ChangeParticlesColor(float4 color)
{
	color /= 255.0f;

	colors[0] = color;
	UpdateAllGradients();

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

	node["velocityRandomFactor1X"] = std::to_string(velocityRandomFactor1.x);
	node["velocityRandomFactor1Y"] = std::to_string(velocityRandomFactor1.y);
	node["velocityRandomFactor1Z"] = std::to_string(velocityRandomFactor1.z);
	node["velocityRandomFactor2X"] = std::to_string(velocityRandomFactor2.x);
	node["velocityRandomFactor2Y"] = std::to_string(velocityRandomFactor2.y);
	node["velocityRandomFactor2Z"] = std::to_string(velocityRandomFactor2.z);
	node["velocityconstants"] = std::to_string(velocityconstants);

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

	node["particleScaleRandomFactor1"] = std::to_string(particlesScaleRandomFactor1);
	node["particleScaleRandomFactor2"] = std::to_string(particlesScaleRandomFactor2);

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
	node["rotationconstants"] = std::to_string(rotationconstants);

	node["num_curves"] = std::to_string(curves.size());
	for (int i = 0; i < curves.size(); ++i) {
		CurveEditor* curve = curves[i];
		node["curves"][i]["num_points"] = std::to_string(curve->pointsCurveTangents.size());
		node["curves"][i]["name"] = curve->name.c_str();
		node["curves"][i]["type"] = std::to_string(curve->type);
		for (int j = 0; j < curve->pointsCurveTangents.size(); ++j) {
			node["curves"][i][std::to_string(j).c_str()]["PrevX"] = std::to_string(curve->pointsCurveTangents[j].prev_tangent.x);
			node["curves"][i][std::to_string(j).c_str()]["PrevY"] = std::to_string(curve->pointsCurveTangents[j].prev_tangent.y);
			node["curves"][i][std::to_string(j).c_str()]["PX"] = std::to_string(curve->pointsCurveTangents[j].p.x);
			node["curves"][i][std::to_string(j).c_str()]["PY"] = std::to_string(curve->pointsCurveTangents[j].p.y);
			node["curves"][i][std::to_string(j).c_str()]["NextX"] = std::to_string(curve->pointsCurveTangents[j].next_tangent.x);
			node["curves"][i][std::to_string(j).c_str()]["NextY"] = std::to_string(curve->pointsCurveTangents[j].next_tangent.y);
		}
	}

	return node;
}

void ComponentParticleEmitter::Load(json& node)
{
	for (int i = 0; i < curves.size(); ++i) {
		delete curves[i];
	}
	curves.clear();
	scaleCurve = nullptr;
	rotateCurve = nullptr;


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

	std::string LvelocityRandomFactor1x = node["velocityRandomFactor1X"].is_null() ? "0" : node["velocityRandomFactor1X"];
	std::string LvelocityRandomFactor1y = node["velocityRandomFactor1Y"].is_null() ? "0" : node["velocityRandomFactor1Y"];
	std::string LvelocityRandomFactor1z = node["velocityRandomFactor1Z"].is_null() ? "0" : node["velocityRandomFactor1Z"];
	std::string LvelocityRandomFactor2x = node["velocityRandomFactor2X"].is_null() ? "0" : node["velocityRandomFactor2X"];
	std::string LvelocityRandomFactor2y = node["velocityRandomFactor2Y"].is_null() ? "0" : node["velocityRandomFactor2Y"];
	std::string LvelocityRandomFactor2z = node["velocityRandomFactor2Z"].is_null() ? "0" : node["velocityRandomFactor2Z"];
	std::string _velocityconstants = node["velocityconstants"].is_null() ? "0" : node["velocityconstants"];

	std::string LparticlesLifeTime = node["particlesLifeTime"].is_null() ? "0" : node["particlesLifeTime"];
	std::string LparticlesLifeTime1 = node["particlesLifeTime1"].is_null() ? "0" : node["particlesLifeTime1"];
	std::string LparticlesLifeTime2 = node["particlesLifeTime2"].is_null() ? "0" : node["particlesLifeTime2"];
	std::string _lifetimeconstants = node["lifetimeconstants"].is_null() ? "0" : node["lifetimeconstants"];

	std::string LParticlesSize = node["particlesSize"].is_null() ? "0" : node["particlesSize"];

	std::string _animation = node["animation"].is_null() ? "0" : node["animation"];
	std::string _tiles_X = node["tiles_X"].is_null() ? "1" : node["tiles_X"];
	std::string _tiles_Y = node["tiles_Y"].is_null() ? "1" : node["tiles_Y"];
	std::string _cycles = node["cycles"].is_null() ? "1" : node["cycles"];
	std::string _startFrame = node["startFrame"].is_null() ? "0" : node["startFrame"];

	std::string LDuration = node["Duration"].is_null() ? "0" : node["Duration"];

	std::string LParticlesScaleX = node["particlesScaleX"].is_null() ? "1" : node["particlesScaleX"];
	std::string LParticlesScaleY = node["particlesScaleY"].is_null() ? "1" : node["particlesScaleY"];

	std::string LParticleScaleRandomFactor1 = node["particleScaleRandomFactor1"].is_null() ? "1" : node["particleScaleRandomFactor1"];
	std::string LParticleScaleRandomFactor2 = node["particleScaleRandomFactor2"].is_null() ? "1" : node["particleScaleRandomFactor2"];

	std::string LScaleOverTime = node["particleScaleOverTime"].is_null() ? "0" : node["particleScaleOverTime"];

	std::string _num_colors = node["num_colors"].is_null() ? "0" : node["num_colors"];
	std::string _num_gradients = node["num_gradients"].is_null() ? "0" : node["num_gradients"];
	std::string _gradientDuration = node["grad_duration"].is_null() ? "0" : node["grad_duration"];
	std::string _num_curves = node["num_curves"].is_null() ? "0" : node["num_curves"];


	std::string _separateAxis = node["separateAxis"].is_null() ? "0" : node["separateAxis"];
	std::string rotationOvertime1_X = node["rotationOvertime1"][0].is_null() ? "0" : node["rotationOvertime1"][0];
	std::string rotationOvertime1_Y = node["rotationOvertime1"][1].is_null() ? "0" : node["rotationOvertime1"][1];
	std::string rotationOvertime1_Z = node["rotationOvertime1"][2].is_null() ? "0" : node["rotationOvertime1"][2];
	std::string rotationOvertime2_X = node["rotationOvertime2"][0].is_null() ? "0" : node["rotationOvertime2"][0];
	std::string rotationOvertime2_Y = node["rotationOvertime2"][1].is_null() ? "0" : node["rotationOvertime2"][1];
	std::string rotationOvertime2_Z = node["rotationOvertime2"][2].is_null() ? "0" : node["rotationOvertime2"][2];
	std::string _rotationconstants = node["rotationconstants"].is_null() ? "0" : node["rotationconstants"];
	std::string _scaleconstants = node["scaleconstants"].is_null() ? "0" : node["scaleconstants"];



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

	num = std::stof(_num_curves);
	for (int i = 0; i < num; ++i) {
		std::string _num_points = node["curves"][i]["num_points"].is_null() ? "0" : node["curves"][i]["num_points"];
		std::string name = node["curves"][i]["name"].is_null() ? "" : node["curves"][i]["name"];
		std::string _type = node["curves"][i]["type"].is_null() ? "0" : node["curves"][i]["type"];
		int points = std::stof(_num_points);
		CurveEditor* curve = new CurveEditor(name.c_str(), (CurveType)(int)std::stof(_type));
		for (int j = 0; j < points; ++j) {
			std::string prev_tangentX = node["curves"][i][std::to_string(j).c_str()]["PrevX"].is_null() ? "0" : node["curves"][i][std::to_string(j).c_str()]["PrevX"];
			std::string prev_tangentY = node["curves"][i][std::to_string(j).c_str()]["PrevY"].is_null() ? "0" : node["curves"][i][std::to_string(j).c_str()]["PrevY"];
			std::string pX = node["curves"][i][std::to_string(j).c_str()]["PX"].is_null() ? "0" : node["curves"][i][std::to_string(j).c_str()]["PX"];
			std::string pY = node["curves"][i][std::to_string(j).c_str()]["PY"].is_null() ? "0" : node["curves"][i][std::to_string(j).c_str()]["PY"];
			std::string post_tangentX = node["curves"][i][std::to_string(j).c_str()]["NextX"].is_null() ? "0" : node["curves"][i][std::to_string(j).c_str()]["NextX"];
			std::string post_tangentY = node["curves"][i][std::to_string(j).c_str()]["NextY"].is_null() ? "0" : node["curves"][i][std::to_string(j).c_str()]["NextY"];
			Point p;
			p.prev_tangent = float2(std::stof(prev_tangentX), std::stof(prev_tangentY));
			p.p = float2(std::stof(pX), std::stof(pY));
			p.next_tangent = float2(std::stof(post_tangentX), std::stof(post_tangentY));
			curve->pointsCurveTangents.push_back(p);
		}
		curves.push_back(curve);
	}
	if (num > 0) {
		scaleCurve = curves[0];
		rotateCurve = curves[1];
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

	velocityRandomFactor1.x = std::stof(LvelocityRandomFactor1x);
	velocityRandomFactor1.y = std::stof(LvelocityRandomFactor1y);
	velocityRandomFactor1.z = std::stof(LvelocityRandomFactor1z);
	velocityRandomFactor2.x = std::stof(LvelocityRandomFactor2x);
	velocityRandomFactor2.y = std::stof(LvelocityRandomFactor2y);
	velocityRandomFactor2.z = std::stof(LvelocityRandomFactor2z);
	velocityconstants = std::stof(_velocityconstants);

	particlesLifeTime = std::stof(LparticlesLifeTime);
	particlesLifeTime1 = std::stof(LparticlesLifeTime1);
	particlesLifeTime2 = std::stof(LparticlesLifeTime2);
	lifetimeconstants = std::stof(_lifetimeconstants);


	duration = std::stoi(LDuration);

	particlesScale.x = std::stof(LParticlesScaleX);
	particlesScale.y = std::stof(LParticlesScaleY);

	particlesScaleRandomFactor1 = std::stof(LParticleScaleRandomFactor1);
	particlesScaleRandomFactor2 = std::stof(LParticleScaleRandomFactor2);

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
	rotationconstants = std::stof(_rotationconstants);
	scaleconstants = std::stof(_scaleconstants);

	if (scaleCurve == nullptr) {
		scaleCurve = new CurveEditor("##scale", LINEAR);
		scaleCurve->Init();
		curves.push_back(scaleCurve);
	}
	if (rotateCurve == nullptr) {
		rotateCurve = new CurveEditor("##rotation", LINEAR);
		rotateCurve->Init();
		curves.push_back(rotateCurve);
	}
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
	if (lifetimeconstants == 0) {
		if (ImGui::DragInt("##SParticlesLifetime1", &particlesLifeTime, 3.0f, 0.0f, 10000.0f))
		{
			UpdateAllGradients();
		}
	}
	else {
		bool changed = false;
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if (ImGui::DragInt("##SParticlesLifetime1", &particlesLifeTime1, 3.0f, 0.0f, 10000.0f)) {
			changed = true;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.3f);
		if(ImGui::DragInt("##SParticlesLifetime2", &particlesLifeTime2, 3.0f, 0.0, 10000.0f)) {
			changed = true;
		}
		if (changed) {
			particlesLifeTime = (particlesLifeTime1 < particlesLifeTime2) ? GetRandomValue(particlesLifeTime1, particlesLifeTime2): GetRandomValue(particlesLifeTime2, particlesLifeTime1);
			UpdateAllGradients();
		}
	}

	ImGui::SameLine();
	if (ImGui::SmallButton("v"))
		ImGui::OpenPopup("Lifetime options");

	if (ImGui::BeginPopup("Lifetime options"))
	{
		if (ImGui::MenuItem("Constant", "", lifetimeconstants == 0 ? true : false))
		{
			lifetimeconstants = 0;
		}
		if (ImGui::MenuItem("Random Between two Constants", "", lifetimeconstants == 1 ? true : false))
		{
			lifetimeconstants = 1;

			particlesLifeTime2 = particlesLifeTime;
			particlesLifeTime1 = particlesLifeTime;
		}
		ImGui::EndPopup();
	}

	int maxParticles = particlesPerCreation/emisionRate * particlesLifeTime;
	ImGui::Text("Total particles alive: %d", maxParticles);

	ImGui::Separator();

	if (ImGui::TreeNode("Direction & velocity"))
	{
		int cursor = 0;
		ImGui::Text("Particles velocity");
		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SVelocityX", &particlesVelocity.x, 0.05f, -100.0f, 100.0f);
		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SVelocityY", &particlesVelocity.y, 0.05f, -100.0f, 100.0f);
		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SVelocityZ", &particlesVelocity.z, 0.05f, -100.0f, 100.0f);
		ImGui::SameLine();
		if (ImGui::SmallButton("v"))
			ImGui::OpenPopup("Velocity options");
		ImGui::Text("Velocity Random Factor");
		cursor = ImGui::GetCursorPosX();
		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SRandomVelocity1X", &velocityRandomFactor1.x, 0.05f, -100.0f, 100.0f);
		ImGui::SameLine();
		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SRandomVelocity1Y", &velocityRandomFactor1.y, 0.05f, -100.0f, 100.0f);
		ImGui::SameLine();
		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SRandomVelocity1Z", &velocityRandomFactor1.z, 0.05f, -100.0f, 100.0f);

		ImGui::SetCursorPosX(cursor);
		ImGui::Text(" ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SRandomVelocity2X", &velocityRandomFactor2.x, 0.05f, -100.0f, 100.0f);
		ImGui::SameLine();
		ImGui::Text(" ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SRandomVelocity2Y", &velocityRandomFactor2.y, 0.05f, -100.0f, 100.0f);
		ImGui::SameLine();
		ImGui::Text(" ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##SRandomVelocity2Z", &velocityRandomFactor2.z, 0.05f, -100.0f, 100.0f);


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
		if (rotationconstants == 2) {
			rotateCurve->DrawCurveEditor(); //Draw Curve Editor
			ImGui::SameLine();
			if (ImGui::SmallButton("v"))
				ImGui::OpenPopup("Component options");
		}
		else {
			ImGui::Text("Separate Axis");
			ImGui::SameLine();
			ImGui::Checkbox("##separateaxis", &separateAxis);
			ImGui::Text("Ang. Vel:");
			ImGui::SameLine();
			int cursor = ImGui::GetCursorPosX();
			if (!separateAxis) {
				ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
				ImGui::DragInt("##Z1", &rotationOvertime1[2]);
				if (rotationconstants) {
					ImGui::SameLine();
					ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
					ImGui::DragInt("##Z2", &rotationOvertime2[2]);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("v"))
					ImGui::OpenPopup("Component options");
			}
			else {
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
				if (rotationconstants) {
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
		}


		if (ImGui::BeginPopup("Component options"))
		{
			if (ImGui::MenuItem("Constant", "", rotationconstants == 0 ? true : false))
			{
				rotationconstants = 0;
			}
			if (ImGui::MenuItem("Random Between two Constants", "", rotationconstants == 1 ? true : false))
			{
				rotationconstants = 1;
			}
			if (ImGui::MenuItem("Curve Editor", "", rotationconstants == 2 ? true : false))
			{
				rotationconstants = 2;
			}
			ImGui::EndPopup();
		}

		ImGui::TreePop();
	}

	ImGui::Separator();

	if (ImGui::TreeNode("Scale over Lifetime"))
	{
		if (scaleconstants == 0) {
			ImGui::Text("Scale");
			ImGui::SameLine();
			ImGui::Text("X");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragFloat("##SParticlesScaleX", &particlesScale.x, 0.05f, 0.1f, 50.0f);
			ImGui::SameLine();
			ImGui::Text("Y");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragFloat("##SParticlesScaleY", &particlesScale.y, 0.05f, 0.1f, 50.0f);
		}
		else if (scaleconstants == 1) {
			ImGui::Text("Random Between:");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragFloat("##SParticlesRandomScaleX", &particlesScaleRandomFactor1, 0.05f, 0.0f, 50.0f);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::DragFloat("##SParticlesRandomScaleY", &particlesScaleRandomFactor2, 0.05f, 0.0f, 50.0f);
		}
		else if (scaleconstants == 2) {
			scaleCurve->DrawCurveEditor();
		}

		ImGui::SameLine();
		if (ImGui::SmallButton("v"))
			ImGui::OpenPopup("Component options");

		if (ImGui::BeginPopup("Component options"))
		{
			if (ImGui::MenuItem("Constant", "", scaleconstants == 0 ? true : false))
			{
				scaleconstants = 0;
			}
			if (ImGui::MenuItem("Random Between two Constants", "", scaleconstants == 1 ? true : false))
			{
				scaleconstants = 1;
			}
			if (ImGui::MenuItem("Curve Editor", "", scaleconstants == 2 ? true : false))
			{
				scaleconstants = 2;
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
		ImGui::DragFloat("##cycle", &cycles, 0.01, 0.01, 100);
		if (cycles <= 0) {
			cycles = 1;
		}

		if (tmpX != tileSize_X || tmpY != tileSize_Y && animation) {
			createdAnim = false;
		}

		ImGui::TreePop();
	}

	ImGui::Separator();

	if (ImGui::TreeNode("Renderer"))
	{
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
			physx::PxVec3 velocity = physx::PxVec3(
				particlesVelocity.x + ((velocityRandomFactor1.x < velocityRandomFactor2.x) ? GetRandomValue(velocityRandomFactor1.x, velocityRandomFactor2.x) : GetRandomValue(velocityRandomFactor2.x, velocityRandomFactor1.x)),
				particlesVelocity.y + ((velocityRandomFactor1.y < velocityRandomFactor2.y) ? GetRandomValue(velocityRandomFactor1.y, velocityRandomFactor2.y) : GetRandomValue(velocityRandomFactor2.y, velocityRandomFactor1.y)),
				particlesVelocity.z + ((velocityRandomFactor1.z < velocityRandomFactor2.z) ? GetRandomValue(velocityRandomFactor1.z, velocityRandomFactor2.z) : GetRandomValue(velocityRandomFactor2.z, velocityRandomFactor1.z)));

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
			if (scaleconstants == 1) {
				float randomScaleValue = GetRandomValue(particlesScaleRandomFactor1, particlesScaleRandomFactor2);
				particles[index[i]]->scale.x = particlesScale.x * randomScaleValue;
				particles[index[i]]->scale.y = particlesScale.y * randomScaleValue;
			}
			else if (scaleconstants == 0) {
				particles[index[i]]->scale.x = particlesScale.x;
				particles[index[i]]->scale.y = particlesScale.y;
			}

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

			if (rotationconstants == 0) {
				particles[index[i]]->rotationSpeed = rot1;
			}
			if (rotationconstants == 1) {
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
	firstEmision = true;
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

void ComponentParticleEmitter::SetVelocityRF(float3 rand1, float3 rand2)
{
	velocityRandomFactor1 = physx::PxVec3(rand1.x, rand1.y, rand1.z);
	velocityRandomFactor2 = physx::PxVec3(rand2.x, rand2.y, rand2.z);
}

void ComponentParticleEmitter::SetDuration(int _duration)
{
	duration = _duration;
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

void ComponentParticleEmitter::SetParticlesScaleRF(float randomFactor1, float randomFactor2)
{
	particlesScaleRandomFactor1 = randomFactor1;
	particlesScaleRandomFactor2 = randomFactor2;
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

	emitterRotation = Quat::FromEulerXYZ(rotation.x * DEGTORAD, rotation.y * DEGTORAD, rotation.z * DEGTORAD);
	eulerRotation = rotation;
}

void ComponentParticleEmitter::SetScale(float x, float y)
{
	particlesScale = { x,y };
}

void ComponentParticleEmitter::SetScaleOverTime(float scale)
{
	scaleOverTime = scale;
}

void ComponentParticleEmitter::SetTexture(uint UID)
{
	Resource* resource = App->resources->GetResource(UID, false);

	if (resource && resource->GetType() == Resource::ResourceType::TEXTURE)
	{
		if (texture)
			texture->Release();

		texture = (ResourceTexture*)App->resources->GetResource(UID);
	}
	else
		ENGINE_CONSOLE_LOG("!(Particles - Set Texture): Couldn't find texture or was invalid to put in Particles");
}


void ComponentParticleEmitter::SetParticlesRotationOverTime(int rotationOverTime)
{
	separateAxis = false;
	rotationOvertime1[2] = rotationOverTime;
}

void ComponentParticleEmitter::SetParticlesRandomRotationOverTime(int randomRotation)
{
	rotationconstants = true;
	separateAxis = false;
	rotationOvertime2[2] = randomRotation;
}

void ComponentParticleEmitter::SetParticles3DRotationOverTime(int rotationOverTimeX, int rotationOverTimeY, int rotationOverTimeZ)
{
	separateAxis = true;
	rotationOvertime1[0] = rotationOverTimeX;
	rotationOvertime1[1] = rotationOverTimeY;
	rotationOvertime1[2] = rotationOverTimeZ;
}

void ComponentParticleEmitter::SetParticles3DRandomRotationOverTime(int rotationOverTimeX, int rotationOverTimeY, int rotationOverTimeZ)
{
	rotationconstants = true;
	separateAxis = true;
	rotationOvertime2[0] = rotationOverTimeX;
	rotationOvertime2[1] = rotationOverTimeY;
	rotationOvertime2[2] = rotationOverTimeZ;
}

void ComponentParticleEmitter::RemoveParticlesRandomRotation()
{
	rotationconstants = false;
}
