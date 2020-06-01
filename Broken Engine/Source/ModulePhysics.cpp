#include "ModulePhysics.h"
#include "Application.h"
#include "ModuleSceneManager.h"
#include "ComponentCollider.h"
#include "ComponentCharacterController.h"
#include "GameObject.h"
#include "PhysxSimulationEvents.h"

#include "Optick/include/optick.h"

#include "ModuleTimeManager.h"
#include "ModuleScripting.h"

#include "PhysX_3.4/Include/extensions/PxDefaultAllocator.h"
#include "PhysX_3.4/Include/extensions/PxDefaultErrorCallback.h"
#include "PhysX_3.4/Include/pvd/PxPvd.h"
#include "PhysX_3.4/Include/pvd/PxPvdSceneClient.h"
#include "PhysX_3.4/Include/pvd/PxPvdTransport.h"
#include "PhysX_3.4/Include/PxPhysicsAPI.h"
#include "PhysX_3.4/Include/characterkinematic/PxControllerManager.h"
#include "PhysX_3.4/Include/foundation/PxAllocatorCallback.h"
#include "PhysX_3.4/Include/PxQueryReport.h"
#include "PhysX_3.4/Include/PxQueryFiltering.h"
#include "PhysX_3.4/Include/extensions/PxRaycastCCD.h"
#include "PhysX_3.4/Include/cooking/PxCooking.h"

#ifndef _WIN64
#	ifndef _DEBUG
#		pragma comment(lib, "PhysX_3.4/libx86/Checked/PhysX3CHECKED_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Checked/PhysX3CommonCHECKED_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Checked/PhysX3ExtensionsCHECKED.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Checked/PxFoundationCHECKED_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Checked/PxPvdSDKCHECKED_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Checked/PhysX3CharacterKinematicCHECKED_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Checked/SceneQueryCHECKED.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Checked/PhysX3CookingCHECKED_x86.lib")
#	else
#		pragma comment(lib, "PhysX_3.4/libx86/Debug/PhysX3CommonDEBUG_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Debug/PhysX3DEBUG_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Debug/PhysX3ExtensionsDEBUG.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Debug/PxFoundationDEBUG_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Debug/PxPvdSDKDEBUG_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Debug/PhysX3CharacterKinematicDEBUG_x86.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Debug/SceneQueryDEBUG.lib")
#		pragma comment(lib, "PhysX_3.4/libx86/Debug/PhysX3CookingDEBUG_x86.lib")
#	endif // _DEBUG
#else
#	ifndef _DEBUG
#		pragma comment(lib, "PhysX_3.4/libx64/Checked/PhysX3CHECKED_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Checked/PhysX3CommonCHECKED_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Checked/PhysX3ExtensionsCHECKED.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Checked/PxFoundationCHECKED_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Checked/PxPvdSDKCHECKED_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Checked/PhysX3CharacterKinematicCHECKED_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Checked/SceneQueryCHECKED.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Checked/PhysX3CookingCHECKED_x64.lib")
#	else
#		pragma comment(lib, "PhysX_3.4/libx64/Debug/PhysX3CommonDEBUG_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Debug/PhysX3DEBUG_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Debug/PhysX3ExtensionsDEBUG.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Debug/PxFoundationDEBUG_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Debug/PxPvdSDKDEBUG_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Debug/PhysX3CharacterKinematicDEBUG_x64.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Debug/SceneQueryDEBUG.lib")
#		pragma comment(lib, "PhysX_3.4/libx64/Debug/PhysX3CookingDEBUG_x64.lib")
#	endif // _DEBUG
#endif

//#include "mmgr/mmgr.h"

using namespace Broken;

ModulePhysics::ModulePhysics(bool start_enabled)
{
	name = "Physics";
}

ModulePhysics::~ModulePhysics()
{
}

void ModulePhysics::setupFiltering(physx::PxRigidActor* actor, physx::PxU32 LayerMask, physx::PxU32 filterMask)
{
	if (actor) {
		physx::PxFilterData filterData;
		filterData.word0 = LayerMask; // word0 = own ID
		filterData.word1 = filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;

		const physx::PxU32 numShapes = actor->getNbShapes();
		physx::PxShape** shapes = (physx::PxShape**)malloc(sizeof(physx::PxShape*) * numShapes);
		actor->getShapes(shapes, numShapes);
		for (physx::PxU32 i = 0; i < numShapes; i++)
		{
			physx::PxShape* shape = shapes[i];

			shape->setSimulationFilterData(filterData);
			shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
			shape->setQueryFilterData(filterData);
		}
	}
}

