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

#ifndef _HPP_MENU_SETTINGS
#define _HPP_MENU_SETTINGS

#include "../utils/gui2/windowmanager.hpp"

#include "../utils/gui2/page.hpp"
#include "../utils/gui2/widgets/root.hpp"
#include "../utils/gui2/widgets/image.hpp"
#include "../utils/gui2/widgets/button.hpp"
#include "../utils/gui2/widgets/slider.hpp"
#include "../utils/gui2/widgets/grid.hpp"
#include "../utils/gui2/widgets/caption.hpp"
#include "../utils/gui2/widgets/capturekey.hpp"

#include "../hid/ihidevice.hpp"
#include "wrap_SDL_keycode.h"

using namespace blunted;

class SettingsPage : public Gui2Page {

  public:
    SettingsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~SettingsPage();

    void GoGameplay();
    void GoController();
    void GoGraphics();
    void GoAudio();

  protected:

};

class GameplayPage : public Gui2Page {

  public:
    GameplayPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GameplayPage();

    virtual void Exit();

  protected:
    Gui2Slider *slider_ShortPass_AutoDirection;
    Gui2Slider *slider_ShortPass_AutoPower;
    Gui2Slider *slider_ThroughPass_AutoDirection;
    Gui2Slider *slider_ThroughPass_AutoPower;
    Gui2Slider *slider_HighPass_AutoDirection;
    Gui2Slider *slider_HighPass_AutoPower;
    Gui2Slider *slider_Shot_AutoDirection;

    Gui2Slider *slider_Agility;
    Gui2Slider *slider_Acceleration;
    Gui2Slider *slider_Quantization;
};

class ControllerPage : public Gui2Page {

  public:
    ControllerPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~ControllerPage();

    void GoKeyboard();
    void GoGamepads();

  protected:

};

class KeyboardPage : public Gui2Page {

  public:
    KeyboardPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~KeyboardPage();

    void OnClose();

    void SetDefaults();
    void SetKeyDone(int buttonID);

  protected:
    SDL_Keycode keyIDs[18];
    Gui2Button *keyButtons[18];

    Gui2Image *bg;
    Gui2Caption *pressKeyCaption;
    Gui2CaptureKey *captureKey;

};

class GamepadsPage : public Gui2Page {

  public:
    GamepadsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GamepadsPage();

    void GoGamepadSetup(int controllerID);

  protected:

};

class GamepadSetupPage : public Gui2Page {

  public:
    GamepadSetupPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GamepadSetupPage();

    void GoGamepadCalibrationPage(int controllerID);
    void GoGamepadMappingPage(int controllerID);
    void GoGamepadFunctionPage(int controllerID);

  protected:

};

class GamepadCalibrationPage : public Gui2Page {

  public:
    GamepadCalibrationPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GamepadCalibrationPage();

    virtual void Process();
    virtual void ProcessWindowingEvent(WindowingEvent *event) { event->Accept(); }
    virtual void ProcessKeyboardEvent(KeyboardEvent *event);

  protected:
    virtual void SaveCalibration();

    int controllerID = 0;

    float min[_JOYSTICK_MAXAXES];
    float max[_JOYSTICK_MAXAXES];

    Gui2Image *bg;
    Gui2Caption *captionExplanation[3];

};

class GamepadMappingPage : public Gui2Page {

  public:
    GamepadMappingPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GamepadMappingPage();

    void OnClose();

    void SetGpbutton(int buttonID, const std::string &name);
    void SetGpbuttonDone(int buttonID);

  protected:
    int controllerID = 0;

    signed int gpbuttonIDs[e_ControllerButton_Size];
    Gui2Button *gpbuttonButtons[e_ControllerButton_Size]; // lol

    Gui2Image *bg;
    Gui2Caption *pressGpbuttonCaption;
    Gui2CaptureJoy *captureGpbutton;

};

class GamepadFunctionPage : public Gui2Page {

  public:
    GamepadFunctionPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GamepadFunctionPage();

    void OnClose();

    void SelectGpbutton(Gui2Button *button, Gui2Grid *grid, e_ControllerButton controllerButtonIDs[]);
    void SetDefaults();

  protected:
    int controllerID = 0;

    e_ControllerButton modifierIDs[4];
    e_ControllerButton onTheBallIDs[4];
    e_ControllerButton offTheBallIDs[4];
    Gui2Button *modifierButtons[4];
    Gui2Button *onTheBallButtons[4];
    Gui2Button *offTheBallButtons[4];

};


struct Resolution {
  int x = 0;
  int y = 0;
  int bpp = 0;
  bool fullscreen = false;
};

class GraphicsPage : public Gui2Page {

  public:
    GraphicsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~GraphicsPage();

  protected:
    void SetResolution(int resIndex);

    std::vector<Resolution> resolutions;

    Gui2Image *bg;
    Gui2Caption *restartCaption1, *restartCaption2, *restartCaption3;
    Gui2CaptureKey *captureKey;
};

class AudioPage : public Gui2Page {

  public:
    AudioPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~AudioPage();

    virtual void Exit();

  protected:
    Gui2Slider *sliderVolume;

};

#endif
