#include "ComponentButton.h"

// -- Modules --
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleUI.h"
#include "ModuleTextures.h"
#include "PanelScene.h"
#include "ModuleGui.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleSceneManager.h"
#include "ModuleFileSystem.h"
#include "ModuleScripting.h"
#include "ModuleInput.h"

// -- Components --
#include "GameObject.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "ComponentCanvas.h"
#include "ComponentScript.h"
#include "ComponentImage.h"

// -- Resources --
#include "ResourceShader.h"
#include "ResourceTexture.h"
#include "ResourceScene.h"
#include "ResourceMesh.h"

// -- Utitilies --
#include "Imgui/imgui.h"
#include "mmgr/mmgr.h"

using namespace Broken;

ComponentButton::ComponentButton(GameObject* gameObject) : UI_Element(gameObject, Component::ComponentType::Button)
{
	name = "Button";
	visible = true;
	interactable = true;
	draggable = false;

	collider = { 0,0,0,0 };
	color = idle_color;

	if (GO->parent && GO->parent->HasComponent(Component::ComponentType::Canvas))
	{
		canvas = (ComponentCanvas*)GO->parent->GetComponent<ComponentCanvas>();

		if(canvas)
			canvas->AddElement(this);
	}

	image = (ComponentImage*)GO->AddComponent(Component::ComponentType::Image);
	image->img_color = { idle_color.r, idle_color.g, idle_color.b, idle_color.a };

	//texture = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "DefaultTexture");

	//font.init("Assets/Fonts/Dukas.ttf", font_size);
	//font.path = "Assets/Fonts/Dukas.ttf";

	func_name = "None";
}

ComponentButton::~ComponentButton()
{
	//if (texture)
	//{
	//	texture->Release();
	//	texture->RemoveUser(GO);
	//}

	if (canvas)
		canvas->RemoveElement(this);
}

void ComponentButton::Update()
{
	if (GO->parent != nullptr && canvas == nullptr && GO->parent->HasComponent(Component::ComponentType::Canvas))
	{
		canvas = (ComponentCanvas*)GO->parent->GetComponent<ComponentCanvas>();
		canvas->AddElement(this);
	}
	else if (GO->parent && !GO->parent->HasComponent(Component::ComponentType::Canvas) && canvas)
		canvas = nullptr;

	if (to_delete)
		this->GetContainerGameObject()->RemoveComponent(this);


	if (func_list[func_pos] != func_name)
	{
		func_list.clear();
		func_list.push_back("None");
		for (uint i = 0; i < script->script_functions.size(); ++i)
			func_list.push_back(script->script_functions[i].name.c_str());

		for (uint i = 0; i < func_list.size(); ++i) //get function pos
		{
			if (strcmp(func_list[i], func_name.c_str()) == 0)
			{
				func_pos = i;
				break;
			}
		}
	}
}

void ComponentButton::Draw()
{
	// --- Frame image with camera ---
	position2D = image->position2D;
	size2D = image->size2D;

	float nearp = App->renderer3D->active_camera->GetNearPlane();
	float3 pos = { position2D.x / App->gui->sceneWidth, position2D.y / App->gui->sceneHeight, nearp + 0.026f };

	//--- Update color depending on state ---
	if (state == IDLE) ChangeColorTo(idle_color);
	if (state == HOVERED) ChangeColorTo(hovered_color);
	if (state == SELECTED || state == DRAGGING) ChangeColorTo(selected_color);
	if (state == LOCKED) ChangeColorTo(locked_color);

	// --- Collider ---
	float2 screenpos = App->renderer3D->active_camera->WorldToScreen({ pos.x, -pos.y, pos.z });
	collider = { (int)(screenpos.x - size2D.x / 2), (int)(screenpos.y - size2D.y / 2), (int)size2D.x, (int)size2D.y };

	// Draw Collider
	if (collider_visible && App->GetAppState() == AppState::EDITOR) //draw only in editor mode
	{
		App->gui->draw_list->AddRect(ImVec2(collider.x, collider.y), ImVec2(collider.x + collider.w, collider.y + collider.h),
			ImU32(ImColor(ImVec4(1.0f, 0.0f, 0.0f, 1.0f))), 0.0f, 0, 1.0f);
	}
}

