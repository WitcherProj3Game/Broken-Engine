#include "ComponentImage.h"
#include "GameObject.h"
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

#include "Component.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "ComponentCanvas.h"
//#include "ModuleWindow.h"
#include "ModuleTimeManager.h"

#include "ResourceShader.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"

#include "Imgui/imgui.h"
#include "mmgr/mmgr.h"

using namespace Broken;

ComponentImage::ComponentImage(GameObject* gameObject) : UI_Element(gameObject, Component::ComponentType::Image)
{
	name = "Image";
	visible = true;

	if (GO->parent && GO->parent->HasComponent(Component::ComponentType::Canvas))
	{
		canvas = (ComponentCanvas*)GO->parent->GetComponent<ComponentCanvas>();

		if (canvas)
			canvas->AddElement(this);
	}

	plane = new ResourceMesh(App->GetRandom().Int(), "ImageMesh");
	App->scene_manager->CreatePlane(1, 1, 1, plane);
	plane->LoadInMemory();
	//texture = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "DefaultTexture");
}

ComponentImage::~ComponentImage()
{
	if (is3D)
		App->ui_system->RemoveElement3D(this);

	if (texture)
	{
		texture->RemoveUser(GO);
		texture->Release();
	}

	if (canvas)
		canvas->RemoveElement(this);

	plane->FreeMemory();
	delete plane;

	for (int i = 0; i < animation_frames.size(); i++)
	{
		animation_frames[i]->FreeMemory();
		delete(animation_frames[i]);
	}
	animation_frames.clear();
}

void ComponentImage::Update()
{
	if (GO->parent != nullptr && canvas == nullptr && GO->parent->HasComponent(Component::ComponentType::Canvas))
	{
		canvas = (ComponentCanvas*)GO->parent->GetComponent<ComponentCanvas>();
		canvas->AddElement(this);
	}
	else if (GO->parent && !GO->parent->HasComponent(Component::ComponentType::Canvas) && canvas)
		canvas = nullptr;

	if (fullscreen)
		size2D = { App->gui->sceneWidth + 30, App->gui->sceneHeight + 30 };

	if (to_delete)
		this->GetContainerGameObject()->RemoveComponent(this);

	else if (!controlled_by_sripting &&
		((App->GetAppState() == AppState::EDITOR && play_animation_editor) ||
			App->GetAppState() == AppState::PLAY) &&
			((!loop && !animation_finished) || (loop)))
	{
		int span = (end_frame - start_frame);
		if (span > 0)
		{
			time += App->time->GetRealTimeDt();
			if (animation && animation_created && time >= (animation_time / ((span > 0) ? span : 1)))
			{
				NextAnimationFrame();
			}
		}
	}
}


// Scripting function
void ComponentImage::PlayAnimation()
{
	time = 0;
	current_frame = start_frame;
	animation_finished = false;
}

// Scripting function
void ComponentImage::NextAnimationFrame()
{
	if (loop)
	{
		current_frame++;
		if (current_frame > end_frame - 1) current_frame = start_frame;
		else if (current_frame < start_frame) current_frame = start_frame;
		//current_frame = (((current_frame + 1) % (end_frame == 0? 1 : end_frame))+start_frame)%animation_frames.size();
		time = 0;

	}
	else if (current_frame < end_frame - 1)
	{
		current_frame++;
		time = 0;
	}
	else
	{
		animation_finished = true;
	}
}

