 #include "ComponentCanvas.h"
#include "GameObject.h"
#include "Application.h"
#include "ModuleResourceManager.h"
#include "ModuleUI.h"
#include "ModuleTextures.h"
#include "PanelScene.h"
#include "ModuleRenderer3D.h"
#include "ModuleGui.h"

#include "ComponentText.h"
#include "ComponentImage.h"
#include "ComponentButton.h"
//#include "ComponentCheckBox.h"
//#include "ComponentInputText.h"
#include "ComponentProgressBar.h"
#include "ComponentCircularBar.h"

#include <queue>
#include "Imgui/imgui.h"
#include "mmgr/mmgr.h"

using namespace Broken;

ComponentCanvas::ComponentCanvas(GameObject* gameObject) : UI_Element(gameObject, Component::ComponentType::Canvas)
{
	name = "Canvas";
	visible = true;
	App->ui_system->AddElement(this);

	if (GO->parent && GO->parent->HasComponent(Component::ComponentType::Canvas))
	{
		canvas = (ComponentCanvas*)GO->parent->GetComponent<ComponentCanvas>();

		if (canvas)
			canvas->AddElement(this);
	}
  }

ComponentCanvas::~ComponentCanvas()
{
	App->ui_system->RemoveElement(this);

	//for (uint i = 0; i < elements.size(); ++i)
	//	elements[i]->to_delete = true;
	//elements.clear();

	if (canvas)
		canvas->RemoveElement(this);

	for (int i = 0; i < elements.size(); i++)
	{
		elements[i]->canvas = nullptr;
	}

	elements.clear();
}

void ComponentCanvas::Update()
{
	OrderCanvas(); //order elements inside canvas by priority

	position2D = GetFinalPosition();

	if (GO->parent != nullptr && canvas == nullptr && GO->parent->HasComponent(Component::ComponentType::Canvas))
	{
		canvas = (ComponentCanvas*)GO->parent->GetComponent<ComponentCanvas>();
		canvas->AddElement(this);
	}
	else if (GO->parent && !GO->parent->HasComponent(Component::ComponentType::Canvas) && canvas)
		canvas = nullptr;

	if (to_delete)
		this->GetContainerGameObject()->RemoveComponent(this);
}

void ComponentCanvas::Draw()
{
	if (this->active)
	{
		for (int i = 0; i < elements.size(); i++)
		{
			if (elements[i]->GetActive() && elements[i]->visible)
				elements[i]->Draw();
		}
	}
}

void ComponentCanvas::RemoveElement(UI_Element* elem)
{
	if (!elements.empty())
	{
		for (std::vector<UI_Element*>::iterator it = elements.begin(); it != elements.end(); ++it)
		{
			if (*it && *it == elem)
			{
				elements.erase(it);
				break;
			}
		}
	}
}

float2 ComponentCanvas::GetParentPos()
{
	if (canvas)
		return canvas->position2D + canvas->GetParentPos();

	return float2::zero;
}

float2 ComponentCanvas::GetFinalPosition()
{
	float2 parent_pos = GetParentPos();
	// origin TOP LEFT is -xhalfwidth +yhalfheight

	float scenex = App->gui->sceneWidth / 2 - size2D.x / 2;
	float sceney = App->gui->sceneHeight / 2 - size2D.y / 2;

	float2 pos = position2DLocal + parent_pos;

	switch (anchor_type)
	{
	case UI_Element::UI_Anchor::TOP_LEFT:
		pos.x -= scenex;
		pos.y += sceney;
		break;
	case UI_Element::UI_Anchor::TOP:
		pos.y += sceney;
		break;
	case UI_Element::UI_Anchor::TOP_RIGHT:
		pos.x += scenex;
		pos.y += sceney;
		break;
	case UI_Element::UI_Anchor::LEFT:
		pos.x -= scenex;
		break;
	case UI_Element::UI_Anchor::RIGHT:
		pos.x += scenex;
		break;
	case UI_Element::UI_Anchor::BOTTOM_LEFT:
		pos.x -= scenex;
		pos.y -= sceney;
		break;
	case UI_Element::UI_Anchor::BOTTOM:
		pos.y -= sceney;
		break;
	case UI_Element::UI_Anchor::BOTTOM_RIGHT:
		pos.x += scenex;
		pos.y -= sceney;
		break;

	default:
		// NONE AND CENTER GOES HERE -> NOTHING TO DO
		break;
	}

	return pos;
}

json ComponentCanvas::Save() const
{
	json node;
	node["Active"] = this->active;
	node["visible"] = std::to_string(visible);
	node["priority"] = std::to_string(priority);
	node["anchor"] = (int)anchor_type;

	node["position2DxLocal"] = std::to_string(position2DLocal.x);
	node["position2DyLocal"] = std::to_string(position2DLocal.y);

	return node;
}

void ComponentCanvas::Load(json& node)
{
	this->active = node.contains("Active") ? (bool)node["Active"] : true;
	std::string visible_str = node.contains("visible") ? node["visible"] : "0";
	std::string priority_str = node.contains("priority") ? node["priority"] : "0";
	visible = bool(std::stoi(visible_str));
	priority = int(std::stoi(priority_str));
	anchor_type = node.contains("anchor") ? (UI_Anchor)node["anchor"].get<int>() : UI_Anchor::NONE;

	std::string position2Dx = node.contains("position2DxLocal") ? node["position2DxLocal"] : "0";
	std::string position2Dy = node.contains("position2DyLocal") ? node["position2DyLocal"] : "0";

	position2DLocal = float2(std::stof(position2Dx), std::stof(position2Dy));

}

void ComponentCanvas::CreateInspectorNode()
{

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
	ImGui::Checkbox("Visible", &visible);
	ImGui::Separator();
	ImGui::SetNextItemWidth(100);
	ImGui::InputInt("Priority", &priority);
	ImGui::Separator();

	int anchor = (int)anchor_type;
	if (ImGui::Combo("Anchor", &anchor, "TOP LEFT\0TOP\0TOP RIGHT\0LEFT\0CENTER\0RIGHT\0BOTTOM LEFT\0BOTTOM\0BOTTOM RIGHT\0NONE\0\0"))
	{
		anchor_type = (UI_Anchor)anchor;
	}

	// Position
	ImGui::Text("Position:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("x##canvasposition", &position2DLocal.x);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("y##canvasposition", &position2DLocal.y);


}



void ComponentCanvas::OrderCanvas()
{
	std::priority_queue<UI_Element*, std::vector<UI_Element*>, PrioritySort> ListOrder;

	for (UI_Element* node : elements)
		ListOrder.push(node);

	elements.clear();

	while (ListOrder.empty() == false)
	{
		elements.push_back(ListOrder.top());
		ListOrder.pop();
	}
}

bool ComponentCanvas::PrioritySort::operator()(UI_Element* const& node1, UI_Element* const& node2) {
	if (node1->priority > node2->priority)
		return true;
	else
		return false;
}