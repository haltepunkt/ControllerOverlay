#include "ControllerOverlay.h"

BAKKESMOD_PLUGIN(ControllerOverlay, "Controller Overlay", "1.2", 0)

/*
	https://docs.unrealengine.com/udk/Three/KeyBinds.html

	XboxTypeS_LeftX - Left Analogue Stick X Axis
	XboxTypeS_RightX - Left Analogue Stick Y Axis
	XboxTypeS_LeftY - Right Analogue Stick X Axis
	XboxTypeS_RightY - Right Analogue Stick Y Axis
	XboxTypeS_DPad_Left - DPad Left
	XboxTypeS_DPad_Right - DPad Right
	XboxTypeS_DPad_Up - DPad Up
	XboxTypeS_DPad_Down - DPad Down
	XboxTypeS_A - A Button
	XboxTypeS_B - B Button
	XboxTypeS_Y - Y Button
	XboxTypeS_X - X Button
	XboxTypeS_Start - Start Button
	XboxTypeS_Back - Back Button
	XboxTypeS_LeftShoulder - Left Shoulder
	XboxTypeS_RightShoulder - Right Shoulder
	XboxTypeS_LeftTrigger - Left Trigger
	XboxTypeS_RightTrigger - Right Trigger
	XboxTypeS_LeftThumbStick - Left Analogue Stick Button
	XboxTypeS_RightThumbStick - Right Analogue Stick Button

	Gamepad_LeftStick_Left - Left Analogue Stick Left
	Gamepad_LeftStick_Right - Left Analogue Stick Right
	Gamepad_LeftStick_Up - Left Analogue Stick Up
	Gamepad_LeftStick_Down - Left Analogue Stick Down
	Gamepad_RightStick_Left - Right Analogue Stick Left
	Gamepad_RightStick_Right - Right Analogue Stick Right
	Gamepad_RightStick_Up - Right Analogue Stick Up
	Gamepad_RightStick_Down - Right Analogue Stick Down
*/

void ControllerOverlay::onLoad()
{
	gameWrapper->SetTimeout([this](GameWrapper* gameWrapper) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}, 1);

	cvarManager->registerCvar("controllerType", "xbox").addOnValueChanged([this](string old, CVarWrapper now) {
		if (cvarManager->getCvar("controllerType").getStringValue() == "ps4") {
			controllerType = ControllerOverlay::ControllerType::PS4;
			
			inputs["XboxTypeS_A"] = { 0, false, BLUE, "Cross" };
			inputs["XboxTypeS_B"] = { 0, false, RED, "Circle" };
			inputs["XboxTypeS_X"] = { 0, false, PURPLE, "Square" };
			inputs["XboxTypeS_Y"] = { 0, false, DARKGREEN, "Triangle" };
			inputs["XboxTypeS_LeftShoulder"] = { 0, false, WHITE, "L1" };
			inputs["XboxTypeS_RightShoulder"] = { 0, false, WHITE, "R1" };
			inputs["XboxTypeS_LeftTrigger"] = { 0, false, WHITE, "L2" };
			inputs["XboxTypeS_RightTrigger"] = { 0, false, WHITE, "R2" };
			inputs["XboxTypeS_LeftThumbStick"] = { 0, false, GREY, "L3" };
		}

		else {
			controllerType = ControllerOverlay::ControllerType::Xbox;

			inputs["XboxTypeS_A"] = { 0, false, GREEN, "A" };
			inputs["XboxTypeS_B"] = { 0, false, RED, "B" };
			inputs["XboxTypeS_X"] = { 0, false, BLUE, "X" };
			inputs["XboxTypeS_Y"] = { 0, false, YELLOW, "Y" };
			inputs["XboxTypeS_LeftShoulder"] = { 0, false, WHITE, "LB" };
			inputs["XboxTypeS_RightShoulder"] = { 0, false, WHITE, "RB" };
			inputs["XboxTypeS_LeftTrigger"] = { 0, false, WHITE, "LT" };
			inputs["XboxTypeS_RightTrigger"] = { 0, false, WHITE, "RT" };
			inputs["XboxTypeS_LeftThumbStick"] = { 0, false, GREY, "LS" };
		}

		for (const pair<const string, Input>& input : inputs) {
			cvarManager->registerCvar(input.first, input.first).addOnValueChanged([this](string old, CVarWrapper now) {
				inputs[now.getStringValue()].index = gameWrapper->GetFNameIndexByString(now.getStringValue());
				});

			cvarManager->getCvar(input.first).notify();
		}
		
		ofstream configurationFile;

		configurationFile.open(configurationFilePath);

		configurationFile << "controllerType \"" + cvarManager->getCvar("controllerType").getStringValue() + "\"";

		configurationFile.close();
		
	});

	if (ifstream(configurationFilePath)) {
		cvarManager->loadCfg(configurationFilePath);
	}

	else {
		cvarManager->getCvar("controllerType").notify();
	}

	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick", bind(&ControllerOverlay::onTick, this, placeholders::_1));
}

