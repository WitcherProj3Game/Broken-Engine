#include "PanelRendering.h"

// -- Modules --
#include "EngineApplication.h"
#include "ModuleRenderer3D.h"
#include "ModuleGui.h"

// -- Utilities --
#include "Imgui/imgui.h"
#include "mmgr/mmgr.h"

PanelRendering::PanelRendering(char* name) : Panel(name)
{
}

PanelRendering::~PanelRendering()
{
}

bool PanelRendering::Draw()
{
	m_GammaCorretionValue = EngineApp->renderer3D->GetGammaCorrection();
	m_AmbientColorValue = EngineApp->renderer3D->GetSceneAmbientColor();

	ImGui::SetCurrentContext(EngineApp->gui->getImgUICtx());

	ImGuiWindowFlags settingsFlags = 0;
	settingsFlags = ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, settingsFlags))
	{
		ImGui::Text("Gamma Correction");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		ImGui::SetNextItemWidth(200.0f);
		ImGui::SliderFloat("##GammaCorrection", &m_GammaCorretionValue, 0.1f, 5.0f);
		ImGui::NewLine();

		ImGui::Text("Ambient Color");
		ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
		ImGui::ColorEdit4("##AmbientColor", (float*)&m_AmbientColorValue, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
		ImGui::NewLine();
	}
	ImGui::End();

	EngineApp->renderer3D->SetGammaCorrection(m_GammaCorretionValue);
	EngineApp->renderer3D->SetSceneAmbientColor(m_AmbientColorValue);
	return false;
}