void ComponentImage::Draw()
{
	// --- Frame image with camera ---
	float4x4 transform;
	uint shaderID = 0;
	if (!is3D)
	{

		shaderID = App->renderer3D->UI_Shader->ID;

		float nearp = App->renderer3D->active_camera->GetNearPlane();
		float3 pos = { GetFinalPosition() , nearp + 0.026f };
		float3 size = { size2D.x / App->gui->sceneWidth, size2D.y / App->gui->sceneHeight, 1.0f };
		transform = transform.FromTRS(pos, Quat::identity, size);
	}
	else
	{
		//ComponentTransform* t = GO->GetComponent<ComponentTransform>();
		//float3 pos = t->GetGlobalPosition();
		//float3 size = t->GetScale();
		//Quat rot = t->GetQuaternionRotation();
		//transform = transform.FromTRS(pos, rot, size);
		transform = GO->GetComponent<ComponentTransform>()->GetGlobalTransform();
		shaderID = App->renderer3D->defaultShader->ID;
	}

	// --- Set Uniforms ---
	//uint shaderID = App->renderer3D->defaultShader->ID;
	glUseProgram(shaderID);

	GLint modelLoc = glGetUniformLocation(shaderID, "u_Model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transform.Transposed().ptr());

	GLint viewLoc = glGetUniformLocation(shaderID, "u_View");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLViewMatrix().ptr());

	GLint projectLoc = glGetUniformLocation(shaderID, "u_Proj");
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLProjectionMatrix().ptr());

	// --- Color & Texturing ---
	glUniform1f(glGetUniformLocation(shaderID, "u_GammaCorrection"), App->renderer3D->GetGammaCorrection());
	glUniform4f(glGetUniformLocation(shaderID, "u_Color"), img_color.x, img_color.y, img_color.z, img_color.w);
	int TextureLocation = glGetUniformLocation(shaderID, "u_UseTextures");
	glUniform1i(glGetUniformLocation(shaderID, "u_HasTransparencies"), 1);
	
	if (texture)
	{
		glUniform1i(TextureLocation, 1);
		glUniform1i(glGetUniformLocation(shaderID, "u_AlbedoTexture"), 1);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, texture->GetTexID());
	}
	else
		glUniform1i(TextureLocation, 0);

	ResourceMesh* drawing_plane = (animation && animation_created) ? animation_frames[current_frame] : plane;

	// --- Draw plane with given texture ---
	glBindVertexArray(drawing_plane->VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawing_plane->EBO);
	glDrawElements(GL_TRIANGLES, drawing_plane->IndicesSize, GL_UNSIGNED_INT, NULL);  // render primitives from array data

	glUniform1i(TextureLocation, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0); // Stop using buffer (texture)
	glActiveTexture(GL_TEXTURE0);
}

void ComponentImage::CreateAnimation(uint w, uint h) {

	if (texture)
	{
		int width = texture->Texture_width / w;
		int height = texture->Texture_height / h;

		for (int i = 0; i < animation_frames.size(); i++)
		{
			animation_frames[i]->FreeMemory();
			delete(animation_frames[i]);
		}
		animation_frames.clear();

		for (int j = h - 1; j >= 0; --j) {
			for (int i = 0; i < w; ++i) {

				ResourceMesh* plane = new ResourceMesh(App->GetRandom().Int(), "FrameMesh");
				App->scene_manager->CreatePlane(1, 1, 1, plane);

				//Set Texture Coords
				plane->vertices[0].texCoord[0] = i * width / (float)texture->Texture_width;
				plane->vertices[0].texCoord[1] = j * height / (float)texture->Texture_height;
				plane->vertices[2].texCoord[0] = ((i * width) + width) / (float)texture->Texture_width;
				plane->vertices[2].texCoord[1] = j * height / (float)texture->Texture_height;
				plane->vertices[1].texCoord[0] = i * width / (float)texture->Texture_width;
				plane->vertices[1].texCoord[1] = ((j * height) + height) / (float)texture->Texture_height;
				plane->vertices[3].texCoord[0] = ((i * width) + width) / (float)texture->Texture_width;
				plane->vertices[3].texCoord[1] = ((j * height) + height) / (float)texture->Texture_height;

				//Update Buffer
				plane->LoadInMemory();
				animation_frames.push_back(plane);

			}
		}
		animation_created = true;
		PlayAnimation();
	}
}
float2 ComponentImage::GetParentPos()
{
	if (canvas)
		return canvas->position2D;

	return float2::zero;
}

