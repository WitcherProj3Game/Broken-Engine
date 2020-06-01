#include "BrokenCore.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleUI.h"
#include "ModuleInput.h"
#include "ModuleSceneManager.h"
#include "ModuleGui.h"
#include "ModuleRenderer3D.h"

#include "UI_Element.h"
#include "Component.h"
#include "ComponentCanvas.h"
#include "ComponentButton.h"
#include "ComponentCamera.h"
#include "ResourceFont.h"
#include "ResourceScene.h"

#include "Optick/include/optick.h"

#include <queue>

#ifndef _WIN64
#	pragma comment( lib, "Freetype/libx86/freetype.lib" )
#else
#	pragma comment( lib, "Freetype/libx64/freetype.lib" )
#endif

#include "mmgr/mmgr.h"

using namespace Broken;

ModuleUI::ModuleUI(bool start_enabled) : Module(start_enabled)
{
	name = "UI System";
}

ModuleUI::~ModuleUI()
{
}

bool ModuleUI::Start()
{
	/*std::string font_name = "calibri.ttf";

	LoadFont(font_name);*/

	ui_camera = new ComponentCamera(nullptr);

	ui_camera->frustum.SetPos(float3(0.0f,0.0f, 1.0f));
	ui_camera->SetFOV(60.0f);
	ui_camera->Look({ 0.0f, 0.0f, 0.0f });

	return true;
}

void ModuleUI::LoadFont(std::string& font_name)
{
	//Moved to resourcegFont
}

bool ModuleUI::Init(json& file)
{
	return true;
}

update_status ModuleUI::PreUpdate(float dt)
{
	OPTICK_CATEGORY("Game UI Update", Optick::Category::GameLogic);
	//#include "Optick/include/optick.h"
	OrderElements(); //order canvas

	for (std::unordered_map<uint, GameObject*>::iterator it = App->scene_manager->currentScene->NoStaticGameObjects.begin(); it != App->scene_manager->currentScene->NoStaticGameObjects.end(); ++it)
	{
		if ((*it).second->HasComponent(Component::ComponentType::Button)) //if has button component
		{
			ComponentButton* element = (ComponentButton*)(*it).second->HasComponent(Component::ComponentType::Button); //single component (change when able to have multiple components of same type)
			element->UpdateState(); //update state
		}
	}

	for (std::unordered_map<uint, GameObject*>::iterator it = App->scene_manager->currentScene->StaticGameObjects.begin(); it != App->scene_manager->currentScene->StaticGameObjects.end(); ++it)
	{
		if ((*it).second->HasComponent(Component::ComponentType::Button)) //if has button component
		{
			ComponentButton* element = (ComponentButton*)(*it).second->HasComponent(Component::ComponentType::Button); //single component (change when able to have multiple components of same type)
			element->UpdateState(); //update state
		}
	}

	return UPDATE_CONTINUE;
}

update_status ModuleUI::PostUpdate(float dt)
{
	OPTICK_CATEGORY("Game UI PostUpdate", Optick::Category::GameLogic);
	return UPDATE_CONTINUE;
}

bool ModuleUI::CleanUp()
{
	
	delete ui_camera;

	return true;
}

void ModuleUI::Draw() const
{
	ComponentCamera* cam = App->renderer3D->active_camera;
	App->renderer3D->active_camera = ui_camera; //set ui camera as active camera

	float3 pos = App->renderer3D->active_camera->frustum.Pos();
	float3 up = App->renderer3D->active_camera->frustum.Up();
	float3 front = App->renderer3D->active_camera->frustum.Front();

	App->renderer3D->active_camera->frustum.SetPos({ 0,0,1 });
	App->renderer3D->active_camera->Look({ 0, 0, 0 });

	/////////////////////////////////////
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//glOrtho(Configuration::viewport[Configuration::l], Configuration::viewport[Configuration::r], Configuration::viewport[Configuration::b], -Configuration::viewport[Configuration::t], Configuration::n, Configuration::f);
	glOrtho(viewport[0], viewport[2], viewport[1], viewport[3], 1, -1);

	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Draw UI
	for (int i = 0; i < elements.size(); i++)
	{
		if (elements[i]->GetType() == Component::ComponentType::Canvas && elements[i]->visible)
			elements[i]->Draw();
	}

	glPopAttrib();

	glPopMatrix();
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	App->renderer3D->active_camera->frustum.SetUp(up);
	App->renderer3D->active_camera->frustum.SetFront(front);
	App->renderer3D->active_camera->frustum.SetPos(pos);

	App->renderer3D->active_camera = cam; //reset to previous active camera
}

void ModuleUI::RemoveElement(UI_Element* c)
{
	for (std::vector<UI_Element*>::iterator it = elements.begin(); it != elements.end(); ++it)
	{
		if(*it && *it == c)
		{
			elements.erase(it);
			break;
		}
	}
}

void ModuleUI::Clear()
{
	elements.clear();
}

bool ModuleUI::CheckMousePos(SDL_Rect* collider) // 0,0 is top left corner
{
	if (App->input->GetKey(SDL_SCANCODE_I) == KEY_DOWN) 
		int i = 0;

	mouse_pos.x = App->input->GetMouseX();
	mouse_pos.y = App->input->GetMouseY();

	SDL_Rect MouseCollider = { mouse_pos.x,mouse_pos.y,1,1 };
	return SDL_HasIntersection(&MouseCollider, collider);
}

bool ModuleUI::CheckClick(bool draggable)
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		drag_start.x = App->input->GetMouseX();
		drag_start.y = App->input->GetMouseY();
		return true;
	}

	if (draggable || App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT)
		return true;

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_UP)
		return false;

	return false;
}

void ModuleUI::OrderElements()
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

bool ModuleUI::PrioritySort::operator()(UI_Element* const& node1, UI_Element* const& node2) {
		if (node1->priority > node2->priority)
			return true;
		else
			return false;
}
