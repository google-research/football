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

#include "visualoptions.hpp"

#include "../main.hpp"

using namespace blunted;

VisualOptionsPage::VisualOptionsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Frame *frame = new Gui2Frame(windowManager, "frame_visualoptions", 15, 50, 70, 40, true);
  this->AddView(frame);
  frame->Show();

  Gui2Caption *title = new Gui2Caption(windowManager, "caption_visualoptions", 5, 5, 20, 3, "Visual options");
  frame->AddView(title);
  title->Show();

  Gui2Grid *grid = new Gui2Grid(windowManager, "grid_visualoptions", 5, 15, 70, 15);

  kitSelectionPulldown[0] = new Gui2Pulldown(windowManager, "pulldown_visualoptions_kitselection_t1", 0, 0, 30, 3);
  Gui2Caption *kitSelectionCaption1 = new Gui2Caption(windowManager, "caption_visualoptions_kitselection_t1", 0, 0, 20, 3, GetGameTask()->GetMatch()->GetTeam(0)->GetTeamData()->GetName() + " kit");
  kitSelectionPulldown[1] = new Gui2Pulldown(windowManager, "pulldown_visualoptions_kitselection_t2", 0, 0, 30, 3);
  Gui2Caption *kitSelectionCaption2 = new Gui2Caption(windowManager, "caption_visualoptions_kitselection_t2", 0, 0, 20, 3, GetGameTask()->GetMatch()->GetTeam(1)->GetTeamData()->GetName() + " kit");

  kitSelectionPulldown[0]->AddEntry("Kit 01", "team1kit01");
  kitSelectionPulldown[0]->AddEntry("Kit 02", "team1kit02");
  kitSelectionPulldown[1]->AddEntry("Kit 01", "team2kit01");
  kitSelectionPulldown[1]->AddEntry("Kit 02", "team2kit02");
  kitSelectionPulldown[1]->SetSelected(1);
  kitSelectionPulldown[0]->sig_OnChange.connect(boost::bind(&VisualOptionsPage::OnChangeKit, this, kitSelectionPulldown[0]));
  kitSelectionPulldown[1]->sig_OnChange.connect(boost::bind(&VisualOptionsPage::OnChangeKit, this, kitSelectionPulldown[1]));

  Gui2Button *randomizeSunButton = new Gui2Button(windowManager, "button_visualoptions_randomizesun", 0, 0, 20, 3, "Randomize sun position");
  randomizeSunButton->sig_OnClick.connect(boost::bind(&VisualOptionsPage::OnRandomizeSun, this));

  grid->AddView(kitSelectionCaption1, 0, 0);
  grid->AddView(kitSelectionPulldown[0], 0, 1);
  grid->AddView(kitSelectionCaption2, 1, 0);
  grid->AddView(kitSelectionPulldown[1], 1, 1);
  grid->AddView(randomizeSunButton, 2, 1);

  frame->AddView(grid);
  grid->UpdateLayout(2.0f);
  grid->Show();

  kitSelectionPulldown[0]->SetFocus();

  this->Show();
}

VisualOptionsPage::~VisualOptionsPage() {
}

void VisualOptionsPage::OnRandomizeSun() {
  GetGameTask()->GetMatch()->SetRandomSunParams();
}

void VisualOptionsPage::OnChangeKit(Gui2Pulldown *pulldown) {
  int teamID = atoi(pulldown->GetSelected().substr(4, 1).c_str()) - 1;
  int kitNumber = atoi(pulldown->GetSelected().substr(8, 2).c_str());
  //printf("string, team id, kit number: %s, %i, %i\n", pulldown->GetSelected().c_str(), teamID, kitNumber);
  GetGameTask()->GetMatch()->GetTeam(teamID)->SetKitNumber(kitNumber);
}
