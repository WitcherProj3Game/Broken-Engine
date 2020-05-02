#ifndef __BE_MODULERENDERER_H__
#define __BE_MODULERENDERER_H__

#include "Module.h"
#include "Color.h"
#include "Math.h"

#define MAX_LIGHTS 8

BE_BEGIN_NAMESPACE
class ComponentCamera;
class ResourceShader;
class ResourceMesh;
class ResourceMaterial;
class ComponentLight;
class math::float4x4;
class ComponentParticleEmitter;
class GameObject;
class ComponentLight;

typedef int RenderMeshFlags;

enum BROKEN_API RenderMeshFlags_
{
	None		= 0,
	outline		= 1 << 0,
	selected	= 1 << 1,
	checkers	= 1 << 2,
	wire		= 1 << 3,
	texture		= 1 << 4,
	color       = 1 << 5
};

struct BROKEN_API RenderMesh
{
	RenderMesh(float4x4 transform, const ResourceMesh* mesh, ResourceMaterial* mat, const RenderMeshFlags flags = 0/*, const Color& color = White*/) : transform(transform), resource_mesh(mesh), mat(mat), flags(flags)/*, color(color)*/ {}

	RenderMesh() {};

	float4x4 transform;
	const ResourceMesh* resource_mesh = nullptr;
	ResourceMaterial* mat = nullptr;
	Color color; // force a color draw, useful if no texture is given


	// temporal!
	const ResourceMesh* deformable_mesh = nullptr;


	// --- Add rendering options here ---
	RenderMeshFlags flags = None;
};

template <typename Box>
struct BROKEN_API RenderBox
{
	RenderBox(const Box* box, const Color& color) : box(box), color(color) {}

	const Box* box;
	Color color;
};

struct BROKEN_API RenderLine
{
	RenderLine(float4x4 transform, const float3& a, const float3& b, const Color& color) : transform(transform), a(a), b(b), color(color) {}

	float4x4 transform;
	float3 a;
	float3 b;
	Color color;
};

enum BROKEN_API AlphaFunction
{
	SRC_ONE_MINUS_SRC = 0,
	ONE_ONE_MINUS_SRC,
	SRC_ONE_MINUS_SCR_ONE
};

class BROKEN_API ModuleRenderer3D : public Module
{
	friend class ModuleResourceManager;
public:

	// --- Module ---
	ModuleRenderer3D(bool start_enabled = true);
	~ModuleRenderer3D();

	bool Init(json& file) override;
	update_status PreUpdate(float dt) override;
	update_status PostUpdate(float dt) override;
	bool CleanUp() override;
	virtual void LoadStatus(const json& file);
	virtual const json& SaveStatus() const;


	void OnResize(int width, int height);

	// --- Lights ---
	void AddLight(ComponentLight* light);
	void PopLight(ComponentLight* light);
	const int GetLightIndex(ComponentLight* light);

	// --- Render Commands --- // Deformable mesh is Temporal!
	void DrawMesh(const float4x4 transform, const ResourceMesh* mesh, ResourceMaterial* mat, const ResourceMesh* deformable_mesh = nullptr, const RenderMeshFlags flags = 0, const Color& color = White);
	void DrawLine(const float4x4 transform, const float3 a, const float3 b, const Color& color);
	void DrawAABB(const AABB& box, const Color& color);
	void DrawOBB(const OBB& box, const Color& color);
	void DrawFrustum(const Frustum& box, const Color& color);
	const std::string& RenderSceneToTexture(std::vector<GameObject*>& scene_gos, uint& texId);

	void ClearRenderOrders();

public:

	// --- Setters ---
	bool SetVSync(bool _vsync);
	void SetActiveCamera(ComponentCamera* camera);
	void SetCullingCamera(ComponentCamera* camera);
	void SetGammaCorrection(const float gammaCorr) { m_GammaCorrection = gammaCorr; }
	void SetSceneAmbientColor(const float3 color) { m_AmbientColor = color; }
	void SetRendererAlphaFunction(const AlphaFunction function) { m_RendererAlphaFunc = function; }