physx::PxFilterFlags customFilterShader(
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	// Let triggers through
	if ((physx::PxFilterObjectType::ePARTICLE_SYSTEM == attributes0 && physx::PxFilterObjectIsTrigger(attributes1)) || (physx::PxFilterObjectType::ePARTICLE_SYSTEM == attributes1 && physx::PxFilterObjectIsTrigger(attributes0))) {
		return physx::PxFilterFlag::eSUPPRESS;
	}

	if ((physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		&& (filterData0.word0 & filterData1.word1 || filterData1.word0 & filterData0.word1)) {
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
	}
	else {

		if ((filterData0.word0 != 0 || filterData1.word0 != 0) &&
			!(filterData0.word0 & filterData1.word1 || filterData1.word0 & filterData0.word1))
			return physx::PxFilterFlag::eSUPPRESS;
		else
		{
			// Generate contacts for all that were not filtered above
			pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
			pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_LOST;
		}
	}

	return physx::PxFilterFlag::eDEFAULT;
}

bool ModulePhysics::Init(json& config)
{
	LoadStatus(config);

	if (!loaded) {
		layer_list.push_back(Layer{ "Default", LayerMask::LAYER_0, true });
		layer_list.push_back(Layer{ "Player", LayerMask::LAYER_1, true });
		layer_list.push_back(Layer{ "Enemy", LayerMask::LAYER_2, true });
		layer_list.push_back(Layer{ "UI", LayerMask::LAYER_3, true });
		layer_list.push_back(Layer{ "Ignore Raycast", LayerMask::LAYER_4, true });

		int count = layer_list.size();
		for (int i = 0; i < 10 - count; ++i) {
			layer_list.push_back(Layer{ "", (LayerMask)(count + i), false });
		}

		for (int i = 0; i < layer_list.size(); ++i) {
			layer_list.at(i).active_layers.resize(layer_list.size(), true);
			if (i == 0)
				layer_list.at(i).UpdateLayerGroup();
			else
				layer_list.at(i).LayerGroup = layer_list.at(0).LayerGroup;
		}
	}
	static physx::PxDefaultErrorCallback gDefaultErrorCallback;
	static physx::PxDefaultAllocator gDefaultAllocatorCallback;

	mFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
	if (!mFoundation)
		ENGINE_CONSOLE_LOG("PxCreateFoundation failed!");

	mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, physx::PxCookingParams(physx::PxTolerancesScale()));
	if (!mCooking)
		ENGINE_CONSOLE_LOG("PxCreateCooking failed!");
	else {
		physx::PxCookingParams params = mCooking->getParams();
		params.convexMeshCookingType = physx::PxConvexMeshCookingType::eQUICKHULL;
		params.gaussMapLimit = 32;
		mCooking->setParams(params);
	}

	bool recordMemoryAllocations = true;

	//Setup Connection-----------------------------------------------------------------------
	physx::PxPvdTransport* mTransport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 10000);
	if (mTransport == NULL)
		return false;

	physx::PxPvdInstrumentationFlags mPvdFlags = physx::PxPvdInstrumentationFlag::eALL;
	mPvd = physx::PxCreatePvd(*mFoundation);
	mPvd->connect(*mTransport, mPvdFlags);
	//---------------------------------------------------------------------------------------

	mPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *mFoundation,
		physx::PxTolerancesScale(), recordMemoryAllocations, mPvd);
	if (!mPhysics) {
		ENGINE_CONSOLE_LOG("PxCreateBasePhysics failed!");
		return false;
	}

	PxRegisterParticles(*mPhysics);

	simulationEventsCallback = new PhysxSimulationEvents(this);

	physx::PxSceneDesc sceneDesc(mPhysics->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -gravity, 0.0f);
	sceneDesc.bounceThresholdVelocity = gravity * 0.2;
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(4);
	//sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
	sceneDesc.flags |= physx::PxSceneFlag::eENABLE_KINEMATIC_PAIRS | physx::PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS;
	sceneDesc.filterShader = customFilterShader;
	sceneDesc.simulationEventCallback = simulationEventsCallback;
	mScene = mPhysics->createScene(sceneDesc);

	// This will enable basic visualization of PhysX objects like - actors collision shapes and their axes.
		// The function PxScene::getRenderBuffer() is used to render any active visualization for scene.
	mScene->setVisualizationParameter(physx::PxVisualizationParameter::eSCALE, 1.0);	//Global visualization scale which gets multiplied with the individual scales
	mScene->setVisualizationParameter(physx::PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);	//Enable visualization of actor's shape
	mScene->setVisualizationParameter(physx::PxVisualizationParameter::eACTOR_AXES, 1.0f);	//Enable visualization of actor's axis

	mMaterial = mPhysics->createMaterial(materialDesc.x, materialDesc.y, materialDesc.z);

	mControllerManager = PxCreateControllerManager(*mScene);

	//Setup Configuration-----------------------------------------------------------------------
	pvdClient = mScene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	//-------------------------------------

	cache = mScene->createVolumeCache(32, 8);
	//PlaneCollider(0, 0, 0);

	return true;
}

