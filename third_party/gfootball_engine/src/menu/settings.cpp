// Copyright 2019 Google LLC & Bastiaan Konings
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "settings.hpp"

#include "../main.hpp"
#include "pagefactory.hpp"

#include <iterator>

SettingsPage::SettingsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings", 20, 20, 60, 3, "Settings");
  this->AddView(title);
  title->Show();

  Gui2Button *buttonGameplay = new Gui2Button(windowManager, "button_gameplay", 0, 0, 30, 3, "gameplay");
  Gui2Button *buttonController = new Gui2Button(windowManager, "button_controller", 0, 0, 30, 3, "controller");
  Gui2Button *buttonGraphics = new Gui2Button(windowManager, "button_graphics", 0, 0, 30, 3, "graphics");
  Gui2Button *buttonAudio = new Gui2Button(windowManager, "button_audio", 0, 0, 30, 3, "audio");

  buttonGameplay->sig_OnClick.connect(boost::bind(&SettingsPage::GoGameplay, this));
  buttonController->sig_OnClick.connect(boost::bind(&SettingsPage::GoController, this));
  buttonGraphics->sig_OnClick.connect(boost::bind(&SettingsPage::GoGraphics, this));
  buttonAudio->sig_OnClick.connect(boost::bind(&SettingsPage::GoAudio, this));

  Gui2Grid *grid = new Gui2Grid(windowManager, "settingsgrid", 20, 25, 60, 55);

  grid->AddView(buttonGameplay, 0, 0);
  grid->AddView(buttonController, 1, 0);
  grid->AddView(buttonGraphics, 2, 0);
  grid->AddView(buttonAudio, 3, 0);

  grid->UpdateLayout(0.5);

  this->AddView(grid);
  grid->Show();

  buttonGameplay->SetFocus();

  this->Show();
}

SettingsPage::~SettingsPage() {
}

void SettingsPage::GoGameplay() {
  CreatePage(e_PageID_Gameplay);
}

void SettingsPage::GoController() {
  CreatePage(e_PageID_Controller);
}

void SettingsPage::GoGraphics() {
  CreatePage(e_PageID_Graphics);
}

void SettingsPage::GoAudio() {
  CreatePage(e_PageID_Audio);
}


// GAMEPLAY MENU

GameplayPage::GameplayPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings_gameplay", 20, 5, 60, 3, "Gameplay tweakage");
  this->AddView(title);
  title->Show();

  Gui2Frame *frame = new Gui2Frame(windowManager, "frame_settings_gameplay", 20, 10, 60, 80);

  slider_ShortPass_AutoDirection = new Gui2Slider(windowManager, "slider_shortpass_autodirection", 0, 0, 30, 6, "short pass - direction assist");
  slider_ShortPass_AutoDirection->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_ShortPass_AutoDirection);
  slider_ShortPass_AutoDirection->SetValue(GetConfiguration()->GetReal("gameplay_shortpass_autodirection", _default_ShortPass_AutoDirection));
  slider_ShortPass_AutoPower = new Gui2Slider(windowManager, "slider_shortpass_autopower", 0, 0, 30, 6, "short pass - power assist");
  slider_ShortPass_AutoPower->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_ShortPass_AutoPower);
  slider_ShortPass_AutoPower->SetValue(GetConfiguration()->GetReal("gameplay_shortpass_autopower", _default_ShortPass_AutoPower));

  slider_ThroughPass_AutoDirection = new Gui2Slider(windowManager, "slider_throughpass_autodirection", 0, 0, 30, 6, "through pass - direction assist");
  slider_ThroughPass_AutoDirection->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_ThroughPass_AutoDirection);
  slider_ThroughPass_AutoDirection->SetValue(GetConfiguration()->GetReal("gameplay_throughpass_autodirection", _default_ThroughPass_AutoDirection));
  slider_ThroughPass_AutoPower = new Gui2Slider(windowManager, "slider_throughpass_autopower", 0, 0, 30, 6, "through pass - power assist");
  slider_ThroughPass_AutoPower->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_ThroughPass_AutoPower);
  slider_ThroughPass_AutoPower->SetValue(GetConfiguration()->GetReal("gameplay_throughpass_autopower", _default_ThroughPass_AutoPower));

  slider_HighPass_AutoDirection = new Gui2Slider(windowManager, "slider_highpass_autodirection", 0, 0, 30, 6, "high pass - direction assist");
  slider_HighPass_AutoDirection->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_HighPass_AutoDirection);
  slider_HighPass_AutoDirection->SetValue(GetConfiguration()->GetReal("gameplay_highpass_autodirection", _default_HighPass_AutoDirection));
  slider_HighPass_AutoPower = new Gui2Slider(windowManager, "slider_highpass_autopower", 0, 0, 30, 6, "high pass - power assist");
  slider_HighPass_AutoPower->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_HighPass_AutoPower);
  slider_HighPass_AutoPower->SetValue(GetConfiguration()->GetReal("gameplay_highpass_autopower", _default_HighPass_AutoPower));

  slider_Shot_AutoDirection = new Gui2Slider(windowManager, "slider_shot_autodirection", 0, 0, 30, 6, "shot - direction assist");
  slider_Shot_AutoDirection->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_Shot_AutoDirection);
  slider_Shot_AutoDirection->SetValue(GetConfiguration()->GetReal("gameplay_shot_autodirection", _default_Shot_AutoDirection));

  slider_Agility = new Gui2Slider(windowManager, "slider_agility", 0, 0, 30, 6, "human agility factor");
  slider_Agility->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_AgilityFactor);
  slider_Agility->SetValue(GetConfiguration()->GetReal("gameplay_agilityfactor", _default_AgilityFactor));
  slider_Acceleration = new Gui2Slider(windowManager, "slider_acceleration", 0, 0, 30, 6, "human acceleration factor");
  slider_Acceleration->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_AccelerationFactor);
  slider_Acceleration->SetValue(GetConfiguration()->GetReal("gameplay_accelerationfactor", _default_AccelerationFactor));
  slider_Quantization = new Gui2Slider(windowManager, "slider_quantization", 0, 0, 30, 6, "8-way-quantization (more D-pad'y)");
  slider_Quantization->AddHelperValue(Vector3(80, 80, 250), "factory setting", _default_QuantizedDirectionBias);
  slider_Quantization->SetValue(GetConfiguration()->GetReal("gameplay_quantizeddirectionbias", _default_QuantizedDirectionBias));

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_settings_gameplay", 0, 0, 60, 80);

  grid->AddView(slider_ShortPass_AutoDirection);
  grid->AddView(slider_ShortPass_AutoPower);

  grid->AddView(slider_ThroughPass_AutoDirection);
  grid->AddView(slider_ThroughPass_AutoPower);

  grid->AddView(slider_HighPass_AutoDirection);
  grid->AddView(slider_HighPass_AutoPower);

  grid->AddView(slider_Shot_AutoDirection);

  grid->AddView(slider_Agility);
  grid->AddView(slider_Acceleration);
  grid->AddView(slider_Quantization);

  grid->UpdateLayout(0.5);
  grid->Show();

  frame->AddView(grid);
  frame->Show();

  this->AddView(frame);

  grid->SetFocus();

  this->Show();
}

GameplayPage::~GameplayPage() {
}

void GameplayPage::Exit() {
  GetConfiguration()->Set("gameplay_shortpass_autodirection", slider_ShortPass_AutoDirection->GetValue());
  GetConfiguration()->Set("gameplay_shortpass_autopower", slider_ShortPass_AutoPower->GetValue());

  GetConfiguration()->Set("gameplay_throughpass_autodirection", slider_ThroughPass_AutoDirection->GetValue());
  GetConfiguration()->Set("gameplay_throughpass_autopower", slider_ThroughPass_AutoPower->GetValue());

  GetConfiguration()->Set("gameplay_highpass_autodirection", slider_HighPass_AutoDirection->GetValue());
  GetConfiguration()->Set("gameplay_highpass_autopower", slider_HighPass_AutoPower->GetValue());

  GetConfiguration()->Set("gameplay_shot_autodirection", slider_Shot_AutoDirection->GetValue());

  GetConfiguration()->Set("gameplay_agilityfactor", slider_Agility->GetValue());
  GetConfiguration()->Set("gameplay_accelerationfactor", slider_Acceleration->GetValue());
  GetConfiguration()->Set("gameplay_quantizeddirectionbias", slider_Quantization->GetValue());

  //printf("%f - %f - %f - %f - %f - %f\n", slider_ShortPass_AutoDirection->GetValue(), slider_ShortPass_AutoPower->GetValue(), slider_ThroughPass_AutoDirection->GetValue(), slider_ThroughPass_AutoPower->GetValue(), slider_HighPass_AutoDirection->GetValue(), slider_HighPass_AutoPower->GetValue());
  GetConfiguration()->SaveFile(GetConfigFilename());

  Gui2Page::Exit();
}


