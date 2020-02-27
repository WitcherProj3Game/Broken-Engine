#include "PanelProject.h"
#include "EngineApplication.h"
#include "ModuleEditorUI.h"
//#include "ModuleFileSystem.h"
//#include "ModuleResourceManager.h"
//#include "ModuleEventManager.h"
//#include "ModuleGui.h"

//#include "ResourceFolder.h"
//#include "ResourceModel.h"

#include "mmgr/mmgr.h"

// --- Event Manager Callbacks ---
void PanelProject::ONGameObjectSelected(const BrokenEngine::Event& e)
{
	EngineApp->editorui->panelProject->SetSelected(nullptr);
}

void PanelProject::ONResourceDestroyed(const BrokenEngine::Event& e)
{
	if (e.uid == EngineApp->editorui->panelProject->selected_uid)
		EngineApp->editorui->panelProject->SetSelected(nullptr);
}

// -------------------------------

PanelProject::PanelProject(char * name) : BrokenEngine::Panel(name)
{
	// --- Add Event Listeners ---
	EngineApp->event_manager->AddListener(BrokenEngine::Event::EventType::GameObject_selected, ONGameObjectSelected);
	EngineApp->event_manager->AddListener(BrokenEngine::Event::EventType::Resource_destroyed, ONResourceDestroyed);

}

PanelProject::~PanelProject()
{
	EngineApp->event_manager->RemoveListener(BrokenEngine::Event::EventType::GameObject_selected, ONGameObjectSelected);
	EngineApp->event_manager->RemoveListener(BrokenEngine::Event::EventType::Resource_destroyed, ONResourceDestroyed);
}


// MYTODO: Clean this up

bool PanelProject::Draw()
{
	ImGuiWindowFlags projectFlags = 0;
	projectFlags = ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_MenuBar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	// --- Draw project panel, Unity style ---
	if (ImGui::Begin(name, &enabled, projectFlags))
	{
		static std::vector<std::string> filters;

		ImGui::BeginMenuBar();
		ImGui::EndMenuBar();

		ImGui::SetCursorScreenPos(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + 38));

		// --- Draw Directories Tree ---
		ImGui::BeginChild("AssetsTree", ImVec2(ImGui::GetWindowSize().x*0.1,ImGui::GetWindowSize().y));

		RecursiveDirectoryDraw(ASSETS_FOLDER, filters);

		ImGui::EndChild();

		// --- Draw Explorer ---
		ImGui::SameLine();

		ImGui::BeginChild("AssetsExplorer", ImVec2(ImGui::GetWindowSize().x*0.9f, ImGui::GetWindowSize().y), true, projectFlags);

		//if(currentDirectory == nullptr)
		//	currentDirectory = EngineApp->resources->GetAssetsFolder();

		DrawFolder(EngineApp->resources->getCurrentDirectory());

		ImGui::SetCursorScreenPos(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 58));


		// --- Item resizer and selected resource path display ---
		ImGui::BeginChild("ExplorerItemResizer", ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);
		ImGui::BeginMenuBar();

		if (selected)
			ImGui::Text(selected->GetName());

		ImGui::Spacing();

		ImGui::SetCursorScreenPos(ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.9f, ImGui::GetWindowPos().y));

		int imageSize_modifier = imageSize_px;
		ImGui::SetNextItemWidth(100.0f);
		if (ImGui::SliderInt("##itemresizer", &imageSize_modifier, 32, 64))
		{
			imageSize_px = imageSize_modifier;
		}

		ImGui::EndMenuBar();
		ImGui::EndChild();


		ImGui::EndChild();
	}

	ImGui::PopStyleVar();

	ImGui::End();


	return true;
}

void PanelProject::SetSelected(BrokenEngine::Resource* new_selected)
{
	selected = new_selected;

	if (selected)
	{
		selected_uid = new_selected->GetUID();
		BrokenEngine::Event e(BrokenEngine::Event::EventType::Resource_selected);
		e.resource = selected;
		EngineApp->event_manager->PushEvent(e);
	}
	else
		selected_uid = 0;
}

//const BrokenEngine::Resource* PanelProject::GetcurrentDirectory() const
//{
//	return currentDirectory;
//}


