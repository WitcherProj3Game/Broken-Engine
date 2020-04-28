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

void Particle::SetAnimation(ResourceMesh* mesh) {
	plane = mesh;
}

void Particle::Draw()
{
	// --- Update transform and rotation to face camera ---
	float3 center = float3(position.x, position.y, position.z);

	// --- Frame image with camera ---
	float4x4 transform = transform.FromTRS(float3(position.x, position.y, position.z),
		App->renderer3D->active_camera->GetOpenGLViewMatrix().RotatePart(),
		float3(scale.x, scale.y, 1));

	// --- Set Uniforms ---
	uint shaderID = App->renderer3D->defaultShader->ID;
	glUseProgram(shaderID);

	GLint modelLoc = glGetUniformLocation(shaderID, "u_Model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transform.Transposed().ptr());

	GLint viewLoc = glGetUniformLocation(shaderID, "u_View");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLViewMatrix().ptr());

	float nearp = App->renderer3D->active_camera->GetNearPlane();

	// right handed projection matrix
	float f = 1.0f / tan(App->renderer3D->active_camera->GetFOV() * DEGTORAD / 2.0f);
	float4x4 proj_RH(
		f / App->renderer3D->active_camera->GetAspectRatio(), 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f,
		position.x * 0.01f, position.y * 0.01f, nearp, 0.0f);

	GLint projectLoc = glGetUniformLocation(shaderID, "u_Proj");
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, proj_RH.ptr());

	//Texturing & Color
	GLint vertexColorLocation = glGetUniformLocation(shaderID, "u_Color");
	glUniform4f(vertexColorLocation, color.x, color.y, color.z, color.w);
	glUniform1i(glGetUniformLocation(shaderID, "u_HasTransparencies"), 1);

	int TextureLocation = glGetUniformLocation(shaderID, "u_UseTextures");
	glUniform1i(TextureLocation, (int)true);

	//ourTexture
	glUniform1i(glGetUniformLocation(shaderID, "u_AlbedoTexture"), 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, texture->GetTexID());

	// --- Draw plane with given texture ---
	glBindVertexArray(plane->VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane->EBO);
	glDrawElements(GL_TRIANGLES, plane->IndicesSize, GL_UNSIGNED_INT, NULL); // render primitives from array data

	glUniform1i(TextureLocation, 0); //reset texture location
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0); // Stop using buffer (texture)

}
