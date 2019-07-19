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

#include "vertexbuffer.hpp"

#include "../rendering/interface_renderer3d.hpp"
#include "../rendering/r3d_messages.hpp"

namespace blunted {

  VertexBuffer::VertexBuffer() : verticesDataSize(0), vertexCount(0), dynamicBuffer(false), sizeChanged(false) {
    //printf("CREATING VertexBufferID\n");
    assert(vertexBufferID.bufferID == -1);
  }

  VertexBuffer::~VertexBuffer() {
    if (vertexBufferID.bufferID != -1) {
      renderer3D->DeleteVertexBuffer(vertexBufferID);
    }
  }

  void VertexBuffer::SetTriangleMesh(const std::vector<float>& vertices, unsigned int verticesDataSize, std::vector<unsigned int> indices) {
    this->vertices = vertices;
    TriangleMeshWasUpdatedExternally(verticesDataSize, indices);
  }

  void VertexBuffer::TriangleMeshWasUpdatedExternally(unsigned int verticesDataSize, std::vector<unsigned int> indices) {
    //printf("%i == %i ?  %i == %i ?\n", this->indices.size(), indices.size(), vertexCount, verticesDataSize / GetTriangleMeshElementCount() / 3);
    if (indices.size() > 0) {
      if (indices.size() != this->indices.size()) sizeChanged = true;
      this->indices = indices;
    }
    this->verticesDataSize = verticesDataSize;
    int tmpVertexCount = verticesDataSize / GetTriangleMeshElementCount() / 3;
    if (tmpVertexCount != vertexCount) sizeChanged = true;
    vertexCount = tmpVertexCount;
  }

  VertexBufferID VertexBuffer::CreateOrUpdateVertexBuffer(Renderer3D *renderer3D, bool dynamicBuffer) {
    this->renderer3D = renderer3D;

    // changed size? delete vbo first!
    if (vertexBufferID.bufferID != -1 && sizeChanged) {
      renderer3D->DeleteVertexBuffer(vertexBufferID);
      vertexBufferID.bufferID = -1;
    }

    if (vertexBufferID.bufferID == -1) {
      this->dynamicBuffer = dynamicBuffer;
      e_VertexBufferUsage usage = e_VertexBufferUsage_StaticDraw;
      if (dynamicBuffer) usage = e_VertexBufferUsage_DynamicDraw;
      vertexBufferID = renderer3D->CreateVertexBuffer(&vertices[0], verticesDataSize, indices, usage);
    } else {
      renderer3D->UpdateVertexBuffer(vertexBufferID, vertices.data(), verticesDataSize);
    }

    sizeChanged = false;

    return vertexBufferID;
  }

  float* VertexBuffer::GetTriangleMesh() {
    return &vertices[0];
  }

  int VertexBuffer::GetVaoID() {
    return vertexBufferID.vertexArrayID;
  }

  int VertexBuffer::GetVerticesDataSize() {
    return verticesDataSize;
  }

}
