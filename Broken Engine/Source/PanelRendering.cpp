#include "PanelRendering.h"

// -- Modules --
#include "ModuleGui.h"
#include "EngineApplication.h"

// -- Utilities --
#include "Imgui/imgui.h"
#include "mmgr/mmgr.h"

PanelRendering::PanelRendering(char* name) : Panel(name)
{
}


PanelRendering::~PanelRendering()
{
}


void PanelRendering::SetupValues()
{
	m_GammaCorretionValue = EngineApp->renderer3D->GetGammaCorrection();
	m_AmbientColorValue = EngineApp->renderer3D->GetSceneAmbientColor();

	m_CurrBlendAutoFunc = EngineApp->renderer3D->GetRendererBlendAutoFunction();
	EngineApp->renderer3D->GetRendererBlendingManualFunction(m_CurrentAlphaSrc, m_CurrentAlphaDst);
	m_CurrBlendEquation = EngineApp->renderer3D->GetRendererBlendingEquation();

	m_SkyboxColorValue = EngineApp->renderer3D->GetSkyboxColor();
	m_SkyboxExposureValue = EngineApp->renderer3D->GetSkyboxExposure();
	m_SkyboxAngle = EngineApp->renderer3D->GetSkyboxRotation();

	m_PPExpHdr = EngineApp->renderer3D->GetPostProHDRExposure();
	m_PPGamma = EngineApp->renderer3D->GetPostProGammaCorrection();

	m_PPBloomMinBright = EngineApp->renderer3D->GetPostProBloomMinBrightness();
	m_PPBloomBrightThresh = EngineApp->renderer3D->GetPostProBloomBrightnessThreshold();
	EngineApp->renderer3D->GetPostProBloomBlur((uint&)m_PPBloomBlurAmount);
	EngineApp->renderer3D->GetPostProBloomWeights(m_PPBloom_Weights1, m_PPBloom_Weights2);
}


void PanelRendering::SetRendererValues()
{
	EngineApp->renderer3D->SetSkyboxColor(m_SkyboxColorValue);
	EngineApp->renderer3D->SetSkyboxExposure(m_SkyboxExposureValue);
	EngineApp->renderer3D->SetSkyboxRotation(m_SkyboxAngle);

	EngineApp->renderer3D->SetRendererBlendingAutoFunction(m_CurrBlendAutoFunc);
	EngineApp->renderer3D->SetRendererBlendingManualFunction(m_CurrentAlphaSrc, m_CurrentAlphaDst);
	EngineApp->renderer3D->SetRendererBlendingEquation(m_CurrBlendEquation);

	EngineApp->renderer3D->SetGammaCorrection(m_GammaCorretionValue);
	EngineApp->renderer3D->SetAmbientColor(m_AmbientColorValue);

	EngineApp->renderer3D->SetPostProHDRExposure(m_PPExpHdr);
	EngineApp->renderer3D->SetPostProGammaCorrection(m_PPGamma);

	EngineApp->renderer3D->SetPostProBloomMinBrightness(m_PPBloomMinBright);
	EngineApp->renderer3D->SetPostProBloomBrightnessThreshold(m_PPBloomBrightThresh);
	EngineApp->renderer3D->SetPostProBloomBlur((uint)m_PPBloomBlurAmount);
	EngineApp->renderer3D->SetPostProBloomWeights(m_PPBloom_Weights1, m_PPBloom_Weights2);
}


bool PanelRendering::Draw()
{
	static bool makeChanges = false;

	// --- Values Set up ---
	SetupValues();	

	// --- ImGui Context ---
	ImGui::SetCurrentContext(EngineApp->gui->getImgUICtx());
	ImGuiWindowFlags settingsFlags = 0;
	settingsFlags = ImGuiWindowFlags_NoFocusOnAppearing;

	// --- ImGui Panel ---
	if (ImGui::Begin(name, &enabled, settingsFlags))
	{
		// --------------------
		// --- General Settings ---
		ImGui::NewLine();
		ImGui::Separator();

		if (ImGui::TreeNodeEx("GENERAL SETTINGS", ImGuiTreeNodeFlags_DefaultOpen))
		{
			GeneralRendererSettings(makeChanges);
			ImGui::NewLine();
			ImGui::TreePop();
		}

		ImGui::Separator();

		// --------------------
		// --- Blending Settings ---
		ImGui::NewLine();
		ImGui::Separator();

		if (ImGui::TreeNode("BLENDING SETTINGS"))
		{
			BlendingSettings(makeChanges);
			ImGui::NewLine();
			ImGui::TreePop();
		}

		ImGui::Separator();

		// --------------------
		// --- Skybox Stuff ---
		ImGui::NewLine();
		ImGui::Separator();

		if (ImGui::TreeNode("SKYBOX SETTINGS"))
		{
			SkyboxSettings(makeChanges);
			ImGui::NewLine();
			ImGui::TreePop();
		}

		ImGui::Separator();

		// --------------------
		// --- PostPro Effects ---
		ImGui::NewLine();
		ImGui::Separator();

		if (ImGui::TreeNode("POSTPRO SETTINGS"))
		{
			PostProcessingSettings(makeChanges);
			ImGui::NewLine();
			ImGui::TreePop();
		}

		ImGui::Separator();
	}

	ImGui::End();
	if (makeChanges)
	{
		SetRendererValues();
		makeChanges = false;
	}

	return false;
}


