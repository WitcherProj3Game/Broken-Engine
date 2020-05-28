#ifndef __RESOURCE_SCENE_H__
#define __RESOURCE_SCENE_H__

#include "Resource.h"
#include <unordered_map>
#include "Math.h"
#include "ModuleRenderer3D.h"

BE_BEGIN_NAMESPACE
class GameObject;
class ResourceTexture;

class BROKEN_API ResourceScene : public Resource
{
	friend class ModuleRenderer3D;
	friend class ImporterScene;
public:
	ResourceScene(uint UID, const char* source_file);
	~ResourceScene();

	bool LoadInMemory() override;
	void FreeMemory() override;

	void DeactivateAllGameObjects();
	void ActivateAllGameObjects();

	void CopyInto(ResourceScene* target);

	GameObject* GetGOWithName(const char* GO_name);
	GameObject* GetGOWithUID(uint UID);

	const float3 GetSceneAmbientColor() const { return m_SceneColor; }
	void SetSceneAmbientColor(float3 color) { m_SceneColor = color; }

public:

	std::unordered_map<uint,GameObject*> NoStaticGameObjects;
	std::unordered_map<uint, GameObject*> StaticGameObjects;

	AABB octreeBox;	

private:

	void OnOverwrite() override;
	void OnDelete() override;

	float3 m_SceneColor = float3::one;
	float m_SceneGammaCorrection = 1.0f;

	float m_Sky_Exposure = 1.0f;
	float3 m_Sky_ColorTint = float3(1.0f);
	float3 m_Sky_Rotation = float3(0.0f);

	//Postpro
	float m_ScenePP_HDRExposure = 1.0f;
	float m_ScenePP_GammaCorr = 1.0f;

	float m_ScenePP_BloomMinBrightness = 1.0f;
	float3 m_ScenePP_BloomBrightnessThreshold = float3(0.2126, 0.7152, 0.0722);
	uint m_ScenePP_BloomBlurAmount = 10;
	
	float3 m_ScenePP_BlurWeights1 = float3(0.2270270270, 0.1945945946, 0.1216216216);
	float2 m_ScenePP_BlurWeights2 = float2(0.0540540541, 0.0162162162);

	ResourceTexture* LUT = nullptr;

	//Blending Stuff
	bool m_SceneAutoBlend = true;
	BlendAutoFunction m_RendererBlendFunc = BlendAutoFunction::STANDARD_INTERPOLATIVE;
	BlendingTypes m_ManualBlend_Src = BlendingTypes::SRC_ALPHA, m_ManualBlend_Dst = BlendingTypes::ONE_MINUS_SRC_ALPHA;
	BlendingEquations m_BlendEquation = BlendingEquations::ADD;

};
BE_END_NAMESPACE
#endif //__RESOURCE_SCENE_H__