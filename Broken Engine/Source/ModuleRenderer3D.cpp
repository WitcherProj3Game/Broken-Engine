#include "ModuleRenderer3D.h"

// -- Modules --
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleGui.h"
#include "ModuleSceneManager.h"
#include "ModuleCamera3D.h"
#include "ModuleUI.h"
#include "ModuleParticles.h"
#include "ModuleTextures.h"
#include "ModuleTimeManager.h"
#include "ModuleSelection.h"
#include "ModuleResourceManager.h"
#include "ModuleFileSystem.h"

// -- Components --
#include "GameObject.h"
#include "Component.h"
#include "ComponentMesh.h"
#include "ComponentCamera.h"
#include "ComponentTransform.h"
#include "ComponentMeshRenderer.h"
#include "ComponentCollider.h"
#include "ComponentCharacterController.h"
#include "ComponentAudioListener.h"
#include "ComponentLight.h"
#include "ComponentParticleEmitter.h"

// -- Resources --
#include "ResourceShader.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourceTexture.h"
#include "ResourceScene.h"
#include "ImporterShader.h"

#include "OpenGL.h"
#include "Math.h"
#include "LUT/lut.hpp"
#include "Optick/include/optick.h"

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */

#include "mmgr/mmgr.h"

using namespace Broken;

// ---------------------------------------------------------------------------------------------
// ------------------------------ Module -------------------------------------------------------
// ---------------------------------------------------------------------------------------------
ModuleRenderer3D::ModuleRenderer3D(bool start_enabled) : Module(start_enabled)
{
	name = "Renderer3D";

	//Auto Blend Functions
	m_BlendAutoFunctionsVec.push_back("STANDARD INTERPOLATIVE");
	m_BlendAutoFunctionsVec.push_back("ADDITIVE");
	m_BlendAutoFunctionsVec.push_back("ADDITIVE ALPHA AFFECTED");
	m_BlendAutoFunctionsVec.push_back("MULTIPLICATIVE");

	//Blending Equations
	m_BlendEquationFunctionsVec.push_back("ADD (Standard)");
	m_BlendEquationFunctionsVec.push_back("SUBTRACT");
	m_BlendEquationFunctionsVec.push_back("REVERSE_SUBTRACT");
	m_BlendEquationFunctionsVec.push_back("MIN");
	m_BlendEquationFunctionsVec.push_back("MAX");

	//Manual Blend Functions
	m_AlphaTypesVec.push_back("GL_ZERO");
	m_AlphaTypesVec.push_back("GL_ONE");
	m_AlphaTypesVec.push_back("GL_SRC_COLOR");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_SRC_COLOR");

	m_AlphaTypesVec.push_back("GL_DST_COLOR");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_DST_COLOR");
	m_AlphaTypesVec.push_back("GL_SRC_ALPHA (Standard)");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_SRC_ALPHA (Standard)");

	m_AlphaTypesVec.push_back("GL_DST_ALPHA");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_DST_ALPHA");
	m_AlphaTypesVec.push_back("GL_CONSTANT_COLOR");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_CONSTANT_COLOR");

	m_AlphaTypesVec.push_back("GL_CONSTANT_ALPHA");
	m_AlphaTypesVec.push_back("GL_ONE_MINUS_CONSTANT_ALPHA");
	m_AlphaTypesVec.push_back("GL_SRC_ALPHA_SATURATE");

	octoon::image::lut::basic_lut(17, 4);
}

// Destructor
ModuleRenderer3D::~ModuleRenderer3D() {
}

// Called before render is available
bool ModuleRenderer3D::Init(json& file)
{
	ENGINE_AND_SYSTEM_CONSOLE_LOG("Creating 3D Renderer context");
	bool ret = true;

	//Create context
	context = SDL_GL_CreateContext(App->window->window);

	if (context == NULL)
	{
		ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]: OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	if (ret == true)
	{
		//Use Vsync
		if (vsync && SDL_GL_SetSwapInterval(1) < 0)
			ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]: Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

		// Initialize glad
		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
		{
			ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]: Error initializing glad! %s\n");
			ret = false;
		}
		else
		{
			GL_SETERRORHANDLER(4, 4); //OpenGL Error Handler
			LoadStatus(file);
		}
	}

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);


	// --- z values from 0 to 1 and not -1 to 1, more precision in far ranges ---
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	// --- Enable stencil testing, set to replace ---
	glEnable(GL_DEPTH_TEST); //For shadows
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);


	// --- Create screen quad ---
	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// --- Check if graphics driver supports shaders in binary format ---
	//GLint formats = 0;
	//glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	//if (formats < 1) {
	//	std::cerr << "Driver does not support any binary formats." << std::endl;
	//	exit(EXIT_FAILURE);
	//}

	ENGINE_AND_SYSTEM_CONSOLE_LOG("OpenGL Version: %s", glGetString(GL_VERSION));
	ENGINE_AND_SYSTEM_CONSOLE_LOG("Glad Version: 0.1.33"); //Glad has no way to check its version


	//Projection matrix for
	OnResize(App->window->GetWindowWidth(), App->window->GetWindowHeight());

	// --- Create adaptive grid ---
	glGenVertexArrays(1, &Grid_VAO);
	glGenBuffers(1, &Grid_VBO);
	CreateGrid(10.0f);

	glGenVertexArrays(1, &PointLineVAO);

	// --- Create camera to take model/meshes screenshots ---
	screenshot_camera = new ComponentCamera(nullptr);
	screenshot_camera->name = "Screenshot";
	screenshot_camera->frustum.SetPos(float3(0.0f, 25.0f, -50.0f));
	screenshot_camera->SetFOV(60.0f);
	screenshot_camera->Look({ 0.0f, 0.0f, 0.0f });


	// MYTODO: Currently creating twice the texture since res manager will import it from Images folder,
	// i should make a default skybox that does not need textures and then let the user change it through a new panel (check Unity)
	// --- Load skybox textures ---
	uint width, height = 0;
	ResourceTexture* Texright = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "SkyboxTexRight");
	ResourceTexture* Texleft = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "SkyboxTexLeft");
	ResourceTexture* Texback = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "SkyboxTexBack");
	ResourceTexture* Texfront = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "SkyboxTexFront");
	ResourceTexture* Textop = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "SkyboxTexTop");
	ResourceTexture* Texbottom = (ResourceTexture*)App->resources->CreateResource(Resource::ResourceType::TEXTURE, "SkyboxTexBottom");

	Texright->SetTextureID(App->textures->CreateTextureFromFile("Assets/Images/right.jpg", width, height, -1));
	Texleft->SetTextureID(App->textures->CreateTextureFromFile("Assets/Images/left.jpg", width, height, -1));
	Texback->SetTextureID(App->textures->CreateTextureFromFile("Assets/Images/back.jpg", width, height, -1));
	Texfront->SetTextureID(App->textures->CreateTextureFromFile("Assets/Images/front.jpg", width, height, -1));
	Textop->SetTextureID(App->textures->CreateTextureFromFile("Assets/Images/top.jpg", width, height, -1));
	Texbottom->SetTextureID(App->textures->CreateTextureFromFile("Assets/Images/bottom.jpg", width, height, -1));

	std::vector<uint> cubemaptexIDs;
	cubemaptexIDs.push_back(Texright->GetTexID());
	cubemaptexIDs.push_back(Texleft->GetTexID());
	cubemaptexIDs.push_back(Texbottom->GetTexID());
	cubemaptexIDs.push_back(Textop->GetTexID());
	cubemaptexIDs.push_back(Texfront->GetTexID());
	cubemaptexIDs.push_back(Texback->GetTexID());


	float skyboxVertices[] = {
		// positions
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	cubemapTexID = App->textures->CreateCubemap(cubemaptexIDs);

	return ret;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	OPTICK_CATEGORY("Renderer PreUpdate", Optick::Category::Rendering);

	// --- Update OpenGL Capabilities ---
	UpdateGLCapabilities();

	// --- Clear stencil buffer, enable write ---
	glStencilMask(0xFF);
	glClearStencil(0);

	// --- Clear framebuffers ---
	float backColor = 0.65f;
	glClearColor(backColor, backColor, backColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearDepth(0.0f);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(backColor, backColor, backColor, 1.0f);
	glClearDepth(0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return UPDATE_CONTINUE;
}


// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	OPTICK_CATEGORY("Renderer PostUpdate", Optick::Category::Rendering);

	// --- Set Shader Matrices ---
	GLint viewLoc = glGetUniformLocation(defaultShader->ID, "u_View");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLViewMatrix().ptr());

	float nearp = App->renderer3D->active_camera->GetNearPlane();

	// right handed projection matrix (just different standard)
	float f = 1.0f / tan(App->renderer3D->active_camera->GetFOV() * DEGTORAD / 2.0f);
	float4x4 proj_RH(
		f / App->renderer3D->active_camera->GetAspectRatio(), 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, nearp, 0.0f);

	GLint projectLoc = glGetUniformLocation(defaultShader->ID, "u_Proj");
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, proj_RH.ptr());

	GLint modelLoc = glGetUniformLocation(defaultShader->ID, "u_Model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, float4x4::identity.Transposed().ptr());

	GLint camPosLoc = glGetUniformLocation(defaultShader->ID, "u_CameraPosition");
	float3 camPos = App->renderer3D->active_camera->GetCameraPosition();
	glUniform3f(camPosLoc, camPos.x, camPos.y, camPos.z);

	// --- Bind fbo ---
    if (renderfbo)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// --- Do not write to the stencil buffer ---
	glStencilMask(0x00);

	OPTICK_PUSH("Skybox Rendering");
	DrawSkybox(); // could not manage to draw it after scene with reversed-z ...
	OPTICK_POP();

	// --- Set depth filter to greater (Passes if the incoming depth value is greater than the stored depth value) ---
	glDepthFunc(GL_GREATER);

	// --- Issue Render orders ---
	OPTICK_PUSH("Scene Rendering");
	App->scene_manager->DrawScene();
	OPTICK_POP();


	for (std::map<uint, ResourceShader*>::const_iterator it = App->resources->shaders.begin(); it != App->resources->shaders.end(); ++it)
	{
		if((*it).second)
			SendShaderUniforms((*it).second->ID);
	}

	// --- Draw Grid ---
	if (display_grid)
		DrawGrid();

	OPTICK_PUSH("Meshes Lines and Boxes Rendering");
	DrawRenderMeshes();
	DrawRenderLines();
	DrawRenderBoxes();
	OPTICK_POP();

	// --- Selected Object Outlining ---
	HandleObjectOutlining();

	// --- Draw ---
	glEnable(GL_BLEND);
	if(m_ChangedBlending)
		SetRendererBlending(); //Set Blending to Renderer's Default

	DrawTransparentRenderMeshes();

	// -- Draw particles ---
	OPTICK_PUSH("Particles Rendering");
	for (int i = 0; i < particleEmitters.size(); ++i)
	{
		particleEmitters[i]->DrawParticles();

		// --- Set Blending to Renderer's Default ---
		if (m_ChangedBlending)
			SetRendererBlending();
	}

	OPTICK_POP();

	glDisable(GL_BLEND);

	OPTICK_PUSH("Lights Rendering");
	std::vector<ComponentLight*>::iterator LightIterator = m_LightsVec.begin();
	for (; LightIterator != m_LightsVec.end(); ++LightIterator)
		(*LightIterator)->Draw();
	OPTICK_POP();

	OPTICK_PUSH("UI Rendering");
	App->ui_system->Draw();
	OPTICK_POP();

	// --- Back to defaults ---
	glDepthFunc(GL_LESS);

	// --- Unbind fbo ---
	if (renderfbo)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// -- Draw framebuffer texture ---
	OPTICK_PUSH("Post Processing Rendering");
	if (post_processing || drawfb)
		DrawPostProcessing();
	OPTICK_POP();

	// --- Draw GUI and swap buffers ---
	OPTICK_PUSH("GUI Rendering");
	App->gui->Draw();
	OPTICK_POP();


	// --- To prevent problems with viewports, disabled due to crashes and conflicts with docking, sets a window as current rendering context ---
	SDL_GL_MakeCurrent(App->window->window, context);
	SDL_GL_SwapWindow(App->window->window);

	// --- Clear render orders ---
	ClearRenderOrders();

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	ENGINE_AND_SYSTEM_CONSOLE_LOG("Destroying 3D Renderer");

	m_LightsVec.clear();

	delete screenshot_camera;

	glDeleteBuffers(1, (GLuint*)&Grid_VBO);
	glDeleteVertexArrays(1, &Grid_VAO);

	glDeleteBuffers(1, (GLuint*)&quadVBO);
	glDeleteVertexArrays(1, &quadVAO);

	glDeleteFramebuffers(1, &fbo);
	SDL_GL_DeleteContext(context);

	return true;
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	// --- Called by UpdateWindowSize() in Window module this when resizing windows to prevent rendering issues ---

	// --- Resetting View matrices ---
	glViewport(0, 0, width, height);

	if (width > height)
		active_camera->SetAspectRatio(width / height);
	else
		active_camera->SetAspectRatio(height / width);

	glDeleteFramebuffers(1, &fbo);
	CreateFramebuffer();
}


