#ifndef __SCENE_MANAGER_H__
#define __SCENE_MANAGER_H__

#include "Module.h"
#include <vector>

class GameObject;
class ComponentMaterial;
struct aiScene;

class ModuleSceneManager : public Module
{
public:

	ModuleSceneManager(bool start_enabled = true);
	~ModuleSceneManager();

	bool Init(json file);
	bool Start();
	update_status PreUpdate(float dt);
	update_status Update(float dt);
	bool CleanUp();

	GameObject* CreateEmptyGameObject();
	ComponentMaterial* CreateEmptyMaterial();
	ComponentMaterial* CreateMaterialFromScene(const aiScene& scene, const char& file_path);

	// --- Getters ---
	uint GetNumGameObjects() const;

	// --- Utilities ---
	void Draw() const;

private:

	std::vector<GameObject*> game_objects;
	std::vector<ComponentMaterial*> Materials;
};

#endif