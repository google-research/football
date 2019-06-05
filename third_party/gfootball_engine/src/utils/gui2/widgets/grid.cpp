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

#include "grid.hpp"

#include "../windowmanager.hpp"

namespace blunted {

  Gui2Grid::Gui2Grid(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent) {
    int x, y, w, h;
    windowManager->GetCoordinates(x_percent, y_percent, width_percent, height_percent, x, y, w, h);

    rows = 0;
    cols = 0;

    selectedRow = 0;
    selectedCol = 0;

    minSwitchDelay_ms = 180;
    switchDelay_ms = minSwitchDelay_ms;

    offsetRows = 0;
    maxVisibleRows = 10000;

    quickScroll = false;
    rowWrap = true;
    colWrap = true;
    hasSelectables = false;

    margin_left_percent = 0.5f;
    margin_right_percent = 0.5f;
    margin_top_percent = 0.5f;
    margin_bottom_percent = 0.5f;

    scrollX = 0;
    scrollY = 0;
  }

  Gui2Grid::~Gui2Grid() {
  }

  void Gui2Grid::Process() {
    if (switchDelay_ms < minSwitchDelay_ms) switchDelay_ms += windowManager->GetTimeStep_ms();

    Gui2View::Process();
  }

  void Gui2Grid::AddView(Gui2View *view, int row, int col) {
    Gui2View::AddView(view);

    int adaptedRow = row;
    int adaptedCol = col;
    if (adaptedRow == -1) adaptedRow = rows;

    GridContainer gridContainer;
    gridContainer.row = adaptedRow;
    gridContainer.col = adaptedCol;
    gridContainer.view = view;
    container.push_back(gridContainer);

    // update maximum grid size?
    if (adaptedRow >= rows) rows = adaptedRow + 1;
    if (adaptedCol >= cols) cols = adaptedCol + 1;

    if (view->IsSelectable()) hasSelectables = true;
  }

  void Gui2Grid::RemoveView(Gui2View *view) {
    std::vector<GridContainer>::iterator iter = container.begin();
    while (iter != container.end()) {
      if (iter->view == view) iter = container.erase(iter); else iter++;
    }
    Gui2View::RemoveView(view);

    hasSelectables = false;
    for (unsigned int i = 0; i < container.size(); i++) {
      if (container.at(i).view->IsSelectable()) {
        hasSelectables = true;
        break;
      }
    }
  }

  void Gui2Grid::RemoveView(int row, int col) {
    Gui2View *view = 0;
    std::vector<GridContainer>::iterator iter = container.begin();
    while (iter != container.end()) {
      if (iter->row == row && iter->col == col) {
        view = iter->view;
        iter = container.erase(iter);
      } else iter++;
    }
    Gui2View::RemoveView(view);
  }

  Gui2View *Gui2Grid::FindView(int row, int col) {
    Gui2View *target = 0;
    for (int i = 0; i < (signed int)container.size(); i++) {
      if (container.at(i).row == row &&
          container.at(i).col == col) {
        target = container.at(i).view;
        break;
      }
    }
    return target;
  }

  int Gui2Grid::GetRow(Gui2View *view) {
    std::vector<GridContainer>::iterator iter = container.begin();
    while (iter != container.end()) {
      if (iter->view == view) {
        return iter->row;
      }
      iter++;
    }
    return -1;
  }

  int Gui2Grid::GetColumn(Gui2View *view) {
    std::vector<GridContainer>::iterator iter = container.begin();
    while (iter != container.end()) {
      if (iter->view == view) {
        return iter->col;
      }
      iter++;
    }
    return -1;
  }

  void Gui2Grid::SetMaxVisibleRows(int visibleRowCount) {
    this->maxVisibleRows = visibleRowCount;
  }

