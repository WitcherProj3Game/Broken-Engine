#ifndef __PANEL_RENDERING_H__
#define __PANEL_RENDERING_H__

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
	void PostProcessingSettings(bool& makeChanges);

private:

	// Gamma Correction, Ambient Color & Skybox Color/Exposure
	float m_GammaCorretionValue = 1.0f;
	float3 m_AmbientColorValue = float3::one;
	float3 m_SkyboxColorValue = float3::one;
	float m_SkyboxExposureValue = 1.0f;
	float3 m_SkyboxAngle = float3::zero;

	//PostPro Stuff
	float m_PPExpHdr = 1.0f;
	float m_PPGamma = 1.8f;
	float m_PPBloomMinBright = 1.0f;
	float3 m_PPBloomBrightThresh = float3(0.2126, 0.7152, 0.0722);
	int m_PPBloomBlurAmount = 10;
	float3 m_PPBloom_Weights1 = float3(0.2270270270, 0.1945945946, 0.1216216216);
	float2 m_PPBloom_Weights2 = float2(0.0540540541, 0.0162162162);

	//Alpha Functions
	int m_CurrBlendAutoFunc;
	int m_CurrBlendEquation;

	//Alpha Manual Functions
	int m_CurrentAlphaSrc;
	int m_CurrentAlphaDst;
};

#endif