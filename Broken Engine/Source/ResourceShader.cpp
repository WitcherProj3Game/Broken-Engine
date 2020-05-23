#include <fstream>
#include <sstream>
#include <iomanip>

#include "ResourceShader.h"
#include "ImporterMeta.h"
#include "ResourceMeta.h"

#include "Application.h"
#include "ModuleGui.h"
#include "ModuleFileSystem.h"
#include "ModuleResourceManager.h"
#include "ModuleRenderer3D.h"

#include "OpenGL.h"

#include "mmgr/mmgr.h"

using namespace Broken;
ResourceShader::ResourceShader(uint UID, const char* source_file) : Resource(Resource::ResourceType::SHADER, UID, source_file) 
{
	extension = ".glsl";
	resource_file = SHADERS_FOLDER + std::to_string(UID) + extension;

	vShaderCode = App->renderer3D->VertexShaderTemplate;
	fShaderCode = App->renderer3D->FragmentShaderTemplate;
	gShaderCode = "none";

	CreateShaderProgram();

	previewTexID = App->gui->shaderTexID;
}

ResourceShader::~ResourceShader() 
{
	//DeleteShaderProgram();
}


bool ResourceShader::LoadInMemory() 
{
	bool ret = true;

	// --- Load from binary file ---
	if (!ret/*App->fs->Exists(resource_file.c_str())*/)
	{
		int success;
		char infoLog[512];
		
		ret = LoadStream(resource_file.c_str());
		
		if (ret)
		{
			// --- Create empty program ---
			DeleteShaderProgram();

			success = CreateShaderProgram();

			if (!success)
			{
				glGetProgramInfoLog(ID, 512, NULL, infoLog);
				ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:SHADER::PROGRAM::LINKING_FAILED: %s", infoLog);

				glDeleteProgram(ID);
			}
			else
			{

				// --- Retrieve data from meta ---
				ImporterMeta* IMeta = App->resources->GetImporter<ImporterMeta>();
				ResourceMeta* meta = (ResourceMeta*)IMeta->Load(original_file.c_str());

				if (meta)
				{
					uint format = meta->ResourceData["FORMAT"].is_null() ? 0 : meta->ResourceData["FORMAT"].get<uint>();

					char* buffer = nullptr;

					uint size = App->fs->Load(resource_file.c_str(), &buffer);

					// --- Load binary program ---
					glProgramBinary(ID, (GLenum)format, (void*)buffer, (GLint)size);

					if(buffer)
						delete[] buffer;
				}


				// --- Print linking errors if any ---
				glGetProgramiv(ID, GL_LINK_STATUS, &success);

				if (!success)
				{
					glGetProgramInfoLog(ID, 512, NULL, infoLog);
					ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]: Could not load binary shader, triggered recompilation %s", infoLog);

					// --- Trigger recompilation ---
					glDeleteProgram(ID);
					CreateShaderProgram();
					ReloadAndCompileShader();
				}

				if (App->fs->Exists(original_file.c_str()))
				{
					// --- Load original code ---
					LoadStream(original_file.c_str());

					// --- Separate vertex and fragment (and geometry if defined) ---
					std::string ftag = "#define FRAGMENT_SHADER";
					uint FragmentLoc = ShaderCode.find(ftag);

					std::string gtag = "#define GEOMETRY_SHADER";
					uint GeometryLoc = ShaderCode.find(gtag);

					vShaderCode = ShaderCode.substr(0, FragmentLoc - 1);
					fShaderCode = std::string("#version 440 core\n").append(ShaderCode.substr(FragmentLoc, GeometryLoc != std::string::npos ? GeometryLoc - 1: ShaderCode.size()));

					if (GeometryLoc != std::string::npos)
						gShaderCode = std::string("#version 440 core\n").append(ShaderCode.substr(GeometryLoc, ShaderCode.size()));
				}
			}
		}
	}

	// --- Load glsl file ---
	else
	{	
		ret = LoadStream(original_file.c_str());

		// --- If no fs failure occurred... ---
		if (ret) 
		{
			DeleteShaderProgram();

			// --- Separate vertex and fragment (and geometry if defined) ---
			std::string ftag = "#define FRAGMENT_SHADER";
			uint FragmentLoc = ShaderCode.find(ftag);

			std::string gtag = "#define GEOMETRY_SHADER";
			uint GeometryLoc = ShaderCode.find(gtag);

			vShaderCode = ShaderCode.substr(0, FragmentLoc - 1);
			fShaderCode = std::string("#version 440 core\n").append(ShaderCode.substr(FragmentLoc, GeometryLoc != std::string::npos ? GeometryLoc - 1 : ShaderCode.size()));

			if (GeometryLoc != std::string::npos)
				gShaderCode = std::string("#version 440 core\n").append(ShaderCode.substr(GeometryLoc, ShaderCode.size()));

			// --- Compile shaders ---
			int success = 0;
			char infoLog[512];
	
			success = CreateVertexShader(vertex, vShaderCode.data());
	
			if (!success) 
			{
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:Vertex Shader compilation error: %s", infoLog);
			}
	
			success = CreateFragmentShader(fragment, fShaderCode.data());
	
			if (!success) 
			{
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:Fragment Shader compilation error: %s", infoLog);
			}

			if (gShaderCode != "none")
			{
				success = CreateGeometryShader(geometry, gShaderCode.data());

				if (!success)
				{
					glGetShaderInfoLog(geometry, 512, NULL, infoLog);
					ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:Geometry Shader compilation error: %s", infoLog);
				}
			}
	
			success = CreateShaderProgram(vertex, fragment, geometry);
	
			if (!success) 
			{
				glGetProgramInfoLog(ID, 512, NULL, infoLog);
				ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:SHADER::PROGRAM::LINKING_FAILED: %s", infoLog);
			}
	
			// delete the shaders as they're linked into our program now and no longer necessary
			glDeleteShader(vertex);
			glDeleteShader(fragment);
			glDeleteShader(geometry);
		}
	}

	return ret;
}

