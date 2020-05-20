#include "ScriptingInterface.h"
#include "Application.h"
#include "ModuleScripting.h"
#include "ModuleSceneManager.h"
#include "ResourceScene.h"
#include "ScriptData.h"
#include "ComponentProgressBar.h"
#include "ComponentCircularBar.h"
#include "ComponentText.h"
#include "ComponentImage.h"
#include "ComponentButton.h"
#include "GameObject.h"

using namespace Broken;

void ScriptingInterface::MakeUIComponentVisible(const char* comp_type, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar")
		{
			ComponentProgressBar* comp_bar = go->GetComponent<ComponentProgressBar>();
			if(comp_bar)
				comp_bar->visible = true;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementVisible) Alert! Couldn't find Bar Component");
		}
		else if (name == "CircularBar")
		{
			ComponentCircularBar* comp_bar = go->GetComponent<ComponentCircularBar>();
			if (comp_bar)
				comp_bar->visible = true;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementVisible) Alert! Couldn't find Circular Bar Component");
		}
		else if (name == "Text")
		{
			ComponentText* comp_text = go->GetComponent<ComponentText>();
			if (comp_text)
				comp_text->visible = true;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementVisible) Alert! Couldn't find Text Component");
		}
		else if (name == "Image")
		{
			ComponentImage* comp_image = go->GetComponent<ComponentImage>();
			if (comp_image)
				comp_image->visible = true;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementVisible) Alert! Couldn't find Image Component");		
		}
		else if (name == "Button")
		{
			ComponentButton* comp_button = go->GetComponent<ComponentButton>();
			if (comp_button)
				comp_button->visible = true;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementVisible) Alert! Couldn't find Button Component");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (MakeElementVisible) Alert! GameObject with UUID %d has not a %s component", go_UUID, comp_type);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (MakeElementVisible) Alert! Could not find GameObject with UUID %d", go_UUID);
}

void ScriptingInterface::MakeUIComponentInvisible(const char* comp_type, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar")
		{
			ComponentProgressBar* comp_bar = go->GetComponent<ComponentProgressBar>();
			if (comp_bar)
				comp_bar->visible = false;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementInvisible) Alert! Couldn't find Bar Component");
		}
		else if (name == "CircularBar")
		{
			ComponentCircularBar* comp_bar = go->GetComponent<ComponentCircularBar>();
			if (comp_bar)
				comp_bar->visible = false;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementInvisible) Alert! Couldn't find Circular Bar Component");
		}
		else if (name == "Text")
		{
			ComponentText* comp_text = go->GetComponent<ComponentText>();
			if (comp_text)
				comp_text->visible = false;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementInvisible) Alert! Couldn't find Text Component");
		}
		else if (name == "Image")
		{
			ComponentImage* comp_image = go->GetComponent<ComponentImage>();
			if (comp_image)
				comp_image->visible = false;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementInvisible) Alert! Couldn't find Image Component");
		}
		else if (name == "Button")
		{
			ComponentButton* comp_button = go->GetComponent<ComponentButton>();
			if (comp_button)
				comp_button->visible = false;
			else
				ENGINE_CONSOLE_LOG("![Script]: (MakeElementInvisible) Alert! Couldn't find Button Component");
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (MakeElementInvisible) Alert! GameObject with UUID %d has not a %s component", go_UUID, comp_type);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (MakeElementInvisible) Alert! Could not find GameObject with UUID %d", go_UUID);
}

// --- Setters ---
void ScriptingInterface::SetBarPercentage(float percentage, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		ComponentProgressBar* bar = go->GetComponent<ComponentProgressBar>();
		if (bar)
			bar->SetPercentage(percentage);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetUIBarPercentage) Alert! ProgressBar component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetUIBarPercentage) Alert! Could not find GameObject with UUID %d", go_UUID);
}

void ScriptingInterface::SetCircularBarPercentage(float percentage, uint go_UUID)
{
<<<<<<< HEAD
	GameObject *go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);

=======
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
>>>>>>> origin/development
	if (go)
	{
		ComponentCircularBar *bar = go->GetComponent<ComponentCircularBar>();
		if (bar)
			bar->SetPercentage(percentage);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetUICircularBarPercentage) Alert! CircularProgressBar component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetUIBarPercentage) Alert! Could not find GameObject with UUID %d", go_UUID);
}

