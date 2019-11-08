#pragma once
#ifndef IMGUI_VERSION
#   error "include imgui.h before this header"
#endif
namespace ImGui
{
	float GetItemsLineHeightWithSpacing();
}

inline float ImGui::GetItemsLineHeightWithSpacing()
{
	return ImGui::GetTextLineHeightWithSpacing();
}