void ResourceShader::FreeMemory() 
{
	DeleteShaderProgram();
}

void ResourceShader::ReloadAndCompileShader() 
{
	uint new_vertex, new_fragment, new_geometry = 0;

	// --- Compile new data ---

	const char* vertexcode = vShaderCode.c_str();
	const char* fragmentcode = fShaderCode.c_str();
	const char* geometrycode = gShaderCode.c_str();


	GLint success = 0;
	GLint accumulated_errors = 0;
	char infoLog[512];

	// --- Compile new vertex shader ---

	success = CreateVertexShader(new_vertex, vertexcode);

	if (!success) 
	{
		glGetShaderInfoLog(new_vertex, 512, NULL, infoLog);
		ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:Vertex Shader compilation error: %s", infoLog);
		accumulated_errors++;
	}
	else
		ENGINE_AND_SYSTEM_CONSOLE_LOG("Vertex Shader compiled successfully");

	// --- Compile new fragment shader ---

	success = CreateFragmentShader(new_fragment, fragmentcode);

	if (!success) 
	{
		glGetShaderInfoLog(new_fragment, 512, NULL, infoLog);
		ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:Fragment Shader compilation error: %s", infoLog);
		accumulated_errors++;
	}
	else
		ENGINE_AND_SYSTEM_CONSOLE_LOG("Fragment Shader compiled successfully");

	// --- Compile new geometry shader ---

	if (gShaderCode != "none")
	{
		success = CreateGeometryShader(new_geometry, geometrycode);

		if (!success)
		{
			glGetShaderInfoLog(new_geometry, 512, NULL, infoLog);
			ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:Geometry Shader compilation error: %s", infoLog);
			accumulated_errors++;
		}
		else
			ENGINE_AND_SYSTEM_CONSOLE_LOG("Geometry Shader compiled successfully");
	}

	if (accumulated_errors == 0) 
	{
		// --- Delete previous shader data ---
		glDetachShader(ID, vertex);
		glDetachShader(ID, fragment);

		if (gShaderCode != "none")
			glDetachShader(ID, geometry);

		// --- Attach new shader objects and link ---
		glAttachShader(ID, new_vertex);
		glAttachShader(ID, new_fragment);

		if (gShaderCode != "none")
			glAttachShader(ID, new_geometry);

		glLinkProgram(ID);
		//glValidateProgram(ID);
		glGetProgramiv(ID, GL_LINK_STATUS, &success);

		if (!success) 
		{
			GLint length;
			glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &length);
			std::vector<GLchar> message(length);
			glGetProgramInfoLog(ID, length, &length, &message[0]);

			ENGINE_AND_SYSTEM_CONSOLE_LOG("Error in Shader Program Linking: %s", message.data());

			//glGetProgramInfoLog(ID, 512, NULL, infoLog);
			ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:SHADER::PROGRAM::LINKING_FAILED: %s", infoLog);

			// --- Detach new shader objects ---
			glDetachShader(ID, new_vertex);
			glDetachShader(ID, new_fragment);

			if (gShaderCode != "none")
				glDetachShader(ID, new_geometry);

			glDeleteShader(new_vertex);
			glDeleteShader(new_fragment);

			if (gShaderCode != "none")
				glDeleteShader(new_geometry);

			// --- Attach old shader objects ---
			glAttachShader(ID, vertex);
			glAttachShader(ID, fragment);

			if (gShaderCode != "none")
				glAttachShader(ID, geometry);
		}
		else 
		{
			// --- On success, delete old shader objects and update ids ---
			glDeleteShader(vertex);
			glDeleteShader(fragment);

			if (gShaderCode != "none")
				glDeleteShader(geometry);

			vertex = new_vertex;
			fragment = new_fragment;

			if (gShaderCode != "none")
				geometry = new_geometry;

			ENGINE_AND_SYSTEM_CONSOLE_LOG("Shader Program linked successfully");
		}
	}
	else 
	{
		glDeleteShader(new_vertex);
		glDeleteShader(new_fragment);

		if (gShaderCode != "none")
			glDeleteShader(new_geometry);
	}

}

