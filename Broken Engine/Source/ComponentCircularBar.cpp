#include "ComponentCircularBar.h"
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

ComponentCircularBar::ComponentCircularBar(GameObject* gameObject) : UI_Element(gameObject, Component::ComponentType::CircularBar)
{
	visible = true;
	size2D = { 125, 125 };

	//texture = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "DefaultTexture");
	if (GO->parent && GO->parent->HasComponent(Component::ComponentType::Canvas))
	{
		canvas = (ComponentCanvas*)GO->parent->GetComponent<ComponentCanvas>();

		if(canvas)
			canvas->AddElement(this);
	}
}

ComponentCircularBar::~ComponentCircularBar()
{
	if (texture)
	{
		texture->Release();
		texture->RemoveUser(GO);
	}

	if (canvas)
		canvas->RemoveElement(this);
}

void ComponentCircularBar::Update()
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

void ComponentCircularBar::Draw()
{
	DrawCircle(colorP1);
	DrawCircle(colorP2, axis, percentage);
}

void ComponentCircularBar::DrawCircle(Color color, bool axis, float _percentage)
{
	// --- Frame image with camera ---
	float nearp = App->renderer3D->active_camera->GetNearPlane();
	float3 pos = { position2D.x / App->gui->sceneWidth, position2D.y / App->gui->sceneHeight, nearp + 0.026f };
	
	float size_x = size2D.x, size_y = size2D.y;
	if (axis == 0)								//X Axis
		size_x *= (_percentage /100);
	else										// Y Axis
		size_y *= (_percentage / 100);

	float3 size = { size_x / App->gui->sceneWidth, size_y / App->gui->sceneHeight, 1.0f };
	float4x4 transform = transform.FromTRS(pos, Quat::identity, size);

	//float3 position = App->renderer3D->active_camera->frustum.NearPlanePos(-1, -1);
	//float2 new_size;
	//
	//if (axis == 0) //x axis
	//	new_size = float2((size2D.x * _percentage) / 100, size2D.y);
	//else //y axis
	//	new_size = float2(size2D.x, (size2D.y * _percentage) / 100);
	//float4x4 transform = transform.FromTRS(position, App->renderer3D->active_camera->GetOpenGLViewMatrix().RotatePart(), float3(new_size * 0.01f, 1.0f));

	// --- Set Uniforms ---
	uint shaderID = App->renderer3D->UI_Shader->ID;
	//uint shaderID = App->renderer3D->defaultShader->ID;
	glUseProgram(shaderID);

	GLint modelLoc = glGetUniformLocation(shaderID, "u_Model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transform.Transposed().ptr());

	GLint viewLoc = glGetUniformLocation(shaderID, "u_View");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLViewMatrix().ptr());


	// right handed projection matrix
	//float f = 1.0f / tan(App->renderer3D->active_camera->GetFOV() * DEGTORAD / 2.0f);
	//float4x4 proj_RH(
	//	f / App->renderer3D->active_camera->GetAspectRatio(), 0.0f, 0.0f, 0.0f,
	//	0.0f, f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 0.0f, -1.0f,
	//	0.0f, 0.0f, nearp, 0.0f);

	GLint projectLoc = glGetUniformLocation(shaderID, "u_Proj");
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLProjectionMatrix().ptr());

	// --- Texturing & Coloring ---
	glUniform1f(glGetUniformLocation(shaderID, "u_GammaCorrection"), App->renderer3D->GetGammaCorrection());
	GLint vertexColorLocation = glGetUniformLocation(shaderID, "u_Color");
	glUniform4f(vertexColorLocation, color.r, color.g, color.b, color.a);
	glUniform1i(glGetUniformLocation(shaderID, "u_HasTransparencies"), 1);
	
	int TextureLocation = glGetUniformLocation(shaderID, "u_UseTextures");
	if (texture)
	{
		glUniform1i(TextureLocation, 1);
		glUniform1i(glGetUniformLocation(shaderID, "u_AlbedoTexture"), 1);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, App->textures->GetDefaultTextureID());
	}
	else
		glUniform1i(TextureLocation, 0);

	// --- Draw circle with given texture ---
	glBindVertexArray(App->scene_manager->GetDiskMesh()->VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, App->scene_manager->GetDiskMesh()->EBO);
	glDrawElements(GL_TRIANGLES, App->scene_manager->GetDiskMesh()->IndicesSize, GL_UNSIGNED_INT, NULL); // render primitives from array data

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0); // Stop using buffer (texture)
	glActiveTexture(GL_TEXTURE0);

	// --- Set uniforms back to defaults ---
	glUniform1i(TextureLocation, 0);
	glUniform3f(vertexColorLocation, 1.0f, 1.0f, 1.0f);
}