void PanelRendering::GeneralRendererSettings(bool& makeChanges)
{
	ImGui::NewLine();
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);

	// --- Gamma Correction ---
	ImGui::Text("Gamma Correction");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
	ImGui::SetNextItemWidth(200.0f);
	if (ImGui::SliderFloat("##GammaCorrection", &m_GammaCorretionValue, 0.1f, 5.0f))
		makeChanges = true;

	// --- Scene Ambient Color ---
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
	ImGui::Text("Ambient Color");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 31.0f);
	if (ImGui::ColorEdit3("##AmbientColor", (float*)&m_AmbientColorValue, ImGuiColorEditFlags_NoInputs))
		makeChanges = true;
}


void PanelRendering::SkyboxSettings(bool& makeChanges)
{
	ImGui::NewLine();
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);

	// --- Skybox Exposure ---
	ImGui::Text("Skybox Exposure");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
	ImGui::SetNextItemWidth(200.0f);
	if (ImGui::SliderFloat("##SkyboxEXP", &m_SkyboxExposureValue, 0.1f, 5.0f))
		makeChanges = true;

	// --- Skybox Color ---
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
	ImGui::Text("Skybox Color Tint");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
	if (ImGui::ColorEdit3("##SkyboxColorTint", (float*)&m_SkyboxColorValue, ImGuiColorEditFlags_NoInputs))
		makeChanges = true;

	// --- Skybox Rotation ---
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
	ImGui::Text("Skybox Rotation");
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
	if (ImGui::DragFloat3("##SkybRot", m_SkyboxAngle.ptr())) makeChanges = true;
	ImGui::NewLine();
}


void PanelRendering::BlendingSettings(bool& makeChanges)
{
	ImGui::NewLine();
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);

	// --- Set Blending Equation ---
	ImGui::Text("Rendering Blend Equation");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
	ImGui::SetNextItemWidth(200.0f);

	std::vector<const char*> blendEqF_Vec = EngineApp->renderer3D->m_BlendEquationFunctionsVec;
	int index = (int)m_CurrBlendEquation;
	if (EngineApp->gui->HandleDropdownSelector(index, "##AlphaEq", blendEqF_Vec.data(), blendEqF_Vec.size()))
	{
		m_CurrBlendEquation = (Broken::BlendingEquations)index;
		makeChanges = true;
	}

	// --- Set Alpha Auto Function ---
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
	ImGui::Text("Rendering Blend Mode");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 38.0f);
	ImGui::SetNextItemWidth(200.0f);

	std::vector<const char*> blendAutoF_Vec = EngineApp->renderer3D->m_BlendAutoFunctionsVec;
	int index1 = (int)m_CurrBlendAutoFunc;
	if (EngineApp->gui->HandleDropdownSelector(index1, "##AlphaAutoFunction", blendAutoF_Vec.data(), blendAutoF_Vec.size()))
	{
		m_CurrBlendAutoFunc = (Broken::BlendAutoFunction)index1;
		makeChanges = true;
	}

	//Help Marker
	std::string desc = "Stand. = SRC, 1-SRCALPH\nAdd. = ONE, ONE\nAddAlph. = SRC_ALPH, ONE\nMult. = DSTCOL, ZERO";
	ImGui::SameLine();
	EngineApp->gui->HelpMarker(desc.c_str());

	// --- Set Alpha Manual Function ---
	ImGui::NewLine();
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
	ImGui::Checkbox("Automatic Alpha Selection", &EngineApp->renderer3D->m_AutomaticBlendingFunc);
	if (!EngineApp->renderer3D->m_AutomaticBlendingFunc)
	{
		//ImGui::Separator();
		ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
		if (ImGui::TreeNodeEx("Manual Alpha", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//Source
			//ImGui::NewLine();
			ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 30.0f);
			ImGui::Text("Source Alpha"); ImGui::SameLine();
			ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 55.0f);
			ImGui::SetNextItemWidth(200.0f);

			std::vector<const char*> blendTypes_Vec = EngineApp->renderer3D->m_AlphaTypesVec;
			int index2 = (int)m_CurrentAlphaSrc;
			if (EngineApp->gui->HandleDropdownSelector(index2, "##ManualAlphaSrc", blendTypes_Vec.data(), blendTypes_Vec.size()))
			{
				m_CurrentAlphaSrc = (Broken::BlendingTypes)index2;
				makeChanges = true;
			}

			//Destination
			ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 30.0f);
			ImGui::Text("Destination Alpha");
			ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
			ImGui::SetNextItemWidth(200.0f);

			int index3 = (int)m_CurrentAlphaDst;
			if (EngineApp->gui->HandleDropdownSelector(index3, "##ManualAlphaDst", blendTypes_Vec.data(), blendTypes_Vec.size()))
			{
				m_CurrentAlphaDst = (Broken::BlendingTypes)index3;
				makeChanges = true;
			}

			ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 30.0f);
			if (ImGui::Button("Reference (Test Blend)", { 180, 18 })) EngineApp->gui->RequestBrowser("https://www.andersriggelsen.dk/glblendfunc.php");
			ImGui::TreePop();
		}
	}
}