void ResourceShader::GetAllUniforms(std::vector<Uniform*>& uniforms) 
{
	std::vector<Uniform*> new_uniforms;

	int uniform_count;
	glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &uniform_count);

	char name[128];
	GLenum type;
	GLint size;

	for (uint i = 0; i < uniform_count; ++i) 
	{
		glGetActiveUniform(ID, i, 128, nullptr, &size, &type, name);

		if (strcmp(name, "u_Model") == 0
			|| strcmp(name, "u_View") == 0
			|| strcmp(name, "u_Proj") == 0
			|| strcmp(name, "u_Color") == 0
			|| strcmp(name, "u_CameraPosition") == 0
			|| strcmp(name, "u_Shininess") == 0
			|| strcmp(name, "u_UseTextures") == 0
			|| strcmp(name, "u_HasDiffuseTexture") == 0
			|| strcmp(name, "u_HasSpecularTexture") == 0
			|| strcmp(name, "u_HasNormalMap") == 0
			|| strcmp(name, "u_DrawNormalMapping") == 0
			|| strcmp(name, "u_DrawNormalMapping_Lit") == 0
			|| strcmp(name, "u_DrawNormalMapping_Lit_Adv") == 0
			|| strcmp(name, "u_LightsNumber") == 0
			|| strcmp(name, "u_GammaCorrection") == 0
			|| strcmp(name, "u_AmbientColor") == 0
			|| strcmp(name, "u_HasTransparencies") == 0
			|| strcmp(name, "u_IsText") == 0
			|| strcmp(name, "u_Exposure") == 0
			|| strcmp(name, "u_ReceiveShadows") == 0
			|| strcmp(name, "u_ShadowIntensity") == 0	|| strcmp(name, "u_ShadowBias") == 0				|| strcmp(name, "u_ShadowsSmoothMultiplicator") == 0
			|| strcmp(name, "u_ShadowPoissonBlur") == 0	|| strcmp(name, "u_ShadowOffsetBlur") == 0			|| strcmp(name, "u_ShadowPCFDivisor") == 0
			|| strcmp(name, "u_ShadowSmootherPCF") == 0	|| strcmp(name, "u_ShadowSmootherPoissonDisk") == 0	|| strcmp(name, "u_ShadowSmootherBoth") == 0
			|| strcmp(name, "u_ClampShadows") == 0
			|| std::string(name).find("u_BkLights") != std::string::npos
			|| strcmp(name, "time") == 0)
			continue;

		Uniform* uniform = nullptr;

		// --- This switch prevents retrieving unwanted/unsupported uniforms ---
		// MYTODO: we may avoid the switch by defining some filters and testing against them

		switch (type) 
		{
		case GL_INT:
			uniform = new Uniform();
			FillUniform(uniform, name, type);
			break;

		case GL_FLOAT:
			uniform = new Uniform();
			FillUniform(uniform, name, type);
			break;

		case GL_FLOAT_VEC2:
			uniform = new Uniform();
			FillUniform(uniform, name, type);
			break;

		case GL_FLOAT_VEC3:
			uniform = new Uniform();
			FillUniform(uniform, name, type);
			break;

		case GL_FLOAT_VEC4:
			uniform = new Uniform();
			FillUniform(uniform, name, type);
			break;

		case GL_INT_VEC2:
			uniform = new Uniform();
			FillUniform(uniform, name, type);
			break;

		case GL_INT_VEC3:
			uniform = new Uniform();
			FillUniform(uniform, name, type);
			break;

		case GL_INT_VEC4:
			uniform = new Uniform();
			FillUniform(uniform, name, type);
			break;

		default:
			continue;
			break;

		}

		// --- Conserve previous uniform values if they still exist ---
		for (uint i = 0; i < uniforms.size(); ++i) 
		{
			if (uniforms[i]->name == uniform->name && uniforms[i]->type == uniform->type)
				uniform->value = uniforms[i]->value;
		}

		new_uniforms.push_back(uniform);
	}

	// --- Delete previous uniforms and retrieve new ones ---
	for (uint i = 0; i < uniforms.size(); ++i) 
	{
		delete uniforms[i];
	}

	uniforms.clear();

	uniforms = new_uniforms;
}


