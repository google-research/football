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
  };

  class Gui2Page : public Gui2Frame {

    public:
      Gui2Page(Gui2WindowManager *windowManager, const Gui2PageData &pageData);
      virtual ~Gui2Page();
    protected:
      Gui2PageData pageData;

  };

  class Gui2PageFactory {

    public:
      Gui2PageFactory();
      virtual ~Gui2PageFactory();

      virtual void SetWindowManager(Gui2WindowManager *wm);

      virtual Gui2Page *CreatePage(int pageID, void *data = 0);
      virtual Gui2Page *CreatePage(const Gui2PageData &pageData) = 0;

    protected:
      Gui2WindowManager *windowManager;
  };

  class Gui2PagePath {

    public:
      Gui2PagePath() { DO_VALIDATION;};
      virtual ~Gui2PagePath() { DO_VALIDATION; Clear(); };

      bool Empty() { DO_VALIDATION; return path.empty(); }
      void Push(const Gui2PageData &pageData,
                Gui2Page *mostRecentlyCreatedPage) { DO_VALIDATION;
        CHECK(!this->mostRecentlyCreatedPage);
        path.push_back(pageData);
        this->mostRecentlyCreatedPage = mostRecentlyCreatedPage;
      }
      void Pop() { DO_VALIDATION;
        path.pop_back();
      }
      Gui2PageData GetLast() { DO_VALIDATION;
        return path.back();
      }
      void Clear() { DO_VALIDATION;
        DeleteCurrent();
        path.clear();
      }
      Gui2Page* GetMostRecentlyCreatedPage() { DO_VALIDATION;
        return mostRecentlyCreatedPage;
      }
      void DeleteCurrent() { DO_VALIDATION;
        if (mostRecentlyCreatedPage) { DO_VALIDATION;
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