// ---------------------------------------------------------------------------------------------
// ------------------------------ Setters ------------------------------------------------------
// ---------------------------------------------------------------------------------------------
void ModuleRenderer3D::SetAmbientColor(const float3& color)
{
	if(App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_SceneColor = color;
}

void ModuleRenderer3D::SetGammaCorrection(float gammaCorr)
{
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_SceneGammaCorrection = gammaCorr;
}

void ModuleRenderer3D::SetRendererBlendingAutoFunction(BlendAutoFunction function) 
{ 
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_RendererBlendFunc = function;
}

void ModuleRenderer3D::SetRendererBlendingEquation(BlendingEquations eq) 
{
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_BlendEquation = eq;
}

void ModuleRenderer3D::SetRendererBlendingManualFunction(BlendingTypes src, BlendingTypes dst) 
{ 
	if (App->scene_manager->currentScene)
	{
		App->scene_manager->currentScene->m_ManualBlend_Src = src;
		App->scene_manager->currentScene->m_ManualBlend_Dst = dst;
	}
}

void ModuleRenderer3D::SetSkyboxColor(const float3& color) 
{ 
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_Sky_ColorTint = color;
}

void ModuleRenderer3D::SetSkyboxExposure(float value) 
{ 
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_Sky_Exposure = value;
}

void ModuleRenderer3D::SetSkyboxRotation(const float3& rot)
{
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_Sky_Rotation = rot;
}


void ModuleRenderer3D::SetPostProHDRExposure(float exposure) 
{ 
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_ScenePP_HDRExposure = exposure;
}

void ModuleRenderer3D::SetPostProGammaCorrection(float value) 
{ 
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_ScenePP_GammaCorr = value;
}

void ModuleRenderer3D::SetPostProBloomMinBrightness(float value)
{
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_ScenePP_BloomMinBrightness = value;
}

void ModuleRenderer3D::SetPostProBloomBrightnessThreshold(float3 value)
{
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_ScenePP_BloomBrightnessThreshold = value;
}

void ModuleRenderer3D::SetPostProBloomBlur(uint amount)
{
	if (App->scene_manager->currentScene)
		App->scene_manager->currentScene->m_ScenePP_BloomBlurAmount = amount;
}

void ModuleRenderer3D::SetPostProBloomWeights(float3 weights1, float2 weights2)
{
	if (App->scene_manager->currentScene)
	{
		App->scene_manager->currentScene->m_ScenePP_BlurWeights1 = weights1;
		App->scene_manager->currentScene->m_ScenePP_BlurWeights2 = weights2;
	}
}

// --- Getters ---
float ModuleRenderer3D::GetGammaCorrection() const 
{
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_SceneGammaCorrection;
	else
		return -1.0f;
}

float3 ModuleRenderer3D::GetSceneAmbientColor() const 
{
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_SceneColor;
	else
		return -float3::one;
}

BlendAutoFunction ModuleRenderer3D::GetRendererBlendAutoFunction() const 
{ 
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_RendererBlendFunc;
	else
		return BlendAutoFunction::STANDARD_INTERPOLATIVE;
}

BlendingEquations ModuleRenderer3D::GetRendererBlendingEquation() 
{
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_BlendEquation;
	else
		return BlendingEquations::ADD;
}

void ModuleRenderer3D::GetRendererBlendingManualFunction(BlendingTypes& src, BlendingTypes& dst) const 
{ 
	if (App->scene_manager->currentScene)
	{
		src = App->scene_manager->currentScene->m_ManualBlend_Src;
		dst = App->scene_manager->currentScene->m_ManualBlend_Dst;
	}
	else
	{
		src = BlendingTypes::ZERO;
		dst = BlendingTypes::ZERO;
	}
}

float3 ModuleRenderer3D::GetSkyboxColor() const 
{
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_Sky_ColorTint;
	else
		return -float3::one;
}

float ModuleRenderer3D::GetSkyboxExposure() const 
{
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_Sky_Exposure;
	else
		return -1.0f;
}

float3 ModuleRenderer3D::GetSkyboxRotation() const
{
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_Sky_Rotation;
	else
		return -float3::one;
}

float ModuleRenderer3D::GetPostProGammaCorrection() const 
{
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_ScenePP_GammaCorr;
	else
		return -1.0f;
}

float ModuleRenderer3D::GetPostProHDRExposure() const 
{ 
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_ScenePP_HDRExposure;
	else
		return -1.0f;
}

float ModuleRenderer3D::GetPostProBloomMinBrightness() const
{
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_ScenePP_BloomMinBrightness;
	else
		return -1.0f;
}

float3 ModuleRenderer3D::GetPostProBloomBrightnessThreshold() const
{
	if (App->scene_manager->currentScene)
		return App->scene_manager->currentScene->m_ScenePP_BloomBrightnessThreshold;
	else
		return -float3::one;
}

void ModuleRenderer3D::GetPostProBloomWeights(float3& weights1, float2& weights2)
{
	if (App->scene_manager->currentScene)
	{
		weights1 = App->scene_manager->currentScene->m_ScenePP_BlurWeights1;
		weights2 = App->scene_manager->currentScene->m_ScenePP_BlurWeights2;
	}
}

void ModuleRenderer3D::GetPostProBloomBlur(uint& amount)
{
	if (App->scene_manager->currentScene)
		amount = App->scene_manager->currentScene->m_ScenePP_BloomBlurAmount;
}

void ModuleRenderer3D::SetLUT(ResourceTexture* newLUT)
{
	if (newLUT)
		currentLUT = newLUT->GetTexID();
	else
		currentLUT = 0;
}



bool ModuleRenderer3D::SetVSync(bool _vsync)
{
	bool ret = true;

	vsync = _vsync;

	if (vsync) {

		if (SDL_GL_SetSwapInterval(1) == -1)
		{
			ret = false;
			ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]: Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		}
	}
	else {

		if (SDL_GL_SetSwapInterval(0) == -1)
		{
			ret = false;
			ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]: Warning: Unable to set immediate updates! SDL Error: %s\n", SDL_GetError());
		}
	}

	return ret;
}