json ComponentButton::Save() const
{
	json node;

	//node["Resources"]["ResourceTexture"];

	//if (texture)
	//	node["Resources"]["ResourceTexture"] = std::string(texture->GetResourceFile());

	node["Active"] = this->active;
	node["visible"] = std::to_string(visible);
	node["draggable"] = std::to_string(draggable);
	node["interactable"] = std::to_string(interactable);
	node["priority"] = std::to_string(priority);

	node["position2Dx"] = std::to_string(position2D.x);
	node["position2Dy"] = std::to_string(position2D.y);

	node["size2Dx"] = std::to_string(size2D.x);
	node["size2Dy"] = std::to_string(size2D.y);

	node["colliderx"] = std::to_string(collider.x);
	node["collidery"] = std::to_string(collider.y);
	node["colliderw"] = std::to_string(collider.w);
	node["colliderh"] = std::to_string(collider.h);

	if (script)
	{
		node["script"] = std::to_string(script_obj->GetUID());
		node["function"] = func_name;
	}

	node["idle_color.r"] = std::to_string(idle_color.r);
	node["idle_color.g"] = std::to_string(idle_color.g);
	node["idle_color.b"] = std::to_string(idle_color.b);
	node["idle_color.a"] = std::to_string(idle_color.a);

	node["hovered_color.r"] = std::to_string(hovered_color.r);
	node["hovered_color.g"] = std::to_string(hovered_color.g);
	node["hovered_color.b"] = std::to_string(hovered_color.b);
	node["hovered_color.a"] = std::to_string(hovered_color.a);

	node["selected_color.r"] = std::to_string(selected_color.r);
	node["selected_color.g"] = std::to_string(selected_color.g);
	node["selected_color.b"] = std::to_string(selected_color.b);
	node["selected_color.a"] = std::to_string(selected_color.a);

	node["locked_color.r"] = std::to_string(locked_color.r);
	node["locked_color.g"] = std::to_string(locked_color.g);
	node["locked_color.b"] = std::to_string(locked_color.b);
	node["locked_color.a"] = std::to_string(locked_color.a);

	return node;
}