void PanelRendering::PostProcessingSettings(bool& makeChanges)
{
	ImGui::NewLine();
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
	ImGui::Checkbox("Use PostPro Effects", &EngineApp->renderer3D->post_processing);

	// --- PostPro Gamma Correction ---
	ImGui::NewLine();
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
	ImGui::Text("PostPro Gamma Correction");
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
	ImGui::SetNextItemWidth(200.0f);
	if (ImGui::SliderFloat("##PPGammaCorrection", &m_PPGamma, 0.1f, 5.0f))
		makeChanges = true;

	ImGui::NewLine();

	// --- Use HDR & Exposure Slider ---
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
	if (ImGui::Checkbox("Use HDR", &EngineApp->renderer3D->m_UseHDR))
	{
		if (!EngineApp->renderer3D->m_UseHDR)
			EngineApp->renderer3D->m_UseBloom = false;
	}

	if (EngineApp->renderer3D->m_UseHDR)
	{
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 24.0f);
		ImGui::Text("Exposure");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 50.0f);
		ImGui::SetNextItemWidth(200.0f);
		if (ImGui::SliderFloat("##HDREXP", &m_PPExpHdr, 0.0f, 10.0f))
			makeChanges = true;
	}

	ImGui::NewLine();

	// --- Use Bloom & Its stuff ---
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
	if (ImGui::Checkbox("Use Bloom", &EngineApp->renderer3D->m_UseBloom))
	{
		if (EngineApp->renderer3D->m_UseBloom)
			EngineApp->renderer3D->m_UseHDR = true;
	}

	if (EngineApp->renderer3D->m_UseBloom)
	{
		//Blur Amount (how many times the for applyuing blur shader runs)
		ImGui::NewLine();
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 165.0f);
		ImGui::Text("Blur Amount");
		ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 165.0f);
		ImGui::SetNextItemWidth(200.0f);
		if (ImGui::SliderInt("##PPBBlurAmount", &m_PPBloomBlurAmount, 0, 30))
			makeChanges = true;

		//Minimum Brightness to blur (bloom PP)
		ImGui::NewLine(); ImGui::NewLine();
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 165.0f);
		ImGui::Text("Minimum Bright to Blur");
		ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 165.0f);
		ImGui::SetNextItemWidth(200.0f);
		if (ImGui::SliderFloat("##PPBMinBright", &m_PPBloomMinBright, 0.0f, 1.0f))
			makeChanges = true;

		//Brightness Threshold on Bloom
		ImGui::NewLine(); ImGui::NewLine();
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 165.0f);
		ImGui::Text("Brightness Threshold (R-G-B)");
		ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 115.0f);
		ImGui::SetNextItemWidth(300.0f);
		if (ImGui::DragFloat3("##PPBBrightThresh", m_PPBloomBrightThresh.ptr(), 0.001f, 0.0f, 1.0f, "%.5f"))
			makeChanges = true;

		//Blur Weights
		ImGui::NewLine(); ImGui::NewLine();
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 165.0f);
		ImGui::Text("Blur Weight");
		ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 115.0f);
		ImGui::SetNextItemWidth(300.0f);
		if (ImGui::DragFloat3("##PPBBlurWeight1", m_PPBloom_Weights1.ptr(), 0.0000001f, 0.0f, 1.0f, "%.10f"))
			makeChanges = true;

		ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 165.0f);
		ImGui::SetNextItemWidth(200.0f);
		if (ImGui::DragFloat2("##PPBBlurWeight2", m_PPBloom_Weights2.ptr(), 0.0000001f, 0.0f, 1.0f, "%.10f"))
			makeChanges = true;

		// Render ONLY Bloom (blurred) Texture
		ImGui::NewLine(); ImGui::NewLine();
		ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
		ImGui::Checkbox("Render Bloom Only", &EngineApp->renderer3D->m_RenderBloomOnly);

		ImGui::NewLine();
	}
	else if (EngineApp->renderer3D->m_RenderBloomOnly)
		EngineApp->renderer3D->m_RenderBloomOnly = false;
}