void ModuleRenderer3D::SetActiveCamera(ComponentCamera* camera)
{
	if (this->active_camera)
		this->active_camera->active_camera = false;

	// if camera is not nullptr, then we set it as active camera, else we set editor camera as active camera
	if (camera != nullptr)
	{
		this->active_camera = camera;
		camera->active_camera = true;
	}
	else
		this->active_camera = App->camera->camera;
}

void ModuleRenderer3D::SetCullingCamera(ComponentCamera* camera)
{
	if (culling_camera)
	{
		culling_camera->culling = false;
	}
	// if camera is not nullptr, then we set it as culling camera, else we set editor camera as culling camera
	this->culling_camera = camera ? camera : App->camera->camera;
	if (camera)
	{
		camera->culling = true;
	}
}


// ---------------------------------------------------------------------------------------------
// ------------------------------ Render Commands ----------------------------------------------
// ---------------------------------------------------------------------------------------------

// --- Add render order to queue ---
void ModuleRenderer3D::DrawMesh(const float4x4 transform, const ResourceMesh* mesh, ResourceMaterial* mat, const ResourceMesh* deformable_mesh, const RenderMeshFlags flags, const Color& color)
{
	// --- Check data validity
	if (transform.IsFinite() && mesh && mat)
	{

		if (mat->has_transparencies)
		{
			// --- Add given instance to relevant vector ---
			if (transparent_render_meshes.find(mesh->GetUID()) != transparent_render_meshes.end())
			{
				RenderMesh rmesh = RenderMesh(transform, mesh, mat, flags/*, color*/);
				rmesh.deformable_mesh = deformable_mesh; // TEMPORAL!
				rmesh.color = color;

				//// --- Search for Character Controller Component ---
				//ComponentCharacterController* cct = App->scene_manager->GetSelectedGameObject()->GetComponent<ComponentCharacterController>();

				//// --- If Found, draw Character Controller shape ---
				//if (cct && cct->IsEnabled())
				//	cct->Draw();

				transparent_render_meshes[mesh->GetUID()].push_back(rmesh);
			}
			else
			{
				// --- Build new vector to store mesh's instances ---
				std::vector<RenderMesh> new_vec;

				RenderMesh rmesh = RenderMesh(transform, mesh, mat, flags/*, color*/);
				rmesh.deformable_mesh = deformable_mesh; // TEMPORAL!
				rmesh.color = color;

				new_vec.push_back(rmesh);
				transparent_render_meshes[mesh->GetUID()] = new_vec;
			}
		}
		else
		{
			// --- Add given instance to relevant vector ---
			if (render_meshes.find(mesh->GetUID()) != render_meshes.end())
			{
				RenderMesh rmesh = RenderMesh(transform, mesh, mat, flags/*, color*/);
				rmesh.deformable_mesh = deformable_mesh; // TEMPORAL!
				rmesh.color = color;

				//// --- Search for Character Controller Component ---
				//ComponentCharacterController* cct = App->scene_manager->GetSelectedGameObject()->GetComponent<ComponentCharacterController>();

				//// --- If Found, draw Character Controller shape ---
				//if (cct && cct->IsEnabled())
				//	cct->Draw();

				render_meshes[mesh->GetUID()].push_back(rmesh);
			}
			else
			{
				// --- Build new vector to store mesh's instances ---
				std::vector<RenderMesh> new_vec;

				RenderMesh rmesh = RenderMesh(transform, mesh, mat, flags/*, color*/);
				rmesh.deformable_mesh = deformable_mesh; // TEMPORAL!
				rmesh.color = color;

				new_vec.push_back(rmesh);
				render_meshes[mesh->GetUID()] = new_vec;
			}
		}
	}
}

void ModuleRenderer3D::DrawLine(const float4x4 transform, const float3 a, const float3 b, const Color& color)
{
	render_lines.push_back(RenderLine(transform, a, b, color));
}

void ModuleRenderer3D::DrawAABB(const AABB& box, const Color& color)
{
	if (box.IsFinite())
		render_aabbs.push_back(RenderBox<AABB>(&box, color));
}

void ModuleRenderer3D::DrawOBB(const OBB& box, const Color& color)
{
	if (box.IsFinite())
		render_obbs.push_back(RenderBox<OBB>(&box, color));
}

void ModuleRenderer3D::DrawFrustum(const Frustum& box, const Color& color)
{
	if (box.IsFinite())
		render_frustums.push_back(RenderBox<Frustum>(&box, color));
}


const std::string & ModuleRenderer3D::RenderSceneToTexture(std::vector<GameObject*>& scene_gos, uint& texId)
{
	if (scene_gos.size() == 0)
		return std::string("");

	// --- Issue render calls and obtain an AABB that encloses all meshes ---

	AABB aabb;
	aabb.SetNegativeInfinity();

	for (uint i = 0; i < scene_gos.size(); ++i)
	{
		scene_gos[i]->Draw();
		aabb.Enclose(scene_gos[i]->GetAABB());
	}

	// --- Frame aabb ---
	screenshot_camera->frustum.SetPos(float3(0.0f, 25.0f, -50.0f));
	screenshot_camera->SetFOV(60.0f);
	screenshot_camera->Look({ 0.0f, 0.0f, 0.0f });

	float3 center = aabb.CenterPoint();

	ComponentCamera* previous_cam = active_camera;
	SetActiveCamera(screenshot_camera);

	float diagonal = aabb.Diagonal().Length() * 0.75f;

	float3 Movement = active_camera->frustum.Front() * (diagonal);

	if (Movement.IsFinite())
		screenshot_camera->frustum.SetPos(center - Movement);

	PreUpdate(0.0f);

	//// --- Set Shader Matrices ---
	//GLint viewLoc = glGetUniformLocation(defaultShader->ID, "view");
	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLViewMatrix().ptr());

	//float nearp = App->renderer3D->active_camera->GetNearPlane();

	//// right handed projection matrix (just different standard)
	//float f = 1.0f / tan(App->renderer3D->active_camera->GetFOV() * DEGTORAD / 2.0f);
	//float4x4 proj_RH(
	//	f / App->renderer3D->active_camera->GetAspectRatio(), 0.0f, 0.0f, 0.0f,
	//	0.0f, f, 0.0f, 0.0f,
	//	0.0f, 0.0f, 0.0f, -1.0f,
	//	0.0f, 0.0f, nearp, 0.0f);

	//GLint projectLoc = glGetUniformLocation(defaultShader->ID, "projection");
	//glUniformMatrix4fv(projectLoc, 1, GL_FALSE, proj_RH.ptr());

	//GLint modelLoc = glGetUniformLocation(defaultShader->ID, "model_matrix");
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, float4x4::identity.Transposed().ptr());

	// --- Bind fbo ---
	if (renderfbo)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// --- Set depth filter to greater (Passes if the incoming depth value is greater than the stored depth value) ---
	glDepthFunc(GL_GREATER);

	// --- Do not write to the stencil buffer ---
	glStencilMask(0x00);

	//// --- Draw Grid ---
	//DrawGrid();

	SendShaderUniforms(defaultShader->ID);

	// --- Draw ---
	DrawRenderMeshes();

	// --- Back to defaults ---
	glDepthFunc(GL_LESS);

	// --- Unbind fbo ---
	if (renderfbo)
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// --- Clear render orders ---
	ClearRenderOrders();

	SDL_Surface* surface = SDL_GetWindowSurface(App->window->window);

	GLubyte* pixels = new GLubyte[surface->w * surface->h * 3];

	glBindTexture(GL_TEXTURE_2D, rendertexture);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

	static std::string out_path;
	uint uid = App->GetRandom().Int();
	texId = App->textures->CreateTextureFromPixels(GL_RGB, surface->w, surface->h, GL_RGB, pixels);
	App->textures->CreateAndSaveTextureFromPixels(uid, GL_RGB, surface->w, surface->h, GL_RGB, (void*)pixels, out_path);

	delete[] pixels;

	SetActiveCamera(previous_cam);

	PreUpdate(0.0f);

	return out_path;
}

void ModuleRenderer3D::ClearRenderOrders()
{
	render_meshes.clear();
	transparent_render_meshes.clear();
	render_obbs.clear();
	render_aabbs.clear();
	render_frustums.clear();
	render_lines.clear();
}


// ---------------------------------------------------------------------------------------------
// ------------------------------ Draw Commands ------------------------------------------------
// ---------------------------------------------------------------------------------------------
void ModuleRenderer3D::DrawRenderMeshes()
{
	// --- Activate wireframe mode ---
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// --- Draw Game Object Meshes ---
	for (std::map<uint, std::vector<RenderMesh>>::const_iterator it = render_meshes.begin(); it != render_meshes.end(); ++it)
	{
		DrawRenderMesh((*it).second);
	}

	// --- DeActivate wireframe mode ---
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

}

