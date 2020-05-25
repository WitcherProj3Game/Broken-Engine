#ifndef __COMPONENT_LIGHT_H__
#define __COMPONENT_LIGHT_H__

#include "Component.h"
#include "Color.h"
#include "Math.h"

BE_BEGIN_NAMESPACE

class ResourceMesh;

enum class LightType { NONE = -1, DIRECTIONAL, POINTLIGHT, SPOTLIGHT, MAX_LIGHT_TYPES };
enum class ShadowSmoother { PCF, POISSON_DISK, BOTH };

class BROKEN_API ComponentLight : public Component
{
public:

	ComponentLight(GameObject* ContainerGO);
	~ComponentLight();

	void Update() override;
	void SendUniforms(uint shaderID, uint lightIndex);
	void Draw();

	static inline Component::ComponentType GetType() { return Component::ComponentType::Light; }

	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;

	// --- Inspector Node (UI) ---
	void CreateInspectorNode() override;

public:

	// --- Getters --
	inline const float3 GetLightDirection()				const { return m_Direction; }
	inline const float3 GetLightColor()					const { return m_Color; }
	inline const float3 GetLightAttenuationKLQ()		const { return m_AttenuationKLQFactors; }
	inline const float2 GetLightInOutCutoff()			const { return m_InOutCutoffDegrees; }
	inline const float  GetLightIntensity()				const { return m_Intensity; }
	inline const float  GetLightDistanceMultiplier()	const { return m_DistanceMultiplier; }

	inline const LightType GetLightType()				const { return m_LightType; }

	const float4x4 GetFrustViewMatrix()				const;
	const float4x4 GetFrustProjectionMatrix()		const;
	const float4x4 GetFrustViewProjMatrix()			const;

	// -- Setters ---
	void SetLightDirection(float3 dir) { m_Direction = dir; }

	void SetLightColor(float3 color) { m_Color = color; } //NOT THAT EASY! (material?)
	void SetLightColor(Color color) { m_Color = float3(color.r, color.g, color.b); } //NOT THAT EASY! (material?)

	void SetLightAttenuationFactors(float3 AttenuationKLQ) { m_AttenuationKLQFactors = AttenuationKLQ; }
	void SetLightAttenuationFactors(float K, float L, float Q) { m_AttenuationKLQFactors = float3(K, L, Q); }

	void SetLightInOutCutoff(float2 IOCutoff) { m_InOutCutoffDegrees = IOCutoff; }
	void SetLightInOutCutoff(float innerCutoff, float outerCutoff) { m_InOutCutoffDegrees = float2(innerCutoff, outerCutoff); }

	void SetLightIntensity(float intensity) { m_Intensity = intensity; }
	void SetLightDistanceMultiplier(float distMulti) { m_DistanceMultiplier = distMulti; }

	void SetLightType(LightType type) { if (type != LightType::NONE && (uint)type < (uint)LightType::MAX_LIGHT_TYPES) m_LightType = type; }

	void SetLightShadowsFrustumPlanes(float nearp, float farp);
	void SetLightShadowsFrustumSize(float x, float y);

private:

	const std::string GetLightUniform(uint lightIndex, const char* uniformArrayName);
	void DrawFrustum();
	void DrawDirectionLines(bool spotlight, float3 pos, float3 front);

private:

	// --- Light Data ---
	float3 m_Direction = float3(0.0f);
	float3 m_Color = float3(1.0f);

	float3 m_AttenuationKLQFactors = float3(1.0f, 0.09f, 0.032f);
	float2 m_InOutCutoffDegrees = float2(12.5f, 45.0f);

	float m_Intensity = 0.5f;
	float m_DistanceMultiplier = 1.0f;

	// --- Others ---
	LightType m_LightType = LightType::NONE;

	//ResourceMesh* rmeshLight = nullptr;

	bool m_SetToZero = false;
	bool m_DrawMesh = true;

	const char* m_ShadowBlurAlgorithmsNamesVec[3] = { "PCF", "Poisson Disk", "Both" };
	bool dir_debug_draw = false;

	float2 frustum_size = float2(50.0f, 50.0f);
	float2 frusutm_planes = float2(2.0f, 75.0f);

public:
	Frustum m_LightFrustum;
	bool m_CurrentShadower = false;
	float m_ShadowsIntensity = 1.067f;
	float m_ShadowBias = 0.001f;

	bool m_ClampShadows = false;
	float m_ShadowSmoothMultiplier = 1.0f;

	ShadowSmoother m_ShadowsSmoother = ShadowSmoother::PCF;
	float m_ShadowOffsetBlur = 0.2f; //Between 0 and 1
	float m_ShadowPoissonBlur = 700.0f; //Between 0 and 50000
	float m_ShadowPCFDivisor = 9.0f; //Between 0.1 and 20.0
};

BE_END_NAMESPACE
#endif //__COMPONENT_LIGHT_H__