// Internal use only!
bool ResourceShader::CreateVertexShader(unsigned int& vertex, const char* vShaderCode) 
{
	GLint success = 0;

	// vertex Shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	// print compile errors if any
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);

	return success;
}

// Internal use only!
bool ResourceShader::CreateFragmentShader(unsigned int& fragment, const char* fShaderCode) 
{
	GLint success = 0;

	// similar for Fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	// print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	return success;
}

bool ResourceShader::CreateGeometryShader(unsigned int& geometry, const char* gShaderCode)
{
	GLint success = 0;

	// similar for Fragment Shader
	fragment = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(fragment, 1, &gShaderCode, NULL);
	glCompileShader(fragment);
	// print compile errors if any
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);

	return success;
}

// Internal use only!
bool ResourceShader::CreateShaderProgram(unsigned int vertex, unsigned int fragment, unsigned int geometry) 
{
	GLint success = 0;

	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glAttachShader(ID, geometry);
	glLinkProgram(ID);
	// print linking errors if any
	glGetProgramiv(ID, GL_LINK_STATUS, &success);

	return success;
}

bool ResourceShader::CreateShaderProgram() 
{
	GLint success = 1;
	char infoLog[512];

	// shader Program
	ID = glCreateProgram();
	//glLinkProgram(ID);
	//// print linking errors if any
	//glGetProgramiv(ID, GL_LINK_STATUS, &success);

	//if (!success)
	//{
	//	glGetShaderInfoLog(ID, 512, NULL, infoLog);
	//	ENGINE_CONSOLE_LOG("|[error]: Shader creation error: %s", infoLog);
	//}
	//else
	//	ENGINE_CONSOLE_LOG("Shader created successfully");

	return success;
}