  void Gui2Grid::UpdateLayout(float margin_left_percent, float margin_right_percent, float margin_top_percent, float margin_bottom_percent) {

    if (maxVisibleRows != 10000) UpdateScrolling();

    this->margin_left_percent = margin_left_percent;
    this->margin_right_percent = margin_right_percent;
    this->margin_top_percent = margin_top_percent;
    this->margin_bottom_percent = margin_bottom_percent;

    // all sizes are in percentages

    float widths[cols + 1];
    float heights[rows + 1];
    memset(&widths[0], 0, sizeof(float) * cols);
    memset(&heights[0], 0, sizeof(float) * rows);


    // find max sizes

    for (unsigned int i = 0; i < container.size(); i++) {
      if (container.at(i).row >= offsetRows && container.at(i).row < rows) {
        float width, height;
        container.at(i).view->GetSize(width, height);
        if (widths[container.at(i).col + 1] < width) widths[container.at(i).col + 1] = width;
        if (heights[container.at(i).row + 1] < height) heights[container.at(i).row + 1] = height;
      }
    }


    // accumulate positions

    float currentX = 0;//margin_left_percent;
    for (int x = 0; x < cols + 1; x++) {
      float tmp = widths[x];
      currentX += margin_left_percent;
      widths[x] += currentX;
      currentX += tmp + margin_right_percent;
    }

    float currentY = 0;
    float visibleY = 0;
    for (int y = offsetRows; y < rows + 1; y++) {
      float tmp = heights[y];
      currentY += margin_top_percent;
      heights[y] += currentY;
      currentY += tmp + margin_bottom_percent;
      if (y - offsetRows <= maxVisibleRows) visibleY = currentY;
    }

    this->SetSize(currentX, visibleY);


    // toss resulting positions towards contained views

    for (unsigned int i = 0; i < container.size(); i++) {
      if (container.at(i).row >= offsetRows && container.at(i).row < rows) {
        container.at(i).view->SetPosition(widths[container.at(i).col], heights[container.at(i).row]);
      }
      //printf("%f, %f\n", widths[container.at(i).col], heights[container.at(i).row]);
    }

    UpdateScrollbars();
  }

  void Gui2Grid::UpdateScrolling() {
    if (selectedRow >= offsetRows + maxVisibleRows) {
      offsetRows = selectedRow - maxVisibleRows + 1;
    }

    if (selectedRow < offsetRows) {
      offsetRows = selectedRow;
    }

    for (unsigned int i = 0; i < container.size(); i++) {
      if (container.at(i).row >= offsetRows && container.at(i).row < offsetRows + maxVisibleRows) container.at(i).view->Show();
                                                                                             else container.at(i).view->Hide();
    }
  }

  void Gui2Grid::UpdateScrollbars() {
    return; // disabled: grids with scrollbars somehow crash on deletion
    if (rows > maxVisibleRows) {
      if (scrollY == 0) {
        scrollY = new Gui2Scrollbar(windowManager, name + "_scroll_vert", x_percent + width_percent - 1, y_percent, 1, height_percent);
        Gui2View::AddView(scrollY);
        scrollY->Show();
      }
      scrollY->SetSizePercent((maxVisibleRows / (float)rows) * 100);
      scrollY->SetProgressPercent((offsetRows / (float)(rows - maxVisibleRows)) * 100);
      scrollY->Redraw();
    } else {
      if (scrollY != 0) {
        this->RemoveView(scrollY);
        scrollY->Exit();
        delete scrollY;
        scrollY = 0;
      }
    }

  }

