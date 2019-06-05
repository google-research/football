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

#include "filebrowser.hpp"

#define BOOST_FILESYSTEM_NO_DEPRECATED
#define BOOST_FILESYSTEM_VERSION 3
#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

namespace blunted {

  Gui2FileBrowser::Gui2FileBrowser(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent, const std::string &startDir, e_DirEntryType selectType) : Gui2View(windowManager, name, x_percent, y_percent, width_percent, height_percent), directory(startDir), selectType(selectType) {
    clickedID = -1;

    grid = new Gui2Grid(windowManager, name + "_grid", 0, 0, width_percent, height_percent);
    this->AddView(grid);

    DisplayDirectory();
  }

  Gui2FileBrowser::~Gui2FileBrowser() {
  }

  void Gui2FileBrowser::OnGainFocus() {
    Gui2View *target = grid->FindView(0, 0);
    if (target) target->SetFocus();
  }

  void Gui2FileBrowser::OnClick() {
    Gui2View *selected = grid->GetSelectedView();
    clickedID = -1;
    for (unsigned int i = 0; i < dirContents.size(); i++) {
      if (selected == dirContents.at(i).button) clickedID = i;
    }
    sig_OnClick(this);
  }

  DirEntry Gui2FileBrowser::GetClickedEntry() {
    if (clickedID != -1) {
      return dirContents.at(clickedID);
    } else {
      DirEntry empty;
      return empty;
    }
  }

  void Gui2FileBrowser::DisplayDirectory() {
    GetDirectoryContents();

    clickedID = -1;

    for (unsigned int i = 0; i < dirContents.size(); i++) {
      Vector3 color;
      if (dirContents.at(i).type == e_DirEntryType_File) color = Vector3(240, 140, 60);
                                                    else color = Vector3(80, 140, 255);
      Gui2Button *button = new Gui2Button(windowManager, this->GetName() + "_" + dirContents.at(i).name, 0, 0, width_percent - 1, 3, dirContents.at(i).name);
      button->sig_OnClick.connect(boost::bind(&Gui2FileBrowser::OnClick, this));
      button->SetColor(color);
      grid->AddView(button, i, 0);
      dirContents.at(i).button = button;
    }

    grid->SetMaxVisibleRows(16);
    grid->UpdateLayout(0.5);
  }

  bool DirSortFunc(const DirEntry &a, const DirEntry &b) {
    if (a.type != b.type) return a.type > b.type;
    else return a.name < b.name;
  }

  void Gui2FileBrowser::GetDirectoryContents() {
    dirContents.clear();

    if (selectType == e_DirEntryType_File) {
      DirEntry entry;
      entry.name = "../";
      entry.type = e_DirEntryType_Directory;
      dirContents.push_back(entry);
    }

    fs::path path = fs::system_complete(fs::path(directory.c_str()));
    if (!fs::exists(path) || !fs::is_directory(path)) Log(e_Error, "GuiFileBrowser", "GetDirectoryContents", "Could not open directory " + directory + " for reading");

    fs::directory_iterator dirIter(path);
    fs::directory_iterator endIter;
    while (dirIter != endIter) {
      DirEntry entry;
      entry.name = dirIter->path().filename().string(); // used to be .native() .. todo, check out why
      entry.type = is_directory(dirIter->status()) ? e_DirEntryType_Directory : e_DirEntryType_File;
      dirContents.push_back(entry);

      dirIter++;
    }

    std::sort(dirContents.begin(), dirContents.end(), DirSortFunc);
  }

}
