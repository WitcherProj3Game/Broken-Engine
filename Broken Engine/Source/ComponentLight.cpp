#include "ComponentLight.h"

// -- Modules --
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleSceneManager.h"
#include "ModuleSelection.h"
#include "ModuleGui.h"

// -- Resources --
#include "ResourceShader.h"
#include "ResourceMesh.h"

// -- Components --
#include "GameObject.h"
#include "ComponentTransform.h"
#include "ComponentCamera.h"

#include "Imgui/imgui.h"
#include "mmgr/mmgr.h"

using namespace Broken;

const std::string GetStringFromLightType(LightType type)
{
	std::string ret = "";
	switch (type)
	{
		case LightType::DIRECTIONAL:		ret = "Directional"; break;
		case LightType::POINTLIGHT:			ret = "Pointlight"; break;
		case LightType::SPOTLIGHT:			ret = "Spotlight"; break;
		case LightType::MAX_LIGHT_TYPES:	ret = "No LightType Defined"; break;
		case LightType::NONE:				ret = "No LightType Defined"; break;
		default:							ret = "No LightType Defined"; break;
	}

	BROKEN_ASSERT((ret != ""), "Unexisting or Unsupported Light Type");
	return ret;
}

ComponentLight::ComponentLight(GameObject* ContainerGO) : Component(ContainerGO, Component::ComponentType::Light)
{
	name = "Light";
	App->renderer3D->AddLight(this);

	//m_LightFrustum
	m_LightFrustum.SetKind(FrustumSpaceGL, FrustumRightHanded);
	m_LightFrustum.SetPos(float3::zero);
	m_LightFrustum.SetFront(float3::unitZ);
	m_LightFrustum.SetUp(float3::unitY);	

	m_LightFrustum.SetViewPlaneDistances(frusutm_planes.x, frusutm_planes.y);

	if(m_LightType == LightType::DIRECTIONAL)
		m_LightFrustum.SetOrthographic(frustum_size.x, frustum_size.y);
	else
	{
		m_LightFrustum.SetPerspective(1.0f, 1.0f);
		m_LightFrustum.SetVerticalFovAndAspectRatio(90, 1.0);
	}

	//m_LightFrustum.SetHorizontalFovAndAspectRatio(m_LightFrustum.HorizontalFov(), 1.0f);
}

ComponentLight::~ComponentLight()
{
	Disable();
	SendUniforms(App->renderer3D->defaultShader->ID, App->renderer3D->GetLightIndex(this));
	App->renderer3D->PopLight(this);
}


void ComponentLight::Update()
{
	ComponentTransform* trans = GetContainerGameObject()->GetComponent<ComponentTransform>();
	if (trans && m_LightType != LightType::NONE && m_LightType != LightType::MAX_LIGHT_TYPES)
	{
		// --- Transformation Calculations ---
		float3 position = float3::zero, scale = float3::one;
		Quat q = Quat::identity;
		trans->GetGlobalTransform().Decompose(position, q, scale);

		m_LightFrustum.SetPos(position);
		float3 fr = trans->GetGlobalTransform().Col3(2);
		float3 upvec = trans->GetGlobalTransform().Col3(1);
		m_LightFrustum.SetFront(fr);
		m_LightFrustum.SetUp(upvec);

		if (m_LightType == LightType::DIRECTIONAL)
			m_Direction = -fr.Normalized();
		else
		{
			float3 orientation_vec = float3(2 * (q.x * q.z + q.w * q.y), 2 * (q.y * q.z - q.w * q.x), 1 - 2 * (q.x * q.x + q.y * q.y));
			m_Direction = -orientation_vec;
		}

		// --- Visual Debug ---
		if(m_LightType != LightType::POINTLIGHT && App->selection->IsSelected(GetContainerGameObject()))
			DrawDirectionLines(m_LightType == LightType::SPOTLIGHT, position, fr);

		if (dir_debug_draw && m_LightType == LightType::DIRECTIONAL)
			DrawFrustum();
	}
	else
		m_Direction = float3(0.0f);
}