void ControllerOverlay::onUnload()
{
	if (renderImgui) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

void ControllerOverlay::onTick(string eventName)
{
	if (gameWrapper->IsInGame() || gameWrapper->IsInOnlineGame()) {
		for (const pair<const string, Input>& input : inputs) {
			if (input.second.index > 0) {
				inputs[input.first].pressed = gameWrapper->IsKeyPressed(input.second.index);
			}
		}

		CarWrapper car = gameWrapper->GetLocalCar();

		if (!car.IsNull()) {
			controllerInput = car.GetInput();
		}

		else {
			controllerInput.Steer = 0;
			controllerInput.Pitch = 0;
		}
	}
}

void ControllerOverlay::Render()
{
	if (!renderImgui) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());

		return;
	}

	if (gameWrapper->IsInOnlineGame()) {
		ServerWrapper server = gameWrapper->GetOnlineGame();

		if (!server.IsNull()) {
			if (!server.GetbMatchEnded()) {
				ControllerOverlay::RenderImGui(server);
			}
		}
	}

	else if (gameWrapper->IsInGame()) {
		ServerWrapper server = gameWrapper->GetGameEventAsServer();

		if (!server.IsNull()) {
			if (!server.GetbMatchEnded()) {
				ControllerOverlay::RenderImGui(server);
			}
		}
	}
}

