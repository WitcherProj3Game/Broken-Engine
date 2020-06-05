#include "ComponentMeshRenderer.h"
#include "ComponentMesh.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"
#include "GameObject.h"
#include "OpenGL.h"
#include "Color.h"

#include "Application.h"
#include "ModuleTextures.h"
#include "ModuleSceneManager.h"
#include "ModuleRenderer3D.h"
#include "ModuleTimeManager.h"
#include "ModuleWindow.h"
#include "ModuleResourceManager.h"
#include "ModuleSelection.h"
#include "ModuleFileSystem.h"

#include "ResourceMesh.h"
#include "ResourceShader.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"

#include "ImporterMeta.h"
#include "ImporterMaterial.h"
#include "ResourceMeta.h"

#include "Imgui/imgui.h"

#include "mmgr/mmgr.h"


using namespace Broken;
ComponentMeshRenderer::ComponentMeshRenderer(GameObject* ContainerGO) : Component(ContainerGO, Component::ComponentType::MeshRenderer) 
{
	name = "Mesh Renderer";
	material = (ResourceMaterial*)App->resources->GetResource(App->resources->GetDefaultMaterialUID());
}

ComponentMeshRenderer::~ComponentMeshRenderer()
{
	if (material && material->IsInMemory()) 
	{
		material->Release();
		material->RemoveUser(GO);
	}
}

void ComponentMeshRenderer::Update()
{
	if (to_delete)
		this->GetContainerGameObject()->RemoveComponent(this);		
}

void ComponentMeshRenderer::DrawComponent() 
{
	RenderMeshFlags flags = texture;
	ComponentMesh* cmesh = GO->GetComponent<ComponentMesh>();

	if (App->selection->IsSelected(GO))
		flags |= selected;

	if (checkers)
		flags |= checkers;

	if (cast_shadows)
		flags |= castShadows;

	if (receive_shadows)
		flags |= receiveShadows;

	if(light_affected)
		flags |= lightAffected;

	if (cmesh && cmesh->resource_mesh && material)
	{
		App->renderer3D->DrawMesh(GO->GetComponent<ComponentTransform>()->GetGlobalTransform(), cmesh->resource_mesh, material, cmesh->deformable_mesh, flags, Broken::White, only_shadows);
		DrawNormals(*cmesh->resource_mesh, *GO->GetComponent<ComponentTransform>());
	}
}

void ComponentMeshRenderer::DrawNormals(const ResourceMesh& mesh, const ComponentTransform& transform) const
{
	float3 origin = float3::zero;
	float3 end = float3::zero;
	Color color(255, 255, 0);
	float4x4 transf = transform.GetGlobalTransform();

	// --- Draw vertex normals ---

	if (draw_vertexnormals && mesh.vertices->normal)
	{
		for (uint i = 0; i < mesh.IndicesSize; ++i) 
		{
			// --- Normals ---
			origin = float3(mesh.vertices[mesh.Indices[i]].position[0], mesh.vertices[mesh.Indices[i]].position[1], mesh.vertices[mesh.Indices[i]].position[2]);
			end = float3(mesh.vertices[mesh.Indices[i]].position[0] + mesh.vertices[mesh.Indices[i]].normal[0] * NORMAL_LENGTH, mesh.vertices[mesh.Indices[i]].position[1] + mesh.vertices[mesh.Indices[i]].normal[1] * NORMAL_LENGTH, mesh.vertices[mesh.Indices[i]].position[2] + mesh.vertices[mesh.Indices[i]].normal[2] * NORMAL_LENGTH);

			App->renderer3D->DrawLine(transf, origin, end, color);
		}
	}

	// --- Draw Face Normals ---
	if (draw_facenormals)
	{
		Triangle face;
		float3 face_center;
		float3 face_normal;

		// --- Compute face normals ---
		for (uint j = 0; j < mesh.IndicesSize / 3; ++j) 
		{
			face.a = float3(mesh.vertices[mesh.Indices[j * 3]].position);
			face.b = float3(mesh.vertices[mesh.Indices[(j * 3) + 1]].position);
			face.c = float3(mesh.vertices[mesh.Indices[(j * 3) + 2]].position);

			face_center = face.Centroid();

			face_normal = Cross(face.b - face.a, face.c - face.b);

			face_normal.Normalize();

			origin = float3(face_center.x, face_center.y, face_center.z);
			end = float3(face_center.x + face_normal.x * NORMAL_LENGTH, face_center.y + face_normal.y * NORMAL_LENGTH, face_center.z + face_normal.z * NORMAL_LENGTH);
			
			App->renderer3D->DrawLine(transf, origin, end, color);
		}
	}
}

