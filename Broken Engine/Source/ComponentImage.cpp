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

	
	// right handed projection matrix
	float f = 1.0f / tan(App->renderer3D->active_camera->GetFOV() * DEGTORAD / 2.0f);
	float4x4 proj_RH(
		f / App->renderer3D->active_camera->GetAspectRatio(), 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, nearp, 0.0f);

	GLint projectLoc = glGetUniformLocation(shaderID, "u_Proj");
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, proj_RH.ptr());

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

	// --- Draw plane with given texture ---
	glBindVertexArray(App->scene_manager->plane->VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, App->scene_manager->plane->EBO);
	glDrawElements(GL_TRIANGLES, App->scene_manager->plane->IndicesSize, GL_UNSIGNED_INT, NULL); // render primitives from array data

	glUniform1i(TextureLocation, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0); // Stop using buffer (texture)
	glActiveTexture(GL_TEXTURE0);
}

json ComponentImage::Save() const
{
	json node;
	node["Active"] = this->active;
	node["visible"] = std::to_string(visible);
	node["priority"] = std::to_string(priority);

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

	return node;
}

void ComponentImage::Load(json& node)
{
	this->active = node["Active"].is_null() ? true : (bool)node["Active"];
	std::string visible_str = node["visible"].is_null() ? "0" : node["visible"];
	std::string priority_str = node["priority"].is_null() ? "0" : node["priority"];
	visible = bool(std::stoi(visible_str));
	priority = int(std::stoi(priority_str));

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

	// Rotation
	//ImGui::Text("Rotation:");
	//ImGui::SameLine();
	//ImGui::SetNextItemWidth(60);
	//ImGui::DragFloat("##imagerotation", &rotation2D);

	// ------------------------------------------

	// Image
	ImGui::Separator();
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