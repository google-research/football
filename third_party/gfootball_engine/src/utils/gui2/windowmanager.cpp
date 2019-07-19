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

#include "widgets/root.hpp"

#include "../../managers/resourcemanagerpool.hpp"
#include "../../scene/objectfactory.hpp"

namespace blunted {

  Gui2WindowManager::Gui2WindowManager(boost::shared_ptr<Scene2D> scene2D, float aspectRatio, float margin) : scene2D(scene2D), aspectRatio(aspectRatio), margin(margin), pageFactory(0) {
    root = new Gui2Root(this, "root", 0, 0, 100, 100);
    timeStep_ms = 10;

    focus = root;

    style = new Gui2Style();

    int contextW, contextH, bpp; // context
    scene2D->GetContextSize(contextW, contextH, bpp);

    contextW -= margin * 2;
    contextH -= margin * 2;

    float contextAspectRatio = (float)contextW / (float)contextH;

    if (contextAspectRatio > aspectRatio) { // context width is larger than "virtual context's" width, so cap by height
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

    boost::intrusive_ptr < Resource <Surface> > resource = ResourceManagerPool::getSurfaceManager()->Fetch("gui2_blackoutbackground", false, true);
    Surface *surface = resource->GetResource();

    surface->SetData(sdlSurface);

    blackoutBackground = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject("gui2_blackoutbackground", e_ObjectType_Image2D));
    scene2D->CreateSystemObjects(blackoutBackground);
    blackoutBackground->SetImage(resource);
    blackoutBackground->DrawRectangle(0, 0, contextW, contextH, Vector3(0, 0, 0), 255);
    blackoutBackground->OnChange();

    blackoutBackground->SetPosition(0, 0);
    blackoutBackground->Disable();
    scene2D->AddObject(blackoutBackground);

    pagePath = new Gui2PagePath();
  }

  Gui2WindowManager::~Gui2WindowManager() {
    delete style;
    scene2D->DeleteObject(blackoutBackground);
    blackoutBackground.reset();

    delete pagePath;
  }

  void Gui2WindowManager::Exit() {
    for (unsigned int i = 0; i < pendingDelete.size(); i++) {
      pendingDelete[i]->Exit();
      delete pendingDelete[i];
    }
    pendingDelete.clear();

    std::vector < boost::intrusive_ptr<Image2D> > images;
    root->GetImages(images);
    for (unsigned int i = 0; i < images.size(); i++) {
      boost::intrusive_ptr<Image2D> &image = images[i];
      if (image != boost::intrusive_ptr<Image2D>()) {
        Log(e_Warning, "Gui2WindowManager", "Exit", "GUI2 image still here on wm exit: " + image->GetName());
      }
    }

    root->Exit();
    delete root;
  }

  void Gui2WindowManager::SetFocus(Gui2View *view) {
    if (focus == view) return;
    if (focus) {
      focus->SetInFocusPath(false);
      focus->OnLoseFocus();
    }
    focus = view;
    if (focus) {
      focus->SetInFocusPath(true);
      focus->OnGainFocus();
    }
  }

  void Gui2WindowManager::Process() {
    for (int i = 0; i < (signed int)pendingDelete.size(); i++) {
      pendingDelete[i]->Exit();
      delete pendingDelete[i];
    }
    pendingDelete.clear();

    root->Process();
  }

  void Gui2WindowManager::GetCoordinates(float x_percent, float y_percent, float width_percent, float height_percent, int &x, int &y, int &width, int &height) const {

    int contextW, contextH, bpp; // context
    scene2D->GetContextSize(contextW, contextH, bpp);

    contextW -= margin * 2;
    contextH -= margin * 2;

    float contextAspectRatio = (float)contextW / (float)contextH;

    int startX, startY;
    if (contextAspectRatio > aspectRatio) { // context width is larger than "virtual context's" width, so cap by height
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
    return pixels / (effectiveW * 0.01f);
  }

  float Gui2WindowManager::GetHeightPercent(int pixels) {
    return pixels / (effectiveH * 0.01f);
  }

  boost::intrusive_ptr<Image2D> Gui2WindowManager::CreateImage2D(const std::string &name, int width, int height, bool sceneRegister) {

    SDL_Surface *sdlSurface = CreateSDLSurface(width, height);

    boost::intrusive_ptr < Resource <Surface> > resource = ResourceManagerPool::getSurfaceManager()->Fetch(name, false, true);
    Surface *surface = resource->GetResource();

    surface->SetData(sdlSurface);

    boost::intrusive_ptr<Image2D> image = boost::static_pointer_cast<Image2D>(ObjectFactory::GetInstance().CreateObject(name, e_ObjectType_Image2D));
    if (sceneRegister) scene2D->CreateSystemObjects(image);
    image->SetImage(resource);

    // temporary hack to hide views that haven't been setposition'ed yet
    int contextW, contextH, bpp; // context
    scene2D->GetContextSize(contextW, contextH, bpp);
    image->SetPosition(contextW, contextH);

    if (sceneRegister) {
      image->Disable();
      scene2D->AddObject(image);
    }

    return image;
  }

  void Gui2WindowManager::UpdateImagePosition(Gui2View *view) const {
    std::vector < boost::intrusive_ptr<Image2D> > images;
    view->GetImages(images);
    for (unsigned int i = 0; i < images.size(); i++) {
      boost::intrusive_ptr<Image2D> &image = images[i];
      if (image != boost::intrusive_ptr<Image2D>()) {
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

  void Gui2WindowManager::MarkForDeletion(Gui2View *view) {
    pendingDelete.push_back(view);
  }

  void Gui2WindowManager::Show(Gui2View *view) {
    std::vector < boost::intrusive_ptr<Image2D> > images;
    view->GetImages(images);
    for (unsigned int i = 0; i < images.size(); i++) {
      boost::intrusive_ptr<Image2D> &image = images[i];
      if (image != boost::intrusive_ptr<Image2D>()) {
        image->Enable();
      }
    }
  }

  void Gui2WindowManager::Hide(Gui2View *view) {
    std::vector < boost::intrusive_ptr<Image2D> > images;
    view->GetImages(images);
    for (unsigned int i = 0; i < images.size(); i++) {
      boost::intrusive_ptr<Image2D> &image = images[i];
      if (image != boost::intrusive_ptr<Image2D>()) {
        image->Disable();
      }
    }
  }

}