json ComponentCircularBar::Save() const
{
	json node;

	node["Resources"]["ResourceTexture"];

	if (texture)
		node["Resources"]["ResourceTexture"] = std::string(texture->GetResourceFile());

	node["Active"] = this->active;
	node["visible"] = std::to_string(visible);
	node["priority"] = std::to_string(priority);

	node["position2Dx"] = std::to_string(position2D.x);
	node["position2Dy"] = std::to_string(position2D.y);

	node["size2Dx"] = std::to_string(size2D.x);
	node["size2Dy"] = std::to_string(size2D.y);

	node["Color1_R"] = std::to_string(colorP1.r);
	node["Color1_G"] = std::to_string(colorP1.g);
	node["Color1_B"] = std::to_string(colorP1.b);
	node["Color1_A"] = std::to_string(colorP1.a);

	node["Color2_R"] = std::to_string(colorP2.r);
	node["Color2_G"] = std::to_string(colorP2.g);
	node["Color2_B"] = std::to_string(colorP2.b);
	node["Color2_A"] = std::to_string(colorP2.a);

	node["Percentage"] = percentage;

	return node;
}

void ComponentCircularBar::Load(json& node)
{
	std::string path = node["Resources"]["ResourceTexture"].is_null() ? "0" : node["Resources"]["ResourceTexture"];
	App->fs->SplitFilePath(path.c_str(), nullptr, &path);
	path = path.substr(0, path.find_last_of("."));

	texture = (ResourceTexture*)App->resources->GetResource(std::stoi(path));

	if (texture)
		texture->AddUser(GO);

	this->active = node["Active"].is_null() ? true : (bool)node["Active"];
	std::string visible_str = node["visible"].is_null() ? "0" : node["visible"];
	std::string priority_str = node["priority"].is_null() ? "0" : node["priority"];

	std::string position2Dx = node["position2Dx"].is_null() ? "0" : node["position2Dx"];
	std::string position2Dy = node["position2Dy"].is_null() ? "0" : node["position2Dy"];

	std::string size2Dx = node["size2Dx"].is_null() ? "0" : node["size2Dx"];
	std::string size2Dy = node["size2Dy"].is_null() ? "0" : node["size2Dy"];

	position2D = float2(std::stof(position2Dx), std::stof(position2Dy));
	size2D = float2(std::stof(size2Dx), std::stof(size2Dy));

	std::string Color1_R = node["Color1_R"].is_null() ? "0" : node["Color1_R"];
	std::string Color1_G = node["Color1_G"].is_null() ? "0" : node["Color1_G"];
	std::string Color1_B = node["Color1_B"].is_null() ? "0" : node["Color1_B"];
	std::string Color1_A = node["Color1_A"].is_null() ? "0" : node["Color1_A"];

	std::string Color2_R = node["Color2_R"].is_null() ? "0" : node["Color2_R"];
	std::string Color2_G = node["Color2_G"].is_null() ? "0" : node["Color2_G"];
	std::string Color2_B = node["Color2_B"].is_null() ? "0" : node["Color2_B"];
	std::string Color2_A = node["Color2_A"].is_null() ? "0" : node["Color2_A"];

	percentage = node["Percentage"].is_null() ? 100 : (float)node["Percentage"];

	visible = bool(std::stoi(visible_str));
	priority = int(std::stoi(priority_str));

	colorP1 = { std::stof(Color1_R),std::stof(Color1_G), std::stof(Color1_B), std::stof(Color1_A) };
	colorP2 = { std::stof(Color2_R),std::stof(Color2_G), std::stof(Color2_B), std::stof(Color2_A) };
}

void ComponentCircularBar::CreateInspectorNode()
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
	ImGui::Checkbox("Visible", &visible);
	ImGui::Separator();

	ImGui::SetNextItemWidth(100);
	ImGui::InputInt("Priority", &priority);
	ImGui::Separator();

	// Percentage (test)
	ImGui::Text("Percentage:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("##percentage", &percentage, 0.1f, 0.0f, 100.0f);

	// Axis
	ImGui::Checkbox("Vertical Axis", &axis);

	// Position
	ImGui::Text("Position:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("x##imageposition", &position2D.x);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("y##imageposition", &position2D.y);

	// Size Planes
	ImGui::Text("Bar Size:  ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("x##imagesize", &size2D.x, 0.5f, 0.0f, INFINITY);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(60);
	ImGui::DragFloat("y##imagesize", &size2D.y, 0.5f, 0.0f, INFINITY);

	// Rotation
	//ImGui::Text("Rotation:");
	//ImGui::SameLine();
	//ImGui::SetNextItemWidth(60);
	//ImGui::DragFloat("##imagerotation", &rotation2D);

	// Planes Colors
	ImGui::Separator();
	ImGui::ColorEdit4("##ColorP1", (float*)&colorP1, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("BG color");

	ImGui::ColorEdit4("##ColorP2", (float*)&colorP2, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("Bar color");

	ImGui::Separator();
}