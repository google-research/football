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

#ifndef _HPP_RENDERER3D_MESSAGES
#define _HPP_RENDERER3D_MESSAGES

#include "interface_renderer3d.hpp"

#include "../../../managers/environmentmanager.hpp"

namespace blunted {

  // 'camera' view
  struct ViewBuffer {
    std::deque<VertexBufferQueueEntry> visibleGeometry;
    std::deque<LightQueueEntry> visibleLights;
    std::deque<VertexBufferQueueEntry> skyboxes;

    Matrix4 cameraMatrix;
    float cameraFOV = 0.0f;
    float cameraNearCap = 0.0f;
    float cameraFarCap = 0.0f;
  };


  // messages

  class Renderer3DMessage_CreateContext : public Command {

    public:
      Renderer3DMessage_CreateContext(int width, int height, int bpp, bool fullscreen) : Command("r3dmsg_CreateContext"), width(width), height(height), bpp(bpp), fullscreen(fullscreen) {};

      // return values
      bool success = false;

    protected:
      virtual bool Execute(void *caller = NULL) {
        success = static_cast<Renderer3D*>(caller)->CreateContext(width, height, bpp, fullscreen);
        return true;
      }

      // parameters
      int width, height, bpp;
      bool fullscreen = false;

  };

  class Renderer3DMessage_SwapBuffers : public Command {

    public:
      Renderer3DMessage_SwapBuffers() : Command("r3dmsg_SwapBuffers"), readyTime_ms(0) {};

      unsigned long GetReadyTime_ms() { return readyTime_ms; }

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->SwapBuffers();
        readyTime_ms = EnvironmentManager::GetInstance().GetTime_ms();
        return true;
      }

