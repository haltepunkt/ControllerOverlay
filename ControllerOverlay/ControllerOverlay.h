#pragma comment(lib, "BakkesMod.lib")

#include <fstream>

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"

#include "imgui/imgui.h"

#define RED ImColor(255, 0, 0, 255)
#define BLUE ImColor(0, 0, 255, 255)
#define GREEN ImColor(0, 255, 0, 255)
#define DARKGREEN ImColor(0, 128, 0, 255)
#define BLACK ImColor(0, 0, 0, 255)
#define WHITE ImColor(255, 255, 255, 255)
#define GREY ImColor(170, 170, 170, 255)
#define DARKGREY ImColor(85, 85, 85, 255)
#define YELLOW ImColor(255, 255, 0, 255)
#define PURPLE ImColor(128, 0, 128, 255)

using namespace placeholders;

struct Input {
	int index;
	bool pressed;
	ImColor color;
	string name;
};

class ControllerOverlay : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
public:
	void onLoad();
	void onUnload();

	void writeCfg();

	void onTick(string eventName);

	void Render();
	void RenderImGui();
	string GetMenuName();
	string GetMenuTitle();
	void SetImGuiContext(uintptr_t ctx);
	bool ShouldBlockInput();
	bool IsActiveOverlay();
	void OnOpen();
	void OnClose();

	bool renderImgui = false;

	string configurationFilePath = "./bakkesmod/cfg/controlleroverlay.cfg";

	enum class ControllerType { Xbox, PS4 };
	ControllerType controllerType;

	bool doubleSize = false;
	bool titleBar = true;
	float transparency = 1.0f;

	map<string, Input> inputs;
	ControllerInput controllerInput;
};