void ScriptingInterface::SetUIText(const char* text, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		ComponentText* CompText = go->GetComponent<ComponentText>();
		if (CompText)
			CompText->SetText(text);
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetText) Alert! Text Component is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetText) Alert! Could not find GameObject with UUID %d", go_UUID);
}

void ScriptingInterface::SetUITextAndNumber(const char* text, float number, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		ComponentText* CompText = go->GetComponent<ComponentText>();
		if (CompText && text)
		{
			// String streams aren't very performative, but we need them to keep OK the number's decimals
			std::ostringstream ss;
			ss << number;
			CompText->SetText((text + ss.str()).c_str());
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetTextAndNumber) Alert! Text Component or text passed is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetTextAndNumber) Alert! Could not find GameObject with UUID %d", go_UUID);
}

void ScriptingInterface::SetUITextNumber(float number, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		ComponentText* CompText = go->GetComponent<ComponentText>();
		if (CompText)
		{
			// String streams aren't very performative, but we need them to keep OK the number's decimals
			std::ostringstream ss;
			ss << number;
			CompText->SetText(ss.str().c_str());
		}
		else
			ENGINE_CONSOLE_LOG("![Script]: (SetTextNumber) Alert! Text Component or text passed is NULL");
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (SetTextNumber) Alert! Could not find GameObject with UUID %d", go_UUID);
}


// --- Colors ---
void ScriptingInterface::ChangeUIComponentColor(const char* comp_type, float r, float g, float b, float a, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar" || name == "CircularBar")
			ENGINE_CONSOLE_LOG("[Script]: (ChangeUIComponentColor) Bar and CircularBar color cannot be changed from this function. Try ChangeUIBarColor() function instead.");
		else if (name == "Text")
		{
			ComponentText* comp_text = go->GetComponent<ComponentText>();
			if (comp_text)
				comp_text->color = Color(r, g, b, a);
			else
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIComponentColor) Alert! Couldn't find Text Component");
		}
		else if (name == "Image")
		{
			ComponentImage* comp_image = go->GetComponent<ComponentImage>();
			if (comp_image)
				comp_image->img_color = float4(r, g, b, a);
			else
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIComponentColor) Alert! Couldn't find Image Component");
		}
		else if (name == "Button")
		{
			ComponentButton* comp_button = go->GetComponent<ComponentButton>();
			if (comp_button)
				comp_button->ChangeColorTo(Color(r, g, b, a));
			else
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIComponentColor) Alert! Couldn't find Button Component");
		}
		else
			ENGINE_CONSOLE_LOG("[Script]: (ChangeUIComponentColor) Invalid or wrong Component Type passed", go_UUID, comp_type);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (ChangeUIComponentColor) Alert! Could not find GameObject with UUID %d", go_UUID);
}

void ScriptingInterface::ChangeUIComponentAlpha(const char* comp_type, float alpha, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar" || name == "CircularBar")
			ENGINE_CONSOLE_LOG("[Script]: (ChangeUIComponentAlpha) Bar and CircularBar alpha cannot be changed from this function. Try ChangeUIBarAlpha() function instead.");
		else if (name == "Text")
		{
			ComponentText* comp_text = go->GetComponent<ComponentText>();
			if (comp_text)
				comp_text->color.a = alpha;
			else
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIComponentAlpha) Alert! Couldn't find Text Component");
		}
		else if (name == "Image")
		{
			ComponentImage* comp_image = go->GetComponent<ComponentImage>();
			if (comp_image)
				comp_image->img_color.w = alpha;
			else
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIComponentAlpha) Alert! Couldn't find Image Component");
		}
		else if (name == "Button")
		{
			ComponentButton* comp_button = go->GetComponent<ComponentButton>();
			if (comp_button)
			{
				Color new_color = comp_button->GetButtonColor();
				comp_button->ChangeColorTo(Color(new_color.r, new_color.g, new_color.b, alpha));
			}
			else
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIComponentAlpha) Alert! Couldn't find Button Component");
		}
		else
			ENGINE_CONSOLE_LOG("[Script]: (ChangeUIComponentAlpha) Invalid or wrong Component Type passed", go_UUID, comp_type);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (ChangeUIComponentAlpha) Alert! Could not find GameObject with UUID %d", go_UUID);
}

