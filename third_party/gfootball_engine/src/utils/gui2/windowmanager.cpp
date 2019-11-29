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

#include "windowmanager.hpp"

#include <cmath>

#include "../../main.hpp"
#include "../../scene/objectfactory.hpp"
#include "widgets/root.hpp"

namespace blunted {

Gui2WindowManager::Gui2WindowManager(boost::shared_ptr<Scene2D> scene2D,
                                     float aspectRatio, float margin)
    : scene2D(scene2D),
      aspectRatio(aspectRatio),
      margin(margin),
      pageFactory(0) {
  DO_VALIDATION;
  root = new Gui2Root(this, "root", 0, 0, 100, 100);
  timeStep_ms = 10;

  focus = root;

  style = new Gui2Style();

  int contextW, contextH, bpp;  // context
  scene2D->GetContextSize(contextW, contextH, bpp);

  contextW -= margin * 2;
  contextH -= margin * 2;

  float contextAspectRatio = (float)contextW / (float)contextH;

  if (contextAspectRatio > aspectRatio) {
    DO_VALIDATION;  // context width is larger than "virtual context's" width,
                    // so cap by height
    effectiveW = contextH * aspectRatio;
    effectiveH = contextH;
  } else {
    effectiveW = contextW;
    effectiveH = contextW / aspectRatio;
  }

  effectiveW = clamp(effectiveW, 1, contextW);
  effectiveH = clamp(effectiveH, 1, contextH);

  // blackout background

  scene2D->GetContextSize(contextW, contextH, bpp);
  SDL_Surface *sdlSurface = CreateSDLSurface(contextW, contextH);

  boost::intrusive_ptr<Resource<Surface> > resource =
      GetContext().surface_manager.Fetch("gui2_blackoutbackground", false,
                                         true);
  Surface *surface = resource->GetResource();

  surface->SetData(sdlSurface);

  blackoutBackground = new Image2D("gui2_blackoutbackground");
  scene2D->CreateSystemObjects(blackoutBackground);
  blackoutBackground->SetImage(resource);
  blackoutBackground->DrawRectangle(0, 0, contextW, contextH, Vector3(0, 0, 0),
                                    255);
  blackoutBackground->OnChange();

  blackoutBackground->SetPosition(0, 0);
  blackoutBackground->Disable();
  scene2D->AddObject(blackoutBackground);

  pagePath = new Gui2PagePath();
}

Gui2WindowManager::~Gui2WindowManager() {
  DO_VALIDATION;
  delete style;
  scene2D->DeleteObject(blackoutBackground);
  blackoutBackground.reset();

  delete pagePath;
}

void Gui2WindowManager::Exit() {
  DO_VALIDATION;
  std::vector<boost::intrusive_ptr<Image2D> > images;
  root->GetImages(images);
  for (unsigned int i = 0; i < images.size(); i++) {
    DO_VALIDATION;
    boost::intrusive_ptr<Image2D> &image = images[i];
    if (image != boost::intrusive_ptr<Image2D>()) {
      DO_VALIDATION;
      Log(e_Warning, "Gui2WindowManager", "Exit",
          "GUI2 image still here on wm exit: " + image->GetName());
    }
  }

  root->Exit();
  delete root;
}

void Gui2WindowManager::SetFocus(Gui2View *view) {
  DO_VALIDATION;
  if (focus == view) return;
  if (focus) {
    DO_VALIDATION;
    focus->SetInFocusPath(false);
    focus->OnLoseFocus();
  }
  focus = view;
  if (focus) {
    DO_VALIDATION;
    focus->SetInFocusPath(true);
    focus->OnGainFocus();
  }
}

void Gui2WindowManager::GetCoordinates(float x_percent, float y_percent, float width_percent, float height_percent, int &x, int &y, int &width, int &height) const {

    int contextW, contextH, bpp; // context
    scene2D->GetContextSize(contextW, contextH, bpp);

    contextW -= margin * 2;
    contextH -= margin * 2;

    float contextAspectRatio = (float)contextW / (float)contextH;

    int startX, startY;
    if (contextAspectRatio > aspectRatio) {
      DO_VALIDATION;  // context width is larger than "virtual context's" width,
                      // so cap by height
      startX = margin + (contextW - effectiveW) * 0.5f;
      startY = margin;
    } else {
      startX = margin;
      startY = margin + (contextH - effectiveH) * 0.5f;
    }

    width = int(std::round(effectiveW * width_percent * 0.01f));
    height = int(std::round(effectiveH * height_percent * 0.01f));

    x = startX + int(std::round(effectiveW * x_percent * 0.01f));
    y = startY + int(std::round(effectiveH * y_percent * 0.01f));

    if (width < 1) width = 1;
    if (height < 1) height = 1;
  }

