#ifndef __PANEL_RENDERING_H__
#define __PANEL_RENDERING_H__

#include "ModuleRenderer3D.h"
#include "Panel.h"
#include "Math.h"

class PanelRendering : public Panel
{
public:

	PanelRendering(char* name);
	~PanelRendering();

	bool Draw();

private:

	void SetupValues();
	void SetRendererValues();
	void BlendingSettings(bool& makeChanges);
	void GeneralRendererSettings(bool& makeChanges);
	void SkyboxSettings(bool& makeChanges);

private:

	// Gamma Correction, Ambient Color & Skybox Color/Exposure
	float m_GammaCorretionValue = 1.0f;
	float3 m_AmbientColorValue = float3::one;
	float3 m_SkyboxColorValue = float3::one;
	float m_SkyboxExposureValue = 1.0f;

	//PostPro Stuff
	float m_ExpHdr = 1.0f;
	float m_PPGamma = 1.8f;

	//Alpha Functions
	Broken::BlendAutoFunction m_CurrBlendAutoFunc = Broken::BlendAutoFunction::STANDARD_INTERPOLATIVE;
	Broken::BlendingEquations m_CurrBlendEquation = Broken::BlendingEquations::ADD;

	//Alpha Manual Functions
	Broken::BlendingTypes m_CurrentAlphaSrc = Broken::BlendingTypes::SRC_ALPHA;
	Broken::BlendingTypes m_CurrentAlphaDst = Broken::BlendingTypes::ONE_MINUS_SRC_ALPHA;
};

#endif