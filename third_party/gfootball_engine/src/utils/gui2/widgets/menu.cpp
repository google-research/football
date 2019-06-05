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

// written by bastiaan konings schuiling 2008 - 2014
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include "menu.hpp"

#include "frame.hpp"

#include "../windowmanager.hpp"

namespace blunted {

  Gui2Menu::Gui2Menu(Gui2WindowManager *windowManager, Gui2Menu *parentMenu, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent) : Gui2Frame(windowManager, name, x_percent, y_percent, width_percent, height_percent), parentMenu(parentMenu) {
    if (parentMenu) {
      parentMenu->_AddSubmenu(this);
      parentMenu->Deactivate();
    }
    lastFocus = this;
  }

  Gui2Menu::~Gui2Menu() {
    if (parentMenu) {
      parentMenu->Reactivate();
      parentMenu->_RemoveSubmenu(this);
    }

    for (int i = 0; i < (signed int)subMenus.size(); i++) {
      subMenus.at(i)->DecoupleParent();
    }
    subMenus.clear();
  }

  void Gui2Menu::Process() {
    Gui2View::Process();
  }

  void Gui2Menu::DecoupleParent() {
    parentMenu = 0;
  }

  void Gui2Menu::_AddSubmenu(Gui2Menu *view) {
    subMenus.push_back(view);
  }

  void Gui2Menu::_RemoveSubmenu(Gui2Menu *view) {
    std::vector<Gui2Menu*>::iterator iter = subMenus.begin();
    while (iter != subMenus.end()) {
      if (*iter == view) iter = subMenus.erase(iter); else iter++;
    }
  }

  void Gui2Menu::Deactivate() {
    // hide all child widgets
    lastFocus = windowManager->GetFocus();
    this->Hide();
  }

  void Gui2Menu::Reactivate() {
    // show all child widgets
    lastFocus->SetFocus();
    this->Show();
  }

  void Gui2Menu::ProcessWindowingEvent(WindowingEvent *event) {
    if (event->IsEscape()) {

      this->Exit();
      delete this;

    } else {
      event->Ignore();
    }
  }

}