// CONTROLLER MENU

ControllerPage::ControllerPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings_controller", 20, 20, 60, 3, "Controller setup");
  this->AddView(title);
  title->Show();

  Gui2Button *buttonKeyboard = new Gui2Button(windowManager, "button_keyboard", 0, 0, 30, 3, "keyboard");
  Gui2Button *buttonGamepads = new Gui2Button(windowManager, "button_gamepads", 0, 0, 30, 3, "gamepad(s)");

  buttonKeyboard->sig_OnClick.connect(boost::bind(&ControllerPage::GoKeyboard, this));
  buttonGamepads->sig_OnClick.connect(boost::bind(&ControllerPage::GoGamepads, this));

  Gui2Grid *grid = new Gui2Grid(windowManager, "controllersettingsgrid", 20, 25, 60, 55);

  grid->AddView(buttonKeyboard, 0, 0);
  grid->AddView(buttonGamepads, 1, 0);

  grid->UpdateLayout(0.5);

  this->AddView(grid);
  grid->Show();

  buttonKeyboard->SetFocus();

  this->Show();
}

ControllerPage::~ControllerPage() {
}

void ControllerPage::GoKeyboard() {
  CreatePage(e_PageID_Keyboard);
}

void ControllerPage::GoGamepads() {
  CreatePage(e_PageID_Gamepads);
}


// CONTROLLERS: KEYBOARD MENU

KeyboardPage::KeyboardPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings_controller_keyboard", 20, 10, 60, 3, "Keyboard setup");
  this->AddView(title);
  title->Show();

  for (int i = 0; i < 18; i++) {
    keyButtons[i] = 0;
    keyIDs[i] = (SDL_Keycode)GetConfiguration()->GetInt(("input_keyboard_" + int_to_str(i)).c_str(), (SDL_Keycode)defaultKeyIDs[i]);//48);
  }

  Gui2Button *buttonDefaults = new Gui2Button(windowManager, "button_keyboard_defaults", 0, 0, 30, 3, "reset to defaults");
  buttonDefaults->sig_OnClick.connect(boost::bind(&KeyboardPage::SetDefaults, this));


  Gui2Caption *captionNorth = new Gui2Caption(windowManager, "caption_keyboard_north", 0, 0, 20, 3, "walk north");
  keyButtons[0] = new Gui2Button(windowManager, "button_keyboard_up", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[0]));

  Gui2Caption *captionEast = new Gui2Caption(windowManager, "caption_keyboard_east", 0, 0, 20, 3, "walk east");
  keyButtons[1] = new Gui2Button(windowManager, "button_keyboard_east", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[1]));

  Gui2Caption *captionSouth = new Gui2Caption(windowManager, "caption_keyboard_south", 0, 0, 20, 3, "walk south");
  keyButtons[2] = new Gui2Button(windowManager, "button_keyboard_south", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[2]));

  Gui2Caption *captionWest = new Gui2Caption(windowManager, "caption_keyboard_west", 0, 0, 20, 3, "walk west");
  keyButtons[3] = new Gui2Button(windowManager, "button_keyboard_west", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[3]));


  Gui2Caption *captionThrough = new Gui2Caption(windowManager, "caption_keyboard_through", 0, 0, 20, 3, "(on ball) through pass");
  keyButtons[4] = new Gui2Button(windowManager, "button_keyboard_through", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[4]));

  Gui2Caption *captionHigh = new Gui2Caption(windowManager, "caption_keyboard_high", 0, 0, 20, 3, "(on ball) high pass");
  keyButtons[5] = new Gui2Button(windowManager, "button_keyboard_high", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[5]));

  Gui2Caption *captionPass = new Gui2Caption(windowManager, "caption_keyboard_pass", 0, 0, 20, 3, "(on ball) normal pass");
  keyButtons[6] = new Gui2Button(windowManager, "button_keyboard_pass", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[6]));

  Gui2Caption *captionShoot = new Gui2Caption(windowManager, "caption_keyboard_shoot", 0, 0, 20, 3, "(on ball) shoot");
  keyButtons[7] = new Gui2Button(windowManager, "button_keyboard_shoot", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[7]));

  Gui2Caption *captionKeeper = new Gui2Caption(windowManager, "caption_keyboard_keeper", 0, 0, 20, 3, "(off ball) keeper to ball");
  keyButtons[8] = new Gui2Button(windowManager, "button_keyboard_keeper", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[8]));

  Gui2Caption *captionSliding = new Gui2Caption(windowManager, "caption_keyboard_sliding", 0, 0, 20, 3, "(off ball) sliding");
  keyButtons[9] = new Gui2Button(windowManager, "button_keyboard_sliding", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[9]));

  Gui2Caption *captionPressure = new Gui2Caption(windowManager, "caption_keyboard_pressure", 0, 0, 20, 3, "(off ball) pressure");
  keyButtons[10] = new Gui2Button(windowManager, "button_keyboard_pressure", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[10]));

  Gui2Caption *captionTeamPressure = new Gui2Caption(windowManager, "caption_keyboard_teampressure", 0, 0, 20, 3, "(off ball) team pressure");
  keyButtons[11] = new Gui2Button(windowManager, "button_keyboard_teampressure", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[11]));

  Gui2Caption *captionSwitch = new Gui2Caption(windowManager, "caption_keyboard_switch", 0, 0, 20, 3, "switch player");
  keyButtons[12] = new Gui2Button(windowManager, "button_keyboard_switch", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[12]));

  Gui2Caption *captionSpecial = new Gui2Caption(windowManager, "caption_keyboard_special", 0, 0, 20, 3, "special");
  keyButtons[13] = new Gui2Button(windowManager, "button_keyboard_special", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[13]));

  Gui2Caption *captionSprint = new Gui2Caption(windowManager, "caption_keyboard_sprint", 0, 0, 20, 3, "sprint");
  keyButtons[14] = new Gui2Button(windowManager, "button_keyboard_sprint", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[14]));

  Gui2Caption *captionSlow = new Gui2Caption(windowManager, "caption_keyboard_dribble", 0, 0, 20, 3, "slow dribble");
  keyButtons[15] = new Gui2Button(windowManager, "button_keyboard_dribble", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[15]));

  Gui2Caption *captionSelect = new Gui2Caption(windowManager, "caption_keyboard_select", 0, 0, 20, 3, "select");
  keyButtons[16] = new Gui2Button(windowManager, "button_keyboard_select", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[16]));

  Gui2Caption *captionStart = new Gui2Caption(windowManager, "caption_keyboard_start", 0, 0, 20, 3, "start");
  keyButtons[17] = new Gui2Button(windowManager, "button_keyboard_start", 0, 0, 16, 3, SDL_GetKeyName(keyIDs[17]));

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_keyboard_settings_keys", 0, 0, 60, 50);
  grid->SetWrapping(false);

  grid->AddView(captionNorth, 0, 0);
  grid->AddView(keyButtons[0], 0, 1);
  grid->AddView(captionEast, 1, 0);
  grid->AddView(keyButtons[1], 1, 1);
  grid->AddView(captionSouth, 2, 0);
  grid->AddView(keyButtons[2], 2, 1);
  grid->AddView(captionWest, 3, 0);
  grid->AddView(keyButtons[3], 3, 1);

  grid->AddView(captionThrough, 4, 0);
  grid->AddView(keyButtons[4], 4, 1);
  grid->AddView(captionHigh, 5, 0);
  grid->AddView(keyButtons[5], 5, 1);
  grid->AddView(captionPass, 6, 0);
  grid->AddView(keyButtons[6], 6, 1);
  grid->AddView(captionShoot, 7, 0);
  grid->AddView(keyButtons[7], 7, 1);

  grid->AddView(captionKeeper, 8, 0);
  grid->AddView(keyButtons[8], 8, 1);
  grid->AddView(captionSliding, 9, 0);
  grid->AddView(keyButtons[9], 9, 1);
  grid->AddView(captionPressure, 10, 0);
  grid->AddView(keyButtons[10], 10, 1);
  grid->AddView(captionTeamPressure, 11, 0);
  grid->AddView(keyButtons[11], 11, 1);

  grid->AddView(captionSwitch, 12, 0);
  grid->AddView(keyButtons[12], 12, 1);
  grid->AddView(captionSpecial, 13, 0);
  grid->AddView(keyButtons[13], 13, 1);
  grid->AddView(captionSprint, 14, 0);
  grid->AddView(keyButtons[14], 14, 1);
  grid->AddView(captionSlow, 15, 0);
  grid->AddView(keyButtons[15], 15, 1);

  grid->AddView(captionSelect, 16, 0);
  grid->AddView(keyButtons[16], 16, 1);
  grid->AddView(captionStart, 17, 0);
  grid->AddView(keyButtons[17], 17, 1);


  Gui2Grid *wrapperGrid = new Gui2Grid(windowManager, "grid_keyboard_settings_wrapper", 20, 15, 60, 55);
  wrapperGrid->SetWrapping(false);

  wrapperGrid->AddView(buttonDefaults, 0, 0);
  wrapperGrid->AddView(grid, 1, 0);

  grid->UpdateLayout(0.5);
  wrapperGrid->UpdateLayout(0.0);

  this->AddView(wrapperGrid);
  wrapperGrid->Show();

  keyButtons[0]->SetFocus();

  this->Show();

  sig_OnClose.connect(boost::bind(&KeyboardPage::OnClose, this));
}