void ModuleRenderer3D::DrawTransparentRenderMeshes()
{
	// --- Activate wireframe mode ---
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// --- Sort meshes so we draw the most distant object to camera first and the closest last ---
	std::map<float, RenderMesh> sorted_meshes;
	std::vector<RenderMesh> to_draw;

	for (std::map<uint, std::vector<RenderMesh>>::const_iterator it = transparent_render_meshes.begin(); it != transparent_render_meshes.end(); ++it)
	{

		for (uint i = 0; i < (*it).second.size(); ++i)
		{
			float distance = float3(active_camera->GetCameraPosition() - (*it).second[i].transform.TranslatePart()).Length();

			if (sorted_meshes.find(distance) != sorted_meshes.end())
			{
				sorted_meshes[distance + 0.1f] = (*it).second[i];
			}
			else
				sorted_meshes[distance] = (*it).second[i];
		}
	}



	// --- Copy to vector ---
	for (std::map<float, RenderMesh>::reverse_iterator it = sorted_meshes.rbegin(); it != sorted_meshes.rend(); ++it)
	{
		to_draw.push_back((*it).second);
	}

	// --- Draw transparent meshes in the correct order ---
	DrawRenderMesh(to_draw);

	// --- DeActivate wireframe mode ---
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void ModuleRenderer3D::DrawRenderMesh(std::vector<RenderMesh> meshInstances)
{
	// --- Meshes ---
	for (uint i = 0; i < meshInstances.size(); ++i)
	{
		uint shader = defaultShader->ID;
		RenderMesh* mesh = &meshInstances[i];
		float4x4 model = mesh->transform;
		float3 colorToDraw = float3(1.0f);

		// --- Select/Outline ---
		if (mesh->flags & RenderMeshFlags_::selected)
		{
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			glStencilMask(0xFF);
		}

		if (mesh->flags & RenderMeshFlags_::outline)
		{
			shader = OutlineShader->ID;
			colorToDraw = { 1.0f, 0.65f, 0.0f };
			float3 scale = float3(1.05f, 1.05f, 1.05f);
			model = float4x4::FromTRS(model.TranslatePart(), model.RotatePart(), scale);
		}

		// --- Display Z buffer ---
		if (zdrawer)
			shader = ZDrawerShader->ID;


		// --- Get Mesh Material ---
		if (mesh->mat->shader && shader != OutlineShader->ID && !zdrawer)
		{
			shader = mesh->mat->shader->ID;
			mesh->mat->UpdateUniforms();
		}

		// --- Draw Wireframe if we must ---
		if (mesh->flags & RenderMeshFlags_::wire)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		// ------------------------ Shader Stuff ------------------------
		glUseProgram(shader);

		if (!mesh->mat->has_culling)
			glDisable(GL_CULL_FACE);

		// --- Send Color ---
		glUniform4f(glGetUniformLocation(shader, "u_Color"), colorToDraw.x, colorToDraw.y, colorToDraw.z, 1.0f);

		// --- Set Model Matrix Uniform ---
		glUniformMatrix4fv(glGetUniformLocation(shader, "u_Model"), 1, GL_FALSE, model.Transposed().ptr());

		// --- General Rendering Uniforms (material - texture - color related) ---
		if (mesh->resource_mesh->vertices && mesh->resource_mesh->Indices) // if mesh to draw
		{
			const ResourceMesh* rmesh = mesh->resource_mesh;
			if (mesh->deformable_mesh)
				rmesh = mesh->deformable_mesh;

			// Material
			if (mesh->mat)
			{
				mesh->mat->SetBlending();
				glUniform1i(glGetUniformLocation(shader, "u_HasTransparencies"), (int)mesh->mat->has_transparencies);
				glUniform1f(glGetUniformLocation(shader, "u_Shininess"), mesh->mat->m_Shininess);
				glUniform4f(glGetUniformLocation(shader, "u_Color"), mesh->mat->m_AmbientColor.x, mesh->mat->m_AmbientColor.y, mesh->mat->m_AmbientColor.z, mesh->mat->m_AmbientColor.w);

				//Textures
				glUniform1i(glGetUniformLocation(shader, "u_UseTextures"), (int)mesh->mat->m_UseTexture);

				if (mesh->flags & RenderMeshFlags_::texture)
				{
					if (mesh->flags & RenderMeshFlags_::checkers)
						glBindTexture(GL_TEXTURE_2D, App->textures->GetCheckerTextureID()); // start using texture
					else
					{
						if (!mesh->mat->m_DiffuseResTexture && !mesh->mat->m_SpecularResTexture && !mesh->mat->m_NormalResTexture)
							glUniform1i(glGetUniformLocation(shader, "u_UseTextures"), 0);

						// Diffuse/Albedo
						if (mesh->mat->m_DiffuseResTexture)
						{
							glUniform1i(glGetUniformLocation(shader, "u_HasDiffuseTexture"), 1);

							glUniform1i(glGetUniformLocation(shader, "u_AlbedoTexture"), 1);
							glActiveTexture(GL_TEXTURE0 + 1);
							glBindTexture(GL_TEXTURE_2D, mesh->mat->m_DiffuseResTexture->GetTexID());
						}
						else
						{
							glUniform1i(glGetUniformLocation(shader, "u_AlbedoTexture"), 0);
							glUniform1i(glGetUniformLocation(shader, "u_HasDiffuseTexture"), 0);
						}

						// Specular
						if (mesh->mat->m_SpecularResTexture)
						{
							glUniform1i(glGetUniformLocation(shader, "u_HasSpecularTexture"), 1);

							glUniform1i(glGetUniformLocation(shader, "u_SpecularTexture"), 2);
							glActiveTexture(GL_TEXTURE0 + 2);
							glBindTexture(GL_TEXTURE_2D, mesh->mat->m_SpecularResTexture->GetTexID());
						}
						else
						{
							glUniform1i(glGetUniformLocation(shader, "u_SpecularTexture"), 0);
							glUniform1i(glGetUniformLocation(shader, "u_HasSpecularTexture"), 0);
						}

						// Normal
						if (mesh->mat->m_NormalResTexture)
						{
							glUniform1i(glGetUniformLocation(shader, "u_HasNormalMap"), 1);

							glUniform1i(glGetUniformLocation(shader, "u_NormalTexture"), 3);
							glActiveTexture(GL_TEXTURE0 + 3);
							glBindTexture(GL_TEXTURE_2D, mesh->mat->m_NormalResTexture->GetTexID());
						}
						else
						{
							glUniform1i(glGetUniformLocation(shader, "u_HasNormalMap"), 0);
							glUniform1i(glGetUniformLocation(shader, "u_HasNormalMap"), 0);
						}
					}
				}
				else if (mesh->flags & color)
				{
					glUniform4f(glGetUniformLocation(shader, "u_Color"), mesh->color.r / 255, mesh->color.g / 255, mesh->color.b / 255, 1.0f);
					glUniform1i(glGetUniformLocation(shader, "u_UseTextures"), (int)false);
				}
				else
					glUniform1i(glGetUniformLocation(shader, "u_UseTextures"), (int)false);

			}

			// --- Render ---
			glBindVertexArray(rmesh->VAO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rmesh->EBO);
			glDrawElements(GL_TRIANGLES, rmesh->IndicesSize, GL_UNSIGNED_INT, NULL); //render from array data

			// --- Unbind Buffers ---
			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE0);
		}

		// --- DeActivate wireframe mode ---
		if (mesh->flags & RenderMeshFlags_::wire)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (mesh->flags & RenderMeshFlags_::selected)
			glStencilMask(0x00);

		if (!mesh->mat->has_culling)
			glEnable(GL_CULL_FACE);

		// --- Set Blending to Renderer's Default ---
		if (m_ChangedBlending)
			SetRendererBlending();

		// --- Set color back to default ---
		glUniform4f(glGetUniformLocation(shader, "u_Color"), 1.0f, 1.0f, 1.0f, 1.0f);
	}

	glUseProgram(0);
}


