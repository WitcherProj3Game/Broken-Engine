#ifndef __RESOURCE_FOLDER_H__
#define __RESOURCE_FOLDER_H__

#include "Resource.h"
#include <vector>

BE_BEGIN_NAMESPACE
class BROKEN_API ResourceFolder : public Resource {
public:
	ResourceFolder(uint UID, const char* source_file);
	~ResourceFolder();

	bool LoadInMemory() override;
	void FreeMemory() override;

	void AddResource(Resource* resource);
	void RemoveResource(Resource* resource);
	void AddChild(ResourceFolder* resource);
	void RemoveChild(ResourceFolder* child);
	std::vector<Resource*>& GetResources();
	std::vector<ResourceFolder*>& GetChilds();
	ResourceFolder* GetParent() const;

private:
	bool HasResource(Resource* resource);
	bool HasChild(ResourceFolder* resource);

	std::vector<Resource*> resources;
	std::vector<ResourceFolder*> childs;
	ResourceFolder* parent = nullptr;

private:
	void OnOverwrite() override;
	void OnDelete() override;
};
BE_END_NAMESPACE
#endif //__RESOURCE_FOLDER_H__