void ComponentButton::Load(json& node)
{
	std::string path = node["Resources"]["ResourceTexture"].is_null() ? "0" : node["Resources"]["ResourceTexture"];
	App->fs->SplitFilePath(path.c_str(), nullptr, &path);
	path = path.substr(0, path.find_last_of("."));

	ResourceTexture* texture = (ResourceTexture*)App->resources->GetResource(std::stoi(path));

	//if (texture == nullptr)
	//	texture = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "DefaultTexture");
	//
	if (texture)
	{
		image->texture = texture;
		texture->AddUser(image->GetContainerGameObject());
	}

	this->active = node["Active"].is_null() ? true : (bool)node["Active"];
	std::string visible_str = node["visible"].is_null() ? "0" : node["visible"];
	std::string draggable_str = node["visible"].is_null() ? "0" : node["draggable"];
	std::string interactable_str = node["visible"].is_null() ? "0" : node["interactable"];
	std::string priority_str = node["priority"].is_null() ? "0" : node["priority"];

	std::string position2Dx = node["position2Dx"].is_null() ? "0" : node["position2Dx"];
	std::string position2Dy = node["position2Dy"].is_null() ? "0" : node["position2Dy"];

	std::string size2Dx = node["size2Dx"].is_null() ? "0" : node["size2Dx"];
	std::string size2Dy = node["size2Dy"].is_null() ? "0" : node["size2Dy"];

	std::string colliderx = node["colliderx"].is_null() ? "0" : node["colliderx"];
	std::string collidery = node["collidery"].is_null() ? "0" : node["collidery"];
	std::string colliderw = node["colliderw"].is_null() ? "0" : node["colliderw"];
	std::string colliderh = node["colliderh"].is_null() ? "0" : node["colliderh"];

	std::string script_str = node["script"].is_null() ? "0" : node["script"];
	std::string function_str = node["function"].is_null() ? "None" : node["function"];

	std::string idle_color_r = node["idle_color.r"].is_null() ? "0" : node["idle_color.r"];
	std::string idle_color_g = node["idle_color.g"].is_null() ? "0" : node["idle_color.g"];
	std::string idle_color_b = node["idle_color.b"].is_null() ? "0" : node["idle_color.b"];
	std::string idle_color_a = node["idle_color.a"].is_null() ? "0" : node["idle_color.a"];

	std::string hovered_color_r = node["hovered_color.r"].is_null() ? "0" : node["hovered_color.r"];
	std::string hovered_color_g = node["hovered_color.g"].is_null() ? "0" : node["hovered_color.g"];
	std::string hovered_color_b = node["hovered_color.b"].is_null() ? "0" : node["hovered_color.b"];
	std::string hovered_color_a = node["hovered_color.a"].is_null() ? "0" : node["hovered_color.a"];

	std::string selected_color_r = node["selected_color.r"].is_null() ? "0" : node["selected_color.r"];
	std::string selected_color_g = node["selected_color.g"].is_null() ? "0" : node["selected_color.g"];
	std::string selected_color_b = node["selected_color.b"].is_null() ? "0" : node["selected_color.b"];
	std::string selected_color_a = node["selected_color.a"].is_null() ? "0" : node["selected_color.a"];

	std::string locked_color_r = node["locked_color.r"].is_null() ? "0" : node["locked_color.r"];
	std::string locked_color_g = node["locked_color.g"].is_null() ? "0" : node["locked_color.g"];
	std::string locked_color_b = node["locked_color.b"].is_null() ? "0" : node["locked_color.b"];
	std::string locked_color_a = node["locked_color.a"].is_null() ? "0" : node["locked_color.a"];

	//-------
	visible = bool(std::stoi(visible_str));
	draggable = bool(std::stoi(draggable_str));
	interactable = bool(std::stoi(interactable_str));
	priority = int(std::stoi(priority_str));

	position2D = float2(std::stof(position2Dx), std::stof(position2Dy));
	size2D = float2(std::stof(size2Dx), std::stof(size2Dy));

	collider.x = int(std::stoi(colliderx));
	collider.y = int(std::stoi(collidery));
	collider.w = int(std::stoi(colliderw));
	collider.h = int(std::stoi(colliderh));

	script_obj = App->scene_manager->currentScene->GetGOWithUID(uint(std::stoi(script_str)));
	if (script_obj)
	{
		script = (ComponentScript*)script_obj->HasComponent(Component::ComponentType::Script);
		 
		if (script != nullptr)
		{
			func_name = function_str.c_str();

			func_list.push_back("None");
			for (uint i = 0; i < script->script_functions.size(); ++i)
				func_list.push_back(script->script_functions[i].name.c_str());

			for (uint i = 0; i < func_list.size(); ++i) //get function pos
			{
				if (strcmp(func_list[i], func_name.c_str()) == 0)
				{
					func_pos = i;
					break;
				}
			}
		}
	}

	idle_color.r = float(std::stof(idle_color_r));
	idle_color.g = float(std::stof(idle_color_g));
	idle_color.b = float(std::stof(idle_color_b));
	idle_color.a = float(std::stof(idle_color_a));

	hovered_color.r = float(std::stof(hovered_color_r));
	hovered_color.g = float(std::stof(hovered_color_g));
	hovered_color.b = float(std::stof(hovered_color_b));
	hovered_color.a = float(std::stof(hovered_color_a));

	selected_color.r = float(std::stof(selected_color_r));
	selected_color.g = float(std::stof(selected_color_g));
	selected_color.b = float(std::stof(selected_color_b));
	selected_color.a = float(std::stof(selected_color_a));

	locked_color.r = float(std::stof(locked_color_r));
	locked_color.g = float(std::stof(locked_color_g));
	locked_color.b = float(std::stof(locked_color_b));
	locked_color.a = float(std::stof(locked_color_a));
}

