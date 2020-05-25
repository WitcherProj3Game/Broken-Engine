#include "Particle.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleSceneManager.h"
#include "OpenGL.h"
#include "ModuleTextures.h"
#include "ModuleResourceManager.h"

#include "ResourceTexture.h"
#include "ResourceMesh.h"

#include "ResourceShader.h"
#include "ResourceMesh.h"
#include "ComponentCamera.h"
#include "ComponentLight.h"


#include "mmgr/mmgr.h"

using namespace Broken;

Particle::Particle()
{
	position = { 0.0f,0.0f,0.0f };
	lifeTime = 1000;
	diameter = 1;
	plane = App->scene_manager->plane;
}

Particle::~Particle()
{}

void Particle::SetAnimation(ResourceMesh* mesh)
{
	plane = mesh;
}

void Particle::Draw(bool shadowsPass)
{
	// --- Update transform and rotation to face camera ---
	float3 center = float3(position.x, position.y, position.z);

	// --- Frame image with camera ---
	float4x4 rot = float4x4::FromEulerXYZ(rotation.x, rotation.y, rotation.z);
	float3x3 camRot = App->renderer3D->active_camera->GetOpenGLViewMatrix().RotatePart();
	float4x4 finalRot = float4x4::identity;

	if (v_billboard)
	{
		finalRot = camRot * rot;
		finalRot.SetCol(1, { 0.0f, 1.0f, 0.0f, 0.0f });
	}
	else if (h_billboard)
		finalRot.SetRotatePartX(-1.57f);
	else
		finalRot = camRot * rot;

	// --- Shader Choice ---
	uint shaderID = 0;
	if (shadowsPass)
		shaderID = App->renderer3D->shadowsShader->ID;
	else
		shaderID = App->renderer3D->defaultShader->ID;

	// --- Bind ---
	glUseProgram(shaderID);

	// --- Shader Matrices ---
	float4x4 transform = transform.FromTRS(float3(position.x, position.y, position.z), finalRot, float3(scale.x, scale.y, 1));
	float4x4 projMat = float4x4::identity;
	float4x4 viewMat = float4x4::identity;
	const ComponentLight* light = App->renderer3D->GetShadowerLight();

	if (shadowsPass)
	{
		if (light)
		{
			projMat = light->GetFrustProjectionMatrix();
			viewMat = light->GetFrustViewMatrix();
		}
	}
	else
	{
		projMat = App->renderer3D->active_camera->GetOpenGLProjectionMatrix();
		viewMat = App->renderer3D->active_camera->GetOpenGLViewMatrix();
	}

	if (!shadowsPass && light)
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "u_LightSpace"), 1, GL_FALSE, light->GetFrustViewProjMatrix().ptr());

	glUniformMatrix4fv(glGetUniformLocation(shaderID, "u_View"), 1, GL_FALSE, viewMat.ptr());
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "u_Proj"), 1, GL_FALSE, projMat.ptr());

	GLint modelLoc = glGetUniformLocation(shaderID, "u_Model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transform.Transposed().ptr());

	// --- Texturing & Color ---
	// Texture
	glUniform4f(glGetUniformLocation(shaderID, "u_Color"), color.x, color.y, color.z, color.w);	
	if (texture)
	{
		glUniform1i(glGetUniformLocation(shaderID, "u_UseTextures"), (int)true);
		glUniform1i(glGetUniformLocation(shaderID, "u_HasDiffuseTexture"), (int)true);
		glUniform1i(glGetUniformLocation(shaderID, "u_AlbedoTexture"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->GetTexID());
	}

	if (!shadowsPass)
	{
		glUniform1i(glGetUniformLocation(shaderID, "u_HasTransparencies"), 1);
		glUniform1i(glGetUniformLocation(shaderID, "u_SceneColorAffected"), scene_colorAffected);
		glUniform1i(glGetUniformLocation(shaderID, "u_LightAffected"), light_Affected);

		// --- Particle Receiving Shadows ---
		glUniform1i(glGetUniformLocation(shaderID, "u_ReceiveShadows"), receive_shadows);
	}

	// --- Draw plane with given texture ---
	glBindVertexArray(plane->VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane->EBO);
	glDrawElements(GL_TRIANGLES, plane->IndicesSize, GL_UNSIGNED_INT, NULL); // render primitives from array data

	// --- Back to Defaults ---
	glBindVertexArray(0);
	glUniform1i(glGetUniformLocation(shaderID, "u_HasDiffuseTexture"), 0); //reset texture location
	glUniform1i(glGetUniformLocation(shaderID, "u_UseTextures"), 0);
	glBindTexture(GL_TEXTURE_2D, 0); // Stop using buffer (texture)
}