void PanelProject::DrawFolder(BrokenEngine::ResourceFolder* folder)
{
	// --- Draw menuBar / path to current folder ---
	ImGui::BeginMenuBar();

	BrokenEngine::ResourceFolder* curr = folder;

	if (EngineApp->resources->getCurrentDirectory() == EngineApp->resources->GetAssetsFolder())
		ImGui::TextColored(ImVec4(0, 120, 255, 255), EngineApp->resources->GetAssetsFolder()->GetName());
	else
	ImGui::Text(EngineApp->resources->GetAssetsFolder()->GetName());

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
		EngineApp->resources->setCurrentDirectory(EngineApp->resources->GetAssetsFolder());

	std::vector<BrokenEngine::ResourceFolder*> folders_path;

	while (curr->GetParent())
	{
		folders_path.push_back(curr);
		curr = curr->GetParent();
	}

	for (std::vector<BrokenEngine::ResourceFolder*>::const_reverse_iterator it = folders_path.rbegin(); it != folders_path.rend(); ++it)
	{
		if (EngineApp->resources->getCurrentDirectory == *it)
			ImGui::TextColored(ImVec4(0, 120, 255, 255),(*it)->GetName());
		else
		ImGui::Text((*it)->GetName());

		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
			EngineApp->resources->setCurrentDirectory(*it);

		ImGui::SameLine();
	}

	ImGui::EndMenuBar();


	// --- Draw File Explorer ---

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(item_spacingX_px, item_spacingY_px));

	if (folder)
	{
		const std::vector<BrokenEngine::Resource*>* resources = &folder->GetResources();
		const std::vector<BrokenEngine::ResourceFolder*>* directories = &folder->GetChilds();
		uint i = 0;
		uint row = 0;
		maxColumns = ImGui::GetWindowSize().x / (imageSize_px + item_spacingX_px);
		ImVec4 color = ImVec4(255, 255, 255, 255);

		ImVec2 vec = ImGui::GetCursorPos();

		// --- Draw sub-folders ---
		for (std::vector<BrokenEngine::ResourceFolder*>::const_iterator it = directories->begin(); it != directories->end(); ++it)
		{
			if (!*it)
				continue;

			ImGui::SetCursorPosX(vec.x + (i - row * maxColumns) * (imageSize_px + item_spacingX_px) + item_spacingX_px);
			ImGui::SetCursorPosY(vec.y + row * (imageSize_px + item_spacingY_px) + item_spacingY_px);

			std::string item_name = (*it)->GetName();
			item_name.pop_back();
			LimitText(item_name);

			if (selected && selected->GetUID() == (*it)->GetUID())
				color = ImVec4(0, 120, 255, 255);

			ImGui::Image((ImTextureID)(*it)->GetPreviewTexID(), ImVec2(imageSize_px, imageSize_px), ImVec2(0, 1), ImVec2(1, 0), color);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				uint UID = (*it)->GetUID();
				ImGui::SetDragDropPayload("resource", &UID, sizeof(uint));
				ImGui::Text(item_name.c_str());
				ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y - 20.0f));
				ImGui::Image((ImTextureID)(*it)->GetPreviewTexID(), ImVec2(imageSize_px, imageSize_px), ImVec2(0, 1), ImVec2(1, 0));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleVar();

			if (ImGui::IsItemClicked())
				SetSelected(*it);

			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				EngineApp->resources->setCurrentDirectory(*it);

			ImGui::SetCursorPosX(vec.x + (i - row * maxColumns) * (imageSize_px + item_spacingX_px) + item_spacingX_px + ((imageSize_px - ImGui::CalcTextSize(item_name.c_str(), nullptr).x)/2));
			ImGui::SetCursorPosY(vec.y + row * (imageSize_px + item_spacingY_px) + item_spacingY_px + imageSize_px);

			ImGui::TextColored(color, item_name.c_str());

			if (selected && selected->GetUID() == (*it)->GetUID())
				color = ImVec4(255, 255, 255, 255);

			if ((i + 1) % maxColumns == 0)
				row++;
			else
				ImGui::SameLine();

			i++;
		}

		// --- Draw the rest of files ---
		for (std::vector<BrokenEngine::Resource*>::const_iterator it = resources->begin(); it != resources->end(); ++it)
		{
			if (!*it)
				continue;

			if ((*it)->has_parent)
				continue;

			DrawFile(*it, i, row, vec, color);

			bool opened = false;

			// --- Draw model childs ---
			if ((*it)->GetType() == BrokenEngine::Resource::ResourceType::MODEL)
			{
				opened = true;

				if ((i + 1) % maxColumns == 0)
					row++;
				else
					ImGui::SameLine();

				i++;

				ImGui::PushID((*it)->GetUID());

				uint arrowSize = imageSize_px / 4;

				ImGui::SetCursorPosX(vec.x + (i - row * maxColumns) * (imageSize_px + item_spacingX_px/1.1) + imageSize_px/10);
				ImGui::SetCursorPosY(vec.y + row * (imageSize_px + item_spacingY_px) + item_spacingY_px + imageSize_px/2);

				BrokenEngine::ResourceModel* model = (BrokenEngine::ResourceModel*)*it;

				ImVec2 uvx = { 0,1 };
				ImVec2 uvy = { 1,0 };
				// --- Set arrow direction ---
				if (model->openInProject)
				{
					uvx = { 1,0 }; 
					uvy = { 0,1 };
				}

				if (ImGui::ImageButton((ImTextureID)EngineApp->gui->playbuttonTexID, ImVec2(arrowSize, arrowSize), uvx, uvy, 0))
					model->openInProject = !model->openInProject;

				if (model->openInProject)
				{
					std::vector<BrokenEngine::Resource*>* model_resources = model->GetResources();

					for (std::vector<BrokenEngine::Resource*>::const_iterator res = model_resources->begin(); res != model_resources->end(); ++res)
					{
						DrawFile(*res, i, row, vec, color, true);

						if ((i + 1) % maxColumns == 0)
							row++;
						else
							ImGui::SameLine();

						i++;
					}
				}

				ImGui::PopID();
			}

			if ((i + 1) % maxColumns == 0)
				row++;
			else
				ImGui::SameLine();

			if(!opened)
			i++;
		}
	}

	ImGui::PopStyleVar();
}