KeyboardPage::~KeyboardPage() {
}

void KeyboardPage::OnClose() {
  const std::vector<IHIDevice*> &controllers = GetControllers();
  IHIDevice *controller = controllers.at(0);
  for (int i = 0; i < e_ButtonFunction_Size; i++) {
    static_cast<HIDKeyboard*>(controller)->SetFunctionMapping(i, keyIDs[i]);
  }

  controller->SaveConfig();

}

void KeyboardPage::SetDefaults() {
  for (int i = 0; i < 18; i++) {
    keyButtons[i]->GetCaptionWidget()->SetCaption(SDL_GetKeyName((SDL_Keycode)defaultKeyIDs[i]));
    keyIDs[i] = (SDL_Keycode)defaultKeyIDs[i];
  }
}

void KeyboardPage::SetKeyDone(int buttonID) {
  SDL_Keycode value = (SDL_Keycode)captureKey->GetKeyID();
  //printf("clickah! %i\n", value);
  keyButtons[buttonID]->GetCaptionWidget()->SetCaption(SDL_GetKeyName(value));
  keyIDs[buttonID] = value;
  keyButtons[buttonID]->SetFocus();

  bg->Exit();
  delete bg;
  pressKeyCaption->Exit();
  delete pressKeyCaption;
  captureKey->Exit();
  delete captureKey;
}


// CONTROLLERS: GAMEPADS MENU

GamepadsPage::GamepadsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {
  GetMenuTask()->SetActiveJoystickID(0);

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings_controller_gamepads", 20, 20, 60, 3, "Gamepads setup");
  this->AddView(title);
  title->Show();
  title->SetFocus();

  const std::vector<IHIDevice*> &controllers = GetControllers();

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_settings_controller_gamepads", 20, 25, 60, 55);

  int x = 0;
  for (unsigned int i = 0; i < controllers.size(); i++) {
    if (controllers.at(i)->GetDeviceType() == e_HIDeviceType_Gamepad) {
      Gui2Button *buttonGamepad = new Gui2Button(windowManager, "button_gamepadsmenu_gamepad" + int_to_str(i), 0, 0, 60, 3, controllers.at(i)->GetIdentifier());
      buttonGamepad->sig_OnClick.connect(boost::bind(&GamepadsPage::GoGamepadSetup, this, i));
      grid->AddView(buttonGamepad, x, 0);
      if (x == 0) buttonGamepad->SetFocus();
      x++;
    }
  }

  grid->UpdateLayout(0.5);
  this->AddView(grid);
  grid->Show();

  this->Show();
}

GamepadsPage::~GamepadsPage() {
}

void GamepadsPage::GoGamepadSetup(int controllerID) {

  const std::vector<IHIDevice*> &controllers = GetControllers();
  HIDGamepad *controller = static_cast<HIDGamepad*>(controllers.at(controllerID));
  GetMenuTask()->SetActiveJoystickID(controller->GetGamepadID());

  Properties properties;
  properties.Set("controllerID", controllerID);
  CreatePage(e_PageID_GamepadSetup, properties);
}


// (SPECIFIC) GAMEPAD SETUP

GamepadSetupPage::GamepadSetupPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {
  int controllerID = pageData.properties->GetInt("controllerID");

  const std::vector<IHIDevice*> &controllers = GetControllers();
  IHIDevice *controller = controllers.at(controllerID);

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings_controller_gamepadsetup", 20, 20, 60, 3, "'" + controllers.at(controllerID)->GetIdentifier() + "' setup");
  this->AddView(title);
  title->Show();

  Gui2Button *buttonCalibration = new Gui2Button(windowManager, "button_gamepadsetupmenu_calibration", 0, 0, 30, 3, "axes calibration");
  buttonCalibration->sig_OnClick.connect(boost::bind(&GamepadSetupPage::GoGamepadCalibrationPage, this, controllerID));
  Gui2Button *buttonMapping = new Gui2Button(windowManager, "button_gamepadsetupmenu_mapping", 0, 0, 30, 3, "button mapping");
  buttonMapping->sig_OnClick.connect(boost::bind(&GamepadSetupPage::GoGamepadMappingPage, this, controllerID));
  Gui2Button *buttonFunction = new Gui2Button(windowManager, "button_gamepadsetupmenu_function", 0, 0, 30, 3, "function setup");
  buttonFunction->sig_OnClick.connect(boost::bind(&GamepadSetupPage::GoGamepadFunctionPage, this, controllerID));

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_settings_controller_gamepadsetup", 20, 25, 60, 55);

  grid->AddView(buttonCalibration, 0, 0);
  grid->AddView(buttonMapping, 1, 0);
  grid->AddView(buttonFunction, 2, 0);

  grid->UpdateLayout(0.5);
  this->AddView(grid);
  grid->Show();

  buttonCalibration->SetFocus();

  this->Show();
}

GamepadSetupPage::~GamepadSetupPage() {
}

void GamepadSetupPage::GoGamepadCalibrationPage(int controllerID) {
  Properties properties;
  properties.Set("controllerID", controllerID);
  CreatePage(e_PageID_GamepadCalibration, properties);
}

void GamepadSetupPage::GoGamepadMappingPage(int controllerID) {
  Properties properties;
  properties.Set("controllerID", controllerID);
  CreatePage(e_PageID_GamepadMapping, properties);
}

void GamepadSetupPage::GoGamepadFunctionPage(int controllerID) {
  Properties properties;
  properties.Set("controllerID", controllerID);
  CreatePage(e_PageID_GamepadFunction, properties);
}


// GAMEPAD CALIBRATION

GamepadCalibrationPage::GamepadCalibrationPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {
  controllerID = pageData.properties->GetInt("controllerID");

  // inverted default values
  for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
    min[i] = 32768;
    max[i] = -32767;
  }

  bg = new Gui2Image(windowManager, "image_settings_calibration_bg", 0, 0, 100, 13);
  bg->LoadImage("media/menu/backgrounds/black.png");

  captionExplanation[0] = new Gui2Caption(windowManager, "caption_settings_calibration_info1", 0, 0, 100, 3, "1) rotate all axes and analog buttons into their extreme positions");
  captionExplanation[1] = new Gui2Caption(windowManager, "caption_settings_calibration_info2", 0, 0, 100, 3, "2) release all axes and analog buttons in their untouched positions");
  captionExplanation[2] = new Gui2Caption(windowManager, "caption_settings_calibration_info3", 0, 0, 100, 3, "3) press return when done or escape to cancel");
  captionExplanation[0]->SetPosition(50 - captionExplanation[0]->GetTextWidthPercent() * 0.5, 44);
  captionExplanation[1]->SetPosition(50 - captionExplanation[1]->GetTextWidthPercent() * 0.5, 48);
  captionExplanation[2]->SetPosition(50 - captionExplanation[2]->GetTextWidthPercent() * 0.5, 52);

  bg->SetPosition(50 - captionExplanation[0]->GetTextWidthPercent() * 0.5 - 1, 43);
  bg->SetSize(captionExplanation[0]->GetTextWidthPercent() + 2, 13);

  this->AddView(bg);
  this->AddView(captionExplanation[0]);
  this->AddView(captionExplanation[1]);
  this->AddView(captionExplanation[2]);
  bg->Show();
  captionExplanation[0]->Show();
  captionExplanation[1]->Show();
  captionExplanation[2]->Show();

  this->SetFocus();
  this->Show();
}

GamepadCalibrationPage::~GamepadCalibrationPage() {
}

void GamepadCalibrationPage::Process() {
  const std::vector<IHIDevice*> &controllers = GetControllers();
  HIDGamepad *controller = static_cast<HIDGamepad*>(controllers.at(controllerID));

  for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
    float value = UserEventManager::GetInstance().GetJoystickAxisRaw(controller->GetGamepadID(), i);
    if (value < min[i]) min[i] = value;
    if (value > max[i]) max[i] = value;
  }
}