update_status ModulePhysics::Update(float dt)
{
	OPTICK_CATEGORY("Physics Update", Optick::Category::Physics);
	//if (App->GetAppState() == AppState::PLAY)
	//	SimulatePhysics(dt);

	if (App->GetAppState() == AppState::PLAY && !App->time->gamePaused)
	{
		// --- Step physics simulation ---
		physAccumulatedTime += App->time->GetGameDt();

		// --- If enough time has elapsed, update ---
		if (physAccumulatedTime >= physx::fixed_dt)
		{
			physAccumulatedTime -= physx::fixed_dt;

			//FixedUpdate();

			mScene->simulate(physx::fixed_dt);
			mScene->fetchResults(true);
		}
	}

	return update_status::UPDATE_CONTINUE;
}

void ModulePhysics::FixedUpdate()
{
	//App->scripting->GameUpdate(physx::fixed_dt);
}

bool ModulePhysics::CleanUp()
{
	cache->release();	//195
	mControllerManager->release();//182
	mMaterial->release();
	mCooking->release();
	mScene->release(); //172
	mPhysics->release(); //153
	mPvd->release(); //149
	mFoundation->release(); //136
	RELEASE(simulationEventsCallback);
	RELEASE(raycastManager);
	RELEASE(detected_objects);
	cooked_meshes.clear();
	actors.clear();

	mControllerManager = nullptr;
	mPhysics = nullptr;
	mFoundation = nullptr;
	mScene = nullptr;
	mPvd = nullptr;

	return true;
}

// Creating static plane
void ModulePhysics::PlaneCollider(float posX, float posY, float posZ)
{
	physx::PxTransform position = physx::PxTransform(physx::PxVec3(posX, posY, posZ), physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0.0f, 0, 0.0f)));
	plane = mPhysics->createRigidStatic(position);
	plane = PxCreatePlane(*mPhysics, physx::PxPlane(physx::PxVec3(0, 1, 0), 0), *mMaterial);
	mScene->addActor(*plane);
}

void ModulePhysics::BoxCollider(float posX, float posY, float posZ)
{
	physx::PxRigidDynamic* box;
	physx::PxTransform position(physx::PxVec3(posX, posY, posZ));
	physx::PxBoxGeometry geometry(physx::PxVec3(0.5f, 0.5f, 0.5f));

	box = PxCreateDynamic(*mPhysics, position, geometry, *mMaterial, 1.0f);
	mScene->addActor(*box);

	ENGINE_CONSOLE_LOG("box created");
}

void ModulePhysics::SimulatePhysics(float dt, float speed)
{
	mScene->simulate(dt * speed);
	mScene->fetchResults(true);
}


void ModulePhysics::addActor(physx::PxRigidActor* actor, GameObject* gameObject) {
	actors[actor] = gameObject; //.insert(std::pair<physx::PxRigidActor*, GameObject*>(actor, gameObject));
	mScene->addActor(*actor);
}

void ModulePhysics::AddParticleActor(physx::PxActor* actor, GameObject* gameObject)
{
	particleActors.insert(std::pair<physx::PxActor*, GameObject*>(actor, gameObject));
	mScene->addActor(*actor);
}