void ModuleRenderer3D::SendShaderUniforms(uint shader)
{
	// --- Display Z buffer ---
	if (zdrawer)
		shader = ZDrawerShader->ID;

	glUseProgram(shader);

	// --- Give ZDrawer near and far camera frustum planes pos ---
	if (zdrawer)
		glUniform2f(glGetUniformLocation(shader, "nearfar"), active_camera->GetNearPlane(), active_camera->GetFarPlane());

	// --- Set Matrix Uniforms ---
	glUniformMatrix4fv(glGetUniformLocation(shader, "u_View"), 1, GL_FALSE, active_camera->GetOpenGLViewMatrix().ptr());
	glUniform1f(glGetUniformLocation(shader, "time"), App->time->time);

	//// Right handed projection matrix
	float f = 1.0f / tan(active_camera->GetFOV() * DEGTORAD / 2.0f);
	float4x4 proj_RH(
		f / active_camera->GetAspectRatio(), 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, active_camera->GetNearPlane(), 0.0f);

	glUniformMatrix4fv(glGetUniformLocation(shader, "u_Proj"), 1, GL_FALSE, proj_RH.ptr());

	// --- Set Normal Mapping Draw ---
	glUniform1i(glGetUniformLocation(shader, "u_DrawNormalMapping"), (int)m_Draw_normalMapping);

	// --- Set HasTextures to none ---
	glUniform1i(glGetUniformLocation(shader, "u_HasDiffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(shader, "u_HasSpecularTexture"), 0);
	glUniform1i(glGetUniformLocation(shader, "u_HasNormalMap"), 0);

	// --- Set Normal Mapping Draw ---
	glUniform1i(glGetUniformLocation(shader, "u_DrawNormalMapping_Lit"), (int)m_Draw_normalMapping_Lit);
	glUniform1i(glGetUniformLocation(shader, "u_DrawNormalMapping_Lit_Adv"), (int)m_Draw_normalMapping_Lit_Adv);

	// --- Gamma Correction & Ambient Color Values ---
	if (App->scene_manager->currentScene && shader == defaultShader->ID)
	{
		float3 sc_color = App->scene_manager->currentScene->m_SceneColor;
		glUniform1f(glGetUniformLocation(shader, "u_GammaCorrection"), App->scene_manager->currentScene->m_SceneGammaCorrection);
		glUniform4f(glGetUniformLocation(shader, "u_AmbientColor"), sc_color.x, sc_color.y, sc_color.z, 1.0f);

		//Bloom Brightness for PostPro
		float3 sc_brightThresh = App->scene_manager->currentScene->m_ScenePP_BloomBrightnessThreshold;

		int xdd = glGetUniformLocation(shader, "u_MinBrightness");
		int xdddd = glGetUniformLocation(shader, "u_BrightnessThreshold");

		glUniform1f(glGetUniformLocation(shader, "u_MinBrightness"), App->scene_manager->currentScene->m_ScenePP_BloomMinBrightness);
		glUniform3f(glGetUniformLocation(shader, "u_BrightnessThreshold"), sc_brightThresh.x, sc_brightThresh.y, sc_brightThresh.z);
	}

	// --- Scene Skybox ---
	int skyboxUnifLoc = glGetUniformLocation(shader, "skybox");
	if (skyboxUnifLoc != -1)
	{
		glUniform1i(skyboxUnifLoc, 0);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexID);
	}

	int lightsNumLoc = glGetUniformLocation(shader, "u_LightsNumber");
	if (shader == defaultShader->ID || lightsNumLoc != -1)
	{
		// --- Send Lights ---
		glUniform1i(lightsNumLoc, m_LightsVec.size());
		for (uint i = 0; i < m_LightsVec.size(); ++i)
			m_LightsVec[i]->SendUniforms(shader, i);
	}
	else
		glUniform1i(glGetUniformLocation(shader, "u_LightsNumber"), 0);
}


void ModuleRenderer3D::DrawPostProcessing()
{
	glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
	glDisable(GL_BLEND); // we do not want blending

	// --- First Step: Color Correction
	if (m_UseColorCorrection && currentLUT)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glUseProgram(LUTShader->ID);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rendertexture);	// use the color attachment texture as the texture of the quad plane

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, currentLUT);
z
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(0);
	}

	// --- Second Step: HDR and Bloom ---
	bool horizontal = true, first_iteration = true;
	ResourceScene* currScene = App->scene_manager->currentScene;
	uint BlurAmount = currScene->m_ScenePP_BloomBlurAmount;

	if (m_UseBloom && currScene)
	{
		glUseProgram(BlurShader->ID);
		for (uint i = 0; i < BlurAmount; ++i)
		{
			float3 weights = currScene->m_ScenePP_BlurWeights1;
			glBindFramebuffer(GL_FRAMEBUFFER, fbo_pingpong[horizontal]);
			glUniform1i(glGetUniformLocation(BlurShader->ID, "u_HorizontalPass"), horizontal);
			glUniform3f(glGetUniformLocation(BlurShader->ID, "u_BlurWeights1"), weights.x, weights.y, weights.z);
			glUniform2f(glGetUniformLocation(BlurShader->ID, "u_BlurWeights2"), currScene->m_ScenePP_BlurWeights2.x, currScene->m_ScenePP_BlurWeights2.y);
			uint textureToBlur = first_iteration ? brightnessTexture : pingpongBuffers[!horizontal];

			//glUniform1i(glGetUniformLocation(BlurShader->ID, "u_ImageToBlur"), 0);
			//glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureToBlur);

			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);

			horizontal = !horizontal;
			if (first_iteration) first_iteration = false;
		}

		glUseProgram(0); glBindVertexArray(0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); glBindTexture(GL_TEXTURE_2D, 0);
	}

	// --- Standard/Normal FBO ---	
	if (!drawfb)
		glBindFramebuffer(GL_FRAMEBUFFER, fbo); // if we do not want to draw the framebuffer we bind to draw the postprocessing into it
	
	uint shader = screenShader->ID; 
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "u_UseHDR"), m_UseHDR && !m_RenderBloomOnly);
	glUniform1i(glGetUniformLocation(shader, "u_UseBloom"), m_UseBloom && !m_RenderBloomOnly);
	if (currScene)
	{
		glUniform1f(glGetUniformLocation(shader, "u_GammaCorrection"), currScene->m_ScenePP_GammaCorr);
		glUniform1f(glGetUniformLocation(shader, "u_HDR_Exposure"), currScene->m_ScenePP_HDRExposure);
	}

	uint textureToRend = rendertexture; //rendertexture brightnessTexture
	if (m_RenderBloomOnly)
		textureToRend = pingpongBuffers[!horizontal];

	//glUniform1i(glGetUniformLocation(shader, "screenTexture"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureToRend);	// use the color attachment texture as the texture of the quad plane

	//glUniform1i(glGetUniformLocation(shader, "bloomBlurTexture"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, pingpongBuffers[!horizontal]);	

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// --- Unbind buffers ---
	glEnable(GL_BLEND);
	if (!drawfb)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
}

// ---------------------------------------------------------------------------------------------
// ------------------------------ Lights -------------------------------------------------------
// ---------------------------------------------------------------------------------------------
void ModuleRenderer3D::AddLight(ComponentLight* light)
{
	if (light)
		m_LightsVec.push_back(light);
}

void ModuleRenderer3D::PopLight(ComponentLight* light)
{
	if (light)
	{
		std::vector<ComponentLight*>::iterator it = std::find(m_LightsVec.begin(), m_LightsVec.end(), light);

		if (it != m_LightsVec.end())
			m_LightsVec.erase(it);
	}
}

const int ModuleRenderer3D::GetLightIndex(ComponentLight* light)
{
	if (light)
	{
		for (int i = 0; i < m_LightsVec.size(); ++i)
		{
			if (m_LightsVec[i] == light)
				return i;
		}
	}

	return -1;
}


// ---------------------------------------------------------------------------------------------
// ------------------------------ Buffers ------------------------------------------------------
// ---------------------------------------------------------------------------------------------
uint ModuleRenderer3D::CreateBufferFromData(uint Targetbuffer, uint size, void* data) const
{
	uint ID = 0;

	glGenBuffers(1, (GLuint*)&ID); // create buffer
	glBindBuffer(Targetbuffer, ID); // start using created buffer
	glBufferData(Targetbuffer, size, data, GL_STATIC_DRAW); // send data to VRAM
	glBindBuffer(Targetbuffer, 0); // Stop using buffer

	return ID;
}

void ModuleRenderer3D::CreateFramebuffer()
{
	// --- Create a texture to use it as depthMap for shadowing ---
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, App->window->GetWindowWidth(), App->window->GetWindowHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenFramebuffers(1, &depthbufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthbufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// --- Create a texture to use it as render target ---
	glGenTextures(1, &rendertexture);
	glBindTexture(GL_TEXTURE_2D, rendertexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, App->window->GetWindowWidth(), App->window->GetWindowHeight());
	glBindTexture(GL_TEXTURE_2D, 0);

	// --- Create a 2nd Color Buffer (texture) to use as brightness output of shader (for bloom) ---
	glGenTextures(1, &brightnessTexture);
	glBindTexture(GL_TEXTURE_2D, brightnessTexture);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, App->window->GetWindowWidth(), App->window->GetWindowHeight());
	glBindTexture(GL_TEXTURE_2D, 0);

	// --- Generate attachments, DEPTH and STENCIL ---
	glGenTextures(1, &depthbuffer);
	glBindTexture(GL_TEXTURE_2D, depthbuffer);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, App->window->GetWindowWidth(), App->window->GetWindowHeight());
	glBindTexture(GL_TEXTURE_2D, 0);

	// --- Generate framebuffer object (fbo) ---
	uint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rendertexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, brightnessTexture, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthbuffer, 0);
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// --- Generate PingPong FBOs to bloom ---
	glGenFramebuffers(2, fbo_pingpong);
	glGenTextures(2, pingpongBuffers);
	for (uint i = 0; i < 2; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo_pingpong[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i]);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, App->window->GetWindowWidth(), App->window->GetWindowHeight());
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i], 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// ---------------------------------------------------------------------------------------------
// ------------------------------ Utilities ----------------------------------------------------
// ---------------------------------------------------------------------------------------------
void ModuleRenderer3D::SetRendererBlending()
{
	if (App->scene_manager->currentScene == nullptr)
		return;

	ResourceScene* scene = App->scene_manager->currentScene;

	//if (m_AutomaticBlendingFunc)
	//	PickBlendingAutoFunction(m_RendererBlendFunc, m_BlendEquation);
	//else
	//	PickBlendingManualFunction(m_ManualBlend_Src, m_ManualBlend_Dst, m_BlendEquation);

	if (m_AutomaticBlendingFunc)
		PickBlendingAutoFunction(scene->m_RendererBlendFunc, scene->m_BlendEquation);
	else
		PickBlendingManualFunction(scene->m_ManualBlend_Src, scene->m_ManualBlend_Dst, scene->m_BlendEquation);

	m_ChangedBlending = false;
}

void ModuleRenderer3D::PickBlendingEquation(BlendingEquations eq)
{
	switch (eq)
	{
		case (BlendingEquations::ADD):
			glBlendEquation(GL_FUNC_ADD);
			break;
		case (BlendingEquations::SUBTRACT):
			glBlendEquation(GL_FUNC_SUBTRACT);
			break;
		case (BlendingEquations::REVERSE_SUBTRACT):
			glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
			break;
		case (BlendingEquations::MIN):
			glBlendEquation(GL_MIN);
			break;
		case (BlendingEquations::MAX):
			glBlendEquation(GL_MAX);
			break;
		default:
			glBlendEquation(GL_FUNC_ADD);
			break;
	}
}

void ModuleRenderer3D::PickBlendingAutoFunction(BlendAutoFunction blend_func, BlendingEquations eq)
{
	switch (blend_func)
	{
		case (BlendAutoFunction::STANDARD_INTERPOLATIVE):
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case (BlendAutoFunction::ADDITIVE):
			glBlendFunc(GL_ONE, GL_ONE);
			break;
		case(BlendAutoFunction::ADDITIVE_ALPHA_AFFECTED):
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case(BlendAutoFunction::MULTIPLICATIVE):
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
			break;
		default:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
	}

	PickBlendingEquation(eq);
}

void  ModuleRenderer3D::PickBlendingManualFunction(BlendingTypes src, BlendingTypes dst, BlendingEquations eq)
{
	glBlendFunc(BlendingTypesToOGL(src), BlendingTypesToOGL(dst));
	PickBlendingEquation(eq);
}