void ComponentLight::SendUniforms(uint shaderID, uint lightIndex)
{
	if ((!active || m_LightType == LightType::NONE || m_LightType == LightType::MAX_LIGHT_TYPES) && m_SetToZero)
		return;

	// ------------------ Passing Light Uniforms to Shader ------------------
	// --- Light Uniform name + Index to string ---	
	std::string light_index_str = GetLightUniform(lightIndex, "u_BkLights");

	// --- Uniforms Location Collection ---
	int dirLoc = glGetUniformLocation(shaderID, (light_index_str + ".dir").c_str());
	int posLoc = glGetUniformLocation(shaderID, (light_index_str + ".pos").c_str());
	int colorLoc = glGetUniformLocation(shaderID, (light_index_str + ".color").c_str());
	int intensityLoc = glGetUniformLocation(shaderID, (light_index_str + ".intensity").c_str());
	int attLoc = glGetUniformLocation(shaderID, (light_index_str + ".attenuationKLQ").c_str());
	int cutoffLoc = glGetUniformLocation(shaderID, (light_index_str + ".InOutCutoff").c_str());
	int LtypeLoc = glGetUniformLocation(shaderID, (light_index_str + ".LightType").c_str());
	int distMultiLoc = glGetUniformLocation(shaderID, (light_index_str + ".distanceMultiplier").c_str());
	int LshLoc = glGetUniformLocation(shaderID, (light_index_str + ".LightCastingShadows").c_str());
	
	if ((!active || m_LightType == LightType::NONE || m_LightType == LightType::MAX_LIGHT_TYPES) && !m_SetToZero)
	{
		//Set uniforms to 0 and m_SetToZero to true (if the light doesn't has to light)
		// --- Passing Position ---
		glUniform3f(posLoc, 0.0f, 0.0f, 0.0f);
		
		// --- Passing Direction ---
		glUniform3f(dirLoc, 0.0f, 0.0f, 0.0f);

		// --- Passing Color ---
		glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);

		// --- Passing Intensity & Light Type
		glUniform1i(LtypeLoc, 0);
		glUniform1f(intensityLoc, 0.0f);

		// --- Passing Light Cutoff
		glUniform2f(cutoffLoc, 0.0f, 0.0f);

		// --- Passing Light Attenuation
		glUniform3f(attLoc, 0.0f, 0.0f, 0.0f);

		// --- Passing Light Distance Multiplicator ---
		glUniform1f(distMultiLoc, 1.0f);
		
		// --- Shadower Light ---
		glUniform1f(LshLoc, false);

		m_SetToZero = true;
	}
	else
	{
		//Set uniforms and m_SetToZero to false (if the light has to light)
		// --- Passing Position ---
		ComponentTransform* trans = GetContainerGameObject()->GetComponent<ComponentTransform>();
		float3 pos = float3(0.0f);
		if (trans)
			pos = trans->GetPosition();

		glUniform3f(posLoc, pos.x, pos.y, pos.z);

		// --- Passing Direction ---
		glUniform3f(dirLoc, m_Direction.x, m_Direction.y, m_Direction.z);

		// --- Passing Color ---
		glUniform3f(colorLoc, m_Color.x, m_Color.y, m_Color.z);

		// --- Passing Intensity & Light Type
		glUniform1i(LtypeLoc, (int)m_LightType);
		glUniform1f(intensityLoc, m_Intensity);

		// --- Passing Light Cutoff
		glUniform2f(cutoffLoc, Cos(math::DegToRad(m_InOutCutoffDegrees.x)), Cos(math::DegToRad(m_InOutCutoffDegrees.y)));

		// --- Passing Light Attenuation
		glUniform3f(attLoc, m_AttenuationKLQFactors.x, m_AttenuationKLQFactors.y, m_AttenuationKLQFactors.z);

		// --- Passing Light Distance Multiplicator ---
		glUniform1f(distMultiLoc, m_DistanceMultiplier);

		// --- Shadower Light ---
		glUniform1f(LshLoc, App->renderer3D->GetShadowerLight() == this);		

		if(m_SetToZero)
			m_SetToZero = false;
	}
}

void ComponentLight::SetLightShadowsFrustumPlanes(float nearp, float farp)
{
	frusutm_planes = float2(nearp, farp);
	m_LightFrustum.SetViewPlaneDistances(frusutm_planes.x, frusutm_planes.y);	
}

void ComponentLight::SetLightShadowsFrustumSize(float x, float y)
{
	frustum_size = float2(x, y);
	m_LightFrustum.SetOrthographic(frustum_size.x, frustum_size.y);
}

