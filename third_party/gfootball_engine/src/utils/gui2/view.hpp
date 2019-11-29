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

#ifndef _HPP_GUI2_VIEW
#define _HPP_GUI2_VIEW

#include "../../defines.hpp"

#include "../../scene/objects/image2d.hpp"

namespace blunted {

  class Gui2WindowManager;

  class Gui2View {

    public:
      Gui2View(Gui2WindowManager *windowManager, const std::string &name, float x_percent, float y_percent, float width_percent, float height_percent);
      virtual ~Gui2View();

      virtual void Exit();

      virtual void UpdateImagePosition();
      virtual void UpdateImageVisibility();
      virtual void AddView(Gui2View *view);
      virtual void RemoveView(Gui2View *view);
      virtual void SetParent(Gui2View *view);
      virtual Gui2View *GetParent();
      virtual void SetPosition(float x_percent, float y_percent);
      virtual void GetSize(float &width_percent, float &height_percent) const;
      virtual void GetPosition(float &x_percent, float &y_percent) const;
      virtual void GetDerivedPosition(float &x_percent, float &y_percent) const;
      virtual void CenterPosition();
      virtual void GetImages(std::vector < boost::intrusive_ptr<Image2D> > &target);

      virtual void Redraw() { DO_VALIDATION;}

      bool IsFocussed();
      void SetFocus();
      virtual void OnGainFocus() { DO_VALIDATION; if (!children.empty()) children.at(0)->SetFocus(); }
      virtual void OnLoseFocus() { DO_VALIDATION;}
      virtual void SetInFocusPath(bool onOff) { DO_VALIDATION;
        isInFocusPath = onOff;
        if (parent) parent->SetInFocusPath(onOff);
      }

      virtual bool IsVisible() { DO_VALIDATION; if (isVisible) { DO_VALIDATION; if (parent) return parent->IsVisible(); else return true; } else return false; }
      virtual bool IsSelectable() { DO_VALIDATION; return isSelectable; }
      virtual bool IsOverlay() { DO_VALIDATION; return isOverlay; }

      virtual void Show();
      virtual void ShowAllChildren();
      virtual void Hide();

      void SetRecursiveZPriority(int prio);
      virtual void SetZPriority(int prio);

    protected:
      Gui2WindowManager *windowManager;
      std::string name;
      Gui2View *parent;

      std::vector<Gui2View*> children;
      bool exit_called = false;

      float x_percent = 0.0f;
      float y_percent = 0.0f;
      float width_percent = 0.0f;
      float height_percent = 0.0f;

      bool isVisible = false;
      bool isSelectable = false;
      bool isInFocusPath = false;
      bool isOverlay = false;
  };

}

#endif