json ComponentMeshRenderer::Save() const 
{
	json node;
	node["Active"] = this->active;

	node["CastsShadows"] = cast_shadows;
	node["ReceivesShadows"] = receive_shadows;
	node["OnlyShadows"] = only_shadows;
	node["LightAffected"] = light_affected;

	node["Resources"]["ResourceMaterial"]["path"];

	if (material)
		node["Resources"]["ResourceMaterial"]["path"] = std::string(material->GetResourceFile());

	return node;
}

void ComponentMeshRenderer::Load(json& node)
{
	this->active = node["Active"].is_null() ? true : (bool)node["Active"];

	cast_shadows = node.find("CastsShadows") == node.end() ? true : node["CastsShadows"].get<bool>();
	receive_shadows = node.find("ReceivesShadows") == node.end() ? true : node["ReceivesShadows"].get<bool>();
	only_shadows = node.find("OnlyShadows") == node.end() ? false : node["OnlyShadows"].get<bool>();
	light_affected = node.find("LightAffected") == node.end() ? true : node["LightAffected"].get<bool>();

	std::string mat_path = node["Resources"]["ResourceMaterial"]["path"].is_null() ? "0" : node["Resources"]["ResourceMaterial"]["path"];
	ImporterMeta* IMeta = App->resources->GetImporter<ImporterMeta>();

	if (IMeta) 
	{
		ResourceMeta* meta = (ResourceMeta*)IMeta->Load(mat_path.c_str());

		if (material && meta)
			material->Release();

		if (meta)
			material = (ResourceMaterial*)App->resources->GetResource(meta->GetUID());

		// --- We want to be notified of any resource event ---
		if (material)
			material->AddUser(GO);
	}
}

void ComponentMeshRenderer::ONResourceEvent(uint UID, Resource::ResourceNotificationType type) 
{
	// --- Always check if your resources are already invalidated, since go sends events from all of its components resources ---

	switch (type) 
	{
		case Resource::ResourceNotificationType::Overwrite:
			if (material && UID == material->GetUID()) material = (ResourceMaterial*)App->resources->GetResource(UID);
			break;
		case Resource::ResourceNotificationType::Deletion:
			if (material && UID == material->GetUID()) material = nullptr;
			break;
		default:
			break;
	}
}