  void Gui2Grid::ProcessWindowingEvent(WindowingEvent *event) {

    if (container.size() == 0) return;

    // check if someone else changed the focus and update likewise
    for (int i = 0; i < (signed int)container.size(); i++) {
      if (container.at(i).view->IsFocussed()) {
        if (container.at(i).row != selectedRow) selectedRow = container.at(i).row;
        if (container.at(i).col != selectedCol) selectedCol = container.at(i).col;
        break;
      }
    }

    Vector3 direction = event->GetDirection();
    int xoffset = 0;
    int yoffset = 0;
    if (direction.coords[0] < -0.75) xoffset = -1;
    if (direction.coords[0] > 0.75) xoffset = 1;
    if (direction.coords[1] < -0.75) yoffset = -1;
    if (direction.coords[1] > 0.75) yoffset = 1;

    if (xoffset != 0 || yoffset != 0) event->Accept(); else event->Ignore();

    bool movedOutOfGrid = false; // maybe we need to ignore this event so parent can receive it instead

    if ((xoffset != 0 || yoffset != 0) && switchDelay_ms >= minSwitchDelay_ms && hasSelectables) {
      movedOutOfGrid = false;

      int provisionalSelectedRow = selectedRow;
      int provisionalSelectedCol = selectedCol;

      Gui2View *provisionalTarget = 0;
      bool selectableFound = false;
      while (!selectableFound && !movedOutOfGrid) {

        provisionalSelectedRow += yoffset;

        if (rowWrap) {
          if (provisionalSelectedRow > rows - 1) provisionalSelectedRow = 0;
          if (provisionalSelectedRow < 0) provisionalSelectedRow = rows - 1;
        } else {
          if (provisionalSelectedRow > rows - 1) {
            provisionalSelectedRow = rows - 1;
            movedOutOfGrid = true;
          }
          if (provisionalSelectedRow < 0) {
            provisionalSelectedRow = 0;
            movedOutOfGrid = true;
          }
        }

        provisionalSelectedCol += xoffset;

        if (colWrap) {
          if (provisionalSelectedCol > cols - 1) provisionalSelectedCol = 0;
          if (provisionalSelectedCol < 0) provisionalSelectedCol = cols - 1;
        } else {
          if (provisionalSelectedCol > cols - 1) {
            provisionalSelectedCol = cols - 1;
            movedOutOfGrid = true;
          }
          if (provisionalSelectedCol < 0) {
            provisionalSelectedCol = 0;
            movedOutOfGrid = true;
          }

        }

        provisionalTarget = FindView(provisionalSelectedRow, provisionalSelectedCol);

        if (provisionalTarget)
          if (provisionalTarget->IsSelectable()) selectableFound = true;
      }

      if (selectableFound && !movedOutOfGrid) {
        if (!provisionalTarget->IsFocussed()) provisionalTarget->SetFocus();
        selectedRow = provisionalSelectedRow;
        selectedCol = provisionalSelectedCol;
        switchDelay_ms = 0;
      }

      UpdateLayout(margin_left_percent, margin_right_percent, margin_top_percent, margin_bottom_percent);

    } // something happened

    if (movedOutOfGrid == true) {
      event->Ignore();
      switchDelay_ms = 0;
    }

  }

  void Gui2Grid::OnGainFocus() {
    switchDelay_ms = 0;

    if (hasSelectables) {
      for (unsigned int i = 0; i < container.size(); i++) {
        if (container.at(i).view->IsSelectable()) {
          container.at(i).view->SetFocus();
          selectedRow = container.at(i).row;
          selectedCol = container.at(i).col;
          break;
        }
      }
    }
  }

  void Gui2Grid::SetInFocusPath(bool onOff) {
    Gui2View::SetInFocusPath(onOff);

    if (onOff == true) {

      if (hasSelectables) {
        for (unsigned int i = 0; i < container.size(); i++) {
          if (container.at(i).view->IsInFocusPath()) {
            selectedRow = container.at(i).row;
            selectedCol = container.at(i).col;
            break;
          }
        }
      }
    }

  }

  void Gui2Grid::Show() {
    for (int i = 0; i < (signed int)container.size(); i++) {
      if (container.at(i).row >= offsetRows && container.at(i).row < offsetRows + maxVisibleRows) container.at(i).view->Show();
                                                                                             else container.at(i).view->Hide();
    }
    if (scrollX) scrollX->Show();
    if (scrollY) scrollY->Show();

    Gui2View::Show();
  }

  void Gui2Grid::Hide() {
    for (int i = 0; i < (signed int)container.size(); i++) {
      container.at(i).view->Hide();
    }
    if (scrollX) scrollX->Hide();
    if (scrollY) scrollY->Hide();

    Gui2View::Hide();
  }

}
