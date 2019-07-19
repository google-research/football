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

#ifndef _HPP_GUI2_WINDOWMANAGER
#define _HPP_GUI2_WINDOWMANAGER

#include "../../defines.hpp"

#include "../../scene/scene2d/scene2d.hpp"

#include "../../scene/objects/image2d.hpp"

#include "view.hpp"
#include "widgets/root.hpp"

#include "style.hpp"

#include "page.hpp"

namespace blunted {

  class Gui2WindowManager {

    public:
      Gui2WindowManager(boost::shared_ptr<Scene2D> scene2D, float aspectRatio, float margin);
      virtual ~Gui2WindowManager();

      void Exit();

      Gui2Root *GetRoot() { return root; }
      void SetFocus(Gui2View *view);

      void Process();

      void GetCoordinates(float x_percent, float y_percent, float width_percent,
                          float height_percent, int &x, int &y, int &width,
                          int &height) const;
      float GetWidthPercent(int pixels);
      float GetHeightPercent(int pixels);
      boost::intrusive_ptr<Image2D> CreateImage2D(const std::string &name, int width, int height, bool sceneRegister = false);
      void UpdateImagePosition(Gui2View *view) const;
      void RemoveImage(boost::intrusive_ptr<Image2D> image) const;

      void MarkForDeletion(Gui2View *view);

      void SetTimeStep_ms(unsigned long timeStep_ms) {
        this->timeStep_ms = timeStep_ms;
      };

      bool IsFocussed(Gui2View *view) { if (focus == view) return true; else return false; }

      Gui2Style *GetStyle() { return style; }

      void Show(Gui2View *view);
      void Hide(Gui2View *view);

      float GetAspectRatio() const { return aspectRatio; }

      void SetPageFactory(Gui2PageFactory *factory) { pageFactory = factory; factory->SetWindowManager(this); }
      Gui2PageFactory *GetPageFactory() { return pageFactory; }
      Gui2PagePath *GetPagePath() { return pagePath; }

    protected:
      boost::shared_ptr<Scene2D> scene2D;
      float aspectRatio = 0.0f;
      float margin = 0.0f;
      float effectiveW = 0.0f;
      float effectiveH = 0.0f;

      boost::intrusive_ptr<Image2D> blackoutBackground;

      Gui2Root *root;
      Gui2View *focus;

      std::vector<Gui2View*> pendingDelete;

      unsigned long timeStep_ms = 0;

      Gui2Style *style;

      Gui2PageFactory *pageFactory;
      Gui2PagePath *pagePath;

  };

}

#endif
