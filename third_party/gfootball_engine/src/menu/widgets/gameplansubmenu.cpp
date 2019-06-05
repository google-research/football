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

#include "gameplansubmenu.hpp"

GamePlanSubMenu::GamePlanSubMenu(Gui2WindowManager *windowManager, Gui2View *parentFocus, Gui2Grid *mainGrid, const std::string &name) : Gui2View(windowManager, name, 0, 0, 100, 100), mainGrid(mainGrid), parentFocus(parentFocus) {
  grid = new Gui2Grid(windowManager, "gameplan_grid_" + name, 0, 0, 0, 0);
  this->AddView(grid);
  mainGrid->AddView(this, 1, 0);
  mainGrid->UpdateLayout(0.0);
  grid->SetQuickScroll(true);
  grid->Show();

  sig_OnClose.connect(boost::bind(&GamePlanSubMenu::OnClose, this));
}

GamePlanSubMenu::~GamePlanSubMenu() {
}


void GamePlanSubMenu::OnClose() {
}

Gui2Button *GamePlanSubMenu::AddButton(const std::string &buttonName, const std::string &buttonCaption, int row, int column, Vector3 color = Vector3(-1)) {
  if (color.coords[0] < 0) color = windowManager->GetStyle()->GetColor(e_DecorationType_Bright1);
  Gui2Button *theButton = new Gui2Button(windowManager, "gameplan_button_submenu_" + name + "_" + buttonName, 0, 0, 34, 3, buttonCaption);
  theButton->SetColor(color);
  allButtons.push_back(theButton);
  grid->AddView(theButton, row, column);
  grid->SetMaxVisibleRows(11);
  grid->UpdateLayout(0.5);
  mainGrid->UpdateLayout(0.0);
  return theButton;
}

Gui2Slider *GamePlanSubMenu::AddSlider(const std::string &sliderName, const std::string &sliderCaption, int row, int column) {
  Gui2Slider *theSlider = new Gui2Slider(windowManager, "gameplan_slider_submenu_" + name + "_" + sliderName, 0, 0, 34, 6, sliderCaption);
  grid->AddView(theSlider, row, column);
  grid->SetMaxVisibleRows(6);
  grid->UpdateLayout(0.5);
  mainGrid->UpdateLayout(0.0);
  return theSlider;
}

Gui2Button *GamePlanSubMenu::GetToggledButton(Gui2Button *except) {
  for (int i = 0; i < (signed int)allButtons.size(); i++) {
    if (allButtons.at(i) != except) if (allButtons.at(i)->IsToggled()) return allButtons.at(i);
  }
  return 0;
}

void GamePlanSubMenu::ProcessWindowingEvent(WindowingEvent *event) {
  if (event->IsEscape()) {

    mainGrid->RemoveView(1, 0); // removing self!
    parentFocus->SetFocus();

    this->Exit(); // should send sig_OnClose
    delete this;
    return;

  } else {
    event->Ignore();
  }
}