const std::string ComponentLight::GetLightUniform(uint lightIndex, const char* uniformArrayName)
{
	char light_index_chars[10];
	sprintf(light_index_chars, "[%i]", lightIndex);
	return (uniformArrayName + std::string(light_index_chars));
}

const float4x4 ComponentLight::GetFrustViewMatrix() const
{
	math::float4x4 matrix = m_LightFrustum.ViewMatrix();
	return matrix.Transposed();
}

const float4x4 ComponentLight::GetFrustProjectionMatrix() const
{
	math::float4x4 matrix = m_LightFrustum.ProjectionMatrix();
	return matrix.Transposed();
}

const float4x4 ComponentLight::GetFrustViewProjMatrix() const
{
	math::float4x4 matrix = m_LightFrustum.ViewProjMatrix();
	return matrix.Transposed();
}


// -------------------------------------------------------------------------------------------
void ComponentLight::Draw()
{
	if (!m_DrawMesh || !active)
		return;

	// --- Set Uniforms ---
	uint shaderID = App->renderer3D->defaultShader->ID;
	glUseProgram(shaderID);

	int TextureLocation = glGetUniformLocation(shaderID, "u_UseTextures");
	glUniform1i(TextureLocation, 0);
	GLint vertexColorLocation = glGetUniformLocation(shaderID, "u_Color");
	glUniform4f(vertexColorLocation, m_Color.x, m_Color.y, m_Color.z, 1.0f);

	ComponentTransform* trans = GetContainerGameObject()->GetComponent<ComponentTransform>();
	if (trans)
	{
		GLint modelLoc = glGetUniformLocation(shaderID, "u_Model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, trans->GetGlobalTransform().Transposed().ptr());
	}

	GLint viewLoc = glGetUniformLocation(shaderID, "u_View");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLViewMatrix().ptr());

	GLint projectLoc = glGetUniformLocation(shaderID, "u_Proj");
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLProjectionMatrix().ptr());

	glUniform1i(TextureLocation, 0); //reset texture location

	// --- Draw plane with given texture ---
	glBindVertexArray(App->scene_manager->sphere->VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, App->scene_manager->sphere->EBO);
	glDrawElements(GL_TRIANGLES, App->scene_manager->sphere->IndicesSize, GL_UNSIGNED_INT, NULL); // render primitives from array data

	glBindVertexArray(0);
}

void ComponentLight::DrawFrustum()
{
	// --- Draw Frustum ---
	if (App->renderer3D->display_grid)
		App->renderer3D->DrawFrustum(m_LightFrustum, White);
}

void ComponentLight::DrawDirectionLines(bool spotlight, float3 pos, float3 front)
{
	float3 dlA = pos, dlB = front.Normalized() * 1.5f;
	Color dl_color = Color(255.0f, 255.0f, 50.0f);
	float3 dl_Yoff = float3(0.0f, 0.2f, 0.0f), dl_Xoff = float3(0.2f, 0.0f, 0.0f);

	if (spotlight)
		dlB = -dlB;

	// Central Line
	App->renderer3D->DrawLine(float4x4::identity, dlA, (dlA + dlB), dl_color);

	// Lines in Y axis
	App->renderer3D->DrawLine(float4x4::identity, dlA + dl_Yoff, (dlA + dl_Yoff + dlB), dl_color);
	App->renderer3D->DrawLine(float4x4::identity, dlA - dl_Yoff, (dlA - dl_Yoff + dlB), dl_color);
	App->renderer3D->DrawLine(float4x4::identity, dlA + dl_Yoff * 2.0f, (dlA + dl_Yoff * 2.0f + dlB), dl_color);
	App->renderer3D->DrawLine(float4x4::identity, dlA - dl_Yoff * 2.0f, (dlA - dl_Yoff * 2.0f + dlB), dl_color);

	// Lines in X axis
	App->renderer3D->DrawLine(float4x4::identity, dlA + dl_Xoff, (dlA + dl_Xoff + dlB), dl_color);
	App->renderer3D->DrawLine(float4x4::identity, dlA - dl_Xoff, (dlA - dl_Xoff + dlB), dl_color);
	App->renderer3D->DrawLine(float4x4::identity, dlA + dl_Xoff * 2.0f, (dlA + dl_Xoff * 2.0f + dlB), dl_color);
	App->renderer3D->DrawLine(float4x4::identity, dlA - dl_Xoff * 2.0f, (dlA - dl_Xoff * 2.0f + dlB), dl_color);

	// Line in X&Y axis
	App->renderer3D->DrawLine(float4x4::identity, dlA + dl_Xoff + dl_Yoff, (dlA + dl_Xoff + dl_Yoff + dlB), dl_color);
	App->renderer3D->DrawLine(float4x4::identity, dlA - dl_Xoff + dl_Yoff, (dlA - dl_Xoff + dl_Yoff + dlB), dl_color);
	App->renderer3D->DrawLine(float4x4::identity, dlA + dl_Xoff - dl_Yoff, (dlA + dl_Xoff - dl_Yoff + dlB), dl_color);
	App->renderer3D->DrawLine(float4x4::identity, dlA - dl_Xoff - dl_Yoff, (dlA - dl_Xoff - dl_Yoff + dlB), dl_color);
}