void ModulePhysics::UpdateActorLayer(const physx::PxRigidActor* actor, const LayerMask* Layermask)
{
	if (actor)
	{
		physx::PxRigidActor* tmp = (physx::PxRigidActor*)actor;

		physx::PxShape* shape;
		actor->getShapes(&shape, 1);

		physx::PxFilterData* filterData;
		filterData = &shape->getSimulationFilterData();
		filterData->word0 = (1 << *Layermask);

		shape->setSimulationFilterData(*filterData);

		shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		shape->setQueryFilterData(*filterData);
	}
}

void ModulePhysics::UpdateParticleActorLayer(physx::PxActor* actor, const LayerMask* LayerMask)
{
	if (actor)
	{
		physx::PxParticleSystem* pSystem = (physx::PxParticleSystem*)actor;

		mScene->removeActor(*pSystem);

		physx::PxFilterData* filterData;
		filterData = &pSystem->getSimulationFilterData();
		filterData->word0 = (1 << *LayerMask);

		pSystem->setSimulationFilterData(*filterData);

		mScene->addActor(*pSystem);
	}
}

void ModulePhysics::UpdateActorsGroupFilter(LayerMask* updateLayer)
{
	if (actors.size() == 0)
		return;

	for (std::map<physx::PxRigidActor*, GameObject*>::iterator it = actors.begin(); it != actors.end(); ++it)
	{
		if ((*it).first != nullptr && (*it).second != nullptr)
		{
			LayerMask layer1 = (LayerMask)(*it).second->layer;
			LayerMask layer2 = *updateLayer;
			if (layer1 == layer2)
			{
				physx::PxShape* shape;
				(*it).first->getShapes(&shape, 1);

				physx::PxFilterData* filterData;
				filterData = &shape->getSimulationFilterData();
				filterData->word1 = layer_list.at(layer2).LayerGroup;

				shape->setSimulationFilterData(*filterData);
				shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
				shape->setQueryFilterData(*filterData);
				break;
			}
		}
	}

	if (particleActors.size() == 0)
		return;

	for (std::map<physx::PxActor*, GameObject*>::iterator it = particleActors.begin(); it != particleActors.end(); ++it)
	{
		if ((*it).first != nullptr && (*it).second != nullptr)
		{
			LayerMask layer1 = (LayerMask)(*it).second->layer;
			LayerMask layer2 = *updateLayer;
			if (layer1 == layer2)
			{
				physx::PxParticleSystem* pSystem = (physx::PxParticleSystem*)(*it).first;
				mScene->removeActor(*pSystem);

				physx::PxFilterData* filterData;
				filterData = &pSystem->getSimulationFilterData();
				filterData->word1 = layer_list.at(layer2).LayerGroup;

				pSystem->setSimulationFilterData(*filterData);
				mScene->addActor(*pSystem);

				break;
			}
		}
	}
}

bool ModulePhysics::DeleteActor(physx::PxRigidActor* actor, bool dynamic)
{
	if (actors.size() > 0 && actor)
	{
		if (mScene) {
			actors.erase(actor);
			mScene->removeActor(*actor);
		}
		if (dynamic) {
			actors.erase(actor);
			actor->release();
		}

		return true;
	}

	return false;
}

bool ModulePhysics::DeleteActor(physx::PxActor* actor)
{
	if (particleActors.size() > 0 && actor)
	{
		if (mScene) {
			particleActors.erase(actor);
			mScene->removeActor(*actor);
		}

		return true;
	}

	return false;
}

