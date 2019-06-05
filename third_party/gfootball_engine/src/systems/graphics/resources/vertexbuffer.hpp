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

#ifndef _HPP_SYSTEM_GRAPHICS_RESOURCE_VERTEXBUFFER
#define _HPP_SYSTEM_GRAPHICS_RESOURCE_VERTEXBUFFER

#include "../../../defines.hpp"

#include "../../../base/geometry/trianglemeshutils.hpp"

namespace blunted {

  class Renderer3D;

  struct VertexBufferID {
    VertexBufferID() {
      bufferID = -1;
    }
    int bufferID = 0; // -1 if uninitialized
    unsigned int vertexArrayID = 0;
    unsigned int elementArrayID = 0;
  };

  class VertexBuffer {

    public:
      VertexBuffer();
      virtual ~VertexBuffer();

      void SetTriangleMesh(const std::vector<float>& vertices, unsigned int verticesDataSize, std::vector<unsigned int> indices);
      void TriangleMeshWasUpdatedExternally(unsigned int verticesDataSize, std::vector<unsigned int> indices);
      VertexBufferID CreateOrUpdateVertexBuffer(Renderer3D *renderer3D, bool dynamicBuffer);

      float* GetTriangleMesh();

      int GetVaoID();

      int GetVerticesDataSize();

     protected:
      std::vector<float> vertices;
      int verticesDataSize = 0;
      std::vector<unsigned int> indices;
      VertexBufferID vertexBufferID;
      int vertexCount = 0;
      Renderer3D *renderer3D;
      bool dynamicBuffer = false;

      bool sizeChanged = false;

  };

}

#endif