// -------------------------------------------------------------------------------------------
// --------------------------------------- UI Inspector --------------------------------------
// -------------------------------------------------------------------------------------------
void ComponentLight::CreateInspectorNode()
{
	// --- Type ---
	ImGui::NewLine(); ImGui::Separator();
	static ImGuiComboFlags flags = 0;

	ImGui::Text("Light Type");
	ImGui::SameLine();

	std::string current_type = GetStringFromLightType(m_LightType);
	if (ImGui::BeginCombo("##LightType", current_type.c_str(), flags))
	{
		for (uint i = 0; i < (uint)LightType::MAX_LIGHT_TYPES; ++i)
		{
			std::string selectable_type = GetStringFromLightType(LightType(i));
			bool is_selected = (current_type == selectable_type);

			if (ImGui::Selectable(selectable_type.c_str(), is_selected))
				SetLightType(LightType(i));
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}

		ImGui::EndCombo();
	}

	// --- Draw Mesh ---
	ImGui::Text("Draw Mesh");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 11.0f);
	ImGui::Checkbox("##DrawMesh", &m_DrawMesh);

	// --- Color ---
	ImGui::Separator(); ImGui::NewLine();
	ImGui::Text("Color");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 129.0f);
	ImGui::ColorEdit4("##LightColor", (float*)&m_Color, ImGuiColorEditFlags_NoInputs);		

	// --- Intensity ---
	ImGui::Text("Intensity");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 102.0f);
	ImGui::SetNextItemWidth(300.0f);
	ImGui::SliderFloat("##Light_Intensity", &m_Intensity, 0.0f, 100.0f, "%.3f", 2.0f);
	//ImGui::NewLine();

	// --- Distance Multiplier ---
	ImGui::Text("Distance Multiplier");
	ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 32.0f);
	ImGui::SetNextItemWidth(65.0f);
	ImGui::DragFloat("##DistMulti", &m_DistanceMultiplier, 0.1f, 0.1f, INFINITY, "%.4f");
	ImGui::NewLine();

	// --- Directional Editor ---
	if (m_LightType == LightType::DIRECTIONAL)
	{
		if (ImGui::TreeNodeEx("Light Shadows", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// --- Set as Shadows Source ---
			bool changeLight = false;
			ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
			ImGui::Text("Shadows Source");
			ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 41.0f);
			if (ImGui::Checkbox("##Light_ShadowsSource", &m_CurrentShadower)) changeLight = true;
			ImGui::NewLine();

			if (changeLight)
			{
				if (m_CurrentShadower)
					App->renderer3D->SetShadowerLight(this);
				else
					App->renderer3D->SetShadowerLight(nullptr);

				changeLight = false;
			}

			if (App->renderer3D->GetShadowerLight() == this)
			{
				// --- Draw Light Frustum ---
				//ImGui::NewLine();
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::Text("Draw Frustum");
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 55.0f);
				ImGui::Checkbox("##LightFrustumDraw", &dir_debug_draw);
				ImGui::NewLine();

				if (dir_debug_draw)
				{
					// --- Frustum Size ---
					ImGui::NewLine();
					ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
					ImGui::Text("Frustum Size");
					ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 35.0f);
					ImGui::SetNextItemWidth(150.0f);
					if (ImGui::DragFloat2("##DLightFrustumSize", frustum_size.ptr(), 1.0f, 0.01f, INFINITY, "%.2f"))
						m_LightFrustum.SetOrthographic(frustum_size.x, frustum_size.y);

					ImGui::NewLine();
					ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
					ImGui::Text("Frustum Planes");
					ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 21.0f);
					ImGui::SetNextItemWidth(75.0f);
					if (ImGui::DragFloat("##DLightFrustumPlanesX", &frusutm_planes.x, 0.001f, 0.001f, frusutm_planes.y - 0.001f))
						m_LightFrustum.SetViewPlaneDistances(frusutm_planes.x, frusutm_planes.y);
					
					ImGui::SameLine(); ImGui::SetNextItemWidth(75.0f);
					if (ImGui::DragFloat("##DLightFrustumPlanesY", &frusutm_planes.y, 0.005f, frusutm_planes.x + 0.001f, INFINITY))
						m_LightFrustum.SetViewPlaneDistances(frusutm_planes.x, frusutm_planes.y);

					ImGui::NewLine();
				}

				// --- Shadows Intensity ---
				ImGui::NewLine();
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
				ImGui::Text("Shadows Intensity");
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::SetNextItemWidth(300.0f);
				ImGui::SliderFloat("##Shadow_Intensity", &m_ShadowsIntensity, 0.0f, 30.0f, "%.3f", 2.0f);

				// --- Shadows Bias ---
				ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
				ImGui::Text("Shadows Bias");
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 35.0f);
				ImGui::SetNextItemWidth(300.0f);
				ImGui::SliderFloat("##Shadow_Bias", &m_ShadowBias, 0.0000f, 0.1000f, "%.5f", 2.0f);
				ImGui::NewLine();

				// --- Shadows Clamp ---
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
				ImGui::Text("Clamp Shadows");
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 28.0f);
				ImGui::SetNextItemWidth(300.0f);
				ImGui::Checkbox("##ShadowsClamp", &m_ClampShadows);
				ImGui::NewLine();

				// --- Shadows Smooth Multipliplier ---
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
				ImGui::Text("Smooth Multiplier");
				ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
				ImGui::SetNextItemWidth(300.0f);
				ImGui::SliderFloat("##Shadow_SmMulti", &m_ShadowSmoothMultiplier, -3.0f, 3.0f, "%.2f");
				ImGui::NewLine();

				// --- Shadow Smoother Algorithm ---
				int index = (int)m_ShadowsSmoother;
				ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 20.0f);
				if (App->gui->HandleDropdownSelector(index, "##ShadowsBlurAlgSel", m_ShadowBlurAlgorithmsNamesVec, 3))
					m_ShadowsSmoother = (ShadowSmoother)index;

				if (m_ShadowsSmoother == ShadowSmoother::POISSON_DISK || m_ShadowsSmoother == ShadowSmoother::BOTH)
				{
					// --- Poisson Offset Blur ---
					ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
					ImGui::Text("Poisson Offset");
					ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 17.0f);
					ImGui::SetNextItemWidth(300.0f);
					ImGui::SliderFloat("##Shadow_PoissOffBlur", &m_ShadowOffsetBlur, 0.0000f, 1.0000f, "%.5f", 2.0f);

					// --- Poisson Smoother Quantity ---
					ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
					ImGui::Text("Poisson Blur");
					ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 31.0f);
					ImGui::SetNextItemWidth(300.0f);
					ImGui::SliderFloat("##Shadow_PoissBlurQuantity", &m_ShadowPoissonBlur, 0.0f, 20000.0f, "%.1f", 2.0f);
					ImGui::NewLine();
				}

				if (m_ShadowsSmoother == ShadowSmoother::PCF || m_ShadowsSmoother == ShadowSmoother::BOTH)
				{
					// --- PCF Smoother Divisor ---
					ImGui::NewLine(); ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 23.0f);
					ImGui::Text("PCF Smoother");
					ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x + 31.0f);
					ImGui::SetNextItemWidth(300.0f);
					ImGui::SliderFloat("##Shadow_PCFBlurDiv", &m_ShadowPCFDivisor, 0.01f, 20.0f, "%.2f");
					ImGui::NewLine();
				}
			}

			ImGui::TreePop();
		}
	}
	else if(m_CurrentShadower)
		App->renderer3D->SetShadowerLight(nullptr);

	// --- Spotlight & Pointlight Editor ---
	if (m_LightType == LightType::SPOTLIGHT)
	{
		// --- Cutoff ---
		ImGui::Separator(); ImGui::NewLine();
		ImGui::Text("Inner Cutoff:"); ImGui::SameLine(); ImGui::SetNextItemWidth(65.0f);
		ImGui::DragFloat("##InCut", &m_InOutCutoffDegrees.x, 0.1f, 0.00f, m_InOutCutoffDegrees.y - 0.01f);
		ImGui::Text("Outer Cutoff:"); ImGui::SameLine(); ImGui::SetNextItemWidth(65.0f);
		ImGui::DragFloat("##OutCut", &m_InOutCutoffDegrees.y, 0.01f, m_InOutCutoffDegrees.x + 0.01f, 360.00f);
		ImGui::NewLine();
	}

	if (m_LightType == LightType::SPOTLIGHT || m_LightType == LightType::POINTLIGHT)
	{
		// --- Attenuation ---	
		ImGui::Separator(); ImGui::NewLine();
		ImGui::Text("Constant Attenuation Value (K):"); ImGui::SameLine(); ImGui::SetNextItemWidth(65.0f);
		ImGui::DragFloat("##AttK", &m_AttenuationKLQFactors.x, 0.001f, 0.000f);
		ImGui::Text("Linear Attenuation Value (L):"); ImGui::SameLine(); ImGui::SetNextItemWidth(65.0f);
		ImGui::DragFloat("##AttL", &m_AttenuationKLQFactors.y, 0.001f, 0.000f);
		ImGui::Text("Quadratic Attenuation Value (Q):"); ImGui::SameLine(); ImGui::SetNextItemWidth(65.0f);
		ImGui::DragFloat("##AttQ", &m_AttenuationKLQFactors.z, 0.00001f, 0.000000f, 10.0f, "%.5f");

		// --- Back to Default Values ---
		if (ImGui::Button("Default", { 57, 18 }))
			m_AttenuationKLQFactors = float3(1.0f, 0.09f, 0.032f);
	}
}