void GamepadCalibrationPage::ProcessKeyboardEvent(KeyboardEvent *event) {
  if (event->GetKeyOnce(SDLK_RETURN)) {
    SaveCalibration();
    GoBack();
    return;
  }
  if (event->GetKeyOnce(SDLK_ESCAPE)) {
    GoBack();
    return;
  }
}

void GamepadCalibrationPage::SaveCalibration() {
  const std::vector<IHIDevice*> &controllers = GetControllers();
  HIDGamepad *controller = static_cast<HIDGamepad*>(controllers.at(controllerID));

  for (int i = 0; i < _JOYSTICK_MAXAXES; i++) {
    if (min[i] > max[i]) min[i] = max[i]; // happens if people forget to excite some joystick.. oh my
    float rest = UserEventManager::GetInstance().GetJoystickAxisRaw(controller->GetGamepadID(), i);
    UserEventManager::GetInstance().SetJoystickAxisCalibration(controller->GetGamepadID(), i, min[i], max[i], rest);
    GetConfiguration()->Set(("input_gamepad_" + controller->GetIdentifier() + "_calibration_" + int_to_str(i) + "_min").c_str(), min[i]);
    GetConfiguration()->Set(("input_gamepad_" + controller->GetIdentifier() + "_calibration_" + int_to_str(i) + "_max").c_str(), max[i]);
    GetConfiguration()->Set(("input_gamepad_" + controller->GetIdentifier() + "_calibration_" + int_to_str(i) + "_rest").c_str(), rest);
  }

  GetConfiguration()->SaveFile(GetConfigFilename());
}


// GAMEPAD BUTTON MAPPING

GamepadMappingPage::GamepadMappingPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {
  controllerID = pageData.properties->GetInt("controllerID");

  const std::vector<IHIDevice*> &controllers = GetControllers();
  IHIDevice *controller = controllers.at(controllerID);

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings_controller_gamepadmapping", 10, 20, 80, 3, "'" + controllers.at(controllerID)->GetIdentifier() + "' mapping");
  this->AddView(title);
  title->Show();

  Gui2Image *controller_left = new Gui2Image(windowManager, "image_controller_left", 0, 0, 20, 30);
  controller_left->LoadImage("media/menu/controller/controller_left.png");

  Gui2Image *controller_right = new Gui2Image(windowManager, "image_controller_right", 0, 0, 20, 30);
  controller_right->LoadImage("media/menu/controller/controller_right.png");

  std::string gpbuttonIDs_string[e_ControllerButton_Size];
  for (int i = 0; i < e_ControllerButton_Size; i++) {
    gpbuttonButtons[i] = 0;
    gpbuttonIDs[i] = static_cast<HIDGamepad*>(controller)->GetControllerMapping((e_ControllerButton)i);
    if (gpbuttonIDs[i] >= 0) { // button
      gpbuttonIDs_string[i] = int_to_str(gpbuttonIDs[i]);
    } else { // axis
      // decode axis info
      signed int value = -gpbuttonIDs[i] - 1;
      signed int sign = ((value % 2) * 2) - 1;
      value /= 2;
      std::string signStr;
      (sign == -1) ? signStr = "-" : signStr = "+";
      gpbuttonIDs_string[i] = "A" + int_to_str(value) + signStr;
    }
  }

  Gui2Caption *captionUp = new Gui2Caption(windowManager, "caption_gamepad_up", 0, 0, 10, 3, "up");
  gpbuttonButtons[0] = new Gui2Button(windowManager, "button_gamepad_up", 0, 0, 8, 3, gpbuttonIDs_string[0]);
  gpbuttonButtons[0]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 0, "up"));
  gpbuttonButtons[0]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left_up.png"));
  gpbuttonButtons[0]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left.png"));

  Gui2Caption *captionRight = new Gui2Caption(windowManager, "caption_gamepad_right", 0, 0, 10, 3, "right");
  gpbuttonButtons[1] = new Gui2Button(windowManager, "button_gamepad_right", 0, 0, 8, 3, gpbuttonIDs_string[1]);
  gpbuttonButtons[1]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 1, "right"));
  gpbuttonButtons[1]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left_right.png"));
  gpbuttonButtons[1]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left.png"));

  Gui2Caption *captionDown = new Gui2Caption(windowManager, "caption_gamepad_down", 0, 0, 10, 3, "down");
  gpbuttonButtons[2] = new Gui2Button(windowManager, "button_gamepad_down", 0, 0, 8, 3, gpbuttonIDs_string[2]);
  gpbuttonButtons[2]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 2, "down"));
  gpbuttonButtons[2]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left_down.png"));
  gpbuttonButtons[2]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left.png"));

  Gui2Caption *captionLeft = new Gui2Caption(windowManager, "caption_gamepad_left", 0, 0, 10, 3, "left");
  gpbuttonButtons[3] = new Gui2Button(windowManager, "button_gamepad_left", 0, 0, 8, 3, gpbuttonIDs_string[3]);
  gpbuttonButtons[3]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 3, "left"));
  gpbuttonButtons[3]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left_left.png"));
  gpbuttonButtons[3]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left.png"));


  Gui2Caption *captionY = new Gui2Caption(windowManager, "caption_gamepad_Y", 0, 0, 10, 3, "Y");
  gpbuttonButtons[4] = new Gui2Button(windowManager, "button_gamepad_Y", 0, 0, 8, 3, gpbuttonIDs_string[4]);
  gpbuttonButtons[4]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 4, "Y"));
  gpbuttonButtons[4]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_Y.png"));
  gpbuttonButtons[4]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));

  Gui2Caption *captionB = new Gui2Caption(windowManager, "caption_gamepad_B", 0, 0, 10, 3, "B");
  gpbuttonButtons[5] = new Gui2Button(windowManager, "button_gamepad_B", 0, 0, 8, 3, gpbuttonIDs_string[5]);
  gpbuttonButtons[5]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 5, "B"));
  gpbuttonButtons[5]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_B.png"));
  gpbuttonButtons[5]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));

  Gui2Caption *captionA = new Gui2Caption(windowManager, "caption_gamepad_A", 0, 0, 10, 3, "A");
  gpbuttonButtons[6] = new Gui2Button(windowManager, "button_gamepad_A", 0, 0, 8, 3, gpbuttonIDs_string[6]);
  gpbuttonButtons[6]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 6, "A"));
  gpbuttonButtons[6]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_A.png"));
  gpbuttonButtons[6]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));

  Gui2Caption *captionX = new Gui2Caption(windowManager, "caption_gamepad_X", 0, 0, 10, 3, "X");
  gpbuttonButtons[7] = new Gui2Button(windowManager, "button_gamepad_X", 0, 0, 8, 3, gpbuttonIDs_string[7]);
  gpbuttonButtons[7]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 7, "X"));
  gpbuttonButtons[7]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_X.png"));
  gpbuttonButtons[7]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));


  Gui2Caption *captionL1 = new Gui2Caption(windowManager, "caption_gamepad_L1", 0, 0, 10, 3, "L1");
  gpbuttonButtons[8] = new Gui2Button(windowManager, "button_gamepad_L1", 0, 0, 8, 3, gpbuttonIDs_string[8]);
  gpbuttonButtons[8]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 8, "L1"));
  gpbuttonButtons[8]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left_L1.png"));
  gpbuttonButtons[8]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left.png"));

  Gui2Caption *captionL2 = new Gui2Caption(windowManager, "caption_gamepad_L2", 0, 0, 10, 3, "L2");
  gpbuttonButtons[9] = new Gui2Button(windowManager, "button_gamepad_L2", 0, 0, 8, 3, gpbuttonIDs_string[9]);
  gpbuttonButtons[9]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 9, "L2"));
  gpbuttonButtons[9]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left_L2.png"));
  gpbuttonButtons[9]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left.png"));

  Gui2Caption *captionR1 = new Gui2Caption(windowManager, "caption_gamepad_R1", 0, 0, 10, 3, "R1");
  gpbuttonButtons[10] = new Gui2Button(windowManager, "button_gamepad_R1", 0, 0, 8, 3, gpbuttonIDs_string[10]);
  gpbuttonButtons[10]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 10, "R1"));
  gpbuttonButtons[10]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_R1.png"));
  gpbuttonButtons[10]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));

  Gui2Caption *captionR2 = new Gui2Caption(windowManager, "caption_gamepad_R2", 0, 0, 10, 3, "R2");
  gpbuttonButtons[11] = new Gui2Button(windowManager, "button_gamepad_R2", 0, 0, 8, 3, gpbuttonIDs_string[11]);
  gpbuttonButtons[11]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 11, "R2"));
  gpbuttonButtons[11]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_R2.png"));
  gpbuttonButtons[11]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));


  Gui2Caption *captionSelect = new Gui2Caption(windowManager, "caption_gamepad_select", 0, 0, 10, 3, "select");
  gpbuttonButtons[12] = new Gui2Button(windowManager, "button_gamepad_select", 0, 0, 8, 3, gpbuttonIDs_string[12]);
  gpbuttonButtons[12]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 12, "select"));
  gpbuttonButtons[12]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left_select.png"));
  gpbuttonButtons[12]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_left, "media/menu/controller/controller_left.png"));

  Gui2Caption *captionStart = new Gui2Caption(windowManager, "caption_gamepad_start", 0, 0, 10, 3, "start");
  gpbuttonButtons[13] = new Gui2Button(windowManager, "button_gamepad_start", 0, 0, 8, 3, gpbuttonIDs_string[13]);
  gpbuttonButtons[13]->sig_OnClick.connect(boost::bind(&GamepadMappingPage::SetGpbutton, this, 13, "start"));
  gpbuttonButtons[13]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_start.png"));
  gpbuttonButtons[13]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));


  Gui2Grid *gridLeft = new Gui2Grid(windowManager, "grid_gamepad_mapping_left", 0, 0, 20, 50);
  gridLeft->SetWrapping(true, false);
  Gui2Grid *gridRight = new Gui2Grid(windowManager, "grid_gamepad_mapping_right", 0, 0, 20, 50);
  gridRight->SetWrapping(true, false);

  gridLeft->AddView(captionL1, 0, 0);
  gridLeft->AddView(gpbuttonButtons[8], 0, 1);
  gridLeft->AddView(captionL2, 1, 0);
  gridLeft->AddView(gpbuttonButtons[9], 1, 1);
  gridLeft->AddView(captionUp, 2, 0);
  gridLeft->AddView(gpbuttonButtons[0], 2, 1);
  gridLeft->AddView(captionRight, 3, 0);
  gridLeft->AddView(gpbuttonButtons[1], 3, 1);
  gridLeft->AddView(captionDown, 4, 0);
  gridLeft->AddView(gpbuttonButtons[2], 4, 1);
  gridLeft->AddView(captionLeft, 5, 0);
  gridLeft->AddView(gpbuttonButtons[3], 5, 1);
  gridLeft->AddView(captionSelect, 6, 0);
  gridLeft->AddView(gpbuttonButtons[12], 6, 1);

  gridRight->AddView(captionR1, 0, 0);
  gridRight->AddView(gpbuttonButtons[10], 0, 1);
  gridRight->AddView(captionR2, 1, 0);
  gridRight->AddView(gpbuttonButtons[11], 1, 1);
  gridRight->AddView(captionY, 2, 0);
  gridRight->AddView(gpbuttonButtons[4], 2, 1);
  gridRight->AddView(captionB, 3, 0);
  gridRight->AddView(gpbuttonButtons[5], 3, 1);
  gridRight->AddView(captionA, 4, 0);
  gridRight->AddView(gpbuttonButtons[6], 4, 1);
  gridRight->AddView(captionX, 5, 0);
  gridRight->AddView(gpbuttonButtons[7], 5, 1);
  gridRight->AddView(captionStart, 6, 0);
  gridRight->AddView(gpbuttonButtons[13], 6, 1);

  Gui2Grid *wrapperGrid = new Gui2Grid(windowManager, "grid_gamepad_mappings_wrapper", 10, 25, 80, 50);
  wrapperGrid->SetWrapping(false, true);

  wrapperGrid->AddView(gridLeft, 0, 0);
  wrapperGrid->AddView(controller_left, 0, 1);
  wrapperGrid->AddView(controller_right, 0, 2);
  wrapperGrid->AddView(gridRight, 0, 3);

  gridLeft->UpdateLayout(0.5);
  gridRight->UpdateLayout(0.5);

  wrapperGrid->UpdateLayout(0.0);

  this->AddView(wrapperGrid);
  wrapperGrid->Show();

  gpbuttonButtons[8]->SetFocus();

  this->Show();

  sig_OnClose.connect(boost::bind(&GamepadMappingPage::OnClose, this));
}