float2 ComponentImage::GetFinalPosition()
{
	float2 parent_pos = GetParentPos();
	// origin TOP LEFT is -xhalfwidth +yhalfheight
	float scenex = App->gui->sceneWidth / 2 - size2D.x / 2;
	float sceney = App->gui->sceneHeight / 2 - size2D.y / 2;

	float2 pos = position2D + parent_pos;

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
	float2 final_pos = { pos.x / App->gui->sceneWidth,
					pos.y / App->gui->sceneHeight };

	return final_pos;
}
json ComponentImage::Save() const
{
	json node;
	node["Active"] = this->active;
	node["visible"] = std::to_string(visible);
	node["priority"] = std::to_string(priority);
	node["fullscreen"] = fullscreen;
	node["anchor"] = (int)anchor_type;

	node["Resources"]["ResourceTexture"];

	if (texture)
		node["Resources"]["ResourceTexture"] = std::string(texture->GetResourceFile());

	node["position2Dx"] = std::to_string(position2D.x);
	node["position2Dy"] = std::to_string(position2D.y);

	node["size2Dx"] = std::to_string(size2D.x);
	node["size2Dy"] = std::to_string(size2D.y);

	node["ColorR"] = std::to_string(img_color.x);
	node["ColorG"] = std::to_string(img_color.y);
	node["ColorB"] = std::to_string(img_color.z);
	node["ColorA"] = std::to_string(img_color.w);

	node["NumberFrames"] = std::to_string(animation_frames.size());

	if (animation)
	{
		node["TileDivisionX"] = std::to_string(tile_division_x);
		node["TileDivisionY"] = std::to_string(tile_division_y);

		node["StartFrame"] = std::to_string(start_frame);
		node["EndFrame"] = std::to_string(end_frame);

		node["Loop"] = (loop) ? "1" : "0";
		node["ScriptControl"] = (controlled_by_sripting) ? "1" : "0";
		node["PlayInEditor"] = (play_animation_editor) ? "1" : "0";

		node["AnimationTime"] = std::to_string(animation_time);
	}

	node["MantainAspect"] = (resize) ? "1" : "0";
	node["IsProgressBar"] = (is_progress_bar) ? "1" : "0";
	node["is3d"] = (is3D) ? "1" : "0";

	return node;
}

