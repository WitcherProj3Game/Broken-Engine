#include "ResourceMaterial.h"

// -- Modules --
#include "Application.h"
#include "ModuleGui.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleEventManager.h"

// -- Resources --
#include "ResourceTexture.h"
#include "ResourceShader.h"

#include "ImporterMaterial.h"

#include "Imgui/imgui.h"
#include "OpenGL.h"
#include "mmgr/mmgr.h"

using namespace Broken;
ResourceMaterial::ResourceMaterial(uint UID, const char* source_file) : Resource(Resource::ResourceType::MATERIAL, UID, source_file) 
{
	extension = ".mat";
	resource_file = source_file;
	shader = App->renderer3D->defaultShader;

	previewTexID = App->gui->materialTexID;

	shader->GetAllUniforms(uniforms);

	//LoadToMemory();
}

ResourceMaterial::~ResourceMaterial() 
{
	glDeleteTextures(1, (GLuint*)&previewTexID);
}

bool ResourceMaterial::LoadInMemory() 
{
	//shader->GetAllUniforms(uniforms);

	// --- Texture Stuff ---
	Importer::ImportData IDataDiff(DiffuseResTexturePath.c_str());

	if (DiffuseResTexturePath != "NaN.dds")
		m_DiffuseResTexture = (ResourceTexture*)App->resources->ImportAssets(IDataDiff);

	Importer::ImportData IDataSpec(SpecularResTexturePath.c_str());

	if (SpecularResTexturePath != "NaN.dds")
		m_SpecularResTexture = (ResourceTexture*)App->resources->ImportAssets(IDataSpec);

	Importer::ImportData IDataNormTex(NormalResTexturePath.c_str());

	if (NormalResTexturePath != "NaN.dds")
		m_NormalResTexture = (ResourceTexture*)App->resources->ImportAssets(IDataNormTex);

	return true;
}

void ResourceMaterial::FreeMemory() 
{
	for (uint i = 0; i < uniforms.size(); ++i) 
	{
		delete uniforms[i];
	}

	if (m_DiffuseResTexture)
		m_DiffuseResTexture->Release();
	if (m_SpecularResTexture)
		m_SpecularResTexture->Release();
	if (m_NormalResTexture)
		m_NormalResTexture->Release();

	uniforms.clear();
}

void ResourceMaterial::SetBlending() const
{
	bool blendEq_Same = (m_MatBlendEq == App->renderer3D->GetRendererBlendingEquation());
	if (m_AutoBlending)
	{
		if(m_MatAutoBlendFunc == App->renderer3D->GetRendererBlendAutoFunction() && blendEq_Same)
			return;

		App->renderer3D->PickBlendingAutoFunction(m_MatAutoBlendFunc, m_MatBlendEq);
		App->renderer3D->m_ChangedBlending = true;
	}
	else
	{
		BlendingTypes src, dst;
		App->renderer3D->GetRendererBlendingManualFunction(src, dst);

		bool manualBlend_Same = (m_MatManualBlend_Src == src && m_MatManualBlend_Dst == dst);
		if (manualBlend_Same && blendEq_Same)
			return;

		App->renderer3D->PickBlendingManualFunction(m_MatManualBlend_Src, m_MatManualBlend_Dst, m_MatBlendEq);
		App->renderer3D->m_ChangedBlending = true;
	}
}