GamepadMappingPage::~GamepadMappingPage() {
}

void GamepadMappingPage::OnClose() {

  const std::vector<IHIDevice*> &controllers = GetControllers();
  IHIDevice *controller = controllers.at(controllerID);

  for (int i = 0; i < e_ControllerButton_Size; i++) {
    static_cast<HIDGamepad*>(controller)->SetControllerMapping((e_ControllerButton)i, gpbuttonIDs[i]);
  }

  if (controllerID == 1) GetMenuTask()->SetEventJoyButtons(static_cast<HIDGamepad*>(controller)->GetControllerMapping(e_ControllerButton_A), static_cast<HIDGamepad*>(controller)->GetControllerMapping(e_ControllerButton_B));

  controller->SaveConfig();
}

void GamepadMappingPage::SetGpbutton(int buttonID, const std::string &name) {
  controllerID = pageData.properties->GetInt("controllerID");

  bg = new Gui2Image(windowManager, "image_capturebutton_bg", 0, 47, 100, 5);
  bg->LoadImage("media/menu/backgrounds/black.png");

  pressGpbuttonCaption = new Gui2Caption(windowManager, "caption_settings_gamepad_pressbutton", 0, 48, 100, 3, "press button/stick for '" + name + "'");
  pressGpbuttonCaption->SetPosition(50 - pressGpbuttonCaption->GetTextWidthPercent() * 0.5, 48);

  bg->SetPosition(50 - pressGpbuttonCaption->GetTextWidthPercent() * 0.5 - 1, 47);
  bg->SetSize(pressGpbuttonCaption->GetTextWidthPercent() + 2, 5);

  const std::vector<IHIDevice*> &controllers = GetControllers();
  int gamepadID = static_cast<HIDGamepad*>(controllers.at(controllerID))->GetGamepadID();

  captureGpbutton = new Gui2CaptureJoy(windowManager, "capturekey_settings_gamepad", gamepadID);
  captureGpbutton->sig_OnJoy.connect(boost::bind(&GamepadMappingPage::SetGpbuttonDone, this, buttonID));

  this->AddView(bg);
  this->AddView(pressGpbuttonCaption);
  this->AddView(captureGpbutton);
  bg->Show();
  pressGpbuttonCaption->Show();
  captureGpbutton->Show();
  captureGpbutton->SetFocus();
}

void GamepadMappingPage::SetGpbuttonDone(int buttonID) {
  //printf("clickah! %i\n", value);
  if (captureGpbutton->GetInputType() == e_JoystickInputType_Button) {
    int value = captureGpbutton->GetButtonID();
    if (value == -1) value = 0;
    gpbuttonButtons[buttonID]->GetCaptionWidget()->SetCaption(int_to_str(value));
    gpbuttonIDs[buttonID] = value;
  } else if (captureGpbutton->GetInputType() == e_JoystickInputType_Axis) {
    std::string signStr;
    (captureGpbutton->GetAxisSign() == -1) ? signStr = "-" : signStr = "+";
    gpbuttonButtons[buttonID]->GetCaptionWidget()->SetCaption("A" + int_to_str(captureGpbutton->GetAxisID()) + signStr);
    // encode axis in negative values
    signed int value = captureGpbutton->GetAxisID() * -2 - 1;
    if (captureGpbutton->GetAxisSign() == 1) value -= 1;
    gpbuttonIDs[buttonID] = value;
  }

  gpbuttonButtons[buttonID]->SetFocus();

  bg->Exit();
  delete bg;
  pressGpbuttonCaption->Exit();
  delete pressGpbuttonCaption;
  captureGpbutton->Exit();
  delete captureGpbutton;
}


// GAMEPAD BUTTON FUNCTION

