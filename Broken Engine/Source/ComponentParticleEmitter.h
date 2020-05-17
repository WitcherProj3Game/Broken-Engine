#ifndef __COMPONENT_PARTICLE_EMITTER__
#define __COMPONENT_PARTICLE_EMITTER__

#include "Component.h"
#include "Math.h"
#include "ModuleRenderer3D.h"

#include "PhysX_3.4/Include/PxPhysicsAPI.h"

BE_BEGIN_NAMESPACE

class Particle;
class ResourceTexture;
class CurveEditor;


class BROKEN_API ComponentParticleEmitter : public Component
{
	friend class ModuleParticles;
public:

	ComponentParticleEmitter(GameObject* ContainerGO);
	~ComponentParticleEmitter();

	void Update() override;

	void Enable() override;
	void Disable() override;

	void UpdateParticles(float dt);
	void DrawParticles();
	void ChangeParticlesColor(float4 color);

	static inline Component::ComponentType GetType() { return Component::ComponentType::ParticleEmitter; };

	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;

	// -- Other functionalities
	void CreateInspectorNode() override;
	void DrawEmitterArea();

	//Scripting functions
	//Emitter
	void Play();
	void Stop();
	void SetEmisionRate(float ms);
	void SetParticlesPerCreation(int particlesAmount);
	void SetExternalAcceleration(float x, float y, float z);
	void SetParticlesVelocity(float x, float y, float z);
	void SetVelocityRF(float3 rand1, float3 rand2);
	void SetLooping(bool active);
	void SetOffsetPosition(float x, float y, float z);
	void SetOffsetRotation(float x, float y, float z);
	void SetDuration(int duration);

	//Particles
	void SetLifeTime(int ms);
	void SetParticlesScale(float x, float y);
	void SetParticlesScaleRF(float randomFactor1, float randomFactor2);
	void UpdateActorLayer(const int* layerMask);
	void SetScale(float x, float y);
	void SetScaleOverTime(float scale);
	void SetTexture(uint UID);

	//Rotation
	void SetParticlesRotationOverTime(int rotationOverTime);
	void SetParticlesRandomRotationOverTime(int randomRotation);
	void SetParticles3DRotationOverTime(int rotationOverTimeX, int rotationOverTimeY, int rotationOverTimeZ);
	void SetParticles3DRandomRotationOverTime(int rotationOverTimeX, int rotationOverTimeY, int rotationOverTimeZ);
	void RemoveParticlesRandomRotation();

	//Blending
	void SetEmitterBlending() const;

private:

	void SortParticles();
	void UpdateAllGradients();
	void CreateParticles(uint particlesAmount);
	void CreateAnimation(uint rows, uint cols);
	double GetRandomValue(double min, double max); //MUST EREASE IN THE FUTURE
	void HandleEditorBlendingSelector();

private:
	physx::PxParticleSystem* particleSystem = nullptr;

	std::vector<ResourceMesh*> particleMeshes;
	std::vector<Particle*> particles;
	std::map<float, int> drawingIndices;

	unsigned int maxParticles = 2000;
	bool perParticleRestOffset = false;

	physx::PxParticleExt::IndexPool* indexPool;

	uint validParticles = 0;

	bool firstEmision = true;

	//Emitter properties
	float3 emitterPosition = { 0,0,0 };
	float3 eulerRotation = float3::zero;
	Quat emitterRotation = Quat::identity;
	int particlesPerCreation = 1;
	physx::PxVec3 size = { 0,0,0 };
	float emisionRate = 500.0f;	//in milliseconds
	physx::PxVec3 externalAcceleration = { 0,10,0 };
	physx::PxVec3 particlesVelocity = { 0,0,0 };
	physx::PxVec3 velocityRandomFactor1 = { 5,5,5 };
	physx::PxVec3 velocityRandomFactor2 = { 0,0,0 };
	bool loop = true;
	bool emisionActive = true;
	int duration = 1000;
	uint emisionStart = 0;

	//Sprite rotations
	bool rotationActive = false;
	int rotationOvertime1[3] = { 0,0,0 };
	int rotationOvertime2[3] = { 0,0,0 };
	bool separateAxis = false;
	bool randomInitialRotation = false;
	int minInitialRotation[3] = { 0,0,0 };
	int maxInitialRotation[3] = { 0,0,0 };

	bool verticalBillboarding = false;
	bool horizontalBillboarding = false;

	//Animation
	int tileSize_X = 1;
	int tileSize_Y = 1;
	int startFrame = 0;
	bool randomStartFrame = false;
	float cycles = 1;

	//Particle properties
	int particlesLifeTime = 1000;
	int particlesLifeTime1 = 1000;
	int particlesLifeTime2 = 1000;
	float2 particlesScale = { 1,1 };
	float scaleOverTime = 0.0f;
	float particlesScaleRandomFactor1 = 1;
	float particlesScaleRandomFactor2 = 1;
	ResourceTexture* texture = nullptr;
	int lifetimeconstants = 0;
	int velocityconstants = 0;
	bool followEmitter = true;

	//Colors
	bool colorGradient = false;
	std::vector<float4> colors;
	std::vector<float4> gradients;
	bool animation = false;
	bool createdAnim = false;
	uint colorDuration = 0;
	float spawnClock = 0.0f;

	//Curves
	std::vector<CurveEditor*> curves;
	CurveEditor* scaleCurve = nullptr;
	CurveEditor* rotateCurve = nullptr;
	int rotationconstants = 0;
	int scaleconstants = 0;

	//Blending
	bool m_PartAutoBlending = true;
	BlendingEquations m_PartBlEquation = BlendingEquations::ADD;
	BlendAutoFunction m_PartBlendFunc = BlendAutoFunction::STANDARD_INTERPOLATIVE;
	BlendingTypes m_MPartBlend_Src = BlendingTypes::SRC_ALPHA, m_MPartBlend_Dst = BlendingTypes::ONE_MINUS_SRC_ALPHA;

	//Drawing
	OBB emisionAreaOBB;
};
BE_END_NAMESPACE

#endif //__COMPONENT_PARTICLE_EMITTER__