void ModulePhysics::DeleteActors(GameObject* go)
{
	bool isRoot = false;
	if (go == nullptr) {
		go = App->scene_manager->GetRootGO();
		isRoot = true;
	}

	if (go->childs.size() > 0)
	{
		for (std::vector<GameObject*>::iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		{
			GameObject* GO = *it;
			DeleteActors((GO));
		}
	}

	if (go->GetComponent<ComponentCollider>() != nullptr) {
		ComponentCollider* col = go->GetComponent<ComponentCollider>();
		col->Delete();
	}

	if (go->GetComponent<ComponentCharacterController>() != nullptr) {
		go->GetComponent<ComponentCharacterController>()->Delete();
	}

}

void ModulePhysics::RemoveCookedActors() {
	/*for (std::map<ResourceMesh*, physx::PxBase*>::iterator it = cooked_meshes.begin(); it != cooked_meshes.end(); ++it)
	{
		int i = 0;
		if ((*it).second->getConcreteType() == physx::PxConcreteType::eCONVEX_MESH) {
			physx::PxConvexMesh* mesh = (physx::PxConvexMesh*)(*it).second;
			if (mesh) {
				i = mesh->getReferenceCount();
				while (i > 1) {
					(*it).second->release();
					i = mesh->getReferenceCount();
				}
			}
		}
		else if ((*it).second->getConcreteType() == physx::PxConcreteType::eTRIANGLE_MESH_BVH33 || (*it).second->getConcreteType() == physx::PxConcreteType::eTRIANGLE_MESH_BVH34) {
			physx::PxTriangleMesh* mesh = (physx::PxTriangleMesh*)(*it).second;
			if (mesh) {
				i = mesh->getReferenceCount();
				while (i > 1) {
					(*it).second->release();
					i = mesh->getReferenceCount();
				}
			}
		}
	}*/
	mCooking->release();
	mCooking = PxCreateCooking(PX_PHYSICS_VERSION, *mFoundation, physx::PxCookingParams(physx::PxTolerancesScale()));
	cooked_meshes.clear();
	cooked_convex.clear();
}

void ModulePhysics::OverlapSphere(float3 position, float radius, LayerMask layer, std::vector<uint>& objects)
{
	detected_objects = &objects;

	physx::PxOverlapHit hit[100];
	physx::PxOverlapBuffer hit_buffer(hit, 100);       // [out] Overlap results
	const physx::PxSphereGeometry overlapShape(radius);			// [in] shape to test for overlaps
	const physx::PxTransform shapePose = physx::PxTransform(position.x, position.y, position.z);    // [in] initial shape pose (at distance=0)

	cache->fill(overlapShape, shapePose);

	physx::PxQueryFilterData filterData;
	filterData.data.word0 = App->physics->layer_list.at((int)layer).LayerGroup;
	filterData.flags |= physx::PxQueryFlag::eNO_BLOCK;

	bool status = cache->overlap(overlapShape, shapePose, hit_buffer, filterData);

	if (cache->getNbCachedShapes() > 0) {//DETECT CCT
		iter.layer = layer;
		cache->forEach((physx::PxVolumeCache::Iterator&)iter);
	}

	detected_objects = nullptr;
}

void UserIterator::processShapes(physx::PxU32 count, const physx::PxActorShape* actorShapePairs)
{
	int i = 0;
	for (physx::PxU32 i = 0; i < count; i++) {
		physx::PxRigidActor* actor = (physx::PxRigidActor*)actorShapePairs[i].actor;

		GameObject* GO = App->physics->actors[actor];
		if (GO) {
			if (layer == GO->layer) {
				if (App->physics->detected_objects) {
					App->physics->detected_objects->push_back(GO->GetUID());
				}
			}
		}
	}
}

physx::PxQueryHitType::Enum FilterCallback::preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
{
	// PT: ignore triggers
	if (shape->getFlags() & physx::PxShapeFlag::eTRIGGER_SHAPE)
		return physx::PxQueryHitType::eNONE;

	return physx::PxQueryHitType::eBLOCK;
}

physx::PxQueryHitType::Enum FilterCallback::postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit)
{
	return physx::PxQueryHitType::Enum();
}

const Broken::json& ModulePhysics::SaveStatus() const {
	//MYTODO: Added exception for Build because Build should never be enabled at start
	//maybe we should call SaveStatus on every panel
	static Broken::json config;

	config["gravity"] = -mScene->getGravity().y;
	config["staticFriction"] = mMaterial->getStaticFriction();
	config["dynamicFriction"] = mMaterial->getDynamicFriction();
	config["restitution"] = mMaterial->getRestitution();

	config["count"] = layer_list.size();

	for (uint i = 0; i < layer_list.size(); ++i) {
		Layer layer = layer_list.at(i);
		config["Layer" + std::to_string(i)]["Name"] = layer.name;
		config["Layer" + std::to_string(i)]["Layer"] = layer.layer;
		config["Layer" + std::to_string(i)]["Group"] = layer.LayerGroup;
		config["Layer" + std::to_string(i)]["GroupSize"] = layer.active_layers.size();
		config["Layer" + std::to_string(i)]["Active"] = layer.active;
		for (int j = 0; j < layer.active_layers.size(); ++j) {
			bool active = layer.active_layers.at(j);
			config["Layer" + std::to_string(i)][std::to_string(j)] = active;
		}

	}
	return config;
};

