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

#ifndef _HPP_GUI2_VIEW_PAGE
#define _HPP_GUI2_VIEW_PAGE

#include "../../base/properties.hpp"
#include "widgets/frame.hpp"

namespace blunted {

  class Gui2WindowManager;
  class Gui2Page;

  struct Gui2PageData {
    int pageID = 0;
    boost::shared_ptr<Properties> properties;
    void *data;
  };

  class Gui2Page : public Gui2Frame {

    public:
      Gui2Page(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
      virtual ~Gui2Page();

      void GoBack();
      virtual void ProcessWindowingEvent(WindowingEvent *event);

      // moved to View class: boost::signals2::signal<void()> sig_OnClose;

    protected:
      Gui2PageData pageData;

  };

  class Gui2PageFactory {

    public:
      Gui2PageFactory();
      virtual ~Gui2PageFactory();

      virtual void SetWindowManager(Gui2WindowManager *wm);

      virtual Gui2Page *CreatePage(int pageID, const Properties &properties, void *data = 0);
      virtual Gui2Page *CreatePage(const Gui2PageData &pageData) = 0;
      virtual Gui2Page *GetMostRecentlyCreatedPage() = 0;

    protected:
      Gui2WindowManager *windowManager;
  };

  class Gui2PagePath {

    public:
      Gui2PagePath() {};
      virtual ~Gui2PagePath() { Clear(); };

      bool Empty() { return path.empty(); }
      void Push(const Gui2PageData &pageData,
                Gui2Page *mostRecentlyCreatedPage) {
        CHECK(!this->mostRecentlyCreatedPage);
        path.push_back(pageData);
        this->mostRecentlyCreatedPage = mostRecentlyCreatedPage;
      }
      void Pop() {
        path.pop_back();
      }
      Gui2PageData GetLast() {
        return path.back();
      }
      void Clear() {
        DeleteCurrent();
        path.clear();
      }
      Gui2Page* GetMostRecentlyCreatedPage() {
        return mostRecentlyCreatedPage;
      }
      void DeleteCurrent() {
        if (mostRecentlyCreatedPage) {
          mostRecentlyCreatedPage->Exit();
          delete mostRecentlyCreatedPage;
          mostRecentlyCreatedPage = nullptr;
        }
      }
    protected:
      std::vector<Gui2PageData> path;
      Gui2Page *mostRecentlyCreatedPage = nullptr;
  };

}

#endif
