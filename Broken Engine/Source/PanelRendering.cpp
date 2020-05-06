#include "PanelRendering.h"

// -- Modules --
#include "ModuleGui.h"
#include "EngineApplication.h"

// -- Utilities --
#include "Imgui/imgui.h"
#include "mmgr/mmgr.h"

PanelRendering::PanelRendering(char* name) : Panel(name)
{
	//Auto Blend Functions
	m_BlendAutoFunctions.push_back("STANDARD INTERPOLATIVE");
	m_BlendAutoFunctions.push_back("ADDITIVE");
	m_BlendAutoFunctions.push_back("ADDITIVE ALPHA AFFECTED");
	m_BlendAutoFunctions.push_back("MULTIPLICATIVE");

	//Blending Equations
	m_BlendEquationFunctions.push_back("ADD (Standard)");
	m_BlendEquationFunctions.push_back("SUBTRACT");
	m_BlendEquationFunctions.push_back("REVERSE_SUBTRACT");
	m_BlendEquationFunctions.push_back("MIN");
	m_BlendEquationFunctions.push_back("MAX");

	//Manual Blend Functions
	m_AlphaTypesVec.push_back("GL_ZERO");
	m_AlphaTypesVec.push_back("GL_ONE");
	m_AlphaTypesVec.push_back("GL_SRC_COLOR");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_SRC_COLOR");

	m_AlphaTypesVec.push_back("GL_DST_COLOR");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_DST_COLOR");
	m_AlphaTypesVec.push_back("GL_SRC_ALPHA (Standard)");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_SRC_ALPHA (Standard)");

	m_AlphaTypesVec.push_back("GL_DST_ALPHA");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_DST_ALPHA");
	m_AlphaTypesVec.push_back("GL_CONSTANT_COLOR");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_CONSTANT_COLOR");

	m_AlphaTypesVec.push_back("GL_CONSTANT_ALPHA");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_CONSTANT_ALPHA");
	m_AlphaTypesVec.push_back("GL_SRC_ALPHA_SATURATE");
}

PanelRendering::~PanelRendering()
{
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
		// --- Gamma Correction ---
		ImGui::Text("Gamma Correction");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		ImGui::SetNextItemWidth(200.0f);
		if(ImGui::SliderFloat("##GammaCorrection", &m_GammaCorretionValue, 0.1f, 5.0f)) makeChanges = true;
		ImGui::NewLine();

		// --- Scene Ambient Color ---
		ImGui::Text("Ambient Color");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		if(ImGui::ColorEdit4("##AmbientColor", (float*)&m_AmbientColorValue, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar)) makeChanges = true;
		ImGui::NewLine();
		ImGui::Separator();

		// --------------------
		// --- Set Alpha Auto Function ---
		ImGui::Text("Rendering Blend Equation"); ImGui::SameLine();
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		ImGui::SetNextItemWidth(200.0f);

		static int index = (int)m_CurrBlendEquation;
		if (EngineApp->gui->HandleDropdownSelector(index, "##AlphaEq", m_BlendEquationFunctions.data(), m_BlendEquationFunctions.size()))
		{
			m_CurrBlendEquation = (Broken::BlendingEquations)index;
			makeChanges = true;
		}

		ImGui::Text("Rendering Blend Mode"); ImGui::SameLine();
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		ImGui::SetNextItemWidth(200.0f);

		static int index1 = (int)m_CurrBlendAutoFunc;
		if (EngineApp->gui->HandleDropdownSelector(index1, "##AlphaAutoFunction", m_BlendAutoFunctions.data(), m_BlendAutoFunctions.size()))
		{
			m_CurrBlendAutoFunc = (Broken::BlendAutoFunction)index1;
			makeChanges = true;
		}

		//Help Marker
		std::string desc = "Stand. = SRC, 1-SRCALPH\nAdd. = ONE, ONE\nAddAlph. = SRC_ALPH, ONE\nMult. = DSTCOL, ZERO";
		ImGui::SameLine();
		EngineApp->gui->HelpMarker(desc.c_str());

		// --- Set Alpha Manual Function ---
		ImGui::Checkbox("Automatic Alpha Selection", &EngineApp->renderer3D->m_AutomaticBlendingFunc);
		if (!EngineApp->renderer3D->m_AutomaticBlendingFunc)
		{
			ImGui::Separator();
			if (ImGui::TreeNodeEx("Manual Alpha", ImGuiTreeNodeFlags_DefaultOpen))
			{
				//Source
				ImGui::NewLine();
				ImGui::Text("Source Alpha"); ImGui::SameLine();
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
				ImGui::SetNextItemWidth(200.0f);

				static int index2 = (int)m_CurrentAlphaSrc;
				if (EngineApp->gui->HandleDropdownSelector(index2, "##ManualAlphaSrc", m_AlphaTypesVec.data(), m_AlphaTypesVec.size()))
				{
					m_CurrentAlphaSrc = (Broken::BlendingTypes)index2;
					makeChanges = true;
				}

				//Destination
				ImGui::Text("Destination Alpha"); ImGui::SameLine();
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
				ImGui::SetNextItemWidth(200.0f);

				static int index3 = (int)m_CurrentAlphaDst;
				if (EngineApp->gui->HandleDropdownSelector(index3, "##ManualAlphaDst", m_AlphaTypesVec.data(), m_AlphaTypesVec.size()))
				{
					m_CurrentAlphaDst = (Broken::BlendingTypes)index3;
					makeChanges = true;
				}

				ImGui::NewLine();
				if (ImGui::Button("Reference (Test Blend)", { 180, 18 })) EngineApp->gui->RequestBrowser("https://www.andersriggelsen.dk/glblendfunc.php");
				ImGui::TreePop();
			}
		}

		// --------------------
		// --- Skybox Stuff ---
		ImGui::NewLine();
		ImGui::Separator();
		ImGui::Text("Skybox");
		ImGui::NewLine();

		// --- Skybox Exposure ---
		ImGui::Text("Skybox Exposure");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		ImGui::SetNextItemWidth(200.0f);
		if(ImGui::SliderFloat("##SkyboxEXP", &m_SkyboxExposureValue, 0.1f, 5.0f)) makeChanges = true;
		ImGui::NewLine();

		// --- Skybox Color ---
		ImGui::Text("Skybox Color Tint");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		if(ImGui::ColorEdit3("##SkyboxColorTint", (float*)&m_SkyboxColorValue, ImGuiColorEditFlags_NoInputs)) makeChanges = true;
		ImGui::NewLine();
	}

	ImGui::End();	
	if (makeChanges)
	{
		SetRendererValues();
		makeChanges = false;
	}

	return false;
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
}

void PanelRendering::SetRendererValues()
{
	EngineApp->renderer3D->SetSkyboxColor(m_SkyboxColorValue);
	EngineApp->renderer3D->SetSkyboxExposure(m_SkyboxExposureValue);

	EngineApp->renderer3D->SetRendererBlendingAutoFunction(m_CurrBlendAutoFunc);
	EngineApp->renderer3D->SetRendererBlendingManualFunction(m_CurrentAlphaSrc, m_CurrentAlphaDst);
	EngineApp->renderer3D->SetRendererBlendingEquation(m_CurrBlendEquation);

	EngineApp->renderer3D->SetGammaCorrection(m_GammaCorretionValue);
	EngineApp->renderer3D->SetSceneAmbientColor(m_AmbientColorValue);
}