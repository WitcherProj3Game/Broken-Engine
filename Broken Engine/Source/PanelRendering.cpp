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
	// --- Values Set up ---
	m_GammaCorretionValue = EngineApp->renderer3D->GetGammaCorrection();
	m_AmbientColorValue = EngineApp->renderer3D->GetSceneAmbientColor();
	m_CurrentAlphaFunc = EngineApp->renderer3D->GetRendererAlphaFunction();

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
		ImGui::SliderFloat("##GammaCorrection", &m_GammaCorretionValue, 0.1f, 5.0f);
		ImGui::NewLine();

		// --- Scene Ambient Color ---
		ImGui::Text("Ambient Color");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		ImGui::ColorEdit4("##AmbientColor", (float*)&m_AmbientColorValue, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
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
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		}
	}

	ImGui::End();

	EngineApp->renderer3D->SetRendererAlphaFunction(m_CurrentAlphaFunc);
	EngineApp->renderer3D->SetGammaCorrection(m_GammaCorretionValue);
	EngineApp->renderer3D->SetSceneAmbientColor(m_AmbientColorValue);
	return false;
}