  float Gui2WindowManager::GetWidthPercent(int pixels) {
    DO_VALIDATION;
    return pixels / (effectiveW * 0.01f);
  }

  float Gui2WindowManager::GetHeightPercent(int pixels) {
    DO_VALIDATION;
    return pixels / (effectiveH * 0.01f);
  }

  boost::intrusive_ptr<Image2D> Gui2WindowManager::CreateImage2D(
      const std::string &name, int width, int height, bool sceneRegister) {
    DO_VALIDATION;

    SDL_Surface *sdlSurface = CreateSDLSurface(width, height);

    boost::intrusive_ptr<Resource<Surface> > resource =
        GetContext().surface_manager.Fetch(name, false, true);
    Surface *surface = resource->GetResource();

    surface->SetData(sdlSurface);

    boost::intrusive_ptr<Image2D> image(new Image2D(name));
    if (sceneRegister) scene2D->CreateSystemObjects(image);
    image->SetImage(resource);

    // temporary hack to hide views that haven't been setposition'ed yet
    int contextW, contextH, bpp; // context
    scene2D->GetContextSize(contextW, contextH, bpp);
    image->SetPosition(contextW, contextH);

    if (sceneRegister) {
      DO_VALIDATION;
      image->Disable();
      scene2D->AddObject(image);
    }

    return image;
  }

  void Gui2WindowManager::UpdateImagePosition(Gui2View *view) const {
    std::vector < boost::intrusive_ptr<Image2D> > images;
    view->GetImages(images);
    for (unsigned int i = 0; i < images.size(); i++) {
      DO_VALIDATION;
      boost::intrusive_ptr<Image2D> &image = images[i];
      if (image != boost::intrusive_ptr<Image2D>()) {
        DO_VALIDATION;
        float x_percent, y_percent;
        int x, y, w, h;
        view->GetDerivedPosition(x_percent, y_percent);
        GetCoordinates(x_percent, y_percent, 0, 0, x, y, w, h);
        image->SetPosition(x, y);
      }
    }
  }

  void Gui2WindowManager::RemoveImage(boost::intrusive_ptr<Image2D> image) const {
    scene2D->DeleteObject(image);
  }

  void Gui2WindowManager::Show(Gui2View *view) {
    DO_VALIDATION;
    std::vector < boost::intrusive_ptr<Image2D> > images;
    view->GetImages(images);
    for (unsigned int i = 0; i < images.size(); i++) {
      DO_VALIDATION;
      boost::intrusive_ptr<Image2D> &image = images[i];
      if (image != boost::intrusive_ptr<Image2D>()) {
        DO_VALIDATION;
        image->Enable();
      }
    }
  }

  void Gui2WindowManager::Hide(Gui2View *view) {
    DO_VALIDATION;
    std::vector < boost::intrusive_ptr<Image2D> > images;
    view->GetImages(images);
    for (unsigned int i = 0; i < images.size(); i++) {
      DO_VALIDATION;
      boost::intrusive_ptr<Image2D> &image = images[i];
      if (image != boost::intrusive_ptr<Image2D>()) {
        DO_VALIDATION;
        image->Disable();
      }
    }
  }
}