void ResourceMaterial::CreateInspectorNode()
{
	bool save_material = false;

	// --- Mat preview
	ImGui::Image((void*)(uint)GetPreviewTexID(), ImVec2(30, 30));
	ImGui::SameLine();

	// --------------------------------------- TREE NODE FOR MATERIAL (Shader --> Uniforms --> Color --> Textures)
	static ImGuiComboFlags flags = 0;

	ImGui::Text("Shader");
	ImGui::SameLine();

	// --- Shader ---
	if (shader)
	{
		const char* item_current = shader->GetName();
		if (ImGui::BeginCombo("##Shader", item_current, flags))
		{
				for (std::map<uint, ResourceShader*>::iterator it = App->resources->shaders.begin(); it != App->resources->shaders.end(); ++it)
				{
					bool is_selected = (item_current == it->second->GetName());

					if (ImGui::Selectable(it->second->GetName(), is_selected))
					{
						save_material = true;
						item_current = it->second->GetName();
						shader = it->second;
						shader->GetAllUniforms(uniforms);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
			
			ImGui::EndCombo();
		}

		if (std::string(shader->GetName()).compare("ShaderTexturesTest.glsl") == 0)
			int asda = 1;

		// --- Uniforms ---
		shader->GetAllUniforms(uniforms);
		DisplayAndUpdateUniforms();
	}

	// --- Bools ---
	ImGui::NewLine();
	ImGui::SameLine();
	if(ImGui::Checkbox("Use Textures", &m_UseTexture)) save_material = true;
	ImGui::SameLine();
	if (ImGui::Checkbox("Transparencies", &has_transparencies)) save_material = true;
	ImGui::SameLine();
	if (ImGui::Checkbox("Culling", &has_culling)) save_material = true;

	// --- Color ---
	ImGui::Separator();
	ImGui::NewLine();
	if(ImGui::ColorEdit4("##AmbientColor", (float*)&m_AmbientColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
		save_material = true;
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
	ImGui::Text("MatAmbientColor");
	if (ImGui::Checkbox("Scene Color Affected", &m_AffectedBySceneColor)) save_material = true;

	//--- Shininess ---
	ImGui::Text("Shininess");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
	ImGui::SetNextItemWidth(300.0f);
	if(ImGui::SliderFloat("", &m_Shininess, -0.5f, 500.00f))
		save_material = true;

	// --- Print Texture Width and Height (Diffuse) ---
	HandleTextureDisplay(m_DiffuseResTexture, save_material, "Diffuse", "UnuseDiff");

	// --- Print Texture Width and Height (Specular)
	HandleTextureDisplay(m_SpecularResTexture, save_material, "Specular", "UnuseSpec");

	// --- Print Texture Width and Height (Normal)
	HandleTextureDisplay(m_NormalResTexture, save_material, "Normal", "UnuseNorm");

	// --- Tree Node for Blending
	ImGui::NewLine();
	ImGui::Separator();

	if (ImGui::TreeNode("Material Blending"))
	{
		HandleBlendingSelector(save_material);
		ImGui::TreePop();
	}

	ImGui::NewLine();
	ImGui::Separator();

	if (save_material)
		App->resources->DeferSave(this);
}

void ResourceMaterial::HandleTextureDisplay(ResourceTexture*& texture, bool& save_material, const char* texture_name, const char* unuse_label, GameObject* container)
{
	// Show Texture Size
	uint textSizeX = 0, textSizeY = 0;
	ImGui::NewLine();
	if (texture)
	{
		textSizeX = texture->Texture_width;
		textSizeY = texture->Texture_height;
	}

	ImGui::Text(std::to_string(textSizeX).c_str());
	ImGui::SameLine();
	ImGui::Text(std::to_string(textSizeY).c_str());

	// Show Texture Image
	if (texture)
		ImGui::ImageButton((void*)(uint)texture->GetPreviewTexID(), ImVec2(20, 20));
	else
		ImGui::ImageButton(NULL, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 2);

	// Handle drag & drop
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
				save_material = true;
			}
		}

		ImGui::EndDragDropTarget();
	}

	// Show Texture Name
	ImGui::SameLine();
	ImGui::Text(texture_name);

	// Unuse Texture
	ImGui::SameLine();
	if (ImGui::Button(unuse_label, { 77, 18 }) && texture)
	{
		if(container)
			texture->RemoveUser(container);
		
		texture->Release();
		texture = nullptr;
		save_material = true;
	}
}

void ResourceMaterial::HandleBlendingSelector(bool& save_material)
{
	ImGui::NewLine();
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
	ImGui::Text("Material Blend Equation"); ImGui::SameLine();
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
	ImGui::SetNextItemWidth(200.0f);

	// --- Blend Eq ---
	std::vector<const char*> blendEq = App->renderer3D->m_BlendEquationFunctionsVec;
	int index = (int)m_MatBlendEq;
	if (App->gui->HandleDropdownSelector(index, "##MAlphaEq", blendEq.data(), blendEq.size()))
	{
		m_MatBlendEq = (BlendingEquations)index;
		save_material = true;
	}

	// --- Blend Auto Func ---
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
	ImGui::Text("Material Blend Mode"); ImGui::SameLine();
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 38.0f);
	ImGui::SetNextItemWidth(200.0f);

	std::vector<const char*> blendAutoF_Vec = App->renderer3D->m_BlendAutoFunctionsVec;
	int index1 = (int)m_MatAutoBlendFunc;
	if (App->gui->HandleDropdownSelector(index1, "##MAlphaAutoFunction", blendAutoF_Vec.data(), blendAutoF_Vec.size()))
	{
		m_MatAutoBlendFunc = (BlendAutoFunction)index1;
		save_material = true;
	}

	//Help Marker
	std::string desc = "Stand. = SRC, 1-SRCALPH\nAdd. = ONE, ONE\nAddAlph. = SRC_ALPH, ONE\nMult. = DSTCOL, ZERO";
	ImGui::SameLine();
	App->gui->HelpMarker(desc.c_str());

	// --- Blend Manual Function ---
	ImGui::NewLine();
	ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
	ImGui::Checkbox("Automatic Alpha Selection", &m_AutoBlending);
	if (!m_AutoBlending)
	{
		//ImGui::Separator();
		ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
		if (ImGui::TreeNodeEx("Manual Alpha", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//Source
			//ImGui::NewLine();
			ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 30.0f);
			ImGui::Text("Source Alpha"); ImGui::SameLine();
			ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 45.0f);
			ImGui::SetNextItemWidth(200.0f);

			std::vector<const char*> blendTypes_Vec = App->renderer3D->m_AlphaTypesVec;
			int index2 = (int)m_MatManualBlend_Src;
			if (App->gui->HandleDropdownSelector(index2, "##MManualAlphaSrc", blendTypes_Vec.data(), blendTypes_Vec.size()))
			{
				m_MatManualBlend_Src = (BlendingTypes)index2;
				save_material = true;
			}

			//Destination
			ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 30.0f);
			ImGui::Text("Destination Alpha"); ImGui::SameLine();
			ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 10.0f);
			ImGui::SetNextItemWidth(200.0f);

			int index3 = (int)m_MatManualBlend_Dst;
			if (App->gui->HandleDropdownSelector(index3, "##MManualAlphaDst", blendTypes_Vec.data(), blendTypes_Vec.size()))
			{
				m_MatManualBlend_Dst = (BlendingTypes)index3;
				save_material = true;
			}

			//ImGui::NewLine();
			ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 30.0f);
			if (ImGui::Button("Reference (Test Blend)", { 180, 18 })) App->gui->RequestBrowser("https://www.andersriggelsen.dk/glblendfunc.php");
			ImGui::TreePop();
		}
	}
}