void ControllerOverlay::RenderImGui(ServerWrapper server)
{
	ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(216, 156));

	ImGui::Begin(GetMenuTitle().c_str(), &renderImgui);

	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImVec2 p = ImGui::GetCursorScreenPos();

	p.x += 12;

	float buttonWidth = 48, buttonHeight = 16;

	ImVec2 buttonLBPosition = ImVec2(p.x, p.y);
	ImVec2 buttonRBPosition = ImVec2(p.x + 128, p.y);

	if (inputs["XboxTypeS_LeftTrigger"].pressed) {
		drawList->AddRectFilled(buttonLBPosition, ImVec2(buttonLBPosition.x + buttonWidth, buttonLBPosition.y + buttonHeight), WHITE, 8, ImDrawCornerFlags_TopLeft);
		drawList->AddText(ImVec2(buttonLBPosition.x + 18, buttonLBPosition.y + 1), BLACK, inputs["XboxTypeS_LeftTrigger"].name.c_str());
	}

	else {
		drawList->AddRect(buttonLBPosition, ImVec2(buttonLBPosition.x + buttonWidth, buttonLBPosition.y + buttonHeight), WHITE, 8, ImDrawCornerFlags_TopLeft, 2);
		drawList->AddText(ImVec2(buttonLBPosition.x + 18, buttonLBPosition.y + 1), WHITE, inputs["XboxTypeS_LeftTrigger"].name.c_str());
	}

	if (inputs["XboxTypeS_RightTrigger"].pressed) {
		drawList->AddRectFilled(buttonRBPosition, ImVec2(buttonRBPosition.x + buttonWidth, buttonRBPosition.y + buttonHeight), WHITE, 8, ImDrawCornerFlags_TopRight);
		drawList->AddText(ImVec2(buttonRBPosition.x + 18, buttonRBPosition.y + 1), BLACK, inputs["XboxTypeS_RightTrigger"].name.c_str());
	}

	else {
		drawList->AddRect(buttonRBPosition, ImVec2(buttonRBPosition.x + buttonWidth, buttonRBPosition.y + buttonHeight), WHITE, 8, ImDrawCornerFlags_TopRight, 2);
		drawList->AddText(ImVec2(buttonRBPosition.x + 18, buttonRBPosition.y + 1), WHITE, inputs["XboxTypeS_RightTrigger"].name.c_str());
	}

	p.y += buttonHeight + 4;

	ImVec2 buttonLTPosition = ImVec2(p.x, p.y);
	ImVec2 buttonRTPosition = ImVec2(p.x + 128, p.y);

	if (inputs["XboxTypeS_LeftShoulder"].pressed) {
		drawList->AddRectFilled(buttonLTPosition, ImVec2(buttonLTPosition.x + buttonWidth, buttonLTPosition.y + buttonHeight), WHITE);
		drawList->AddText(ImVec2(buttonLTPosition.x + 18, buttonLTPosition.y + 1), BLACK, inputs["XboxTypeS_LeftShoulder"].name.c_str());
	}

	else {
		drawList->AddRect(buttonLTPosition, ImVec2(buttonLTPosition.x + buttonWidth, buttonLTPosition.y + buttonHeight), WHITE, 0, 0, 2);
		drawList->AddText(ImVec2(buttonLTPosition.x + 18, buttonLTPosition.y + 1), WHITE, inputs["XboxTypeS_LeftShoulder"].name.c_str());
	}

	if (inputs["XboxTypeS_RightShoulder"].pressed) {
		drawList->AddRectFilled(buttonRTPosition, ImVec2(buttonRTPosition.x + buttonWidth, buttonRTPosition.y + buttonHeight), WHITE);
		drawList->AddText(ImVec2(buttonRTPosition.x + 18, buttonRTPosition.y + 1), BLACK, inputs["XboxTypeS_RightShoulder"].name.c_str());
	}

	else {
		drawList->AddRect(buttonRTPosition, ImVec2(buttonRTPosition.x + buttonWidth, buttonRTPosition.y + buttonHeight), WHITE, 0, 0, 2);
		drawList->AddText(ImVec2(buttonRTPosition.x + 18, buttonRTPosition.y + 1), WHITE, inputs["XboxTypeS_RightShoulder"].name.c_str());
	}

	p.y += buttonHeight + 16;

	p.x -= 8;

	float leftStickRadius = 32;
	ImVec2 leftStickCenter = ImVec2(p.x + leftStickRadius, p.y + leftStickRadius);

	drawList->AddCircle(leftStickCenter, 24, WHITE, 32, 2);

	drawList->AddCircleFilled(ImVec2(leftStickCenter.x + (controllerInput.Steer * 8), leftStickCenter.y + (controllerInput.Pitch * 8)), 20, (inputs["XboxTypeS_LeftThumbStick"].pressed ? GREY : WHITE), 32);
	drawList->AddCircleFilled(ImVec2(leftStickCenter.x + (controllerInput.Steer * 8), leftStickCenter.y + (controllerInput.Pitch * 8)), 16, (inputs["XboxTypeS_LeftThumbStick"].pressed ? DARKGREY : GREY), 32);

	float buttonRadius = 12;
	ImVec2 buttonsCenter = ImVec2(leftStickCenter.x + 128, leftStickCenter.y);

	map<string, ImVec2> buttonPositions;
	map<string, ImVec2> buttonTextPositions;

	if (controllerType == ControllerOverlay::ControllerType::Xbox) {
		buttonPositions["XboxTypeS_A"] = ImVec2(buttonsCenter.x, buttonsCenter.y + buttonRadius * 2);
		buttonPositions["XboxTypeS_B"] = ImVec2(buttonsCenter.x + buttonRadius * 2, buttonsCenter.y);
		buttonPositions["XboxTypeS_X"] = ImVec2(buttonsCenter.x - buttonRadius * 2, buttonsCenter.y);
		buttonPositions["XboxTypeS_Y"] = ImVec2(buttonsCenter.x, buttonsCenter.y - buttonRadius * 2);

		buttonTextPositions["XboxTypeS_A"] = ImVec2(3 + buttonPositions["XboxTypeS_A"].x - buttonRadius * 0.5f, buttonPositions["XboxTypeS_A"].y - buttonRadius * 0.5f - 1);
		buttonTextPositions["XboxTypeS_B"] = ImVec2(3 + buttonPositions["XboxTypeS_B"].x - buttonRadius * 0.5f, buttonPositions["XboxTypeS_B"].y - buttonRadius * 0.5f - 1);
		buttonTextPositions["XboxTypeS_X"] = ImVec2(3 + buttonPositions["XboxTypeS_X"].x - buttonRadius * 0.5f, buttonPositions["XboxTypeS_X"].y - buttonRadius * 0.5f - 1);
		buttonTextPositions["XboxTypeS_Y"] = ImVec2(3 + buttonPositions["XboxTypeS_Y"].x - buttonRadius * 0.5f, buttonPositions["XboxTypeS_Y"].y - buttonRadius * 0.5f - 1);

		for (pair<string, ImVec2> buttonPosition : buttonPositions) {
			if (inputs[buttonPosition.first].pressed) {
				drawList->AddCircleFilled(buttonPosition.second, buttonRadius, inputs[buttonPosition.first].color, 32);
				drawList->AddText(buttonTextPositions[buttonPosition.first], BLACK, inputs[buttonPosition.first].name.c_str());
			}

			else {
				drawList->AddCircle(buttonPosition.second, buttonRadius, inputs[buttonPosition.first].color, 32, 2);
				drawList->AddText(buttonTextPositions[buttonPosition.first], inputs[buttonPosition.first].color, inputs[buttonPosition.first].name.c_str());
			}
		}
	}

	else if (controllerType == ControllerOverlay::ControllerType::PS4) {
		buttonPositions["XboxTypeS_A"] = ImVec2(buttonsCenter.x, buttonsCenter.y + buttonRadius * 2);
		buttonPositions["XboxTypeS_B"] = ImVec2(buttonsCenter.x + buttonRadius * 2, buttonsCenter.y);
		buttonPositions["XboxTypeS_X"] = ImVec2(buttonsCenter.x - buttonRadius * 2, buttonsCenter.y);
		buttonPositions["XboxTypeS_Y"] = ImVec2(buttonsCenter.x, buttonsCenter.y - buttonRadius * 2);

		for (pair<string, ImVec2> buttonPosition : buttonPositions) {
			if (inputs[buttonPosition.first].pressed) {
				drawList->AddCircleFilled(buttonPosition.second, buttonRadius, WHITE, 32);
			}

			else {
				drawList->AddCircle(buttonPosition.second, buttonRadius, WHITE, 32, 2);
			}

			if (buttonPosition.first == "XboxTypeS_A") {
				drawList->AddLine(ImVec2(buttonPosition.second.x - 5, buttonPosition.second.y - 5), ImVec2(buttonPosition.second.x + 5, buttonPosition.second.y + 5), inputs[buttonPosition.first].color, 2.f);
				drawList->AddLine(ImVec2(buttonPosition.second.x - 5, buttonPosition.second.y + 5), ImVec2(buttonPosition.second.x + 5, buttonPosition.second.y - 5), inputs[buttonPosition.first].color, 2.f);
			}

			else if (buttonPosition.first == "XboxTypeS_B") {
				drawList->AddCircle(buttonPosition.second, buttonRadius - 6, inputs[buttonPosition.first].color, 16, 2);
			}

			else if (buttonPosition.first == "XboxTypeS_X") {
				drawList->AddQuad(ImVec2(buttonPosition.second.x - 5, buttonPosition.second.y - 5), ImVec2(buttonPosition.second.x + 5, buttonPosition.second.y - 5), ImVec2(buttonPosition.second.x + 5, buttonPosition.second.y + 5), ImVec2(buttonPosition.second.x - 5, buttonPosition.second.y + 5), inputs[buttonPosition.first].color, 2.f);
			}

			else if (buttonPosition.first == "XboxTypeS_Y") {
				drawList->AddTriangle(ImVec2(buttonPosition.second.x, buttonPosition.second.y - 5), ImVec2(buttonPosition.second.x + 5, buttonPosition.second.y + 4), ImVec2(buttonPosition.second.x - 5, buttonPosition.second.y + 4), inputs[buttonPosition.first].color, 2.f);
			}
		}
	}

	ImGui::End();
}

string ControllerOverlay::GetMenuName()
{
	return "controlleroverlay";
}

string ControllerOverlay::GetMenuTitle()
{
	return "Controller Overlay";
}

void ControllerOverlay::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

bool ControllerOverlay::ShouldBlockInput()
{
	return false;
}

bool ControllerOverlay::IsActiveOverlay()
{
	return false;
}

void ControllerOverlay::OnOpen()
{
	renderImgui = true;
}

void ControllerOverlay::OnClose()
{
	renderImgui = false;
}