GamepadFunctionPage::GamepadFunctionPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {
  controllerID = pageData.properties->GetInt("controllerID");

  const std::vector<IHIDevice*> &controllers = GetControllers();
  IHIDevice *controller = controllers.at(controllerID);

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings_controller_gamepadfunction", 10, 10, 80, 3, "'" + controllers.at(controllerID)->GetIdentifier() + "' function");
  this->AddView(title);
  title->Show();

  Gui2Image *controller_left = new Gui2Image(windowManager, "image_controller_left", 50, 15, 20, 30);
  this->AddView(controller_left);
  controller_left->LoadImage("media/menu/controller/controller_left.png");
  controller_left->Show();

  Gui2Image *controller_right = new Gui2Image(windowManager, "image_controller_right", 70, 15, 20, 30);
  this->AddView(controller_right);
  controller_right->LoadImage("media/menu/controller/controller_right.png");
  controller_right->Show();


  // modifiers

  std::string modifierStr[4];
  Gui2Image *modifierTargetImage[4];
  std::string modifierTargetImageLRstr[4];
  modifierIDs[0] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_Switch);
  modifierIDs[1] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_Special);
  modifierIDs[2] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_Sprint);
  modifierIDs[3] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_Dribble);
  for (int i = 0; i < 4; i++) {
    if      (modifierIDs[i] == e_ControllerButton_L1) { modifierStr[i] = "L1"; modifierTargetImage[i] = controller_left; modifierTargetImageLRstr[i] = "left"; }
    else if (modifierIDs[i] == e_ControllerButton_L2) { modifierStr[i] = "L2"; modifierTargetImage[i] = controller_left; modifierTargetImageLRstr[i] = "left"; }
    else if (modifierIDs[i] == e_ControllerButton_R1) { modifierStr[i] = "R1"; modifierTargetImage[i] = controller_right; modifierTargetImageLRstr[i] = "right"; }
    else if (modifierIDs[i] == e_ControllerButton_R2) { modifierStr[i] = "R2"; modifierTargetImage[i] = controller_right; modifierTargetImageLRstr[i] = "right"; }
  }


  Gui2Grid *gridModifier = new Gui2Grid(windowManager, "grid_gamepad_function_modifier", 0, 0, 20, 20);
  gridModifier->SetWrapping(false, false);

  Gui2Caption *captionModifier = new Gui2Caption(windowManager, "caption_gamepadfunction_modifiers", 0, 0, 30, 3, "modifiers");

  Gui2Caption *captionSwitch = new Gui2Caption(windowManager, "caption_gamepadfunction_switch", 0, 0, 10, 3, "switch");
  modifierButtons[0] = new Gui2Button(windowManager, "button_gamepadfunction_modifier0", 0, 0, 8, 3, modifierStr[0]);
  modifierButtons[0]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, modifierButtons[0], gridModifier, modifierIDs));
  modifierButtons[0]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, modifierTargetImage[0], "media/menu/controller/controller_" + modifierTargetImageLRstr[0] + "_" + modifierStr[0] + ".png"));
  modifierButtons[0]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, modifierTargetImage[0], "media/menu/controller/controller_" + modifierTargetImageLRstr[0] + ".png"));
  modifierButtons[0]->SetToggleable(true);

  Gui2Caption *captionSpecial = new Gui2Caption(windowManager, "caption_gamepadfunction_special", 0, 0, 10, 3, "special");
  modifierButtons[1] = new Gui2Button(windowManager, "button_gamepadfunction_modifier1", 0, 0, 8, 3, modifierStr[1]);
  modifierButtons[1]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, modifierButtons[1], gridModifier, modifierIDs));
  modifierButtons[1]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, modifierTargetImage[1], "media/menu/controller/controller_" + modifierTargetImageLRstr[1] + "_" + modifierStr[1] + ".png"));
  modifierButtons[1]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, modifierTargetImage[1], "media/menu/controller/controller_" + modifierTargetImageLRstr[1] + ".png"));
  modifierButtons[1]->SetToggleable(true);

  Gui2Caption *captionSprint = new Gui2Caption(windowManager, "caption_gamepadfunction_sprint", 0, 0, 10, 3, "sprint");
  modifierButtons[2] = new Gui2Button(windowManager, "button_gamepadfunction_modifier2", 0, 0, 8, 3, modifierStr[2]);
  modifierButtons[2]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, modifierButtons[2], gridModifier, modifierIDs));
  modifierButtons[2]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, modifierTargetImage[2], "media/menu/controller/controller_" + modifierTargetImageLRstr[2] + "_" + modifierStr[2] + ".png"));
  modifierButtons[2]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, modifierTargetImage[2], "media/menu/controller/controller_" + modifierTargetImageLRstr[2] + ".png"));
  modifierButtons[2]->SetToggleable(true);

  Gui2Caption *captionDribble = new Gui2Caption(windowManager, "caption_gamepadfunction_dribble", 0, 0, 10, 3, "slow dribble");
  modifierButtons[3] = new Gui2Button(windowManager, "button_gamepadfunction_modifier3", 0, 0, 8, 3, modifierStr[3]);
  modifierButtons[3]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, modifierButtons[3], gridModifier, modifierIDs));
  modifierButtons[3]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, modifierTargetImage[3], "media/menu/controller/controller_" + modifierTargetImageLRstr[3] + "_" + modifierStr[3] + ".png"));
  modifierButtons[3]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, modifierTargetImage[3], "media/menu/controller/controller_" + modifierTargetImageLRstr[3] + ".png"));
  modifierButtons[3]->SetToggleable(true);


  // on the ball

  std::string onTheBallStr[4];
  onTheBallIDs[0] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_LongPass);
  onTheBallIDs[1] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_HighPass);
  onTheBallIDs[2] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_ShortPass);
  onTheBallIDs[3] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_Shot);
  for (int i = 0; i < 4; i++) {
    if      (onTheBallIDs[i] == e_ControllerButton_Y) onTheBallStr[i] = "Y";
    else if (onTheBallIDs[i] == e_ControllerButton_B) onTheBallStr[i] = "B";
    else if (onTheBallIDs[i] == e_ControllerButton_A) onTheBallStr[i] = "A";
    else if (onTheBallIDs[i] == e_ControllerButton_X) onTheBallStr[i] = "X";
  }


  Gui2Grid *gridOnTheBall = new Gui2Grid(windowManager, "grid_gamepad_function_ontheball", 0, 0, 20, 20);
  gridOnTheBall->SetWrapping(false, false);

  Gui2Caption *captionOnTheBall = new Gui2Caption(windowManager, "caption_gamepadfunction_ontheball", 0, 0, 30, 3, "on the ball controls");

  Gui2Caption *captionLongPass = new Gui2Caption(windowManager, "caption_gamepadfunction_longpass", 0, 0, 10, 3, "long pass");
  onTheBallButtons[0] = new Gui2Button(windowManager, "button_gamepadfunction_ontheball0", 0, 0, 8, 3, onTheBallStr[0]);
  onTheBallButtons[0]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, onTheBallButtons[0], gridOnTheBall, onTheBallIDs));
  onTheBallButtons[0]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_" + onTheBallStr[0] + ".png"));
  onTheBallButtons[0]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));
  onTheBallButtons[0]->SetToggleable(true);

  Gui2Caption *captionHighPass = new Gui2Caption(windowManager, "caption_gamepadfunction_highpass", 0, 0, 10, 3, "high pass");
  onTheBallButtons[1] = new Gui2Button(windowManager, "button_gamepadfunction_ontheball1", 0, 0, 8, 3, onTheBallStr[1]);
  onTheBallButtons[1]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, onTheBallButtons[1], gridOnTheBall, onTheBallIDs));
  onTheBallButtons[1]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_" + onTheBallStr[1] + ".png"));
  onTheBallButtons[1]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));
  onTheBallButtons[1]->SetToggleable(true);

  Gui2Caption *captionPass = new Gui2Caption(windowManager, "caption_gamepadfunction_pass", 0, 0, 10, 3, "normal pass");
  onTheBallButtons[2] = new Gui2Button(windowManager, "button_gamepadfunction_ontheball2", 0, 0, 8, 3, onTheBallStr[2]);
  onTheBallButtons[2]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, onTheBallButtons[2], gridOnTheBall, onTheBallIDs));
  onTheBallButtons[2]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_" + onTheBallStr[2] + ".png"));
  onTheBallButtons[2]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));
  onTheBallButtons[2]->SetToggleable(true);

  Gui2Caption *captionShot = new Gui2Caption(windowManager, "caption_gamepadfunction_shot", 0, 0, 10, 3, "shoot");
  onTheBallButtons[3] = new Gui2Button(windowManager, "button_gamepadfunction_ontheball3", 0, 0, 8, 3, onTheBallStr[3]);
  onTheBallButtons[3]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, onTheBallButtons[3], gridOnTheBall, onTheBallIDs));
  onTheBallButtons[3]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_" + onTheBallStr[3] + ".png"));
  onTheBallButtons[3]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));
  onTheBallButtons[3]->SetToggleable(true);


  // off the ball

  std::string offTheBallStr[4];
  offTheBallIDs[0] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_KeeperRush);
  offTheBallIDs[1] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_Sliding);
  offTheBallIDs[2] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_Pressure);
  offTheBallIDs[3] = static_cast<HIDGamepad*>(controller)->GetFunctionMapping(e_ButtonFunction_TeamPressure);
  for (int i = 0; i < 4; i++) {
    if      (offTheBallIDs[i] == e_ControllerButton_Y) offTheBallStr[i] = "Y";
    else if (offTheBallIDs[i] == e_ControllerButton_B) offTheBallStr[i] = "B";
    else if (offTheBallIDs[i] == e_ControllerButton_A) offTheBallStr[i] = "A";
    else if (offTheBallIDs[i] == e_ControllerButton_X) offTheBallStr[i] = "X";
  }


  Gui2Grid *gridOffTheBall = new Gui2Grid(windowManager, "grid_gamepad_function_offtheball", 0, 0, 20, 20);
  gridOffTheBall->SetWrapping(false, false);

  Gui2Caption *captionOffTheBall = new Gui2Caption(windowManager, "caption_gamepadfunction_offtheball", 0, 0, 30, 3, "off the ball controls");

  Gui2Caption *captionKeeper = new Gui2Caption(windowManager, "caption_gamepadfunction_keeper", 0, 0, 10, 3, "keeper to ball");
  offTheBallButtons[0] = new Gui2Button(windowManager, "button_gamepadfunction_offtheball0", 0, 0, 8, 3, offTheBallStr[0]);
  offTheBallButtons[0]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, offTheBallButtons[0], gridOffTheBall, offTheBallIDs));
  offTheBallButtons[0]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_" + offTheBallStr[0] + ".png"));
  offTheBallButtons[0]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));
  offTheBallButtons[0]->SetToggleable(true);

  Gui2Caption *captionSliding = new Gui2Caption(windowManager, "caption_gamepadfunction_sliding", 0, 0, 10, 3, "sliding");
  offTheBallButtons[1] = new Gui2Button(windowManager, "button_gamepadfunction_offtheball1", 0, 0, 8, 3, offTheBallStr[1]);
  offTheBallButtons[1]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, offTheBallButtons[1], gridOffTheBall, offTheBallIDs));
  offTheBallButtons[1]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_" + offTheBallStr[1] + ".png"));
  offTheBallButtons[1]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));
  offTheBallButtons[1]->SetToggleable(true);

  Gui2Caption *captionPressure = new Gui2Caption(windowManager, "caption_gamepadfunction_pressure", 0, 0, 10, 3, "pressure");
  offTheBallButtons[2] = new Gui2Button(windowManager, "button_gamepadfunction_offtheball2", 0, 0, 8, 3, offTheBallStr[2]);
  offTheBallButtons[2]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, offTheBallButtons[2], gridOffTheBall, offTheBallIDs));
  offTheBallButtons[2]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_" + offTheBallStr[2] + ".png"));
  offTheBallButtons[2]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));
  offTheBallButtons[2]->SetToggleable(true);

  Gui2Caption *captionTeamPressure = new Gui2Caption(windowManager, "caption_gamepadfunction_teampressure", 0, 0, 10, 3, "team pressure");
  offTheBallButtons[3] = new Gui2Button(windowManager, "button_gamepadfunction_offtheball3", 0, 0, 8, 3, offTheBallStr[3]);
  offTheBallButtons[3]->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SelectGpbutton, this, offTheBallButtons[3], gridOffTheBall, offTheBallIDs));
  offTheBallButtons[3]->sig_OnGainFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right_" + offTheBallStr[3] + ".png"));
  offTheBallButtons[3]->sig_OnLoseFocus.connect(boost::bind(&Gui2Image::LoadImage, controller_right, "media/menu/controller/controller_right.png"));
  offTheBallButtons[3]->SetToggleable(true);


  gridModifier->AddView(captionSwitch, 0, 0);
  gridModifier->AddView(modifierButtons[0], 0, 1);
  gridModifier->AddView(captionSpecial, 1, 0);
  gridModifier->AddView(modifierButtons[1], 1, 1);
  gridModifier->AddView(captionSprint, 2, 0);
  gridModifier->AddView(modifierButtons[2], 2, 1);
  gridModifier->AddView(captionDribble, 3, 0);
  gridModifier->AddView(modifierButtons[3], 3, 1);

  gridOnTheBall->AddView(captionLongPass, 0, 0);
  gridOnTheBall->AddView(onTheBallButtons[0], 0, 1);
  gridOnTheBall->AddView(captionHighPass, 1, 0);
  gridOnTheBall->AddView(onTheBallButtons[1], 1, 1);
  gridOnTheBall->AddView(captionPass, 2, 0);
  gridOnTheBall->AddView(onTheBallButtons[2], 2, 1);
  gridOnTheBall->AddView(captionShot, 3, 0);
  gridOnTheBall->AddView(onTheBallButtons[3], 3, 1);

  gridOffTheBall->AddView(captionKeeper, 0, 0);
  gridOffTheBall->AddView(offTheBallButtons[0], 0, 1);
  gridOffTheBall->AddView(captionSliding, 1, 0);
  gridOffTheBall->AddView(offTheBallButtons[1], 1, 1);
  gridOffTheBall->AddView(captionPressure, 2, 0);
  gridOffTheBall->AddView(offTheBallButtons[2], 2, 1);
  gridOffTheBall->AddView(captionTeamPressure, 3, 0);
  gridOffTheBall->AddView(offTheBallButtons[3], 3, 1);


  Gui2Button *buttonDefaults = new Gui2Button(windowManager, "button_gamepadfunction_defaults", 0, 0, 30, 3, "reset to defaults");
  buttonDefaults->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::SetDefaults, this));
  buttonDefaults->sig_OnClick.connect(boost::bind(&GamepadFunctionPage::GoBack, this));

  Gui2Image *spacer = new Gui2Image(windowManager, "tmpspacer", 0, 0, 1, 5);

  Gui2Grid *wrapperGrid = new Gui2Grid(windowManager, "grid_gamepad_function_wrapper", 10, 15, 40, 75);
  wrapperGrid->SetWrapping(true, false);

  wrapperGrid->AddView(buttonDefaults, 0, 0);
  wrapperGrid->AddView(spacer, 1, 0);
  wrapperGrid->AddView(captionModifier, 2, 0);
  wrapperGrid->AddView(gridModifier, 3, 0);
  wrapperGrid->AddView(captionOnTheBall, 4, 0);
  wrapperGrid->AddView(gridOnTheBall, 5, 0);
  wrapperGrid->AddView(captionOffTheBall, 6, 0);
  wrapperGrid->AddView(gridOffTheBall, 7, 0);

  gridModifier->UpdateLayout(0.5);
  gridOnTheBall->UpdateLayout(0.5);
  gridOffTheBall->UpdateLayout(0.5);

  wrapperGrid->UpdateLayout(0.0);

  this->AddView(wrapperGrid);
  wrapperGrid->Show();

  modifierButtons[0]->SetFocus();

  this->Show();

  sig_OnClose.connect(boost::bind(&GamepadFunctionPage::OnClose, this));
}