void ComponentMeshRenderer::CreateInspectorNode()
{
	// --- Mesh Node ---
	ImGui::Checkbox("Vertex Normals", &draw_vertexnormals);
	ImGui::SameLine();
	ImGui::Checkbox("Face Normals  ", &draw_facenormals);
	ImGui::SameLine();
	ImGui::Checkbox("Checkers", &checkers);

	// --- Shadows Node ---
	ImGui::NewLine();
	ImGui::Separator();
	ImGui::NewLine();
	ImGui::Text("Shadowing");

	ImGui::Text("Cast Shadows"); ImGui::SameLine();
	if (ImGui::Checkbox("##CastSH", &cast_shadows))
	{
		if (only_shadows)
			cast_shadows = true;
	}

	ImGui::Text("Receive Shadows"); ImGui::SameLine();
	if (ImGui::Checkbox("##ReceiveSH", &receive_shadows))
	{
		if (only_shadows)
			receive_shadows = false;
	}
	
	ImGui::Text("Only Shadows"); ImGui::SameLine();
	if (ImGui::Checkbox("##OnlySH", &only_shadows))
	{
		if (only_shadows)
		{
			cast_shadows = true;
			receive_shadows = false;
		}
	}

	// --- Material node ---
	ImGui::NewLine();
	ImGui::Separator();

	ImGui::Text("Light Affected"); ImGui::SameLine();
	ImGui::Checkbox("##LightAff", &light_affected);
	ImGui::NewLine();	

	ImGui::PushID("Material");
	if (material)
	{
		bool save_material = false;
		bool is_default = material->GetUID() == App->resources->DefaultMaterial->GetUID();

		// --- Mat preview
		ImGui::Image((void*)(uint)material->GetPreviewTexID(), ImVec2(30, 30));
		ImGui::SameLine();

		if (ImGui::TreeNodeEx(material->GetName(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			static ImGuiComboFlags flags = 0;

			ImGui::Text("Shader");
			ImGui::SameLine();

			if (material->shader)
			{
				const char* item_current = material->shader->GetName();
				if (ImGui::BeginCombo("##Shader", item_current, flags))
				{
					for (std::map<uint, ResourceShader*>::iterator it = App->resources->shaders.begin(); it != App->resources->shaders.end(); ++it)
					{
						bool is_selected = (item_current == it->second->GetName());

						if (ImGui::Selectable(it->second->GetName(), is_selected))
						{
							item_current = it->second->GetName();
							material->shader = it->second;
							material->shader->GetAllUniforms(material->uniforms);
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}

					save_material = true;
					ImGui::EndCombo();
				}
			}

			if (!is_default)
			{
				// --- Print Texture Path ---
				//std::string Path = "Path: ";
				//Path.append(material->resource_diffuse->Texture_path);
				//ImGui::Text(Path.data());			

				// --- UNIFORMS ---
				material->DisplayAndUpdateUniforms();

				ImGui::NewLine();
				ImGui::SameLine();
				if(ImGui::Checkbox("Use Textures", &material->m_UseTexture)) save_material = true;
				ImGui::SameLine();
				if (ImGui::Checkbox("Transparencies", &material->has_transparencies)) save_material = true;
				ImGui::SameLine();
				if (ImGui::Checkbox("Culling", &material->has_culling)) save_material = true;

				//Rim Light
				ImGui::Separator();
				ImGui::NewLine();
				ImGui::NewLine(); ImGui::SameLine();
				if (ImGui::Checkbox("Apply Rim Light", &material->m_ApplyRimLight)) save_material = true;

				if (material->m_ApplyRimLight)
				{
					ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
					ImGui::Text("Rim Power");
					ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f); ImGui::SetNextItemWidth(200.0f);
					if (ImGui::DragFloat("##MatRimPowerEd", &material->m_RimPower, 0.005f, -10.0f, 10.0f)) save_material = true;

					ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
					ImGui::Text("Rim Smooth");
					ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 30.0f);

					ImGui::Text("Low"); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f); ImGui::SetNextItemWidth(75.0f);
					if (ImGui::DragFloat("##MatEdLowRimSm", &material->m_RimSmooth.x, 0.005f, -10.0f, 10.0f, "%.4f")) save_material = true;

					ImGui::SameLine(); ImGui::Text("High"); ImGui::SameLine(); ImGui::SetNextItemWidth(75.0f);
					if (ImGui::DragFloat("##MatEdHighRimSm", &material->m_RimSmooth.y, 0.005f, -10.0f, 10.0f, "%.4f")) save_material = true;
				}

				//Color
				ImGui::Separator();
				ImGui::NewLine();
				if(ImGui::ColorEdit4("##AmbientColor", (float*)&material->m_AmbientColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar)) save_material = true;
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("Ambient Color");
				if (ImGui::Checkbox("Scene Color Affected", &material->m_AffectedBySceneColor)) save_material = true;

				//Shininess
				ImGui::Text("Shininess");
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
				ImGui::SetNextItemWidth(300.0f);
				if(ImGui::SliderFloat("##MatShininessEd", &material->m_Shininess, -0.5f, 500.00f, "%.3f", 1.5f)) save_material = true;

				//ImGui::Text("Shader Uniforms");

				//DisplayAndUpdateUniforms(material);
				//ImGui::TreePop();

				// --- Print Texture Width and Height (Diffuse) ---
				material->HandleTextureDisplay(material->m_DiffuseResTexture, save_material, "Diffuse", "UnuseDiff", GetContainerGameObject());

				// --- Print Texture Width and Height (Specular) ---
				material->HandleTextureDisplay(material->m_SpecularResTexture, save_material, "Specular", "UnuseSpec", GetContainerGameObject());

				// --- Print Texture Width and Height (Normal) ---
				material->HandleTextureDisplay(material->m_NormalResTexture, save_material, "Normal", "UnuseNormal", GetContainerGameObject());

				ImGui::NewLine();
				ImGui::Separator();

				if (ImGui::TreeNode("Material Blending"))
				{
					material->HandleBlendingSelector(save_material);
					ImGui::TreePop();
				}

				ImGui::NewLine();
				ImGui::Separator();
			}
			ImGui::TreePop();

			if (ImGui::Button("Unuse Material"))
			{
				if (material && material->IsInMemory())
				{
					save_material = false;
					material->Release();
					material->RemoveUser(GO);
					material = nullptr;
				}
			}

			// --- Save material ---
			if (save_material)
				App->resources->DeferSave(material);
		}		
	}

	// --- Handle drag & drop ---
	ImGui::ImageButton(NULL, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 2);
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource"))
		{
			uint UID = *(const uint*)payload->Data;
			Resource* resource = App->resources->GetResource(UID, false);

			if (resource && resource->GetType() == Resource::ResourceType::MATERIAL)
				material = (ResourceMaterial*)App->resources->GetResource(UID);
		}

		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	ImGui::Text("Drop Material");

	ImGui::PopID();
}