void ModuleRenderer3D::HandleObjectOutlining()
{
	// --- Selected Object Outlining ---
	for (GameObject* obj : *App->selection->GetSelected())
	{
		// --- Draw slightly scaled-up versions of the objects, disable stencil writing
		// The stencil buffer is filled with several 1s. The parts that are 1 are not drawn, only the objects size
		// differences, making it look like borders ---
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);

		// --- Search for Renderer Component ---
		ComponentMeshRenderer* MeshRenderer = obj->GetComponent<ComponentMeshRenderer>();

		// --- If Found, draw the mesh ---
		if (MeshRenderer && MeshRenderer->IsEnabled() && obj->GetActive())
		{
			std::vector<RenderMesh> meshInstances;

			ComponentMesh* cmesh = obj->GetComponent<ComponentMesh>();
			ComponentMeshRenderer* cmesh_renderer = obj->GetComponent<ComponentMeshRenderer>();
			RenderMeshFlags flags = outline;

			if (cmesh && cmesh->resource_mesh && cmesh_renderer && cmesh_renderer->material)
			{
				meshInstances.push_back(RenderMesh(obj->GetComponent<ComponentTransform>()->GetGlobalTransform(), cmesh->resource_mesh, cmesh_renderer->material, flags));
				DrawRenderMesh(meshInstances);
			}
		}
		//MeshRenderer->Draw(true);

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glEnable(GL_DEPTH_TEST);
	}
}


void ModuleRenderer3D::UpdateGLCapabilities() const
{
	// --- Enable/Disable OpenGL Capabilities ---
	if (!depth)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);

	if (!cull_face)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

	if (!lighting)
		glDisable(GL_LIGHTING);
	else
		glEnable(GL_LIGHTING);

	if (!color_material)
		glDisable(GL_COLOR_MATERIAL);
	else
		glEnable(GL_COLOR_MATERIAL);

}

