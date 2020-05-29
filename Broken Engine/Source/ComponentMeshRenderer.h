#ifndef __COMPONENT_MESHRENDERER_H__
#define __COMPONENT_MESHRENDERER_H__

#include "Component.h"
#include "Timer.h"

// Specifying normal vectors length (used when drawing normals)
#define NORMAL_LENGTH 0.5

BE_BEGIN_NAMESPACE

class ResourceMesh;
class ComponentTransform;
class ResourceMaterial;

class BROKEN_API ComponentMeshRenderer : public Component
{
public:

	ComponentMeshRenderer(GameObject* ContainerGO);
	virtual ~ComponentMeshRenderer();

	void Update() override;

	void DrawComponent() override;
	void DrawNormals(const ResourceMesh& mesh, const ComponentTransform& transform) const;

	// --- Save & Load ---
	json Save() const override;
	void Load(json& node) override;
	void ONResourceEvent(uint UID, Resource::ResourceNotificationType type) override;
	void CreateInspectorNode() override;

	static inline Component::ComponentType GetType() { return Component::ComponentType::MeshRenderer; };

public:

	bool draw_vertexnormals = false;
	bool draw_facenormals = false;
	bool checkers = false;
	ResourceMaterial* material = nullptr;

private:

	//Shadows Control
	bool cast_shadows = true;
	bool receive_shadows = true;
	bool only_shadows = false;
	bool light_affected = true;

	bool m_TemporalTextureTest = false;
};

BE_END_NAMESPACE
#endif