void ResourceMaterial::UpdateUniforms() 
{
	glUseProgram(shader->ID);

	for (uint i = 0; i < uniforms.size(); ++i) {

		switch (uniforms[i]->type)
		{
		case GL_INT:
			glUniform1i(uniforms[i]->location, uniforms[i]->value.intU);
			break;

		case GL_FLOAT:
			glUniform1f(uniforms[i]->location, uniforms[i]->value.floatU);
			break;

		case GL_FLOAT_VEC2:
			glUniform2f(uniforms[i]->location, uniforms[i]->value.vec2U.x, uniforms[i]->value.vec2U.y);
			break;

		case GL_FLOAT_VEC3:
			glUniform3f(uniforms[i]->location, uniforms[i]->value.vec3U.x, uniforms[i]->value.vec3U.y, uniforms[i]->value.vec3U.z);
			break;

		case GL_FLOAT_VEC4:
			glUniform4f(uniforms[i]->location, uniforms[i]->value.vec4U.x, uniforms[i]->value.vec4U.y, uniforms[i]->value.vec4U.z, uniforms[i]->value.vec4U.w);
			break;

		case GL_INT_VEC2:
			glUniform2i(uniforms[i]->location, uniforms[i]->value.vec2U.x, uniforms[i]->value.vec2U.y);
			break;

		case GL_INT_VEC3:
			glUniform3i(uniforms[i]->location, uniforms[i]->value.vec3U.x, uniforms[i]->value.vec3U.y, uniforms[i]->value.vec3U.z);
			break;

		case GL_INT_VEC4:
			glUniform4i(uniforms[i]->location, uniforms[i]->value.vec4U.x, uniforms[i]->value.vec4U.y, uniforms[i]->value.vec4U.z, uniforms[i]->value.vec4U.w);
			break;

		case GL_SAMPLER_2D:
			if ((int)uniforms[i]->value.textureU.y > 0 && (int)uniforms[i]->value.textureU.x != -1)
			{
				int loc = glGetUniformLocation(shader->ID, uniforms[i]->name.c_str());
				glUniform1i(uniforms[i]->location, (int)uniforms[i]->value.textureU.x);
				glActiveTexture((int)uniforms[i]->value.textureU.x);
				glBindTexture(GL_TEXTURE_2D, (int)uniforms[i]->value.textureU.y);
			}
			break;
		}
	}

	glUseProgram(App->renderer3D->defaultShader->ID);
}