void ModulePhysics::LoadStatus(const Broken::json& file) {

	gravity = file[name]["gravity"].is_null() ? gravity : (float)file[name]["gravity"];
	materialDesc.x = file[name]["staticFriction"].is_null() ? materialDesc.x : (float)file[name]["staticFriction"];
	materialDesc.y = file[name]["dynamicFriction"].is_null() ? materialDesc.y : (float)file[name]["dynamicFriction"];
	materialDesc.z = file[name]["restitution"].is_null() ? materialDesc.z : (float)file[name]["restitution"];;

	int count = file[name]["count"].is_null() ? 0 : (int)file[name]["count"];

	if (count != 0) {
		for (uint i = 0; i < count; ++i) {
			Layer layer;
			if (file[name].find("Layer" + std::to_string(i)) != file[name].end()) {
				layer.name = file[name]["Layer" + std::to_string(i)]["Name"].get<std::string>();
				layer.layer = (LayerMask)file[name]["Layer" + std::to_string(i)]["Layer"];
				layer.LayerGroup = (physx::PxU32) file[name]["Layer" + std::to_string(i)]["Group"];
				layer.active = (physx::PxU32) file[name]["Layer" + std::to_string(i)]["Active"];

				int size = file[name]["Layer" + std::to_string(i)]["GroupSize"];
				layer.active_layers.resize(size, true);
				for (int j = 0; j < size; ++j) {
					if (file[name]["Layer" + std::to_string(i)].find(std::to_string(j)) != file[name]["Layer" + std::to_string(i)].end()) {
						layer.active_layers.at(j) = file[name]["Layer" + std::to_string(i)][std::to_string(j)];
					}
				}
			}
			layer_list.push_back(layer);
		}
		loaded = true;
	}
}

bool ModulePhysics::Raycast(float3 origin_, float3 direction_, float maxDistance, LayerMask layer, bool hitTriggers)
{
	physx::PxVec3 origin(origin_.x, origin_.y, origin_.z);
	physx::PxVec3 direction(direction_.x, direction_.y, direction_.z);
	direction.normalize();

	physx::PxRaycastBuffer hit;
	physx::PxQueryFilterData filterData;

	filterData.data.word0 = App->physics->layer_list.at((int)layer).LayerGroup;

	bool status = mScene->raycast(origin, direction, maxDistance, hit, physx::PxHitFlag::eDEFAULT, filterData) && !(origin - hit.block.position == physx::PxVec3(0.f, 0.f, 0.f));

	if (status && !hitTriggers)
	{
		GameObject* go = actors[hit.block.actor];

		ComponentCollider* collider = go->GetComponent<ComponentCollider>();
		if (collider)
		{
			if (collider->isTrigger)
				status = false;
		}
	}

	return status;
}

GameObject* ModulePhysics::RaycastGO(float3 origin_, float3 direction_, float maxDistance, LayerMask layer, bool hitTriggers)
{
	physx::PxVec3 origin(origin_.x, origin_.y, origin_.z);
	physx::PxVec3 direction(direction_.x, direction_.y, direction_.z);
	direction.normalize();

	physx::PxRaycastBuffer hit;
	physx::PxQueryFilterData filterData;

	filterData.data.word0 = App->physics->layer_list.at((int)layer).LayerGroup;

	bool status = mScene->raycast(origin, direction, maxDistance, hit, physx::PxHitFlag::eDEFAULT, filterData) && !(origin - hit.block.position == physx::PxVec3(0.f, 0.f, 0.f));

	if (status && !hitTriggers)
	{
		GameObject* go = actors[hit.block.actor];

		ComponentCollider* collider = go->GetComponent<ComponentCollider>();
		if (collider)
		{
			if (!collider->isTrigger)
				return go;
		}
	}

	else if (status)
		return actors[hit.block.actor];

	return nullptr;
}
