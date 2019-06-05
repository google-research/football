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

#include "matchoptions.hpp"

#include "../../main.hpp"

#include "../pagefactory.hpp"

using namespace blunted;

MatchOptionsPage::MatchOptionsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  Gui2Image *bg = new Gui2Image(windowManager, "matchoptions_image_bg1", 30, 20, 40, 70);
  this->AddView(bg);
  bg->LoadImage("media/menu/backgrounds/black.png");
  bg->Show();

  Gui2Caption *header = new Gui2Caption(windowManager, "matchoptions_caption", 30, 15, 40, 3, "Match options");
  this->AddView(header);
  header->Show();

  Gui2Grid *grid = new Gui2Grid(windowManager, "matchoptions_grid", 35, 25, 30, 60);

  difficultySlider = new Gui2Slider(windowManager, "matchoptions_slider_difficulty", 0, 0, 29, 6, "difficulty (when HUMAN vs CPU)");
  matchDurationSlider = new Gui2Slider(windowManager, "matchoptions_slider_matchduration", 0, 0, 29, 6, "match duration (5 minutes .. 25 min.)");
  Gui2Button *buttonStart = new Gui2Button(windowManager, "matchoptions_button_start", 0, 0, 29, 3, "Start match");

  float difficulty = GetConfiguration()->GetReal("match_difficulty", _default_Difficulty);
  float matchDuration = GetConfiguration()->GetReal("match_duration", _default_MatchDuration);
  difficultySlider->SetValue(difficulty);
  matchDurationSlider->SetValue(matchDuration);

  grid->AddView(difficultySlider, 0, 0);
  grid->AddView(matchDurationSlider, 1, 0);
  grid->AddView(buttonStart, 2, 0);
  grid->UpdateLayout(0.5);

  this->AddView(grid);
  grid->Show();

  buttonStart->sig_OnClick.connect(boost::bind(&MatchOptionsPage::GoLoadingMatchPage, this));

  buttonStart->SetFocus();

  this->Show();
}

MatchOptionsPage::~MatchOptionsPage() {
}

void MatchOptionsPage::GoLoadingMatchPage() {

  GetConfiguration()->Set("match_difficulty", difficultySlider->GetValue());
  GetConfiguration()->Set("match_duration", matchDurationSlider->GetValue());
  GetConfiguration()->SaveFile(GetConfigFilename());

  Properties properties;
  windowManager->GetPageFactory()->CreatePage((int)e_PageID_LoadingMatch, properties, 0);
}
