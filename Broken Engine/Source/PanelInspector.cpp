#include "PanelInspector.h"
#include "Imgui/imgui.h"

#include "EngineApplication.h"
//#include "ModuleSceneManager.h"
//#include "ModuleRenderer3D.h"
//#include "ModuleResourceManager.h"
//#include "ModuleGui.h"

//#include "GameObject.h"
//#include "ComponentTransform.h"
//#include "ComponentMesh.h"
//#include "ComponentMeshRenderer.h"
//#include "ComponentCamera.h"

#include "PanelShaderEditor.h"

//#include "ResourceMesh.h"
//#include "ResourceMaterial.h"
//#include "ResourceTexture.h"
//#include "ResourceShader.h"
//#include "ComponentScript.h"

//#include "mmgr/mmgr.h"

PanelInspector::PanelInspector(char * name) : Panel(name)
{
}

PanelInspector::~PanelInspector()
{
}

bool PanelInspector::Draw()
{
	ImGui::SetCurrentContext(EngineApp->gui->getImgUICtx());

	ImGuiWindowFlags settingsFlags = 0;
	settingsFlags = ImGuiWindowFlags_NoFocusOnAppearing;

	if (ImGui::Begin(name, &enabled, settingsFlags))
	{
		Broken::GameObject* Selected = EngineApp->scene_manager->GetSelectedGameObject();

		if (Selected == nullptr)
		{
			ImGui::End();
			return false;
		}

		// --- Game Object ---
		CreateGameObjectNode(*Selected);

		// --- Components ---

		std::vector<Broken::Component*>* components = &Selected->GetComponents();

		for (std::vector<Broken::Component*>::const_iterator it = components->begin(); it != components->end(); ++it)
		{
			if (Startup)
				ImGui::SetNextItemOpen(true);

			(*it)->CreateInspectorNode();
			ImGui::NewLine();
			ImGui::Separator();
		}

		static ImGuiComboFlags flags = 0;

		const char* items[] = { "Default", "Mesh", "Mesh Renderer", "Dynamic RigidBody", "Collider", "Audio Source", "Particle Emitter", "UI Canvas", "UI Image", "UI Text", "UI Button" };
		static const char* item_current = items[0];

		ImGui::NewLine();

		// --- Add component ---
		if (ImGui::BeginCombo("##Components Combo", "Add Component", flags)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool is_selected = (item_current == items[n]);
				if (ImGui::Selectable(items[n], is_selected))
					item_current = items[n];
				if (is_selected)
					ImGui::SetItemDefaultFocus();   // Set the initial focus when opening the combo (scrolling + for keyboard navigation support in the upcoming navigation branch)
			}
			ImGui::EndCombo();
		}

		// --- Handle drag & drop ---
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("resource"))
			{
				uint UID = *(const uint*)payload->Data;
				Broken::Resource* resource = EngineApp->resources->GetResource(UID, false);

				// MYTODO: Instance resource here, put it on scene (depending on resource)
				if (resource && resource->GetType() == Broken::Resource::ResourceType::SCRIPT)
				{
					resource = EngineApp->resources->GetResource(UID);
					Broken::ComponentScript* script = (Broken::ComponentScript*)Selected->AddComponent(Broken::Component::ComponentType::Script);
					script->AssignScript((Broken::ResourceScript*)resource);

				}
			}

			ImGui::EndDragDropTarget();
		}

		// --- Add here temporal conditions to know which component to add ---

		// MYTODO: Note currently you can not add the same type of component to a go (to be changed)

		if (item_current == "Mesh")
		{
			Selected->AddComponent(Broken::Component::ComponentType::Mesh);
		}

		if (item_current == "Mesh Renderer")
		{
			Selected->AddComponent(Broken::Component::ComponentType::MeshRenderer);
		}

		if (item_current == "UI Canvas")
		{
			Selected->AddComponent(Broken::Component::ComponentType::Canvas);
		}

		if (item_current == "UI Image")
		{
			Selected->AddComponent(Broken::Component::ComponentType::Image);
		}

		if (item_current == "UI Text")
		{
			Selected->AddComponent(Broken::Component::ComponentType::Text);
		}

		if (item_current == "UI Button")
		{
			Selected->AddComponent(Broken::Component::ComponentType::Button);
		}

		if (item_current == "Dynamic RigidBody")
		{
			Selected->AddComponent(Broken::Component::ComponentType::DynamicRigidBody);
		}

		if (item_current == "Collider")
		{
			Selected->AddComponent(Broken::Component::ComponentType::Collider);
		}
		if (item_current == "Particle Emitter")
		{
			Selected->AddComponent(Broken::Component::ComponentType::ParticleEmitter);
		}
		if (item_current == "Audio Source")
		{
			Selected->AddComponent(Broken::Component::ComponentType::AudioSource);
		}

		item_current = items[0];
		// MYTODO: move this to the component itself

		// --- Material ---
		//if (Selected->GetComponent<ComponentMaterial>())
		//{
		//	CreateMaterialNode(*Selected);
		//	ImGui::Separator();
		//}

		// --- Camera ---
		//if (Selected->GetComponent<ComponentCamera>())
		//{
		//	CreateCameraNode(*Selected);
		//	ImGui::Separator();
		//}



		if (Startup)
			Startup = false;
	}
	ImGui::End();


	return true;
}

void PanelInspector::CreateGameObjectNode(Broken::GameObject & Selected) const
{
	ImGui::BeginChild("child", ImVec2(0, 35), true);

	if (ImGui::Checkbox("##GOActive", &Selected.GetActive()))
	{
		if (Selected.GetActive())
			Selected.Enable();
		else
			Selected.Disable();
	}

	ImGui::SameLine();

	// --- Game Object Name Setter ---
	static char GOName[100] = "";
	strcpy_s(GOName, 100, Selected.GetName());
	if (ImGui::InputText("", GOName, 100, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
		Selected.SetName(GOName);

	bool objectStatic= Selected.Static;

	ImGui::SameLine();

	if (ImGui::Checkbox("Static", &objectStatic)) {
		
		if (!Selected.childs.empty())
			ImGui::OpenPopup("Static gameObject");
		else
			EngineApp->scene_manager->SetStatic(&Selected, objectStatic,  false);
	}

	ImGui::SetNextWindowSize(ImVec2(400,75));
	if (ImGui::BeginPopup("Static gameObject", ImGuiWindowFlags_NoScrollbar))
	{
		if (Selected.Static) {
			ImGui::Indent(30);
			ImGui::Text("You are about to make this object non-static.");
			ImGui::Spacing();
			
			ImGui::Unindent(10);
			ImGui::Text("Do you want its children to be non-static aswell?");
			
			ImGui::Spacing();
			
			ImGui::Indent(130);
			if (ImGui::Button("Yes")) {
				EngineApp->scene_manager->SetStatic(&Selected, objectStatic, true);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			if (ImGui::Button("No")) {
				EngineApp->scene_manager->SetStatic(&Selected, objectStatic, false);
				ImGui::CloseCurrentPopup();
			}
		}
		else {
			ImGui::Indent(30);
			ImGui::Text("You are about to make this object static.");
			ImGui::Spacing();

			ImGui::Unindent(10);
			ImGui::Text("Do you want its children to be static aswell?");

			ImGui::Spacing();

			ImGui::Indent(130); 
			
			if (ImGui::Button("Yes")) {
				EngineApp->scene_manager->SetStatic(&Selected, objectStatic, true);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			if (ImGui::Button("No")) {
				EngineApp->scene_manager->SetStatic(&Selected, objectStatic, false);
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
	ImGui::EndChild();
}