void ModuleRenderer3D::CreateGrid(float target_distance)
{
	// --- Fill vertex data ---

	float distance = target_distance / 4;

	if (distance < 1)
		distance = 1;

	float3 vertices[164];

	uint i = 0;
	int lines = -20;

	for (i = 0; i < 40; i++)
	{
		vertices[4 * i] = float3(lines * -distance, 0.0f, 20 * -distance);
		vertices[4 * i + 1] = float3(lines * -distance, 0.0f, 20 * distance);
		vertices[4 * i + 2] = float3(20 * -distance, 0.0f, lines * distance);
		vertices[4 * i + 3] = float3(20 * distance, 0.0f, lines * distance);

		lines++;
	}

	vertices[4 * i] = float3(lines * -distance, 0.0f, 20 * -distance);
	vertices[4 * i + 1] = float3(lines * -distance, 0.0f, 20 * distance);
	vertices[4 * i + 2] = float3(20 * -distance, 0.0f, lines * distance);
	vertices[4 * i + 3] = float3(20 * distance, 0.0f, lines * distance);

	// --- Configure vertex attributes ---

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(Grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, Grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


// ---------------------------------------------------------------------------------------------
// ------------------------------ Shaders ------------------------------------------------------
// ---------------------------------------------------------------------------------------------
void ModuleRenderer3D::CreateDefaultShaders()
{
	ImporterShader* IShader = App->resources->GetImporter<ImporterShader>();

	const char* vertexShaderT =
		"#version 440 core \n"
		"#define VERTEX_SHADER \n"
		"#ifdef VERTEX_SHADER \n"
		"layout (location = 0) in vec3 a_Position; \n"
		"layout(location = 1) in vec3 a_Normal; \n"
		"layout(location = 2) in vec3 a_Color; \n"
		"layout (location = 3) in vec2 a_TexCoord; \n"
		"uniform vec4 u_Color; \n"
		"uniform mat4 u_Model; \n"
		"uniform mat4 u_View; \n"
		"uniform mat4 u_Proj; \n"
		"out vec4 v_Color; \n"
		"out vec2 v_TexCoord; \n"
		"void main(){ \n"
		"gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0f); \n"
		"v_Color = u_Color; \n"
		"v_TexCoord = a_TexCoord; \n"
		"}\n"
		"#endif //VERTEX_SHADER\n"
		;

	const char* fragmentShaderT =
		"#version 440 core \n"
		"#define FRAGMENT_SHADER \n"
		"#ifdef FRAGMENT_SHADER \n"
		"in vec4 v_Color; \n"
		"in vec2 v_TexCoord; \n"
		"uniform int u_UseTextures;\n"
		"uniform sampler2D u_AlbedoTexture; \n"
		"out vec4 color; \n"
		"void main(){ \n"
		"color = v_Color;\n"
		"if(u_UseTextures == 1)\n"
		"color = texture(u_AlbedoTexture, v_TexCoord); \n"
		"} \n"
		"#endif //FRAGMENT_SHADER\n"
		;

	VertexShaderTemplate = vertexShaderT;
	FragmentShaderTemplate = fragmentShaderT;


	// --- Creating outline drawing shaders ---
	const char* OutlineVertShaderSrc =
		"#version 440 core \n"
		"#define VERTEX_SHADER \n"
		"#ifdef VERTEX_SHADER \n"
		"layout (location = 0) in vec3 a_Position; \n"
		"uniform mat4 u_Model; \n"
		"uniform mat4 u_View; \n"
		"uniform mat4 u_Proj; \n"
		"void main(){ \n"
		"gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0f); \n"
		"}\n"
		"#endif //VERTEX_SHADER\n"
		;

	const char* OutlineFragShaderSrc =
		"#version 440 core \n"
		"#define FRAGMENT_SHADER \n"
		"#ifdef FRAGMENT_SHADER \n"
		"out vec4 color; \n"
		"void main(){ \n"
		"color = vec4(1.0,0.65,0.0, 1.0); \n"
		"} \n"
		"#endif //FRAGMENT_SHADER \n"
		;

	OutlineShader = (ResourceShader*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SHADER, "Assets/Shaders/OutlineShader.glsl", 8);
	OutlineShader->vShaderCode = OutlineVertShaderSrc;
	OutlineShader->fShaderCode = OutlineFragShaderSrc;
	OutlineShader->ReloadAndCompileShader();
	OutlineShader->SetName("OutlineShader");
	OutlineShader->LoadToMemory();
	IShader->Save(OutlineShader);

	// --- Creating point/line drawing shaders ---
	const char* linePointVertShaderSrc =
		"#version 440 core \n"
		"#define VERTEX_SHADER \n"
		"#ifdef VERTEX_SHADER \n"
		"layout (location = 0) in vec3 a_Position; \n"
		"out vec4 v_Color; \n"
		"uniform vec4 u_Color; \n"
		"uniform mat4 u_Model; \n"
		"uniform mat4 u_View; \n"
		"uniform mat4 u_Proj; \n"
		"void main(){ \n"
		"gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0f); \n"
		"v_Color = u_Color; \n"
		"}\n"
		"#endif //VERTEX_SHADER\n"
		;

	const char* linePointFragShaderSrc =
		"#version 440 core \n"
		"#define FRAGMENT_SHADER \n"
		"#ifdef FRAGMENT_SHADER \n"
		"in vec4 v_Color; \n"
		"out vec4 color; \n"
		"void main(){ \n"
		"color = v_Color; \n"
		"} \n"
		"#endif //FRAGMENT_SHADER\n"
		;

	linepointShader = (ResourceShader*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SHADER, "Assets/Shaders/LinePoint.glsl", 9);
	linepointShader->vShaderCode = linePointVertShaderSrc;
	linepointShader->fShaderCode = linePointFragShaderSrc;
	linepointShader->ReloadAndCompileShader();
	linepointShader->SetName("LinePoint");
	linepointShader->LoadToMemory();
	IShader->Save(linepointShader);


	// --- Creating z buffer shader drawer ---
	const char* zdrawervertex =
		R"(#version 440 core
		#define VERTEX_SHADER
		#ifdef VERTEX_SHADER

		layout (location = 0) in vec3 a_Position;

		uniform vec2 nearfar;
		uniform mat4 u_Model;
		uniform mat4 u_View;
		uniform mat4 u_Proj;

		out mat4 v_Projection;
		out vec2 v_NearFarPlanes;

		void main()
		{
			v_NearFarPlanes = nearfar;
			v_Projection = u_Proj;
			gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
		}
		#endif //VERTEX_SHADER)";

	const char* zdrawerfragment =
		R"(#version 440 core
		#define FRAGMENT_SHADER
		#ifdef FRAGMENT_SHADER

			in vec2 v_NearFarPlanes;
			in mat4 v_Projection;

			out vec4 color;

			float LinearizeDepth(float depth)
			{
				float z = 2.0 * depth - 1.0; // back to NDC
				return 2.0 * v_NearFarPlanes.x * v_NearFarPlanes.y / (v_NearFarPlanes.y + v_NearFarPlanes.x - z * (v_NearFarPlanes.y - v_NearFarPlanes.x));
			}

			void main()
			{
				float depth = LinearizeDepth(gl_FragCoord.z) / v_NearFarPlanes.y;
				color = vec4(vec3(gl_FragCoord.z * v_NearFarPlanes.y * v_NearFarPlanes.x), 1.0);
			}

		#endif //FRAGMENT_SHADER)";

	// NOTE: not removing linearizedepth function because it was needed for the previous z buffer implementation (no reversed-z), just in case I need it again (doubt it though)

	ZDrawerShader = (ResourceShader*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SHADER, "Assets/Shaders/ZDrawer.glsl", 10);
	ZDrawerShader->vShaderCode = zdrawervertex;
	ZDrawerShader->fShaderCode = zdrawerfragment;
	ZDrawerShader->ReloadAndCompileShader();
	ZDrawerShader->SetName("ZDrawer");
	ZDrawerShader->LoadToMemory();
	IShader->Save(ZDrawerShader);

	// --- Creating Default Vertex and Fragment Shaders ---
	const char* vertexShaderSource =
		R"(#version 440 core
		#define VERTEX_SHADER
		#ifdef VERTEX_SHADER

		layout (location = 0) in vec3 a_Position;
		layout(location = 1) in vec3 a_Normal;
		layout(location = 2) in vec3 a_Color;
		layout (location = 3) in vec2 a_TexCoord;

		uniform vec4 u_Color = vec4(1.0);
		uniform mat4 u_Model;
		uniform mat4 u_View;
		uniform mat4 u_Proj;

		out vec4 v_Color;
		out vec2 v_TexCoord;

		void main()
		{
			gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0f);
			v_Color = u_Color;
			v_TexCoord = a_TexCoord;
		}
		#endif //VERTEX_SHADER)";

	const char* fragmentShaderSource =
		R"(#version 440 core
		#define FRAGMENT_SHADER
		#ifdef FRAGMENT_SHADER

		uniform int u_UseTextures;
		uniform sampler2D u_AlbedoTexture;

		in vec4 v_Color;
		in vec2 v_TexCoord;
		out vec4 color;

		void main()
		{
			color = v_Color;
			if(u_UseTextures == -1)
				color = texture(u_AlbedoTexture, v_TexCoord) * v_Color;
		}
		#endif //FRAGMENT_SHADER)";

	defaultShader = (ResourceShader*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SHADER, "Assets/Shaders/Standard.glsl", 12);
	defaultShader->vShaderCode = vertexShaderSource;
	defaultShader->fShaderCode = fragmentShaderSource;
	defaultShader->ReloadAndCompileShader();
	defaultShader->SetName("Standard");
	defaultShader->LoadToMemory();
	defaultShader->ReloadAndCompileShader();
	IShader->Save(defaultShader);

	// ---Creating screen shader ---
	const char* vertexScreenShader =
		R"(#version 440 core
		#define VERTEX_SHADER
		#ifdef VERTEX_SHADER

		layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec2 aTexCoords;

		out vec2 TexCoords;

		void main()
		{
			gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
			TexCoords = aTexCoords;
		}
		#endif //VERTEX_SHADER)";

	const char* fragmentScreenShader =
		R"(#version 440 core
		#define FRAGMENT_SHADER
		#ifdef FRAGMENT_SHADER

		out vec4 FragColor;

		in vec2 TexCoords;

		uniform sampler2D screenTexture;

		void main()
		{
		    FragColor = texture(screenTexture, TexCoords);
		}
		#endif //FRAGMENT_SHADER)";

	screenShader = (ResourceShader*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SHADER, "Assets/Shaders/ScreenShader.glsl", 13);
	screenShader->vShaderCode = vertexScreenShader;
	screenShader->fShaderCode = fragmentScreenShader;
	screenShader->ReloadAndCompileShader();
	screenShader->SetName("Screen shader");
	screenShader->LoadToMemory();
	screenShader->ReloadAndCompileShader();
	IShader->Save(screenShader);

	// --- Creating skybox shader ---
	const char* SkyboxVertShaderSrc =
		"#version 460 core \n"
		"#define VERTEX_SHADER \n"
		"#ifdef VERTEX_SHADER \n"
		"layout (location = 0) in vec3 a_Position; \n"
		"out vec3 TexCoords; \n"
		"uniform mat4 u_View; \n"
		"uniform mat4 u_Proj; \n"
		"uniform mat4 u_Model; \n"
		"void main(){ \n"
		"TexCoords = a_Position * vec3(1,-1,1); \n"
		"gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0); \n"
		"}\n"
		"#endif //VERTEX_SHADER\n"
		;

	const char* SkyboxFragShaderSrc =
		"#version 460 core \n"
		"#define FRAGMENT_SHADER \n"
		"#ifdef FRAGMENT_SHADER \n"
		"in vec3 TexCoords; \n"
		"uniform samplerCube skybox;\n"
		"out vec4 color; \n"
		"void main(){ \n"
		"color = texture(skybox, TexCoords); \n"
		"} \n"
		"#endif //FRAGMENT_SHADER \n"
		;

	SkyboxShader = (ResourceShader*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SHADER, "Assets/Shaders/SkyboxShader.glsl", 15);
	SkyboxShader->vShaderCode = SkyboxVertShaderSrc;
	SkyboxShader->fShaderCode = SkyboxFragShaderSrc;
	SkyboxShader->ReloadAndCompileShader();
	SkyboxShader->SetName("SkyboxShader");
	SkyboxShader->LoadToMemory();
	IShader->Save(SkyboxShader);

	// --- Creating UI shader ---
	const char* vertexUIShader =
		R"(#version 440 core
			#define VERTEX_SHADER
			#ifdef VERTEX_SHADER
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec3 a_Normal;
			layout(location = 2) in vec3 a_Color;
			layout(location = 3) in vec2 a_TexCoord;
			uniform mat4 u_Model;
			uniform mat4 u_View;
			uniform mat4 u_Proj;
			uniform vec4 u_Color = vec4(1.0);
			out vec2 v_TexCoord;
			out vec4 v_Color;
			void main()
			{
				v_Color = u_Color;
				v_TexCoord = a_TexCoord;
				gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
			}
			#endif //VERTEX_SHADER)";

	const char* fragmentUIShader =
		R"(#version 440 core
			#define FRAGMENT_SHADER
			#ifdef FRAGMENT_SHADER
			out vec4 out_color;
			in vec2 v_TexCoord;
			in vec4 v_Color;
			uniform int u_UseTextures = 0;
			uniform int u_HasTransparencies = 0;
			uniform sampler2D u_AlbedoTexture;
			void main()
			{
				float alpha = 1.0;
				if (u_HasTransparencies == 1)
				{
					if (u_UseTextures == 0)
						alpha = v_Color.a;
					else
						alpha = texture(u_AlbedoTexture, v_TexCoord).a * v_Color.a;
				}

				if (alpha < 0.004)
					discard;

				if (u_UseTextures == 0 || (u_UseTextures == 1 && u_HasTransparencies == 0 && texture(u_AlbedoTexture, v_TexCoord).a < 0.1))
					out_color = vec4(v_Color.rgb, alpha);
				else if (u_UseTextures == 1)
					out_color = vec4(v_Color.rgb * texture(u_AlbedoTexture, v_TexCoord).rgb, alpha);
			}
			#endif //FRAGMENT_SHADER)";


	UI_Shader = (ResourceShader*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SHADER, "Assets/Shaders/UIShader.glsl", 16);
	UI_Shader->vShaderCode = vertexUIShader;
	UI_Shader->fShaderCode = fragmentUIShader;
	UI_Shader->ReloadAndCompileShader();
	UI_Shader->SetName("UI Shader");
	UI_Shader->LoadToMemory();
	IShader->Save(UI_Shader);

	const char* vertexBlurShader =
		R"(	#version 440 core
			#define VERTEX_SHADER
			#ifdef VERTEX_SHADER			
			layout (location = 0) in vec3 a_Position;
			layout (location = 3) in vec2 a_TexCoord;			
			out vec2 v_TexCoords;			
			void main()
			{
				v_TexCoords = a_TexCoord;
				gl_Position = vec4(a_Position, 1.0);
			}
			#endif //VERTEX_SHADER)";

	const char* fragmentBlurShader =
		R"(	#version 440 core
			#define FRAGMENT_SHADER
			#ifdef FRAGMENT_SHADER
			uniform sampler2D u_ImageToBlur;
			uniform float u_BlurWeights[5];
			uniform bool u_HorizontalPass = true;
			in vec2 v_TexCoords;
			out vec4 FragColor;
			void main()
			{
				vec2 texture_offset = 1.0/textureSize(u_ImageToBlur, 0);
				vec3 finalColor = texture(u_ImageToBlur, v_TexCoords).rgb * u_BlurWeights[0];
				if(u_HorizontalPass)
				{
					for(int i = 1; i < 5; ++i) //For amount of blur passes
					{
						finalColor += texture(u_ImageToBlur, v_TexCoords + vec2(texture_offset.x*i, 0.0)).rgb * u_BlurWeights[i];
						finalColor += texture(u_ImageToBlur, v_TexCoords - vec2(texture_offset.x*i, 0.0)).rgb * u_BlurWeights[i];
					}
				}
				else
					for(int i = 1; i < 5; ++i) //For amount of blur passes
					{
						finalColor += texture(u_ImageToBlur, v_TexCoords + vec2(0.0, texture_offset.y*i)).rgb * u_BlurWeights[i];
						finalColor += texture(u_ImageToBlur, v_TexCoords - vec2(0.0, texture_offset.y*i)).rgb * u_BlurWeights[i];
					}
				}
				FragColor = vec4(finalColor, 1.0);
			}
			#endif //FRAGMENT_SHADER)";

	BlurShader = (ResourceShader*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SHADER, "Assets/Shaders/BlurShader.glsl", 16);
	BlurShader->vShaderCode = vertexBlurShader;
	BlurShader->fShaderCode = fragmentBlurShader;
	BlurShader->ReloadAndCompileShader();
	BlurShader->SetName("Blur Shader");
	BlurShader->LoadToMemory();
	IShader->Save(BlurShader);

	LUTShader = (ResourceShader*)App->resources->CreateResourceGivenUID(Resource::ResourceType::SHADER, "Assets/Shaders/PostProcessing/ColorCorrection.glsl", 17);
	if (LUTShader)
	{
		LUTShader->SetName("Color Correction Shader");
		LUTShader->LoadToMemory();
	}

	//For further shader creations:
	/*
	const char* fragmentBlurShader =
		R"( //Insert shader body here
		)";
	*/

	defaultShader->use();
}