void ComponentImage::Load(json& node)
{
	this->active = node.contains("Active") ? (bool)node["Active"] : true;
	std::string visible_str = node.contains("visible") ? node["visible"] : "0";
	std::string priority_str = node.contains("priority") ? node["priority"] : "0";
	visible = bool(std::stoi(visible_str));
	priority = int(std::stoi(priority_str));

	anchor_type = node.contains("anchor") ? (UI_Anchor)node["anchor"].get<int>() : UI_Anchor::NONE;

	fullscreen = node.find("fullscreen") == node.end() ? false : node["fullscreen"].get<bool>();

	std::string path = node["Resources"]["ResourceTexture"].is_null() ? "0" : node["Resources"]["ResourceTexture"];
	App->fs->SplitFilePath(path.c_str(), nullptr, &path);
	path = path.substr(0, path.find_last_of("."));

	texture = (ResourceTexture*)App->resources->GetResource(std::stoi(path));

	if (texture)
		texture->AddUser(GO);

	std::string position2Dx = node.contains("position2Dx") ? node["position2Dx"] : "0";
	std::string position2Dy = node.contains("position2Dy") ? node["position2Dy"] : "0";

	std::string size2Dx = node.contains("size2Dx") ? node["size2Dx"] : "0";
	std::string size2Dy = node.contains("size2Dy") ? node["size2Dy"] : "0";

	std::string str_colorR = node.contains("ColorR") ? node["ColorR"] : "1";
	std::string str_colorG = node.contains("ColorG") ? node["ColorG"] : "1";
	std::string str_colorB = node.contains("ColorB") ? node["ColorB"] : "1";
	std::string str_colorA = node.contains("ColorA") ? node["ColorA"] : "1";

	img_color = float4(std::stof(str_colorR), std::stof(str_colorG), std::stof(str_colorB), std::stof(str_colorA));
	position2D = float2(std::stof(position2Dx), std::stof(position2Dy));
	size2D = float2(std::stof(size2Dx), std::stof(size2Dy));

	// Animation ------------

	std::string frames = node.contains("NumberFrames") ? node["NumberFrames"] : "0";

	int number_frames = std::stoi(frames);

	animation = number_frames > 0;

	if (animation)
	{
		std::string tileX = node.contains("TileDivisionX") ? node["TileDivisionX"] : "0";
		std::string tileY = node.contains("TileDivisionY") ? node["TileDivisionY"] : "0";

		tile_division_x = std::stoi(tileX);
		tile_division_y = std::stoi(tileY);

		std::string start_f = node.contains("StartFrame") ? node["StartFrame"] : "0";
		std::string end_f = node.contains("EndFrame") ? node["EndFrame"] : "0";

		start_frame = std::stoi(start_f);
		end_frame = std::stoi(end_f);

		std::string s_loop = node.contains("Loop") ? node["Loop"] : "0";

		loop = std::stoi(s_loop) == 1;

		std::string anim_time = node.contains("AnimationTime") ? node["AnimationTime"] : "1";

		animation_time = std::stof(anim_time);

		std::string playEditor = node.contains("PlayInEditor") ? node["PlayInEditor"] : "0";

		std::string scriptControl = node.contains("ScriptControl") ? node["ScriptControl"] : "0";

		play_animation_editor = std::stoi(playEditor) == 1;

		controlled_by_sripting = std::stoi(scriptControl) == 1;

		CreateAnimation(tile_division_x, tile_division_y);

	}

	std::string aspect = node.contains("MantainAspect") ? node["MantainAspect"] : "0";
	resize = std::stoi(aspect) == 1;

	std::string bar = node.contains("IsProgressBar") ? node["IsProgressBar"] : "0";
	is_progress_bar = std::stoi(bar);

	std::string isUI3D = node.contains("is3d") ? node["is3d"] : "0";
	is3D = std::stoi(isUI3D);

	if (is3D)
		App->ui_system->AddElement3D(this);
}