void ResourceMaterial::DisplayAndUpdateUniforms()
{
	// Note this is being done before any render happens

	static int tmp_int = 0;
	static float tmp_float = 0.0f;
	static float2 tmp_vec2 = { 0.0f,0.0f };
	static float3 tmp_vec3 = { 0.0f,0.0f,0.0f };
	static float4 tmp_vec4 = { 0.0f,0.0f,0.0f,0.0f };

	glUseProgram(shader->ID);

	bool updated = false;

	for (uint i = 0; i < uniforms.size(); ++i)
	{
		ImGui::PushID(GetUID() + i);
		ImGui::Text(uniforms[i]->name.data());
		ImGui::SameLine();

		switch (uniforms[i]->type)
		{
		case GL_INT:
			tmp_int = uniforms[i]->value.intU;

			if (ImGui::InputInt("##inputintuniform", &tmp_int, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				updated = true;
				uniforms[i]->value.intU = tmp_int;
			}
			break;

		case GL_FLOAT:
			tmp_float = uniforms[i]->value.floatU;

			if (ImGui::InputFloat("##inputfloatuniform", &tmp_float, 1.0f, 100.0f, "%3f", ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				updated = true;
				uniforms[i]->value.floatU = tmp_float;
			}

			break;

		case GL_FLOAT_VEC2:
			tmp_vec2 = uniforms[i]->value.vec2U;

			if (ImGui::InputFloat2("##inputfloat2uniform", tmp_vec2.ptr(), 2, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				updated = true;
				uniforms[i]->value.vec2U = tmp_vec2;
			}

			break;

		case GL_FLOAT_VEC3:
			tmp_vec3 = uniforms[i]->value.vec3U;

			if (ImGui::InputFloat3("##inputfloat3uniform", tmp_vec3.ptr(), 2, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				updated = true;
				uniforms[i]->value.vec3U = tmp_vec3;
			}

			break;

		case GL_FLOAT_VEC4:
			tmp_vec4 = uniforms[i]->value.vec4U;

			if (ImGui::InputFloat4("##inputfloat4uniform", tmp_vec4.ptr(), 2, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				updated = true;
				uniforms[i]->value.vec4U = tmp_vec4;
			}

			break;

		case GL_INT_VEC2:
			tmp_vec2 = uniforms[i]->value.vec2U;

			if (ImGui::InputInt2("##inputint2uniform", (int*)tmp_vec2.ptr(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				updated = true;
				uniforms[i]->value.vec2U = tmp_vec2;
			}
			break;

		case GL_INT_VEC3:
			tmp_vec3 = uniforms[i]->value.vec3U;

			if (ImGui::InputInt3("##inputint3uniform", (int*)tmp_vec3.ptr(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				updated = true;
				uniforms[i]->value.vec3U = tmp_vec3;
			}
			break;

		case GL_INT_VEC4:
			tmp_vec4 = uniforms[i]->value.vec4U;

			if (ImGui::InputInt4("##inputint4uniform", (int*)tmp_vec4.ptr(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
			{
				updated = true;
				uniforms[i]->value.vec4U = tmp_vec4;
			}

			break;

		case GL_SAMPLER_2D:

			ResourceTexture* current_texture = nullptr;
			if ((int)uniforms[i]->value.textureU.y > 0)
				current_texture = (ResourceTexture*)App->resources->GetResource((int)uniforms[i]->value.textureU.y, false);

			// Show Texture Size
			uint textSizeX = 0, textSizeY = 0;
			ImGui::NewLine();
			if (current_texture)
			{
				textSizeX = current_texture->Texture_width;
				textSizeY = current_texture->Texture_height;
			}

			ImGui::Text(std::to_string(textSizeX).c_str());
			ImGui::SameLine();
			ImGui::Text(std::to_string(textSizeY).c_str());

			// Show Texture Image
			if (current_texture)
				ImGui::ImageButton((void*)(uint)current_texture->GetPreviewTexID(), ImVec2(20, 20));
			else
				ImGui::ImageButton(NULL, ImVec2(20, 20), ImVec2(0, 0), ImVec2(1, 1), 2);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource"))
				{
					uint UID = *(const uint*)payload->Data;
					Resource* resource = App->resources->GetResource(UID, false);
					if (resource && resource->GetType() == Resource::ResourceType::TEXTURE)
					{
						updated = true;
						uniforms[i]->value.textureU.y = (float)UID;
					}
				}

				ImGui::EndDragDropTarget();
			}

			// Show Texture Name
			//ImGui::SameLine();
			//ImGui::Text(uniforms[i]->name.c_str());
			// Unuse Texture
			ImGui::SameLine();
			if (ImGui::Button("Unuse Texture", { 77, 18 }) && current_texture)
			{
				updated = true;
				uniforms[i]->value.textureU.y = 0;
			}

			break;
		}

		ImGui::PopID();
	}

	if (updated)
	{
		UpdateUniforms();
		App->resources->GetImporter<ImporterMaterial>()->Save(this);
	}

	glUseProgram(App->renderer3D->defaultShader->ID);
}

void ResourceMaterial::OnOverwrite() 
{
	NotifyUsers(ResourceNotificationType::Overwrite);
}

void ResourceMaterial::OnDelete() 
{
	NotifyUsers(ResourceNotificationType::Deletion);

	FreeMemory();
	App->fs->Remove(resource_file.c_str());
	App->fs->Remove(previewTexPath.c_str());

	Resource* diffuse = m_DiffuseResTexture;
	Resource* specular = m_SpecularResTexture;
	Resource* normalMap = m_NormalResTexture;

	if (diffuse)
		diffuse->Release();

	if (specular)
		specular->Release();

	if (normalMap)
		normalMap->Release();

	App->resources->RemoveResourceFromFolder(this);
	App->resources->ONResourceDestroyed(this);
}

void ResourceMaterial::Repath() {
	resource_file = original_file + extension;
}