void ScriptingInterface::ChangeUIBarColor(const char* comp_type, bool topBarPlane, float r, float g, float b, float a, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar")
		{
			ComponentProgressBar* comp_bar = go->GetComponent<ComponentProgressBar>();
			if (comp_bar == nullptr)
			{
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIBarColor) Alert! Couldn't find Bar Component");
				return;
			}

			if (topBarPlane)
				comp_bar->SetTopColor(Color(r, g, b, a));
			else
				comp_bar->SetBotColor(Color(r, g, b, a));
		}
		else if (name == "CircularBar")
		{
			ComponentCircularBar* comp_bar = go->GetComponent<ComponentCircularBar>();
			if (comp_bar == nullptr)
			{
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIBarColor) Alert! Couldn't find Bar Component");
				return;
			}

			if (topBarPlane)
				comp_bar->SetTopColor(Color(r, g, b, a));
			else
				comp_bar->SetBotColor(Color(r, g, b, a));
		}
		else
			ENGINE_CONSOLE_LOG("[Script]: (ChangeUIBarColor) Invalid or wrong Component Type passed", go_UUID, comp_type);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (ChangeUIBarColor) Alert! Could not find GameObject with UUID %d", go_UUID);
}

void ScriptingInterface::ChangeUIBarAlpha(const char* comp_type, bool topBarPlane, float alpha, uint go_UUID)
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(go_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar")
		{
			ComponentProgressBar* comp_bar = go->GetComponent<ComponentProgressBar>();
			if (comp_bar == nullptr)
			{
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIBarAlpha) Alert! Couldn't find Bar Component");
				return;
			}

			if (topBarPlane)
			{
				Color new_color = comp_bar->GetTopColor();
				comp_bar->SetTopColor(Color(new_color.r, new_color.g, new_color.b, alpha));
			}
			else
			{
				Color new_color = comp_bar->GetTopColor();
				comp_bar->SetBotColor(Color(new_color.r, new_color.g, new_color.b, alpha));
			}
		}
		else if (name == "CircularBar")
		{
			ComponentCircularBar* comp_bar = go->GetComponent<ComponentCircularBar>();
			if (comp_bar == nullptr)
			{
				ENGINE_CONSOLE_LOG("![Script]: (ChangeUIBarAlpha) Alert! Couldn't find Bar Component");
				return;
			}

			if (topBarPlane)
			{
				Color new_color = comp_bar->GetTopColor();
				comp_bar->SetTopColor(Color(new_color.r, new_color.g, new_color.b, alpha));
			}
			else
			{
				Color new_color = comp_bar->GetTopColor();
				comp_bar->SetBotColor(Color(new_color.r, new_color.g, new_color.b, alpha));
			}
		}
		else
			ENGINE_CONSOLE_LOG("[Script]: (ChangeUIBarAlpha) Invalid or wrong Component Type passed", go_UUID, comp_type);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (ChangeUIBarAlpha) Alert! Could not find GameObject with UUID %d", go_UUID);
}




// --- Getters ---
luabridge::LuaRef ScriptingInterface::GetUIBarColor(const char* comp_type, bool topBarPlane, uint gameobject_UUID, lua_State* L) const
{
	Color color = White;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar")
		{
			ComponentProgressBar* comp_bar = go->GetComponent<ComponentProgressBar>();
			if (comp_bar == nullptr)
				ENGINE_CONSOLE_LOG("![Script]: (GetUIBarColor) Alert! Couldn't find Bar Component");
			else
			{
				if (topBarPlane)
					color = comp_bar->GetTopColor();
				else
					color = comp_bar->GetBotColor();
			}
		}
		else if (name == "CircularBar")
		{
			ComponentCircularBar* comp_bar = go->GetComponent<ComponentCircularBar>();
			if (comp_bar == nullptr)
				ENGINE_CONSOLE_LOG("![Script]: (GetUIBarColor) Alert! Couldn't find Bar Component");
			else
			{
				if (topBarPlane)
					color = comp_bar->GetTopColor();
				else
					color = comp_bar->GetBotColor();
			}
		}
		else
			ENGINE_CONSOLE_LOG("[Script]: (GetUIBarColor) Invalid or wrong Component Type passed", gameobject_UUID, comp_type);
	}

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(color.r);
	table.append(color.g);
	table.append(color.b);
	table.append(color.a);
	return table;
}

