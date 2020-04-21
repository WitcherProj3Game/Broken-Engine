#include "AutoCompleteFileGen.h"
#include "JSONLoader.h"
#include "ModuleFileSystem.h"
#include "Application.h"

using namespace Broken;

AutoCompleteFileGen::AutoCompleteFileGen()
{
}

AutoCompleteFileGen::~AutoCompleteFileGen()
{
}

//This function iterates the list of functions to deploy them in the snippets.json file to place in VSCode 
void AutoCompleteFileGen::GenerateAutoCompleteFile()
{
	nlohmann::json file;

	file["Test"]["body"] = "body test";
	file["Test"]["scope"] = "scope test";
	file["Test"]["prefix"] = "prefix_test";
	file["Test"]["description"] = "description_test";

	// --- Serialize JSON to string ---
	std::string data;
	App->GetJLoader()->Serialize(file, data);

	// --- Finally Save to file ---
	char* buffer = (char*)data.data();
	uint size = data.length();

	std::string file_path = "snippets.json";
	App->fs->Save(file_path.c_str(), buffer, size);
}