void ComponentButton::CreateInspectorNode()
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
	ImGui::Checkbox("Visible", &visible);

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
	ImGui::Checkbox("Interactable", &interactable);

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
	ImGui::Checkbox("Draggable", &draggable);
	ImGui::Separator();

	ImGui::SetNextItemWidth(100);
	ImGui::InputInt("Priority", &priority);
	ImGui::Separator();

	// States (Colors)
	ImGui::Separator();
	ImGui::ColorEdit4("##Idle", (float*)&idle_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Idle");

	ImGui::ColorEdit4("##Hovered", (float*)&hovered_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Hovered");

	ImGui::ColorEdit4("##Selected", (float*)&selected_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Selected");

	ImGui::ColorEdit4("##Locked", (float*)&locked_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Locked");

	// Collider
    ImGui::Separator();
	ImGui::Text("Collider");
	//ImGui::SameLine();
	ImGui::Checkbox("Visible##2", &collider_visible);

	// Script
	ImGui::Separator();
	ImGui::Text("Script");
	ImGui::ImageButton(NULL, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 2);

	if (ImGui::BeginDragDropTarget()) //drag and drop
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GO"))
		{
			uint UID = *(const uint*)payload->Data;
			script_obj = App->scene_manager->currentScene->GetGOWithUID(UID);
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	if (script_obj == nullptr)
	{
		ImGui::Text("No Script Loaded");
		script = nullptr;
	}
	else
	{
		ImGui::Text("Name: %s", script_obj->GetName());
		script = (ComponentScript*)script_obj->HasComponent(Component::ComponentType::Script); //get script component
	}

	if (script != nullptr)
	{
		func_list.clear();
		func_list.push_back("None");
		for (uint i = 0; i < script->script_functions.size(); ++i)
			func_list.push_back(script->script_functions[i].name.c_str());

		ImGui::Text("OnClick");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(120.0f);
		if (ImGui::BeginCombo("##OnClick", func_list[func_pos], 0))
		{
			for (int n = 0; n < func_list.size(); ++n)
			{
				bool is_selected = (func_name == func_list[n]);
				if (ImGui::Selectable(func_list[n], is_selected))
				{
					func_name = func_list[n];
					func_pos = n;
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}
	else
		ImGui::Text("GO has no ComponentScript");
}	

void ComponentButton::UpdateState()
{
	if (interactable == true && visible == true)
	{
		if (state != DRAGGING)
		{
			if (App->ui_system->CheckMousePos(&collider)) //check if hovering
			{
				if (App->ui_system->CheckClick(draggable)) //if hovering check if click
				{
					if (draggable == true && (App->ui_system->drag_start.x != App->ui_system->mouse_pos.x || App->ui_system->drag_start.y != App->ui_system->mouse_pos.y)) //if draggable and mouse moves
					{
						ChangeStateTo(DRAGGING);
						position2D.x = App->input->GetMouseX();
						position2D.y = App->input->GetMouseY();
					}
					else
					{
						if (state != SELECTED) //On click action
							OnClick();

						ChangeStateTo(SELECTED);
					}
				}
				else
					ChangeStateTo(HOVERED);
			}
			else
				ChangeStateTo(IDLE); //if stop hovering
		}
		else
		{
			if (!App->ui_system->CheckClick(draggable)) //if stop clicking
				ChangeStateTo(IDLE);
		}
	}
}

void ComponentButton::OnClick()
{
	if (script == nullptr || func_name == "None")
		return;

	uint pos = 0;
	for (uint i = 0; i < func_list.size(); ++i) //get function pos
	{
		if (func_list[i] == func_name)
		{
			pos = i - 1; //-1 because we are adding "None" at the start of func_list
			break;
		}
	}
	App->scripting->CallbackScriptFunction(script, script->script_functions[pos]);
}

void ComponentButton::ChangeColorTo(Color new_color)
{
	image->img_color = { new_color.r, new_color.g,new_color.b,new_color.a };
}

void ComponentButton::SetNullptr()
{
	script_obj = nullptr;
	script = nullptr;
}