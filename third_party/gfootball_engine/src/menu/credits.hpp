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

#ifndef _HPP_MENU_CREDITS
#define _HPP_MENU_CREDITS

#include "../utils/gui2/windowmanager.hpp"

#include "../utils/gui2/widgets/menu.hpp"
#include "../utils/gui2/widgets/root.hpp"
#include "../utils/gui2/widgets/image.hpp"
#include "../utils/gui2/widgets/caption.hpp"

#include "../base/math/vector3.hpp"

using namespace blunted;

const int numtexts = 24;
const int numballs = 12;

struct CreditsContents {
  std::string text;
  Vector3 color;
};

class CreditsPage : public Gui2Page {

  public:
    CreditsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
    virtual ~CreditsPage();

    void InitCreditsContents();
    void AddHeader(const std::string &blah);
    void AddSubHeader(const std::string &blah);
    void AddCredit(const std::string &blah);
    void AddWhitespace();

    virtual void Process();
    virtual void ProcessJoystickEvent(JoystickEvent *event);

  protected:
    Gui2Caption *text[numtexts];
    Gui2Image *balls[numballs];
    Vector3 ballPos[numballs];
    Vector3 ballMov[numballs];

    float scrollOffset = 0.0f;
    unsigned int creditOffset = 0;
    unsigned int previousStartIndex = 0;

    std::vector<CreditsContents> credits;

    Gui2Image *bg;

};

#endif
