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

#include "tacticsdebug.hpp"

#include "../../utils/gui2/windowmanager.hpp"

#include <wrap_SDL.h>

#include "../../gamedefines.hpp"

#include "../../onthepitch/match.hpp"

using namespace blunted;

Gui2TacticsDebug::Gui2TacticsDebug(Gui2WindowManager *windowManager,
                                   const std::string &name, float x_percent,
                                   float y_percent, float width_percent,
                                   float height_percent, Match *match)
    : Gui2View(windowManager, name, x_percent, y_percent, width_percent,
               height_percent) {
  int x, y;  // dummy
  windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);
  image = windowManager->CreateImage2D(name, w, h, true);
  _dirtycache = false;
}

Gui2TacticsDebug::~Gui2TacticsDebug() {
}

void Gui2TacticsDebug::GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target) {
  target.push_back(image);
  Gui2View::GetImages(target);
}

float TacticsDebugID2Y(int id) {
  return 0.2f + id * 1.4f;
}

void Gui2TacticsDebug::Redraw() {

  if (_dirtycache) {

    image->DrawRectangle(0, 0, w, h, Vector3(0), 0);

    for (unsigned int i = 0; i < entries.size(); i++) {
      int dudx, absy, dudw, absh;
      windowManager->GetCoordinates(0, TacticsDebugID2Y(i) + 0.7f, 0, 0.2f, dudx, absy, dudw, absh);
      //printf("i: %i, val: %f, y: %i\n", i, entries.at(i).value[0][1], y);
      assert(entries.at(i).value[0][0] >= 0.0f);
      assert(entries.at(i).value[0][0] <= 1.0f);
      image->DrawRectangle(w * 0.35f * (1.0f - entries.at(i).value[0][0]) - 1.0f, absy, w * 0.35f * entries.at(i).value[0][0], absh, entries.at(i).color[0][0], 100);
      image->DrawRectangle(w * 0.65f, absy, w * 0.35f * entries.at(i).value[0][1] + 1.0f, 2, entries.at(i).color[0][1], 100);

      windowManager->GetCoordinates(0, TacticsDebugID2Y(i) + 0.9f, 0, 0.2f, dudx, absy, dudw, absh);
      //printf("i: %i, val: %f, y: %i\n", i, entries.at(i).value[0][1], y);
      assert(entries.at(i).value[1][0] >= 0.0f);
      assert(entries.at(i).value[1][0] <= 1.0f);
      image->DrawRectangle(w * 0.35f * (1.0f - entries.at(i).value[1][0]) - 1.0f, absy, w * 0.35f * entries.at(i).value[1][0], absh, entries.at(i).color[1][0], 100);
      image->DrawRectangle(w * 0.65f, absy, w * 0.35f * entries.at(i).value[1][1] + 1.0f, 2, entries.at(i).color[1][1], 100);

      windowManager->GetCoordinates(0, TacticsDebugID2Y(i) + 1.1f, 0, 0.2f, dudx, absy, dudw, absh);
      //printf("i: %i, val: %f, y: %i\n", i, entries.at(i).value[0][1], y);
      assert(entries.at(i).value[2][0] >= 0.0f);
      assert(entries.at(i).value[2][0] <= 1.0f);
      image->DrawRectangle(w * 0.35f * (1.0f - entries.at(i).value[2][0]) - 1.0f, absy, w * 0.35f * entries.at(i).value[2][0], absh, entries.at(i).color[2][0], 100);
      image->DrawRectangle(w * 0.65f, absy, w * 0.35f * entries.at(i).value[2][1] + 1.0f, 2, entries.at(i).color[2][1], 100);
    }

    image->OnChange();
    _dirtycache = false;
  }
}

unsigned int Gui2TacticsDebug::AddEntry(const std::string &caption, const Vector3 &color1, const Vector3 &color2, const Vector3 &color3) {
  TacticsDebugEntry entry;
  entry.caption = caption;
  entry.value[0][0] = 0.0f;
  entry.value[0][1] = 0.0f;
  entry.color[0][0] = color1;
  entry.color[0][1] = color1;
  entry.value[1][0] = 0.0f;
  entry.value[1][1] = 0.0f;
  entry.color[1][0] = color2;
  entry.color[1][1] = color2;
  entry.value[2][0] = 0.0f;
  entry.value[2][1] = 0.0f;
  entry.color[2][0] = color3;
  entry.color[2][1] = color3;
  entries.push_back(entry);

  int entryID = entries.size();

  float w, h;
  this->GetSize(w, h);

  float y = TacticsDebugID2Y(entryID);
  Gui2Caption *guiCaption = new Gui2Caption(windowManager, "caption_tacticsdebug_entry_" + int_to_str(entryID), 0, 0, 0, 1.2f, caption);
  guiCaption->SetColor(color3);
  this->AddView(guiCaption);
  guiCaption->SetPosition(w * 0.5f - guiCaption->GetTextWidthPercent() * 0.5f, y - 1.0f);
  guiCaption->Show();

  _dirtycache = true;

  return entryID;
}

void Gui2TacticsDebug::SetValue(unsigned int entryID, int typeID, int teamID, float value) {
  assert(entryID < entries.size());
  if (entries.at(entryID).value[typeID][teamID] != value) {
    entries.at(entryID).value[typeID][teamID] = value;
    _dirtycache = true;
  }
  //printf("set value id %i, type id %i, team id %i, value %f\n", entryID, typeID, teamID, value);
}
