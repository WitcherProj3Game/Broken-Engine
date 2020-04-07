#ifndef __MODULE_TEXTURES_H__
#define __MODULE_TEXTURES_H__

#include "Module.h"
#include "ResourceTexture.h"

#define CHECKERS_HEIGHT 32
#define CHECKERS_WIDTH 32

BE_BEGIN_NAMESPACE
class BROKEN_API ModuleTextures : public Module {
public:

	ModuleTextures(bool start_enabled = true);
	~ModuleTextures();

	bool Init(json& file) override;
	bool Start() override;
	bool CleanUp() override;

	uint CreateTextureFromFile(const char* path, uint& width, uint& height, uint& originalFormat, int UID = -1) const;

	uint GetCheckerTextureID() const { return CheckerTexID; }
	uint GetDefaultTextureID() const { return DefaultTexture; }
	const void* GetLastStoredData() const { return &m_LastStoredData; }

private:

	uint LoadCheckImage() const;
	uint LoadDefaultTexture() const;

	uint CheckerTexID = 0;
	uint DefaultTexture = 0;
	mutable void* m_LastStoredData = nullptr;

private:

	// --- Called by CreateTextureFromPixels to split code ---
	void SetTextureParameters(bool CheckersTexture = false) const;

	uint CreateTextureFromPixels(int internalFormat, uint width, uint height, uint format, const void* pixels, bool CheckersTexture = false) const;
	void CreateTextureFromImage(uint& TextureID, uint& width, uint& height, std::string& path, uint& originalFormat) const;
};

BE_END_NAMESPACE
#endif