// ---------------------------------------------------------------------------------------------
// ------------------------------ Draw Utilities -----------------------------------------------
// ---------------------------------------------------------------------------------------------
void ModuleRenderer3D::DrawRenderLines()
{
	// --- Use linepoint shader ---
	glUseProgram(App->renderer3D->linepointShader->ID);

	// --- Get Uniform locations ---
	GLint modelLoc = glGetUniformLocation(App->renderer3D->linepointShader->ID, "u_Model");
	GLint viewLoc = glGetUniformLocation(App->renderer3D->linepointShader->ID, "u_View");
	int vertexColorLocation = glGetUniformLocation(App->renderer3D->linepointShader->ID, "u_Color");
	GLint projectLoc = glGetUniformLocation(App->renderer3D->linepointShader->ID, "u_Proj");

	// --- Set Right handed projection matrix ---

	float nearp = App->renderer3D->active_camera->GetNearPlane();

	float f = 1.0f / tan(App->renderer3D->active_camera->GetFOV() * DEGTORAD / 2.0f);
	float4x4 proj_RH(
		f / App->renderer3D->active_camera->GetAspectRatio(), 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, nearp, 0.0f);

	// --- Set Uniforms ---
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, proj_RH.ptr());
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLViewMatrix().ptr());

	// --- Initialize vars, prepare buffer ---
	float3* vertices = new float3[2];
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindVertexArray(PointLineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// --- Draw Lines ---
	for (std::vector<RenderLine>::const_iterator it = render_lines.begin(); it != render_lines.end(); ++it)
	{
		// --- Assign color and model matrix ---
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (*it).transform.Transposed().ptr());
		glUniform4f(vertexColorLocation, (*it).color.r / 255.0f, (*it).color.g / 255.0f, (*it).color.b / 255.0f, 1.0f);

		// --- Assign line vertices, a and b ---
		vertices[0] = (*it).a;
		vertices[1] = (*it).b;

		// --- Send data ---
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 2, vertices, GL_DYNAMIC_DRAW);

		// --- Draw lines ---
		glLineWidth(3.0f);
		glBindVertexArray(PointLineVAO);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);
		glLineWidth(1.0f);
	}

	// --- Reset stuff ---
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// --- Delete VBO and vertices ---
	glDeleteBuffers(1, &VBO);
	delete[] vertices;

	// --- Back to default ---
	glUseProgram(App->renderer3D->defaultShader->ID);
}

void ModuleRenderer3D::DrawRenderBoxes()
{
	for (uint i = 0; i < render_obbs.size(); ++i)
	{
		DrawWire(*render_obbs[i].box, render_obbs[i].color, PointLineVAO);
	}
	for (uint i = 0; i < render_aabbs.size(); ++i)
	{
		DrawWire(*render_aabbs[i].box, render_aabbs[i].color, PointLineVAO);
	}

	for (uint i = 0; i < render_frustums.size(); ++i)
	{
		DrawWire(*render_frustums[i].box, render_frustums[i].color, PointLineVAO);
	}
}

void ModuleRenderer3D::DrawGrid()
{
	//App->renderer3D->defaultShader->use();
	uint shaderID = App->renderer3D->defaultShader->ID;
	glUseProgram(shaderID);

	GLint modelLoc = glGetUniformLocation(shaderID, "u_Model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, float4x4::identity.ptr());

	float gridColor = 0.8f;
	GLint vertexColorLocation = glGetUniformLocation(shaderID, "u_Color");
	glUniform4f(vertexColorLocation, gridColor, gridColor, gridColor, 1.0f);

	glUniform1i(glGetUniformLocation(shaderID, "u_UseTextures"), 0);

	glLineWidth(1.7f);
	glBindVertexArray(Grid_VAO);
	glDrawArrays(GL_LINES, 0, 164);
	glBindVertexArray(0);
	glLineWidth(1.0f);

	glUseProgram(0);
}

void ModuleRenderer3D::DrawSkybox()
{
	if (!SkyboxShader || App->scene_manager->currentScene == nullptr)
		return;

	glDepthMask(GL_FALSE);

	float3 prevpos = active_camera->frustum.Pos();
	//float3 prevup = App->renderer3D->active_camera->frustum.Up();
	//float3 prevfront = App->renderer3D->active_camera->frustum.Front();

	App->renderer3D->active_camera->frustum.SetPos(float3::zero);

	ResourceScene* scene = App->scene_manager->currentScene;
	float3 skyAngle = scene->m_Sky_Rotation;
	math::Quat rotationX = math::Quat::RotateAxisAngle(float3::unitY, skyAngle.x * DEGTORAD);
	math::Quat rotationY = math::Quat::RotateAxisAngle(float3::unitX, skyAngle.y * DEGTORAD);
	math::Quat rotationZ = math::Quat::RotateAxisAngle(float3::unitZ, skyAngle.z * DEGTORAD);
	math::Quat finalRotation = rotationX * rotationY * rotationZ;

	//App->renderer3D->active_camera->frustum.SetUp(finalRotation.Mul(App->renderer3D->active_camera->frustum.Up()).Normalized());
	//App->renderer3D->active_camera->frustum.SetFront(finalRotation.Mul(App->renderer3D->active_camera->frustum.Front()).Normalized());

	SkyboxShader->use();
	// draw skybox as last
	glDepthFunc(GL_GEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	//view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
	float4x4 view = App->renderer3D->active_camera->GetOpenGLViewMatrix();
	view.SetTranslatePart(float4::zero);

	float nearp = App->renderer3D->active_camera->GetNearPlane();

	// right handed projection matrix
	float f = 1.0f / tan(App->renderer3D->active_camera->GetFOV() * DEGTORAD / 2.0f);
	float4x4 proj_RH(
		f / App->renderer3D->active_camera->GetAspectRatio(), 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, nearp, 0.0f);

	float4x4 model = float4x4::identity;
	model = model.FromQuat(finalRotation);

	GLint modelLoc = glGetUniformLocation(App->renderer3D->SkyboxShader->ID, "u_Model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.ptr());

	GLint viewLoc = glGetUniformLocation(App->renderer3D->SkyboxShader->ID, "u_View");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.ptr());

	GLint projectLoc = glGetUniformLocation(App->renderer3D->SkyboxShader->ID, "u_Proj");
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, proj_RH.ptr());

	//Scene Rendering Elements Affecting Skybox
	glUniform1f(glGetUniformLocation(SkyboxShader->ID, "u_GammaCorrection"), scene->m_SceneGammaCorrection);
	glUniform1f(glGetUniformLocation(SkyboxShader->ID, "u_Exposure"), scene->m_Sky_Exposure);
	glUniform3f(glGetUniformLocation(SkyboxShader->ID, "u_Color"), scene->m_Sky_ColorTint.x, scene->m_Sky_ColorTint.y, scene->m_Sky_ColorTint.z);

	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexID);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	//glDepthFunc(GL_LESS); // set depth function back to default

	defaultShader->use();

	App->renderer3D->active_camera->frustum.SetPos(prevpos);
	//App->renderer3D->active_camera->frustum.SetUp(prevup);
	//App->renderer3D->active_camera->frustum.SetFront(prevfront);

	glDepthMask(GL_TRUE);
}

void ModuleRenderer3D::DrawWireFromVertices(const float3* corners, Color color, uint VAO) {
	float3 vertices[24] =
	{
		//Between-planes right
		corners[1],
		corners[5],
		corners[7],
		corners[3],

		//Between-planes left
		corners[4],
		corners[0],
		corners[2],
		corners[6],

		// Far plane horizontal
		corners[5],
		corners[4],
		corners[6],
		corners[7],

		//Near plane horizontal
		corners[0],
		corners[1],
		corners[3],
		corners[2],

		//Near plane vertical
		corners[1],
		corners[3],
		corners[0],
		corners[2],

		//Far plane vertical
		corners[5],
		corners[7],
		corners[4],
		corners[6]
	};

	// --- Set Uniforms ---
	glUseProgram(App->renderer3D->linepointShader->ID);

	float nearp = App->renderer3D->active_camera->GetNearPlane();

	// right handed projection matrix
	float f = 1.0f / tan(App->renderer3D->active_camera->GetFOV() * DEGTORAD / 2.0f);
	float4x4 proj_RH(
		f / App->renderer3D->active_camera->GetAspectRatio(), 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, -1.0f,
		0.0f, 0.0f, nearp, 0.0f);

	GLint modelLoc = glGetUniformLocation(App->renderer3D->linepointShader->ID, "u_Model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, float4x4::identity.ptr());

	GLint viewLoc = glGetUniformLocation(App->renderer3D->linepointShader->ID, "u_View");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, App->renderer3D->active_camera->GetOpenGLViewMatrix().ptr());

	GLint projectLoc = glGetUniformLocation(App->renderer3D->linepointShader->ID, "u_Proj");
	glUniformMatrix4fv(projectLoc, 1, GL_FALSE, proj_RH.ptr());

	int vertexColorLocation = glGetUniformLocation(App->renderer3D->linepointShader->ID, "u_Color");
	glUniform4f(vertexColorLocation, color.r, color.g, color.b, color.a);

	// --- Create VAO, VBO ---
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// --- Draw lines ---

	glLineWidth(3.0f);
	glBindVertexArray(VAO);
	glDrawArrays(GL_LINES, 0, 24);
	glBindVertexArray(0);
	glLineWidth(1.0f);

	// --- Delete VBO ---
	glDeleteBuffers(1, &VBO);

	glUseProgram(App->renderer3D->defaultShader->ID);
}