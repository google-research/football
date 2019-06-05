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

#include "dialog.hpp"

#include "grid.hpp"
#include "button.hpp"
#include "caption.hpp"

namespace blunted {

  Gui2Dialog::Gui2Dialog(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption) : Gui2Frame(windowManager, name + "_frame", x_percent, y_percent, width_percent, height_percent, true) {

    isOverlay = true;

    grid = new Gui2Grid(windowManager, name + "_maingrid", 0, 0, 100, 100);

    Gui2Caption *title = new Gui2Caption(windowManager, name + "title", 0, 0, 100, 3, caption);
    grid->AddView(title, 0, 0);

    grid->UpdateLayout(1, 1, 1, 1);
    this->AddView(grid);
  }

  Gui2Dialog::~Gui2Dialog() {
  }

  void Gui2Dialog::AddContent(Gui2View *view) {
    grid->AddView(view, 1, 0);
    grid->UpdateLayout(1, 1, 1, 1);
  }

  Gui2Button *Gui2Dialog::AddPosNegButtons(const std::string &posName, const std::string &negName) {
    float buttonWidth = 16;
    Gui2Button *posButton = new Gui2Button(windowManager, name + "_" + posName + "_button", width_percent / 2.0 - buttonWidth - 0.5, height_percent - 4, buttonWidth, 3, posName);
    Gui2Button *negButton = new Gui2Button(windowManager, name + "_" + negName + "_button", width_percent / 2.0 + 0.5, height_percent - 4, buttonWidth, 3, negName);
    posButton->sig_OnClick.connect(boost::bind(boost::ref(Gui2Dialog::sig_OnPositive), this));
    negButton->sig_OnClick.connect(boost::bind(boost::ref(Gui2Dialog::sig_OnNegative), this));
    this->AddView(posButton);
    this->AddView(negButton);
    return posButton;
  }

  Gui2Button *Gui2Dialog::AddSingleButton(const std::string &caption) {
    float buttonWidth = 20;
    Gui2Button *theButton = new Gui2Button(windowManager, name + "_" + caption + "_button", width_percent / 2.0 - buttonWidth / 2.0, height_percent - 4, buttonWidth, 3, caption);
    theButton->sig_OnClick.connect(boost::bind(boost::ref(Gui2Dialog::sig_OnPositive), this));
    this->AddView(theButton);
    return theButton;
  }

  void Gui2Dialog::ProcessWindowingEvent(WindowingEvent *event) {
    if (event->IsEscape()) {
      // not used? (and sig_OnClose is now in View, so at least should be renamed to OnCloseDialog or something): sig_OnClose(this);
    } else {
      event->Ignore();
    }
  }

}
