#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleSceneIntro.h"
#include "ModuleRenderer3D.h"
#include "ModuleCamera3D.h"
#include "ModuleGui.h"
#include "ModuleHardware.h"

#include "mmgr/mmgr.h"

Application::Application()
{

	frames = 0;
	last_frame_ms = -1;
	last_fps = -1;
	capped_ms = 1000 / 60; // Get Display RR!!
	fps_counter = 0;
	appName = "";
	configpath = "Settings/EditorConfig.json";
	//log = "Application Logs:";

	window = new ModuleWindow(this);
	input = new ModuleInput(this);
	hardware = new ModuleHardware(this);
	scene_intro = new ModuleSceneIntro(this);
	renderer3D = new ModuleRenderer3D(this);
	camera = new ModuleCamera3D(this);
	gui = new ModuleGui(this);

	// The order of calls is very important!
	// Modules will Init() Start() and Update in this order
	// They will CleanUp() in reverse order

	// Main Modules
	AddModule(window);
	AddModule(camera);
	AddModule(input);
	AddModule(hardware);
	AddModule(gui);

	// Scenes
	AddModule(scene_intro);

	// Renderer last!
	AddModule(renderer3D);
}

Application::~Application()
{
	std::list<Module*>::reverse_iterator item = list_modules.rbegin();

	while(item != list_modules.rend())
	{
		delete *item;
		item++;
	}
}

bool Application::Init()
{
	bool ret = true;

	// --- Load App data from JSON files ---
	json config = JLoader.Load(configpath.data());

	// --- Create Config with default values if load fails ---
	if (config.is_null())
	{
		config = GetDefaultConfig();
	}

	// --- Reading App Name/ Org Name from json file ---
	std::string tmp = config["Application"]["Title"];
	appName = tmp;

	std::string tmp2 = config["Application"]["Organization"];
	orgName = tmp2;

	// Call Init() in all modules

	std::list<Module*>::const_iterator item = list_modules.begin();

	while(item != list_modules.end() && ret == true)
	{
		ret = (*item)->Init(config);
		item++;
	}

	// After all Init calls we call Start() in all modules
	LOG("Application Start --------------");
	item = list_modules.begin();

	while(item != list_modules.end() && ret == true)
	{
		ret = (*item)->Start();
		item++;
	}
	
	ms_timer.Start();

	SetMaxFramerate(App->window->GetDisplayRefreshRate());

	return ret;
}

// ---------------------------------------------
void Application::PrepareUpdate()
{
	dt = (float)ms_timer.Read() / 1000.0f;
	ms_timer.Start();
}

// ---------------------------------------------
void Application::FinishUpdate()
{
	// Recap on framecount and fps
	++frames;
	++fps_counter;

	if (fps_timer.Read() >= 1000)
	{
		last_fps = fps_counter;
		fps_counter = 0;
		fps_timer.Start();
	}

	last_frame_ms = ms_timer.Read();

	// cap fps
	if (capped_ms > 0 && (last_frame_ms < capped_ms))
		SDL_Delay(capped_ms - last_frame_ms);

	// --- Send data to GUI- PanelSettings Historiograms
	App->gui->LogFPS((float)last_fps, (float)last_frame_ms);

}

void Application::SaveAllStatus()
{
	// --- Create Config with default values ---
	json config = GetDefaultConfig();

	std::string tmp = appName;
	config["Application"]["Title"] = tmp;
	std::string tmp2 = orgName;
	config["Application"]["Organization"] = tmp2;

	// --- Call Save of all modules ---

	std::list<Module*>::const_iterator item = list_modules.begin();

	while (item != list_modules.end())
	{
		(*item)->SaveStatus(config);
		item++;
	}

	JLoader.Save(configpath.data(), config);
}

void Application::LoadAllStatus(json & file)
{
	// --- This function is not called at startup, but later if needed ---

	// --- Reading App name from json file ---
	std::string tmp = file["Application"]["Title"];
	appName = tmp;

	// --- Call Load of all modules ---

	json config = JLoader.Load(configpath.data());

	std::list<Module*>::const_iterator item = list_modules.begin();

	while (item != list_modules.end())
	{
		(*item)->LoadStatus(config);
		item++;
	}
}

// Call PreUpdate, Update and PostUpdate on all modules
update_status Application::Update()
{
	update_status ret = UPDATE_CONTINUE;
	PrepareUpdate();
	
	std::list<Module*>::const_iterator item = list_modules.begin();
	
	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PreUpdate(dt);
		item++;
	}

	item = list_modules.begin();

	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->Update(dt);
		item++;
	}

	item = list_modules.begin();

	while(item != list_modules.end() && ret == UPDATE_CONTINUE)
	{
		ret = (*item)->PostUpdate(dt);
		item++;
	}

	FinishUpdate();

	return ret;
}

bool Application::CleanUp()
{
	// --- Save all Status --- TODO: Should be called by user
	SaveAllStatus();

	bool ret = true;
	std::list<Module*>::reverse_iterator item = list_modules.rbegin();

	while(item != list_modules.rend() && ret == true)
	{
		ret = (*item)->CleanUp();
		item++;
	}
	return ret;
}

void Application::AddModule(Module* mod)
{
	list_modules.push_back(mod);
}


void Application::SetMaxFramerate(uint maxFramerate)
{
	if (maxFramerate > 0)
		capped_ms = 1000 / maxFramerate;
	else
		capped_ms = 0;
}

uint Application::GetMaxFramerate() const
{
	if (capped_ms > 0)
		return (uint)((1.0f / (float)capped_ms) * 1000.0f);
	else
		return 0;
}

const char * Application::GetAppName() const
{
	return appName.data();
}

void Application::SetAppName(const char* name) 
{
	appName.assign(name);
	App->window->SetWinTitle(appName.data());
}

void Application::SetOrganizationName(const char* name)
{
	orgName = name;
}

void Application::Log(const char * entry)
{
	//// --- Append all logs to a string so we can save them to a file upon app close --- 
	log.append(entry);
	

	// --- Ask GUI to print the log on the console ---
	App->gui->ConsoleLog(entry);
}

const char* Application::GetOrganizationName() const
{
	return orgName.data();
}

json Application::GetDefaultConfig() const
{
	// --- Create Config with default values ---
	json config = {
		{"Application", {
			{"Title", "CENTRAL 3D"},
			{"Organization", "CITM - UPC"}
		}},

		{"GUI", {
			{"Inspector", true},
			{"About", false},
			{"Settings", false},
		}},

		{"Window", {
			{"width", 1024},
			{"height", 720},
			{"fullscreen", false},
			{"resizable", true},
			{"borderless", false},
			{"fullscreenDesktop", false}
		}},

		{"Input", {

		}},

		{"Renderer3D", {
			{"VSync", true}
		}},
	};

	return config;
}