void ComponentImage::CreateInspectorNode()
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
	ImGui::Checkbox("Visible", &visible);
	ImGui::Separator();

	if (ImGui::Checkbox("Is 3D", &is3D))
	{
		if (is3D) {
			App->ui_system->AddElement3D(this);
			if (canvas)
				canvas->RemoveElement(this);
		}
		else
		{
			App->ui_system->RemoveElement3D(this);
			if (canvas)
				canvas->AddElement(this);
		}
	}

	ImGui::Separator();

	int anchor = (int)anchor_type;
	if (ImGui::Combo("Anchor", &anchor, "TOP LEFT\0TOP\0TOP RIGHT\0LEFT\0CENTER\0RIGHT\0BOTTOM LEFT\0BOTTOM\0BOTTOM RIGHT\0NONE\0\0"))
	{
		anchor_type = (UI_Anchor)anchor;
	}

	ImGui::SetNextItemWidth(100);
	ImGui::InputInt("Priority", &priority);
	ImGui::Separator();

	ImGui::Checkbox("Fullscreen", &fullscreen);

	if (is_progress_bar == false)
	{
		// Size
		ImGui::Text("Size:    ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(60);
		if (ImGui::DragFloat("x##imagesize", &size2D.x, 1.0f, 0.0f, INFINITY) && resize)
		{
			if (texture)
			{
				if (texture->Texture_height != 0 && texture->Texture_width != 0)
				{
					if (texture->Texture_width <= texture->Texture_height)
						size2D.y = size2D.x * (float(texture->Texture_width) / float(texture->Texture_height));
					else
						size2D.y = size2D.x * (float(texture->Texture_height) / float(texture->Texture_width));
				}
			}
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(60);
		if (ImGui::DragFloat("y##imagesize", &size2D.y, 1.0f, 0.0f, INFINITY) && resize)
		{
			if (texture)
			{
				if (texture->Texture_height != 0 && texture->Texture_width != 0)
				{
					if (texture->Texture_width >= texture->Texture_height)
						size2D.x = size2D.y * (float(texture->Texture_width) / float(texture->Texture_height));
					else
						size2D.x = size2D.y * (float(texture->Texture_height) / float(texture->Texture_width));
				}
			}
		}

		// Position
		ImGui::Text("Position:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(60);
		ImGui::DragFloat("x##imageposition", &position2D.x);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(60);
		ImGui::DragFloat("y##imageposition", &position2D.y);

	}

	// Rotation
	//ImGui::Text("Rotation:");
	//ImGui::SameLine();
	//ImGui::SetNextItemWidth(60);
	//ImGui::DragFloat("##imagerotation", &rotation2D);

	// ------------------------------------------

	// Image
	//ImGui::Separator();

	ImGui::Separator();
	ImGui::Checkbox("Animation", &animation);

	if (animation)
	{
		int tmpX = tile_division_x;
		int tmpY = tile_division_y;
		ImGui::Checkbox("Play in editor", &play_animation_editor);
		ImGui::Checkbox("Script control", &controlled_by_sripting);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Animation is not updated if this option is enabled,\neven with PlayAnimation(). Enable this ONLY if \nyou want per frame control");

		// Debug purposes
		bool b = animation_finished;
		ImGui::Checkbox("Finished", &b);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Read only");

		ImGui::SameLine();
		if (ImGui::Button("Play"))
		{
			PlayAnimation();
		}
		// -------------
		ImGui::Text("Tiles:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragInt("X", &tile_division_x, 1, 1, (texture) ? texture->Texture_width : 100);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragInt("Y", &tile_division_y, 1, 1, (texture) ? texture->Texture_height : 100);
		ImGui::Text("Start Frame:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (ImGui::DragInt("##sframe", &start_frame, 1, 0, end_frame))
		{
			start_frame = (end_frame <= start_frame) ? end_frame : start_frame;
			if (start_frame < 0) start_frame = 0;
			current_frame = start_frame;
		}
		ImGui::SameLine();
		ImGui::Text("End Frame:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		if (ImGui::DragInt("##eframe", &end_frame, 1, 0, animation_frames.size() - 1))
		{
			end_frame = (animation_frames.size() - 1 <= end_frame) ? animation_frames.size() - 1 : end_frame;
			if (end_frame < 0) end_frame = 0;
		}

		ImGui::Text("Animation duration:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
		ImGui::DragFloat("##Animduration", &animation_time, 0.01f, 0, 200);

		ImGui::Checkbox("Loop", &loop);


		if (ImGui::Button("Create animation"))
		{
			CreateAnimation(tile_division_x, tile_division_y);
		}



	}

	ImGui::Text("Image");



	if (texture == nullptr)
		ImGui::Image((ImTextureID)App->textures->GetDefaultTextureID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0)); //default texture
	else
		ImGui::Image((ImTextureID)texture->GetTexID(), ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0)); //loaded texture

	//drag and drop
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource"))
		{
			uint UID = *(const uint*)payload->Data;
			Resource* resource = App->resources->GetResource(UID, false);

			if (resource && resource->GetType() == Resource::ResourceType::TEXTURE)
			{
				if (texture)
					texture->Release();

				texture = (ResourceTexture*)App->resources->GetResource(UID);

				if (resize && texture)
					size2D = float2(texture->Texture_width, texture->Texture_height);
			}
		}
		ImGui::EndDragDropTarget();
	}
	if (ImGui::Button("Delete Texture"))
	{
		if (texture)
		{
			texture->Release();
			texture = nullptr;
		}
	}

	// Color
	ImGui::Separator();
	ImGui::ColorEdit4("##IMGColor", (float*)&img_color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Color");

	// Aspect Ratio
	ImGui::Checkbox("Maintain Aspect Ratio", &resize);

	ImGui::Separator();
}