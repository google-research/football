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

#ifndef _HPP_GRAPHICS3D_OPENGL
#define _HPP_GRAPHICS3D_OPENGL

#include "interface_renderer3d.hpp"

#include <GLee.h>

namespace blunted {

  class OpenGLRenderer3D : public Renderer3D {

    public:
      OpenGLRenderer3D();
      virtual const screenshoot& GetScreen();
      virtual ~OpenGLRenderer3D();

      virtual void SwapBuffers();

      //virtual void SetMatrixMode(e_MatrixMode matrixMode); // TO BE DEPRECATED
      //virtual void IdentityMatrix(); // TO BE DEPRECATED
      virtual void LoadMatrix(const Matrix4 &mat); // TO BE DEPRECATED
      virtual Matrix4 GetMatrix(e_MatrixMode matrixMode) const; // TO BE DEPRECATED
      virtual void SetMatrix(const std::string &shaderUniformName, const Matrix4 &matrix);

      virtual void RenderOverlay2D(const std::vector<Overlay2DQueueEntry> &overlay2DQueue);
      virtual void RenderOverlay2D();
      virtual void RenderLights(std::deque<LightQueueEntry> &lightQueue, const Matrix4 &projectionMatrix, const Matrix4 &viewMatrix);


      // --- new & improved

      // init & exit
      virtual bool CreateContext(int width, int height, int bpp, bool fullscreen);
      virtual void Exit();

      virtual int CreateView(float x_percent, float y_percent, float width_percent, float height_percent);
      virtual View &GetView(int viewID);
      virtual void DeleteView(int viewID);

      // general
      virtual void SetCullingMode(e_CullingMode cullingMode);
      virtual void SetBlendingMode(e_BlendingMode blendingMode);
      virtual void SetDepthFunction(e_DepthFunction depthFunction);
      virtual void SetDepthTesting(bool OnOff);
      virtual void SetDepthMask(bool OnOff);
      virtual void SetBlendingFunction(e_BlendingFunction blendingFunction1, e_BlendingFunction blendingFunction2);
      virtual void SetTextureMode(e_TextureMode textureMode);
      virtual void SetColor(const Vector3 &color, float alpha);
      virtual void SetColorMask(bool r, bool g, bool b, bool alpha);

      virtual void ClearBuffer(const Vector3 &color, bool clearDepth, bool clearColor);

      virtual Matrix4 CreatePerspectiveMatrix(float aspectRatio, float nearCap = -1, float farCap = -1);
      virtual Matrix4 CreateOrthoMatrix(float left, float right, float bottom, float top, float nearCap = -1, float farCap = -1);

      // vertex buffers
      virtual VertexBufferID CreateVertexBuffer(float *vertices, unsigned int verticesDataSize, const std::vector<unsigned int>& indices, e_VertexBufferUsage usage);
      virtual void UpdateVertexBuffer(VertexBufferID vertexBufferID, float *vertices, unsigned int verticesDataSize);
      virtual void DeleteVertexBuffer(VertexBufferID vertexBufferID);
      virtual void RenderVertexBuffer(const std::deque<VertexBufferQueueEntry> &vertexBufferQueue, e_RenderMode renderMode = e_RenderMode_Full);
      virtual void RenderAABB(std::list<VertexBufferQueueEntry> &vertexBufferQueue);
      virtual void RenderAABB(std::list<LightQueueEntry> &lightQueue);

      // lights
      virtual void SetLight(const Vector3 &position, const Vector3 &color, float radius);

      // textures
      virtual int CreateTexture(e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, int width, int height, bool alpha = false, bool repeat = true, bool mipmaps = true, bool filter = true, bool multisample = false, bool compareDepth = false);
      virtual void ResizeTexture(int textureID, SDL_Surface *source, e_InternalPixelFormat internalPixelFormat, e_PixelFormat pixelFormat, bool alpha = false, bool mipmaps = true);
      virtual void UpdateTexture(int textureID, SDL_Surface *source, bool alpha = false, bool mipmaps = true);
      virtual void DeleteTexture(int textureID);
      virtual void CopyFrameBufferToTexture(int textureID, int width, int height);
      virtual void BindTexture(int textureID);
      virtual void SetTextureUnit(int textureUnit);
      virtual void SetClientTextureUnit(int textureUnit);

      // frame buffers
      virtual int CreateFrameBuffer();
      virtual void DeleteFrameBuffer(int fbID);
      virtual void BindFrameBuffer(int fbID);
      virtual void SetFrameBufferRenderBuffer(e_TargetAttachment targetAttachment, int rbID);
      virtual void SetFrameBufferTexture2D(e_TargetAttachment targetAttachment, int texID);
      virtual bool CheckFrameBufferStatus();
      virtual void SetFramebufferGammaCorrection(bool onOff);

      // render buffers
      virtual int CreateRenderBuffer();
      virtual void DeleteRenderBuffer(int rbID);
      virtual void BindRenderBuffer(int rbID);
      virtual void SetRenderBufferStorage(e_InternalPixelFormat internalPixelFormat, int width, int height);

      // render targets
      virtual void SetRenderTargets(std::vector<e_TargetAttachment> targetAttachments);

      // utility
      virtual void SetFOV(float angle);
      virtual void PushAttribute(int attr);
      virtual void PopAttribute();
      virtual void SetViewport(int x, int y, int width, int height);
      virtual void GetContextSize(int &width, int &height, int &bpp);
      virtual void SetPolygonOffset(float scale, float bias);

      // shaders
      virtual void LoadShader(const std::string &name, const std::string &filename);
      virtual void UseShader(const std::string &name);
      virtual void SetUniformInt(const std::string &shaderName, const std::string &varName, int value);
      virtual void SetUniformInt3(const std::string &shaderName, const std::string &varName, int value1, int value2, int value3);
      virtual void SetUniformFloat(const std::string &shaderName, const std::string &varName, float value);
      virtual void SetUniformFloat2(const std::string &shaderName, const std::string &varName, float value1, float value2);
      virtual void SetUniformFloat3(const std::string &shaderName, const std::string &varName, float value1, float value2, float value3);
      virtual void SetUniformFloat3Array(const std::string &shaderName, const std::string &varName, int count, float *values);
      virtual void SetUniformMatrix4(const std::string &shaderName, const std::string &varName, const Matrix4 &mat);

      virtual void HDRCaptureOverallBrightness();
      virtual float HDRGetOverallBrightness();

    protected:
      SDL_GLContext context;
      SDL_Window* window;
      int context_width, context_height, context_bpp;

      float cameraNear = 0.0f;
      float cameraFar = 0.0f;

      int noiseTexID = 0;

      float FOV = 0.0f;

      float overallBrightness = 0.0f;

      GLfloat largest_supported_anisotropy = 0.0f;

      std::map<std::string, GLint> uniformCache;

      std::map<int, int> VBOPingPongMap;
      std::map<int, int> VAOPingPongMap;
      std::map<int, int> VAOReadIndex;

      signed int _cache_activeTextureUnit = 0;
      screenshoot last_screen_;

  };

#ifdef WIN32
  static SDL_SysWMinfo wmInfo;
#endif

}

#endif
