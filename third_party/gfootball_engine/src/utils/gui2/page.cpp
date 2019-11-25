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

#include "page.hpp"

#include "windowmanager.hpp"
#include "../../base/utils.hpp"

namespace blunted {

Gui2Page::Gui2Page(Gui2WindowManager *windowManager,
                   const Gui2PageData &pageData)
    : Gui2Frame(windowManager, "page_" + int_to_str(pageData.pageID), 0, 0, 100,
                100),
      pageData(pageData) {
  DO_VALIDATION;
}

Gui2Page::~Gui2Page() { DO_VALIDATION; }

Gui2PageFactory::Gui2PageFactory() { DO_VALIDATION; }

Gui2PageFactory::~Gui2PageFactory() { DO_VALIDATION; }

void Gui2PageFactory::SetWindowManager(Gui2WindowManager *wm) {
  DO_VALIDATION;
  windowManager = wm;
}

Gui2Page *Gui2PageFactory::CreatePage(int pageID, void *data) {
  DO_VALIDATION;
  Gui2PageData pageData;
  pageData.pageID = pageID;
  Gui2Page *page = CreatePage(pageData);
  return page;
  // not going to work: pointer is not persistent, while pagedata is.
  // pageData.pagePointer = CreatePage(pageData);
  // printf("page created, id %i\n", pageData.pageID);
  // return pageData.pagePointer;
}
}
