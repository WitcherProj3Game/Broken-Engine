#include "PanelShadowedScene.h"
// -- Modules --
#include "EngineApplication.h"
#include "ModuleGui.h"
#include "ModuleRenderer3D.h"

// -- Panels --
#include "PanelScene.h"
#include "PanelHierarchy.h"

// -- Utilitites --
#include "Imgui/imgui.h"
#include "mmgr/mmgr.h"

PanelShadowedScene::PanelShadowedScene(char* name) : Panel(name)
{
}

PanelShadowedScene::~PanelShadowedScene()
{
}

bool PanelShadowedScene::Draw()
{
	ImGui::SetCurrentContext(EngineApp->gui->getImgUICtx());

	ImGuiWindowFlags settingsFlags = 0;
	settingsFlags = ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollWithMouse;

	if (ImGui::Begin(name, &enabled, settingsFlags))
	{
		// --- Set image size
		width = ImGui::GetWindowWidth() * 0.98;
		height = ImGui::GetWindowHeight() * 0.90;
		ImVec2 size = ImVec2(width, height);

		// --- Force Window Size ---
		if (ImGui::GetWindowWidth() < ImGui::GetWindowHeight())
		{
			size.x = size.y;
			width = height;
			ImGui::SetWindowSize(name, size);
		}

		ImGui::Image((ImTextureID)EngineApp->renderer3D->GetDepthMapTexture(), size, ImVec2(0, 1), ImVec2(1, 0));


		height = height;
		width = width;
	}

	ImGui::End();
	return true;
}