GamepadFunctionPage::~GamepadFunctionPage() {
}

void GamepadFunctionPage::OnClose() {
  const std::vector<IHIDevice*> &controllers = GetControllers();
  IHIDevice *controller = controllers.at(controllerID);

  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Switch,  modifierIDs[0]);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Special, modifierIDs[1]);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Sprint,  modifierIDs[2]);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Dribble, modifierIDs[3]);

  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_LongPass,  onTheBallIDs[0]);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_HighPass,  onTheBallIDs[1]);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_ShortPass, onTheBallIDs[2]);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Shot,      onTheBallIDs[3]);

  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_KeeperRush,   offTheBallIDs[0]);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Sliding,      offTheBallIDs[1]);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Pressure,     offTheBallIDs[2]);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_TeamPressure, offTheBallIDs[3]);

  controller->SaveConfig();
}

Gui2Button *GetToggledButton(Gui2Grid *grid, Gui2Button *except) {
  std::vector<Gui2View*> allButtons = grid->GetChildren();
  for (int i = 0; i < (signed int)allButtons.size(); i++) {
    if (allButtons.at(i)->GetName().substr(0, 6).compare("button") == 0)
      if (static_cast<Gui2Button*>(allButtons.at(i)) != except) if (static_cast<Gui2Button*>(allButtons.at(i))->IsToggled()) return static_cast<Gui2Button*>(allButtons.at(i));
  }
  return 0;
}

void GamepadFunctionPage::SelectGpbutton(Gui2Button *button, Gui2Grid *grid, e_ControllerButton controllerButtonIDs[]) {
  Gui2Button *selected = GetToggledButton(grid, button);
  if (selected) {
    // switch buttons
    selected->SetToggled(false);
    button->SetToggled(false);

    int rowSelected = grid->GetRow(selected);
    int rowButton = grid->GetRow(button);
    assert(rowSelected != -1 && rowButton != -1);
    grid->RemoveView(rowSelected, 1);
    grid->RemoveView(rowButton, 1);
    grid->AddView(button, rowSelected, 1);
    button->Show();
    grid->AddView(selected, rowButton, 1);
    selected->Show();
    grid->UpdateLayout(0.5);
    selected->SetFocus();

    int id1 = atoi(selected->GetName().substr(selected->GetName().length() - 1, std::string::npos).c_str());
    int id2 = atoi(  button->GetName().substr(  button->GetName().length() - 1, std::string::npos).c_str());

    // swap
    e_ControllerButton tmp = controllerButtonIDs[id1];
    controllerButtonIDs[id1] = controllerButtonIDs[id2];
    controllerButtonIDs[id2] = tmp;
  }
}

