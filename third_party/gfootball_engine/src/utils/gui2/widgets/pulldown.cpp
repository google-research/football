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

#include "pulldown.hpp"

namespace blunted {

  Gui2Pulldown::Gui2Pulldown(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent) {

    isSelectable = true;
    isOverlay = false; // reminder: dynamic, based on 'is pulled down'

    bg = new Gui2Image(windowManager, name + "_bg", 0, 0, width_percent, height_percent);
    bg->LoadImage("media/menu/backgrounds/black.png");
    bg->Show();

    grid = new Gui2Grid(windowManager, name + "_grid", 0, 0, width_percent, height_percent);
    grid->Show();

    selectedEntry = 0;

    pulldownButton = new Gui2Button(windowManager, name + "_pulldownbutton", 0, 0, width_percent, 3, "tmp");
    pulldownButton->sig_OnClick.connect(boost::bind(&Gui2Pulldown::PullDownOrUp, this));
    this->AddView(pulldownButton);
    pulldownButton->Show();

    pulledDown = false;
  }

  Gui2Pulldown::~Gui2Pulldown() {
    if (!pulledDown) { // grid is not a child then, so remove it manually
      grid->Exit();
      delete grid;
      bg->Exit();
      delete bg;
    }
  }

  void Gui2Pulldown::AddEntry(const std::string &caption, const std::string &name) {

    Gui2Button *button = new Gui2Button(windowManager, name + "_entry" + int_to_str(entries.size()), 0, 0, width_percent - 1, 3, caption);
    button->sig_OnClick.connect(boost::bind(&Gui2Pulldown::Select, this, entries.size()));

    PulldownEntry entry;
    entry.name = name;
    entry.button = button;

    grid->AddView(button, entries.size(), 0);

    entries.push_back(entry);

    if (entries.size() == 1) pulldownButton->SetCaption(caption); // default selected
  }

  void Gui2Pulldown::PullDownOrUp() {
    if (pulledDown == false) {
      this->AddView(bg);
      this->AddView(grid);
      grid->SetMaxVisibleRows(5);
      grid->UpdateLayout(0.0, 0.0, 0.0, 0.0);
      float x, y;
      grid->GetSize(x, y);
      bg->SetSize(x, y);
      entries.at(selectedEntry).button->SetFocus();
      pulldownButton->Hide();
      bg->Show();
      grid->Show();
      pulledDown = true;
      isOverlay = true;
    } else { // we want to pull it up
      grid->Hide();
      bg->Hide();
      pulldownButton->Show();
      this->RemoveView(grid);
      this->RemoveView(bg);
      pulldownButton->SetFocus();
      pulledDown = false;
      isOverlay = false;
    }
  }

  void Gui2Pulldown::SetSelected(int selectedEntry) {
    this->selectedEntry = selectedEntry;
    pulldownButton->SetCaption(entries.at(selectedEntry).button->GetCaption());
  }

  std::string Gui2Pulldown::GetSelected() const {
    return entries.at(selectedEntry).name;
  }

  void Gui2Pulldown::ProcessWindowingEvent(WindowingEvent *event) {
    if (event->IsEscape()) {
      if (pulledDown) PullDownOrUp(); else event->Ignore();
    } else {
      event->Ignore();
    }
  }

  void Gui2Pulldown::Select(int selectedEntry) {
    this->selectedEntry = selectedEntry;
    pulldownButton->SetCaption(entries.at(selectedEntry).button->GetCaption());
    if (pulledDown) sig_OnChange(this);
    PullDownOrUp();
  }

}
