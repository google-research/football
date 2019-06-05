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

#include "iconselector.hpp"

#include <cmath>

#include "../windowmanager.hpp"

namespace blunted {

  Gui2IconSelector::Gui2IconSelector(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &caption) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent), caption(caption) {

    isSelectable = true;

    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);
    image = windowManager->CreateImage2D(name + "bg", w, h, true);

    Gui2Caption *captionView = new Gui2Caption(windowManager, name + "caption", 1, 0.3, width_percent, 2.5, caption);
    this->AddView(captionView);
    captionView->Show();
    selectedCaption = new Gui2Caption(windowManager, name + "selectedcaption", 1, height_percent * 0.83, width_percent, 2.5, "empty");
    this->AddView(selectedCaption);
    selectedCaption->Show();

    selectedEntry = 0;
    visibleSelectedEntry = 0.0;

    fadeOutTime_ms = 200;
    fadeOut_ms = fadeOutTime_ms;

    Redraw();
  }

  Gui2IconSelector::~Gui2IconSelector() {
  }

  void Gui2IconSelector::GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target) {
    target.push_back(image);
    Gui2View::GetImages(target);
  }

  void Gui2IconSelector::Process() {
    //printf("processing %s\n", GetName().c_str());

    float scrollSpeedFactor = 0.5f;

    bool redraw = false;

    if (fadeOut_ms <= fadeOutTime_ms) {
      fadeOut_ms += windowManager->GetTimeStep_ms();
      if (!IsFocussed()) {// && fadeOut_ms <= fadeOutTime_ms) { // cool fadeout effect!
        redraw = true;
      }
    }

    if (visibleSelectedEntry != (float)selectedEntry) redraw = true;
    if (fabs(visibleSelectedEntry - (float)selectedEntry) <= 0.08f * scrollSpeedFactor) {
      visibleSelectedEntry = (float)selectedEntry;
    } else {
      if (visibleSelectedEntry > (float)selectedEntry) visibleSelectedEntry -= 0.08f * scrollSpeedFactor;
      else if (visibleSelectedEntry < (float)selectedEntry) visibleSelectedEntry += 0.08f * scrollSpeedFactor;
    }

    if (redraw) Redraw();

    Gui2View::Process();
  }

  void Gui2IconSelector::Redraw() {

    //printf("redrawing %s\n", GetName().c_str());
    int alpha = 0;
    if (IsFocussed()) {
      alpha = 200;
    } else {
      alpha = int(std::floor(200 - (clamp(fadeOut_ms, 0, fadeOutTime_ms) /
                                    (float)fadeOutTime_ms * 150)));
    }

    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);
    image->DrawRectangle(0, 0, w, h, windowManager->GetStyle()->GetColor(e_DecorationType_Bright2), alpha);
    image->OnChange();

    // calculate sizes and positions for each entry
    for (int i = 0; i < (signed int)entries.size(); i++) {
      float position = i - visibleSelectedEntry;
      float xpos = width_percent * 0.5;
      if (position > 0.5 * -pi && position < 0.5 * pi) {
        float size = fabs(std::cos(position));
        xpos += std::sin(position) * width_percent * 0.4;
        // only resize internals, for speed (no texture recreate)
        entries.at(i).icon->SetPosition(xpos - 8.0 * 0.5, height_percent * 0.5 - 10.0 * 0.5);
        entries.at(i).icon->SetZoom(size, size);
      } else {
        entries.at(i).icon->SetPosition(100, 100);
      }
    }

    if (entries.size() != 0) {
      if (selectedCaption->GetCaption() != entries.at(selectedEntry).caption) {
        selectedCaption->SetCaption(entries.at(selectedEntry).caption);
      }
    } else selectedCaption->SetCaption("None selected");
  }

  void Gui2IconSelector::ClearEntries() {
    for (int i = 0; i < (signed int)entries.size(); i++) {
      entries.at(i).icon->Exit();
      delete entries.at(i).icon;
    }

    entries.clear();

    selectedEntry = 0;
    visibleSelectedEntry = 0.0;

    Redraw();
  }

  void Gui2IconSelector::AddEntry(const std::string &id, const std::string &caption, const std::string &imageFile) {
    Gui2IconSelectorEntry entry;
    entry.caption = caption;
    entry.id = id;
    entry.icon = new Gui2Image(windowManager, name + "_entry_" + id, 0, 0, 8, 10);
    entry.icon->LoadImage(imageFile);
    AddView(entry.icon);
    entry.icon->Show();
    entries.push_back(entry);

    Redraw();
  }

  void Gui2IconSelector::ProcessWindowingEvent(WindowingEvent *event) {
    event->Ignore();

    if (visibleSelectedEntry == (float)selectedEntry) {
      Vector3 direction = event->GetDirection();
      int xoffset = 0;
      if (direction.coords[0] < -0.75) xoffset = -1;
      if (direction.coords[0] > 0.75) xoffset = 1;
      if (xoffset != 0) event->Accept();

      int prevSelectedEntry = selectedEntry;
      selectedEntry += xoffset;
      if (selectedEntry > (signed int)entries.size() - 1) selectedEntry = (signed int)entries.size() - 1;
      if (selectedEntry < 0) selectedEntry = 0;
      if (selectedEntry != prevSelectedEntry) sig_OnChange();
    }

    if (event->IsActivate()) {
      sig_OnClick();
      event->Accept();
    }
  }

  void Gui2IconSelector::OnGainFocus() {
    Redraw();
  }

  void Gui2IconSelector::OnLoseFocus() {
    fadeOut_ms = 0;
  }

}
