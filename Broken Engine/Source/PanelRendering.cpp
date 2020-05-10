#include "PanelRendering.h"

// -- Modules --
#include "ModuleGui.h"
#include "EngineApplication.h"

// -- Utilities --
#include "Imgui/imgui.h"
#include "mmgr/mmgr.h"

PanelRendering::PanelRendering(char* name) : Panel(name)
{
	m_AlphaFunctionsVec.push_back("SRC_ONE_MINUS_SRC");
	m_AlphaFunctionsVec.push_back("ONE_ONE_MINUS_SRC");
	m_AlphaFunctionsVec.push_back("SRC_ONE_MINUS_SCR_ONE");
}

PanelRendering::~PanelRendering()
{
}

bool PanelRendering::Draw()
{
	static bool makeChanges = false;

	// --- Values Set up ---
	m_GammaCorretionValue = EngineApp->renderer3D->GetGammaCorrection();
	m_AmbientColorValue = EngineApp->renderer3D->GetSceneAmbientColor();
	m_CurrentAlphaFunc = EngineApp->renderer3D->GetRendererAlphaFunction();
	m_SkyboxColorValue = EngineApp->renderer3D->GetSkyboxColor();
	m_SkyboxExposureValue = EngineApp->renderer3D->GetSkyboxExposure();

	// --- ImGui Context ---
	ImGui::SetCurrentContext(EngineApp->gui->getImgUICtx());
	ImGuiWindowFlags settingsFlags = 0;
	settingsFlags = ImGuiWindowFlags_NoFocusOnAppearing;

	// --- ImGui Panel ---
	if (ImGui::Begin(name, &enabled, settingsFlags))
	{
		// --- Gamma Correction ---
		ImGui::Text("Gamma Correction");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		ImGui::SetNextItemWidth(200.0f);
		if(ImGui::SliderFloat("##GammaCorrection", &m_GammaCorretionValue, 0.1f, 5.0f)) makeChanges = true;
		ImGui::NewLine();

		// --- Scene Ambient Color ---
		ImGui::Text("Ambient Color");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		if(ImGui::ColorEdit3("##AmbientColor", (float*)&m_AmbientColorValue, ImGuiColorEditFlags_NoInputs)) makeChanges = true;
		ImGui::NewLine();

		// --- Set Alpha Function ---		
		const char* item_current = m_AlphaFunctionsVec[(uint)m_CurrentAlphaFunc];
		if (ImGui::BeginCombo("##AlphaFunction", item_current))
		{
			for (uint i = 0; i < m_AlphaFunctionsVec.size(); ++i)
			{
				bool is_selected = (item_current == m_AlphaFunctionsVec[i]);
				if (ImGui::Selectable(m_AlphaFunctionsVec[i], is_selected))
				{
					item_current = m_AlphaFunctionsVec[i];
					m_CurrentAlphaFunc = (Broken::AlphaFunction)i;
					makeChanges = true;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

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
		EngineApp->renderer3D->SetSkyboxColor(m_SkyboxColorValue);
		EngineApp->renderer3D->SetSkyboxExposure(m_SkyboxExposureValue);
		EngineApp->renderer3D->SetRendererAlphaFunction(m_CurrentAlphaFunc);
		EngineApp->renderer3D->SetGammaCorrection(m_GammaCorretionValue);
		EngineApp->renderer3D->SetSceneAmbientColor(m_AmbientColorValue);

		makeChanges = false;
	}

	return false;
}