luabridge::LuaRef ScriptingInterface::GetUIComponentColor(const char* comp_type, uint gameobject_UUID, lua_State* L) const
{
	float4 color = float4::zero;
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar" || name == "CircularBar")
			ENGINE_CONSOLE_LOG("[Script]: (GetUIComponentColor) Bar and CircularBar color cannot be gotten from this function. Try GetUIBarColor() function instead.");
		else if (name == "Text")
		{
			ComponentText* comp_text = go->GetComponent<ComponentText>();
			if (comp_text)
				color = float4(comp_text->color.r, comp_text->color.g, comp_text->color.b, comp_text->color.a);
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetUIComponentColor) Alert! Couldn't find Text Component");
		}
		else if (name == "Image")
		{
			ComponentImage* comp_image = go->GetComponent<ComponentImage>();
			if (comp_image)
				color = comp_image->img_color;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetUIComponentColor) Alert! Couldn't find Image Component");
		}
		else if (name == "Button")
		{
			ComponentButton* comp_button = go->GetComponent<ComponentButton>();
			if (comp_button)
				color = float4(comp_button->GetButtonColor().r, comp_button->GetButtonColor().g, comp_button->GetButtonColor().b, comp_button->GetButtonColor().a);
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetUIComponentColor) Alert! Couldn't find Button Component");
		}
		else
			ENGINE_CONSOLE_LOG("[Script]: (GetUIComponentColor) Invalid or wrong Component Type passed", gameobject_UUID, comp_type);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetUIComponentColor) Could not find GameObject with UUID %d", gameobject_UUID);

	luabridge::LuaRef table = luabridge::newTable(L);
	table.append(color.x);
	table.append(color.y);
	table.append(color.z);
	table.append(color.w);
	return table;
}

float ScriptingInterface::GetUIBarAlpha(const char* comp_type, bool topBarPlane, uint gameobject_UUID) const
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar")
		{
			ComponentProgressBar* comp_bar = go->GetComponent<ComponentProgressBar>();
			if (comp_bar == nullptr)
			{
				ENGINE_CONSOLE_LOG("![Script]: (GetUIBarAlpha) Alert! Couldn't find Bar Component");
				return -1.0f;
			}

			if (topBarPlane)
				return comp_bar->GetTopColor().a;
			else
				return comp_bar->GetBotColor().a;
		}
		else if (name == "CircularBar")
		{
			ComponentCircularBar* comp_bar = go->GetComponent<ComponentCircularBar>();
			if (comp_bar == nullptr)
			{
				ENGINE_CONSOLE_LOG("![Script]: (GetUIBarAlpha) Alert! Couldn't find Bar Component");
				return -1.0f;
			}

			if (topBarPlane)
				return comp_bar->GetTopColor().a;
			else
				return comp_bar->GetBotColor().a;
		}
		else
			ENGINE_CONSOLE_LOG("[Script]: (GetUIBarAlpha) Invalid or wrong Component Type passed", gameobject_UUID, comp_type);
	}

	return -1.0f;
}


float ScriptingInterface::GetUIComponentAlpha(const char* comp_type, uint gameobject_UUID) const
{
	GameObject* go = App->scene_manager->currentScene->GetGOWithUID(gameobject_UUID);
	if (go)
	{
		std::string name = comp_type;
		if (name == "Bar" || name == "CircularBar")
			ENGINE_CONSOLE_LOG("[Script]: (GetUIComponentAlpha) Bar and CircularBar color cannot be gotten from this function. Try GetUIBarAlpha() function instead.");
		else if (name == "Text")
		{
			ComponentText* comp_text = go->GetComponent<ComponentText>();
			if (comp_text)
				return comp_text->color.a;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetUIComponentAlpha) Alert! Couldn't find Text Component");
		}
		else if (name == "Image")
		{
			ComponentImage* comp_image = go->GetComponent<ComponentImage>();
			if (comp_image)
				comp_image->img_color.w;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetUIComponentAlpha) Alert! Couldn't find Image Component");
		}
		else if (name == "Button")
		{
			ComponentButton* comp_button = go->GetComponent<ComponentButton>();
			if (comp_button)
				comp_button->GetButtonColor().a;
			else
				ENGINE_CONSOLE_LOG("![Script]: (GetUIComponentAlpha) Alert! Couldn't find Button Component");
		}
		else
			ENGINE_CONSOLE_LOG("[Script]: (GetUIComponentAlpha) Invalid or wrong Component Type passed", gameobject_UUID, comp_type);
	}
	else
		ENGINE_CONSOLE_LOG("![Script]: (GetUIComponentAlpha) Alert! Could not find GameObject with UUID %d", gameobject_UUID);

	return -1.0f;
}