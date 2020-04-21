#ifndef __AUTOCOMPLETEFILEGEN_H__
#define __AUTOCOMPLETEFILEGEN_H__

#include "BrokenCore.h"
#include <string>
#include<vector>

BE_BEGIN_NAMESPACE

struct BROKEN_API SerializedFunction
{
	std::string body = "none";
	std::string description = "Description not available";
	std::string scope = "source.lua";
	std::string name = "none";

	SerializedFunction(const char* name, const char* scope): name(name), scope(scope){}
};

class BROKEN_API AutoCompleteFileGen 
{
public:
	AutoCompleteFileGen();
	~AutoCompleteFileGen();

public: //Function

	void GenerateAutoCompleteFile();


public: //Variables

	std::vector<SerializedFunction> engine_functions;

};

BE_END_NAMESPACE

#endif // !__AUTOCOMPLETEFILEGEN_H__