void ResourceShader::DeleteShaderProgram() 
{
	if (glIsProgram(ID)) 
	{
		glDeleteProgram(ID);
		ID = 0;
	}
}

void ResourceShader::FillUniform(Uniform* uniform, const char* name, const uint type) const 
{
	uniform->name = name;
	uniform->location = glGetUniformLocation(ID, name);
	uniform->type = type;
}

bool ResourceShader::LoadStream(const char* path)
{
	bool ret = true;

	// --- Create input stream object ---
	std::ifstream ShaderFile;

	// --- Ensure ifstream objects can throw exceptions ---
	ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// --- Open files ---
		ShaderFile.open(path);
		std::stringstream ShaderStream;
		// --- Read file's buffer contents into streams ---
		ShaderStream << ShaderFile.rdbuf();
		// --- Close file handlers ---
		ShaderFile.close();
		// --- Convert stream into string ---
		ShaderCode = ShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		ENGINE_AND_SYSTEM_CONSOLE_LOG("|[error]:SHADER:FILE_NOT_SUCCESFULLY_READ: %s", e.what());
		ret = false;
	}

	return ret;
}

void ResourceShader::OnOverwrite() 
{
	NotifyUsers(ResourceNotificationType::Overwrite);

	bool ret = LoadStream(original_file.c_str());

	// --- If no fs failure occurred... ---
	if (ret)
	{
		// --- Separate vertex and fragment (and geometry if defined) ---
		std::string ftag = "#define FRAGMENT_SHADER";
		uint FragmentLoc = ShaderCode.find(ftag);

		std::string gtag = "#define GEOMETRY_SHADER";
		uint GeometryLoc = ShaderCode.find(gtag);

		vShaderCode = ShaderCode.substr(0, FragmentLoc - 1);
		fShaderCode = std::string("#version 440 core\n").append(ShaderCode.substr(FragmentLoc, GeometryLoc != std::string::npos ? GeometryLoc - 1 : ShaderCode.size()));

		if (GeometryLoc != std::string::npos)
			gShaderCode = std::string("#version 440 core\n").append(ShaderCode.substr(GeometryLoc, ShaderCode.size()));
	}

	ReloadAndCompileShader();

	//FreeMemory();
	//LoadInMemory();
}

void ResourceShader::OnDelete() 
{
	NotifyUsers(ResourceNotificationType::Deletion);

	FreeMemory();
	App->fs->Remove(resource_file.c_str());

	App->resources->RemoveResourceFromFolder(this);
	App->resources->ONResourceDestroyed(this);
}

void ResourceShader::use() 
{
	glUseProgram(ID);
}

void ResourceShader::setBool(const std::string& name, bool value) const 
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void ResourceShader::setUniform(const char* name, data& unidata, UniformType UniType) const
{
	switch (UniType)
	{
		case Broken::UniformType::intU:
			glUniform1i(glGetUniformLocation(ID, name), unidata.intU);
			break;
		case Broken::UniformType::floatU:
			glUniform1f(glGetUniformLocation(ID, name), unidata.floatU);
			break;
		case Broken::UniformType::vec2U:
			glUniform2f(glGetUniformLocation(ID, name), unidata.vec2U.x, unidata.vec2U.y);
			break;
		case Broken::UniformType::vec3U:
			glUniform3f(glGetUniformLocation(ID, name), unidata.vec3U.x, unidata.vec3U.y, unidata.vec3U.z);
			break;
		case Broken::UniformType::vec4U:
			glUniform4f(glGetUniformLocation(ID, name), unidata.vec4U.x, unidata.vec4U.y, unidata.vec4U.z, unidata.vec4U.w);
			break;
		default:
			break;
	}
}