// -------------------------------------------------------------------------------------------
// --------------------------------------- SAVE & LOAD ---------------------------------------
// -------------------------------------------------------------------------------------------
json ComponentLight::Save() const
{
	json node;

	node["Active"] = this->active;
	node["DrawMesh"] = m_DrawMesh;

	node["DirectionX"] = std::to_string(m_Direction.x);
	node["DirectionY"] = std::to_string(m_Direction.y);
	node["DirectionZ"] = std::to_string(m_Direction.z);

	node["ColorR"] = std::to_string(m_Color.x);
	node["ColorG"] = std::to_string(m_Color.y);
	node["ColorB"] = std::to_string(m_Color.z);

	node["AttenuationK"] = std::to_string(m_AttenuationKLQFactors.x);
	node["AttenuationL"] = std::to_string(m_AttenuationKLQFactors.y);
	node["AttenuationQ"] = std::to_string(m_AttenuationKLQFactors.z);

	node["InnerCutoff"] = std::to_string(m_InOutCutoffDegrees.x);
	node["OuterCutoff"] = std::to_string(m_InOutCutoffDegrees.y);

	node["Intensity"] = std::to_string(m_Intensity);
	node["LightType"] = std::to_string((int)m_LightType);
	node["DistanceMultiplier"] = std::to_string(m_DistanceMultiplier);

	node["ShadowsSource"] = m_CurrentShadower;
	node["ShadowIntensity"] = m_ShadowsIntensity;
	node["ShadowBias"] = m_ShadowBias;
	node["ClampShadows"] = m_ClampShadows;
	node["ShadowsSmoothMulti"] = m_ShadowSmoothMultiplier;
	node["ShadowSmootherAlg"] = (int)m_ShadowsSmoother;
	node["ShadowOffsetBlur"] = m_ShadowOffsetBlur;
	node["ShadowPoissonBlur"] = m_ShadowPoissonBlur;
	node["ShadowPCFDivisor"] = m_ShadowPCFDivisor;
	node["FrustumSizeX"] = frustum_size.x;
	node["FrustumSizeY"] = frustum_size.y;
	node["DebugDraw"] = dir_debug_draw;


	return node;
}

