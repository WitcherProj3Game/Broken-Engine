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
	if (texture)
	{
		texture->Release();
		texture->RemoveUser(GO);
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
	float nearp = App->renderer3D->active_camera->GetNearPlane();
	float3 pos = { position2D.x / App->gui->sceneWidth, position2D.y / App->gui->sceneHeight, nearp + 0.026f };
	float3 size = { size2D.x / App->gui->sceneWidth, size2D.y / App->gui->sceneHeight, 1.0f };
	float4x4 transform = transform.FromTRS(pos, Quat::identity, size);

	// --- Set Uniforms ---
	uint shaderID = App->renderer3D->UI_Shader->ID;
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

json ComponentImage::Save() const
{
	json node;
	node["Active"] = this->active;
	node["visible"] = std::to_string(visible);
	node["priority"] = std::to_string(priority);
	node["fullscreen"] = fullscreen;

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

	return node;
}

void ComponentImage::Load(json& node)
{
	this->active = node["Active"].is_null() ? true : (bool)node["Active"];
	std::string visible_str = node["visible"].is_null() ? "0" : node["visible"];
	std::string priority_str = node["priority"].is_null() ? "0" : node["priority"];
	visible = bool(std::stoi(visible_str));
	priority = int(std::stoi(priority_str));

	fullscreen = node.find("fullscreen") == node.end() ? false : node["fullscreen"].get<bool>();

	std::string path = node["Resources"]["ResourceTexture"].is_null() ? "0" : node["Resources"]["ResourceTexture"];
	App->fs->SplitFilePath(path.c_str(), nullptr, &path);
	path = path.substr(0, path.find_last_of("."));

	texture = (ResourceTexture*)App->resources->GetResource(std::stoi(path));

	if (texture)
		texture->AddUser(GO);

	std::string position2Dx = node["position2Dx"].is_null() ? "0" : node["position2Dx"];
	std::string position2Dy = node["position2Dy"].is_null() ? "0" : node["position2Dy"];

	std::string size2Dx = node["size2Dx"].is_null() ? "0" : node["size2Dx"];
	std::string size2Dy = node["size2Dy"].is_null() ? "0" : node["size2Dy"];

	std::string str_colorR = node["ColorR"].is_null() ? "1" : node["ColorR"];
	std::string str_colorG = node["ColorG"].is_null() ? "1" : node["ColorG"];
	std::string str_colorB = node["ColorB"].is_null() ? "1" : node["ColorB"];
	std::string str_colorA = node["ColorA"].is_null() ? "1" : node["ColorA"];

	img_color = float4(std::stof(str_colorR), std::stof(str_colorG), std::stof(str_colorB), std::stof(str_colorA));
	position2D = float2(std::stof(position2Dx), std::stof(position2Dy));
	size2D = float2(std::stof(size2Dx), std::stof(size2Dy));

	// Animation ------------

	std::string frames = node["NumberFrames"].is_null() ? "0" : node["NumberFrames"];

	int number_frames = std::stoi(frames);

	animation = number_frames > 0;

	if (animation)
	{
		std::string tileX = node["TileDivisionX"].is_null() ? "0" : node["TileDivisionX"];
		std::string tileY = node["TileDivisionY"].is_null() ? "0" : node["TileDivisionY"];

		tile_division_x = std::stoi(tileX);
		tile_division_y = std::stoi(tileY);

		std::string start_f = node["StartFrame"].is_null() ? "0" : node["StartFrame"];
		std::string end_f = node["EndFrame"].is_null() ? "0" : node["EndFrame"];

		start_frame = std::stoi(start_f);
		end_frame = std::stoi(end_f);

		std::string s_loop = node["Loop"].is_null() ? "0" : node["Loop"];

		loop = std::stoi(s_loop) == 1;

		std::string anim_time = node["AnimationTime"].is_null() ? "1" : node["AnimationTime"];

		animation_time = std::stof(anim_time);

		std::string playEditor = node["PlayInEditor"].is_null() ? "0" : node["PlayInEditor"];

		std::string scriptControl = node["ScriptControl"].is_null() ? "0" : node["ScriptControl"];

		play_animation_editor = std::stoi(playEditor) == 1;

		controlled_by_sripting = std::stoi(scriptControl) == 1;

		CreateAnimation(tile_division_x, tile_division_y);

	}

	std::string aspect = node["MantainAspect"].is_null() ? "0" : node["MantainAspect"];
	resize = std::stoi(aspect) == 1;

	std::string bar = node["IsProgressBar"].is_null() ? "0" : node["IsProgressBar"];
	is_progress_bar = std::stoi(bar) == 1;
}

void ComponentImage::CreateInspectorNode()
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
	ImGui::Checkbox("Visible", &visible);
	ImGui::Separator();

	ImGui::SetNextItemWidth(100);
	ImGui::InputInt("Priority", &priority);
	ImGui::Separator();

	ImGui::Checkbox("Fullscreen", &fullscreen);

	if (fullscreen)
		size2D = { App->gui->sceneWidth + 30, App->gui->sceneHeight + 30 };

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