	// --- Getters ---
	bool GetVSync() const { return vsync; }
	const float GetGammaCorrection() const { return m_GammaCorrection; }
	const float3 GetSceneAmbientColor() const { return m_AmbientColor; }
	const AlphaFunction GetRendererAlphaFunction() const { return m_RendererAlphaFunc; }

private:

	// --- Utilities ---
	void UpdateGLCapabilities() const;
	void HandleObjectOutlining();
	void CreateGrid(float target_distance);

	// --- Buffers ---
	uint CreateBufferFromData(uint Targetbuffer, uint size, void* data) const;
	void CreateFramebuffer();

	// --- Shaders ---
	void CreateDefaultShaders();

private:

	// --- Draw Commands ---
	void SendShaderUniforms(uint shader);
	void DrawRenderMeshes();
	void DrawTransparentRenderMeshes();
	void DrawRenderMesh(std::vector<RenderMesh> meshInstances);
	void DrawFramebuffer();

	// --- Draw Utilities ---
	void DrawRenderLines();
	void DrawRenderBoxes();
	void DrawGrid();
	void DrawSkybox();

	// --- Draw Wireframe using given vertices ---
	template <typename Box>
	static void DrawWire(const Box& box, Color color, uint VAO)
	{
		float3 corners[8];
		box.GetCornerPoints(corners);
		DrawWireFromVertices(corners, color, VAO);
	};

	static void DrawWireFromVertices(const float3* corners, Color color, uint VAO);

public:

	// --- Default Shader ---
	ResourceShader* defaultShader = nullptr;
	ResourceShader* linepointShader = nullptr;
	ResourceShader* OutlineShader = nullptr;
	ResourceShader* ZDrawerShader = nullptr;
	ResourceShader* screenShader = nullptr;
	ResourceShader* UI_Shader = nullptr;

	ResourceShader* SkyboxShader = nullptr;

	std::string VertexShaderTemplate;
	std::string FragmentShaderTemplate;

	std::vector<ComponentParticleEmitter*> particleEmitters;

	ComponentCamera* active_camera = nullptr;
	ComponentCamera* culling_camera = nullptr;
	ComponentCamera* screenshot_camera = nullptr;

	SDL_GLContext context;

	// --- Flags ---
	bool vsync = true;
	bool depth = true;
	bool cull_face = true;
	bool lighting = false;
	bool color_material = true;
	bool wireframe = false;
	bool zdrawer = false;
	bool renderfbo = true;
	bool drawfb = false;
	bool display_boundingboxes = false;
	bool display_grid = true;
	bool m_Draw_normalMapping = false;
	bool m_Draw_normalMapping_Lit = false;
	bool m_Draw_normalMapping_Lit_Adv = false;

	uint rendertexture = 0;
	uint depthMapTexture = 0;

private:

	std::map<uint, std::vector<RenderMesh>> render_meshes;
	std::map<uint, std::vector<RenderMesh>> transparent_render_meshes;

	std::vector<RenderBox<AABB>> render_aabbs;
	std::vector<RenderBox<OBB>> render_obbs;
	std::vector<RenderBox<Frustum>> render_frustums;
	std::vector<RenderLine> render_lines;

	//Lights vector
	std::vector<ComponentLight*> m_LightsVec;

	//Rendering General Options
	float m_GammaCorrection = 2.0f;
	float3 m_AmbientColor = float3::one;
	AlphaFunction m_RendererAlphaFunc = SRC_ONE_MINUS_SRC;

	//Other Generic Stuff
	uint fbo = 0;
	uint cubemapTexID = 0;
	uint skyboxVAO = 0;
	uint skyboxVBO = 0;
	uint depthbufferFBO = 0;
	uint depthbuffer = 0;
	uint PointLineVAO = 0;
	uint Grid_VAO = 0;
	uint Grid_VBO = 0;
	uint quadVAO = 0;
	uint quadVBO = 0;
};
BE_END_NAMESPACE
#endif