void ComponentLight::Load(json& node)
{
	this->active = node["Active"].is_null() ? true : (bool)node["Active"];
	m_DrawMesh = node["DrawMesh"].is_null() ? true : (bool)node["DrawMesh"];

	// --- Load Strings ---
	std::string str_dirX = node["DirectionX"].is_null() ? "0" : node["DirectionX"];
	std::string str_dirY = node["DirectionY"].is_null() ? "0" : node["DirectionY"];
	std::string str_dirZ = node["DirectionZ"].is_null() ? "0" : node["DirectionZ"];

	std::string str_colR = node["ColorR"].is_null() ? "1" : node["ColorR"];
	std::string str_colG = node["ColorG"].is_null() ? "1" : node["ColorG"];
	std::string str_colB = node["ColorB"].is_null() ? "1" : node["ColorB"];

	std::string str_AttK = node["AttenuationK"].is_null() ? "1" : node["AttenuationK"];
	std::string str_AttL = node["AttenuationL"].is_null() ? "0.09" : node["AttenuationL"];
	std::string str_AttQ = node["AttenuationQ"].is_null() ? "0.032" : node["AttenuationQ"];

	std::string str_inCut = node["InnerCutoff"].is_null() ? "12.5" : node["InnerCutoff"];
	std::string str_outCut = node["OuterCutoff"].is_null() ? "45" : node["OuterCutoff"];

	std::string str_intensity = node["Intensity"].is_null() ? "0.5" : node["Intensity"];
	std::string str_distMult = node["DistanceMultiplier"].is_null() ? "1.0" : node["DistanceMultiplier"];
	std::string str_LType = node["LightType"].is_null() ? "1" : node["LightType"];

	// --- Pass Strings to the needed Data Type
	m_Direction = float3(std::stof(str_dirX), std::stof(str_dirY), std::stof(str_dirZ));
	m_Color = float3(std::stof(str_colR), std::stof(str_colG), std::stof(str_colB));

	m_AttenuationKLQFactors = float3(std::stof(str_AttK), std::stof(str_AttL), std::stof(str_AttQ));
	m_InOutCutoffDegrees = float2(std::stof(str_inCut), std::stof(str_outCut));

	m_Intensity = std::stof(str_intensity);
	m_LightType = (LightType)(std::stoi(str_LType));
	m_DistanceMultiplier = std::stof(str_distMult);

	// --- Shadows Load ---
	m_CurrentShadower = node.find("ShadowsSource") == node.end() ? false : node["ShadowsSource"].get<bool>();
	m_ShadowsIntensity = node.find("ShadowIntensity") == node.end() ? 1.0f : node["ShadowIntensity"].get<float>();
	m_ShadowBias = node.find("ShadowBias") == node.end() ? 0.001f : node["ShadowBias"].get<float>();
	m_ClampShadows = node.find("ClampShadows") == node.end() ? false : node["ClampShadows"].get<bool>();
	m_ShadowSmoothMultiplier = node.find("ShadowsSmoothMulti") == node.end() ? 1.0f : node["ShadowsSmoothMulti"].get<float>();

	frustum_size.x = node.find("FrustumSizeX") == node.end() ? 50.0f : node["FrustumSizeX"].get<float>();
	frustum_size.y = node.find("FrustumSizeY") == node.end() ? 50.0f : node["FrustumSizeY"].get<float>();

	m_LightFrustum.SetOrthographic(frustum_size.x, frustum_size.y);

	dir_debug_draw = node.find("DebugDraw") == node.end() ? false : node["DebugDraw"].get<bool>();

	m_ShadowsSmoother = node.find("ShadowSmootherAlg") == node.end() ? ShadowSmoother::POISSON_DISK : (ShadowSmoother)node["ShadowSmootherAlg"].get<int>();
	m_ShadowOffsetBlur = node.find("ShadowOffsetBlur") == node.end() ? 0.2f : node["ShadowOffsetBlur"].get<float>();
	m_ShadowPoissonBlur = node.find("ShadowPoissonBlur") == node.end() ? 700.0f : node["ShadowPoissonBlur"].get<float>();
	m_ShadowPCFDivisor = node.find("ShadowPCFDivisor") == node.end() ? 9.0f : node["ShadowPCFDivisor"].get<float>();

	if (m_CurrentShadower)
		App->renderer3D->SetShadowerLight(this);
}