void PanelProject::DrawFile(BrokenEngine::Resource* resource, uint i, uint row, ImVec2& cursor_pos, ImVec4& color, bool child)
{
	ImGui::SetCursorPosX(cursor_pos.x + (i - row * maxColumns) * (imageSize_px + item_spacingX_px) + item_spacingX_px);
	ImGui::SetCursorPosY(cursor_pos.y + row * (imageSize_px + item_spacingY_px) + item_spacingY_px);

	std::string item_name = resource->GetName();
	LimitText(item_name);

	if (selected && selected->GetUID() == resource->GetUID())
		color = ImVec4(0, 120, 255, 255);

	if (child)
		ImGui::ImageButton((ImTextureID)resource->GetPreviewTexID(), ImVec2(imageSize_px, imageSize_px), ImVec2(0, 1), ImVec2(1, 0), 0, ImVec4(0.15, 0.15, 0.15, 1), color);
	else
		ImGui::Image((ImTextureID)resource->GetPreviewTexID(), ImVec2(imageSize_px, imageSize_px), ImVec2(0, 1), ImVec2(1, 0), color);

	if (selected && selected->GetUID() == resource->GetUID()
		&& wasclicked && ImGui::IsMouseReleased(0))
	{
		if (ImGui::IsItemHovered())
		{
			SetSelected(resource);
			wasclicked = false;
		}
		else
			SetSelected(nullptr);
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));

	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
	{
		uint UID = resource->GetUID();
		ImGui::SetDragDropPayload("resource", &UID, sizeof(uint));
		ImGui::Text(item_name.c_str());
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y - 20.0f));
		ImGui::Image((ImTextureID)resource->GetPreviewTexID(), ImVec2(imageSize_px, imageSize_px), ImVec2(0, 1), ImVec2(1, 0));
		ImGui::EndDragDropSource();
	}


	if (ImGui::IsItemClicked())
	{
		selected = resource;
		wasclicked = true;
	}

	ImGui::PopStyleVar();


	ImGui::SetCursorPosX(cursor_pos.x + (i - row * maxColumns) * (imageSize_px + item_spacingX_px) + item_spacingX_px + ((imageSize_px - ImGui::CalcTextSize(item_name.c_str(), nullptr).x) / 2));
	ImGui::SetCursorPosY(cursor_pos.y + row * (imageSize_px + item_spacingY_px) + item_spacingY_px + imageSize_px);

	ImGui::TextColored(color, item_name.c_str());

	if (selected && selected->GetUID() == resource->GetUID())
		color = ImVec4(255, 255, 255, 255);
}

void PanelProject::LimitText(std::string& text)
{
	uint textSizeX_px = ImGui::CalcTextSize(text.c_str(),nullptr).x;
	uint dotsSizeX_px = ImGui::CalcTextSize("...", nullptr, false, 0).x;

	// --- The total pixel space available is the text picel space + dots pixel space ---
	if (imageSize_px < textSizeX_px)
	{
		uint charSizeX_px = textSizeX_px / text.size();
		text = text.substr(0, (imageSize_px - dotsSizeX_px) / charSizeX_px);
		text.append("...");
	}
}

// MYTODO: To be substituted (folders/files are already loaded)
void PanelProject::RecursiveDirectoryDraw(const char * directory, std::vector<std::string>& filters)
{
	std::vector<std::string> files;
	std::vector<std::string> dirs;

	std::string dir((directory) ? directory : "");
	dir += "/";

	EngineApp->fs->DiscoverFiles(dir.c_str(), files, dirs);

	for (std::vector<std::string>::const_iterator it = dirs.begin(); it != dirs.end(); ++it)
	{
		if (ImGui::TreeNodeEx((dir + (*it)).c_str(), 0, "%s/", (*it).c_str()))
		{
			RecursiveDirectoryDraw((dir + (*it)).c_str(), filters);
			ImGui::TreePop();
		}
	}
}