      unsigned long readyTime_ms = 0;

  };

  class Renderer3DMessage_DeleteTexture : public Command {

    public:
      Renderer3DMessage_DeleteTexture(int textureID) : Command("r3dmsg_DeleteTexture"), textureID(textureID) {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->DeleteTexture(textureID);

        return true;
      }

      int textureID = 0;

  };

  class Renderer3DMessage_ResizeTexture : public Command {

    public:
      Renderer3DMessage_ResizeTexture(int textureID, SDL_Surface *source, e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, bool alpha = false, bool mipmaps = true) : Command("r3dmsg_ResizeTexture"), textureID(textureID), internalPixelFormat(internalPixelFormat), pixelFormat(pixelFormat), alpha(alpha), mipmaps(mipmaps) {
        // copy image so caller doesn't have to wait for update to complete
        // DAMN YOU SDL! this function won't actually copy right surface, just make a shallow copy instead. that explains a crash i got. fuuufuuuuuu
        //this->source = SDL_CreateRGBSurfaceFrom(source->pixels, source->w, source->h, 0, source->pitch, 0, 0, 0, 0);
        // use this hax instead. really SDL, no deep surface copy function? pfff
        this->source = SDL_ConvertSurface(source, source->format, 0);
        //assert(source->pixels != this->source->pixels);
      }

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->ResizeTexture(textureID, source, internalPixelFormat, pixelFormat, alpha, mipmaps);

        SDL_FreeSurface(source);

        return true;
      }

      int textureID = 0;
      SDL_Surface *source;
      e_InternalPixelFormat internalPixelFormat;
      e_PixelFormat pixelFormat;
      bool alpha, mipmaps;

  };

  class Renderer3DMessage_UpdateTexture : public Command {

    public:
      Renderer3DMessage_UpdateTexture(int textureID, SDL_Surface *source, bool alpha = false, bool mipmaps = true) : Command("r3dmsg_UpdateTexture"), textureID(textureID), alpha(alpha), mipmaps(mipmaps) {
        // copy image so caller doesn't have to wait for update to complete
        // DAMN YOU SDL! this function won't actually copy right surface, just make a shallow copy instead. that explains a crash i got. fuuufuuuuuu
        //this->source = SDL_CreateRGBSurfaceFrom(source->pixels, source->w, source->h, 0, source->pitch, 0, 0, 0, 0);
        // use this hax instead. really SDL, no deep surface copy function? pfff
        this->source = SDL_ConvertSurface(source, source->format, 0);
        //assert(source->pixels != this->source->pixels);
      }

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->UpdateTexture(textureID, source, alpha, mipmaps);

        SDL_FreeSurface(this->source);

        return true;
      }

      int textureID = 0;
      SDL_Surface *source;
      bool alpha, mipmaps;

  };

  class Renderer3DMessage_UpdateVertexBuffer : public Command {

    public:
      Renderer3DMessage_UpdateVertexBuffer(VertexBufferID vertexBufferID, float *vertices, unsigned int verticesDataSize) : Command("r3dmsg_UpdateVertexBuffer"), vertexBufferID(vertexBufferID), verticesDataSize(verticesDataSize) {
        // copying causes terrible last-level cache misses. so use pointer and make sure at client side that we won't touch them in the meantime
        //this->vertices = new float[verticesDataSize];
        //memcpy(this->vertices, vertices, verticesDataSize * sizeof(float));
        this->vertices = vertices;
      }

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->UpdateVertexBuffer(vertexBufferID, vertices, verticesDataSize);

        return true;
      }

      VertexBufferID vertexBufferID;

      float *vertices;
      int verticesDataSize = 0;

  };

  class Renderer3DMessage_DeleteVertexBuffer : public Command {

    public:
      Renderer3DMessage_DeleteVertexBuffer(VertexBufferID vertexBufferID) : Command("r3dmsg_DeleteVertexBuffer"), vertexBufferID(vertexBufferID) {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->DeleteVertexBuffer(vertexBufferID);

        return true;
      }

      VertexBufferID vertexBufferID;

  };

  class Renderer3DMessage_RenderOverlay2D : public Command {

    public:
      Renderer3DMessage_RenderOverlay2D(MessageQueue<Overlay2DQueueEntry> &overlay2DQueue) : Command("r3dmsg_RenderOverlay2D") {
        bool isMessage = true;
        while (isMessage) {
          Overlay2DQueueEntry queueEntry = overlay2DQueue.GetMessage(isMessage);
          if (isMessage) this->overlay2DQueue.push_back(queueEntry);
        }
      };

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->RenderOverlay2D(overlay2DQueue);

        return true;
      }

      std::vector<Overlay2DQueueEntry> overlay2DQueue;

  };

  class Renderer3DMessage_CreateView : public Command {

    public:
      Renderer3DMessage_CreateView(float x_percent, float y_percent, float width_percent, float height_percent) : Command("r3dmsg_CreateView"), x_percent(x_percent), y_percent(y_percent), width_percent(width_percent), height_percent(height_percent) {};

      int viewID = 0;

    protected:
      virtual bool Execute(void *caller = NULL) {
        viewID = static_cast<Renderer3D*>(caller)->CreateView(x_percent, y_percent, width_percent, height_percent);

        return true;
      }

      float x_percent = 0.0f;
      float y_percent = 0.0f;
      float width_percent = 0.0f;
      float height_percent = 0.0f;

  };

  class Renderer3DMessage_RenderView : public Command {

    public:
      // note 7/20/2010: made buffer param into a copy instead of a ref, so caller does not have to wait to clean up the mess
      // thought: could also leave cleaning to this class itself? but that would destroy acquisition == ownership
      // note 6/29/2011: too slow to copy, using a reference again. breaks acq = ownership though :|
      Renderer3DMessage_RenderView(int viewID, ViewBuffer &buffer) : Command("r3dmsg_RenderView"), viewID(viewID), buffer(buffer) {};

    protected:
      virtual bool Execute(void *caller = NULL);

      int viewID = 0;
      ViewBuffer &buffer;

  };

  class Renderer3DMessage_DeleteView : public Command {

    public:
      Renderer3DMessage_DeleteView(int viewID) : Command("r3dmsg_DeleteView"), viewID(viewID) {};

    protected:
      virtual bool Execute(void *caller = NULL) {
        static_cast<Renderer3D*>(caller)->DeleteView(viewID);

        return true;
      }

      int viewID = 0;

  };

  class Renderer3DMessage_RenderShadowMap : public Command {

    public:
      Renderer3DMessage_RenderShadowMap(const ShadowMap &map) : Command("r3dmsg_RenderShadowMap"), map(map) {};

    protected:
      virtual bool Execute(void *caller = NULL);

      const ShadowMap &map;

  };

  class Renderer3DMessage_CreateFrameBuffer : public Command {

    public:
      Renderer3DMessage_CreateFrameBuffer(e_TargetAttachment target1 = e_TargetAttachment_None, int texID1 = 0,
                                          e_TargetAttachment target2 = e_TargetAttachment_None, int texID2 = 0,
                                          e_TargetAttachment target3 = e_TargetAttachment_None, int texID3 = 0,
                                          e_TargetAttachment target4 = e_TargetAttachment_None, int texID4 = 0,
                                          e_TargetAttachment target5 = e_TargetAttachment_None, int texID5 = 0) : Command("r3dmsg_CreateFrameBuffer"),
                                          target1(target1), texID1(texID1),
                                          target2(target2), texID2(texID2),
                                          target3(target3), texID3(texID3),
                                          target4(target4), texID4(texID4),
                                          target5(target5), texID5(texID5) {};

      int frameBufferID = 0;

    protected:
      virtual bool Execute(void *caller = NULL);

      e_TargetAttachment target1;
      int texID1;
      e_TargetAttachment target2;
      int texID2;
      e_TargetAttachment target3;
      int texID3;
      e_TargetAttachment target4;
      int texID4;
      e_TargetAttachment target5;
      int texID5;
  };

  class Renderer3DMessage_DeleteFrameBuffer : public Command {

    public:
      Renderer3DMessage_DeleteFrameBuffer(int frameBufferID,
                                          e_TargetAttachment target1 = e_TargetAttachment_None,
                                          e_TargetAttachment target2 = e_TargetAttachment_None,
                                          e_TargetAttachment target3 = e_TargetAttachment_None,
                                          e_TargetAttachment target4 = e_TargetAttachment_None,
                                          e_TargetAttachment target5 = e_TargetAttachment_None) : Command("r3dmsg_DeleteFrameBuffer"),
                                          frameBufferID(frameBufferID),
                                          target1(target1),
                                          target2(target2),
                                          target3(target3),
                                          target4(target4),
                                          target5(target5) {};

    protected:
      virtual bool Execute(void *caller = NULL);

      int frameBufferID = 0;
      e_TargetAttachment target1, target2, target3, target4, target5;

  };

}

#endif