void GamepadFunctionPage::SetDefaults() {
  const std::vector<IHIDevice*> &controllers = GetControllers();
  IHIDevice *controller = controllers.at(controllerID);

  modifierIDs[0] = e_ControllerButton_L1;
  modifierIDs[1] = e_ControllerButton_L2;
  modifierIDs[2] = e_ControllerButton_R1;
  modifierIDs[3] = e_ControllerButton_R2;

  onTheBallIDs[0] = e_ControllerButton_Y;
  onTheBallIDs[1] = e_ControllerButton_B;
  onTheBallIDs[2] = e_ControllerButton_A;
  onTheBallIDs[3] = e_ControllerButton_X;

  offTheBallIDs[0] = e_ControllerButton_Y;
  offTheBallIDs[1] = e_ControllerButton_B;
  offTheBallIDs[2] = e_ControllerButton_A;
  offTheBallIDs[3] = e_ControllerButton_X;

  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Switch, e_ControllerButton_L1);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Special, e_ControllerButton_L2);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Sprint, e_ControllerButton_R1);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Dribble, e_ControllerButton_R2);

  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_LongPass, e_ControllerButton_Y);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_HighPass, e_ControllerButton_B);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_ShortPass, e_ControllerButton_A);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Shot, e_ControllerButton_X);

  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_KeeperRush, e_ControllerButton_Y);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Sliding, e_ControllerButton_B);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_Pressure, e_ControllerButton_A);
  static_cast<HIDGamepad*>(controller)->SetFunctionMapping(e_ButtonFunction_TeamPressure, e_ControllerButton_X);
}

GraphicsPage::GraphicsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings_graphics", 20, 10, 60, 3, "Graphics setup");
  this->AddView(title);
  title->Show();
  title->SetFocus();

#ifdef WIN32
  DEVMODE dm;// = { 0 };
  ZeroMemory(&dm, sizeof(dm));
  dm.dmSize = sizeof(dm);
  for (int iModeNum = 0; EnumDisplaySettings(NULL, iModeNum, &dm) != 0; iModeNum++) {
    Resolution res;
    res.x = dm.dmPelsWidth;
    res.y = dm.dmPelsHeight;
    res.bpp = dm.dmBitsPerPel;
    res.fullscreen = false;
    //cout << "Mode #" << iModeNum << " = " << dm.dmPelsWidth << "x" << dm.dmPelsHeight << endl;
    if (res.bpp == 32) if (!CheckDuplicate(resolutions, res.x, res.y)) resolutions.push_back(res);
  }
#else
  SDL_PixelFormat format;
  SDL_Rect **modes;
  int loops(0);
  int bpp(0);
  for (int loops = 0; loops < 3; loops++) {
    switch(loops) {
      case 0://32 bpp
        format.BitsPerPixel = 32;
        bpp = 32;
        break;
      case 1://24 bpp
        format.BitsPerPixel = 24;
        bpp = 24;
        break;
      case 2://16 bpp
        format.BitsPerPixel = 16;
        bpp = 16;
        break;
    }

    //get available fullscreen/hardware modes
    //modes = SDL_ListModes(&format, SDL_FULLSCREEN);
    //if (modes) {
      //for(int i = 0; modes[i]; ++i) {
        Resolution res;
        res.x = 1280;//modes[i]->w;
        res.y = 1024;//modes[i]->h;
        res.bpp = 32;//bpp;
        res.fullscreen = false;
        //if (res.bpp == 32) if (!CheckDuplicate(resolutions, res.x, res.y))
        resolutions.push_back(res);
      //}
    //}
  }
#endif

  // add fullscreen res'es
  unsigned int resolutionsSize = resolutions.size();
  for (unsigned int i = 0; i < resolutionsSize; i++) {
    Resolution res = resolutions.at(i);
    res.fullscreen = true;
    resolutions.push_back(res);
  }

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_settings_graphics_reslist", 20, 15, 60, 75);

  int context_width, context_height, context_bpp;
  GetScene2D()->GetContextSize(context_width, context_height, context_bpp);

  int row = 0;
  int col = 0;

  bool fullscreen = (GetConfiguration()->Get("context_fullscreen", "false").compare("true") == 0) ? true : false;

  for (unsigned int i = 0; i < resolutions.size(); i++) {
    std::string fullscreenString = " windowed";
    if (resolutions.at(i).fullscreen) fullscreenString = " fullscreen";
    Gui2Button *button = new Gui2Button(windowManager, "button_graphics_res" + int_to_str(resolutions.at(i).x) + "x" + int_to_str(resolutions.at(i).y) + fullscreenString, 0, 0, 30, 3, int_to_str(resolutions.at(i).x) + " x " + int_to_str(resolutions.at(i).y) + fullscreenString/* + " @ " + int_to_str(resolutions.at(i).bpp) + " bpp"*/);
    button->sig_OnClick.connect(boost::bind(&GraphicsPage::SetResolution, this, i));
    grid->AddView(button, row, col);

    if (i == 0) button->SetFocus();

    if (context_width == resolutions.at(i).x &&
        context_height == resolutions.at(i).y &&
        fullscreen == resolutions.at(i).fullscreen) {
      button->SetFocus();
    }

    row++;
    if (row > 16) {
      row = 0;
      col++;
    }
  }

  grid->UpdateLayout(0.5);

  this->AddView(grid);
  grid->Show();

  this->Show();
}

GraphicsPage::~GraphicsPage() {
}

void GraphicsPage::SetResolution(int resIndex) {
  GetConfiguration()->SetBool("context_fullscreen", resolutions.at(resIndex).fullscreen);
  GetConfiguration()->Set("context_x", resolutions.at(resIndex).x);
  GetConfiguration()->Set("context_y", resolutions.at(resIndex).y);
  GetConfiguration()->SaveFile(GetConfigFilename());

  bg = new Gui2Image(windowManager, "image_setresolution_bg", 0, 0, 100, 13);
  bg->LoadImage("media/menu/backgrounds/black.png");

  restartCaption1 = new Gui2Caption(windowManager, "caption_settings_resolution_info1", 0, 0, 100, 3, "please restart the game for the changes to become active.");
  restartCaption2 = new Gui2Caption(windowManager, "caption_settings_resolution_info2", 0, 0, 100, 3, "if this resolution doesn't happen to work, you can always");
  restartCaption3 = new Gui2Caption(windowManager, "caption_settings_resolution_info3", 0, 0, 100, 3, "change it manually by editing the file 'football.config'.");
  restartCaption1->SetPosition(50 - restartCaption1->GetTextWidthPercent() * 0.5, 44);
  restartCaption2->SetPosition(50 - restartCaption2->GetTextWidthPercent() * 0.5, 48);
  restartCaption3->SetPosition(50 - restartCaption3->GetTextWidthPercent() * 0.5, 52);

  bg->SetPosition(50 - restartCaption1->GetTextWidthPercent() * 0.5 - 1, 43);
  bg->SetSize(restartCaption1->GetTextWidthPercent() + 2, 13);

  captureKey = new Gui2CaptureKey(windowManager, "capturekey_settings_resolution");
  captureKey->sig_OnKey.connect(boost::bind(&GraphicsPage::GoBack, this));

  this->AddView(bg);
  this->AddView(restartCaption1);
  this->AddView(restartCaption2);
  this->AddView(restartCaption3);
  this->AddView(captureKey);
  bg->Show();
  restartCaption1->Show();
  restartCaption2->Show();
  restartCaption3->Show();
  captureKey->Show();
  captureKey->SetFocus();
}


// AUDIO MENU

AudioPage::AudioPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_settings_audio", 20, 20, 60, 3, "Audio setup");
  this->AddView(title);
  title->Show();

  sliderVolume = new Gui2Slider(windowManager, "volumeslider", 0, 0, 30, 6, "volume");

  Gui2Grid *grid = new Gui2Grid(windowManager, "volumegrid", 20, 25, 50, 50);

  grid->AddView(sliderVolume, 0, 0);

  grid->UpdateLayout(0.5);

  if (Verbose()) printf("volume: %f\n", GetConfiguration()->GetReal("audio_volume", 0.5));
  sliderVolume->SetValue(GetConfiguration()->GetReal("audio_volume", 0.5));

  this->AddView(grid);
  grid->Show();

  sliderVolume->SetFocus();

  this->Show();
}

AudioPage::~AudioPage() {
}

void AudioPage::Exit() {
  GetConfiguration()->Set("audio_volume", sliderVolume->GetValue());
  GetConfiguration()->SaveFile(GetConfigFilename());

  Gui2Page::Exit();
}
