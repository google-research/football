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

#include "graphics_geometry.hpp"

#include "../../../managers/resourcemanagerpool.hpp"

#include "../graphics_scene.hpp"
#include "../graphics_system.hpp"

#include "../../../base/geometry/trianglemeshutils.hpp"
#include "../../../main.hpp"

namespace blunted {

  GraphicsGeometry::GraphicsGeometry(GraphicsScene *graphicsScene) : GraphicsObject(graphicsScene) {
  }

  GraphicsGeometry::~GraphicsGeometry() {
  }

  boost::intrusive_ptr<Interpreter> GraphicsGeometry::GetInterpreter(e_ObjectType objectType) {
    if (objectType == e_ObjectType_Geometry) {
      boost::intrusive_ptr<GraphicsGeometry_GeometryInterpreter> geometryInterpreter(new GraphicsGeometry_GeometryInterpreter(this));
      return geometryInterpreter;
    }
    else if (objectType == e_ObjectType_Skybox) {
      boost::intrusive_ptr<GraphicsGeometry_SkyboxInterpreter> skyboxInterpreter(new GraphicsGeometry_SkyboxInterpreter(this));
      return skyboxInterpreter;
    }
    Log(e_FatalError, "GraphicsGeometry", "GetInterpreter", "No appropriate interpreter found for this ObjectType");
    return boost::intrusive_ptr<GraphicsGeometry_GeometryInterpreter>();
  }

  void GraphicsGeometry::SetPosition(const Vector3 &newPosition) {
    position = newPosition;
  }

  Vector3 GraphicsGeometry::GetPosition() const {
    return position;
  }

  void GraphicsGeometry::SetRotation(const Quaternion &newRotation) {
    rotation = newRotation;
  }

  Quaternion GraphicsGeometry::GetRotation() const {
    return rotation;
  }




  GraphicsGeometry_GeometryInterpreter::GraphicsGeometry_GeometryInterpreter(GraphicsGeometry *caller) : caller(caller), usesIndices(false) {
  }

  void LoadMaterials(Renderer3D *renderer3D, const Material *material, Renderer3DMaterial &r3dMaterial) {
    boost::intrusive_ptr < Resource<Texture> > diffuseTexture;
    boost::intrusive_ptr < Resource<Texture> > normalTexture;
    boost::intrusive_ptr < Resource<Texture> > specularTexture;
    boost::intrusive_ptr < Resource<Texture> > illuminationTexture;
    if (material->diffuseTexture) {
      boost::intrusive_ptr < Resource<Surface> > surface = material->diffuseTexture;

      bool texAlreadyThere = false;
      diffuseTexture =
        ResourceManagerPool::getTextureManager()->
          Fetch(surface->GetIdentString(), false, texAlreadyThere, true); // false == don't try to use loader

      if (!texAlreadyThere) {
        //printf("%s\n", surface->GetIdentString().c_str());
        SDL_Surface *image = surface->GetResource()->GetData();
        diffuseTexture->GetResource()->SetRenderer3D(renderer3D);
        bool repeat = true;
        bool mipmaps = true;
        bool bilinear = true;
        bool alpha = SDL_ISPIXELFORMAT_ALPHA((image->format->format));
        diffuseTexture->GetResource()->CreateTexture(alpha ? e_InternalPixelFormat_SRGBA8 : e_InternalPixelFormat_SRGB8, alpha ? e_PixelFormat_RGBA : e_PixelFormat_RGB, image->w, image->h, alpha, repeat, mipmaps, bilinear);
        diffuseTexture->GetResource()->UpdateTexture(image, alpha, true);
      }
    }

    if (material->normalTexture) {
      boost::intrusive_ptr < Resource<Surface> > surface = material->normalTexture;

      bool texAlreadyThere = false;
      normalTexture =
        ResourceManagerPool::getTextureManager()->
          Fetch(surface->GetIdentString(), false, texAlreadyThere, true); // false == don't try to use loader

      if (!texAlreadyThere) {
        SDL_Surface *image = surface->GetResource()->GetData();
        normalTexture->GetResource()->SetRenderer3D(renderer3D);
        normalTexture->GetResource()->CreateTexture(e_InternalPixelFormat_RGB8, e_PixelFormat_RGB, image->w, image->h, false, true, true, true);
        normalTexture->GetResource()->UpdateTexture(image, false, true);
      }
    }

    if (material->specularTexture) {
      boost::intrusive_ptr < Resource<Surface> > surface = material->specularTexture;

      bool texAlreadyThere = false;
      specularTexture =
        ResourceManagerPool::getTextureManager()->
          Fetch(surface->GetIdentString(), false, texAlreadyThere, true); // false == don't try to use loader

      if (!texAlreadyThere) {
        SDL_Surface *image = surface->GetResource()->GetData();
        specularTexture->GetResource()->SetRenderer3D(renderer3D);
        specularTexture->GetResource()->CreateTexture(e_InternalPixelFormat_RGB8, e_PixelFormat_RGB, image->w, image->h, false, true, true, true);
        specularTexture->GetResource()->UpdateTexture(image, false, true);
      }
    }

    if (material->illuminationTexture) {
      boost::intrusive_ptr < Resource<Surface> > surface = material->illuminationTexture;

      bool texAlreadyThere = false;
      illuminationTexture =
        ResourceManagerPool::getTextureManager()->
          Fetch(surface->GetIdentString(), false, texAlreadyThere, true); // false == don't try to use loader

      if (!texAlreadyThere) {
        SDL_Surface *image = surface->GetResource()->GetData();
        illuminationTexture->GetResource()->SetRenderer3D(renderer3D);
        illuminationTexture->GetResource()->CreateTexture(e_InternalPixelFormat_RGB8, e_PixelFormat_RGB, image->w, image->h, false, true, true, true);
        illuminationTexture->GetResource()->UpdateTexture(image, false, true);
      }
    }

    if (diffuseTexture) r3dMaterial.diffuseTexture = diffuseTexture;
    if (normalTexture) r3dMaterial.normalTexture = normalTexture;
    if (specularTexture) r3dMaterial.specularTexture = specularTexture;
    if (illuminationTexture) r3dMaterial.illuminationTexture = illuminationTexture;
    r3dMaterial.shininess = material->shininess;
    r3dMaterial.specular_amount = material->specular_amount;
    r3dMaterial.self_illumination = material->self_illumination;
  }

  void GraphicsGeometry_GeometryInterpreter::OnLoad(boost::intrusive_ptr<Geometry> geometry) {



    //printf("loading %s\n", geometry->GetName().c_str());

    boost::intrusive_ptr < Resource<GeometryData> > resource = geometry->GetGeometryData();

    bool dynamicBuffer = resource->GetResource()->IsDynamic();

    bool alreadyThere = false;

    caller->vertexBuffer =
      ResourceManagerPool::getVerticesManager()->
        Fetch(resource->GetIdentString(), false, alreadyThere, true); // false == don't try to use loader
    //printf("%s, %i\n", resource->GetIdentString().c_str(), alreadyThere);

    std::vector < MaterializedTriangleMesh > triangleMeshes = resource->GetResource()->GetTriangleMeshes();
    Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();

    //std::vector<Triangle*> triangles;

    std::vector<float> vertices;
    int verticesDataSize = 0;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> indicesTest;
    int indicesSize = 0;
    if (!alreadyThere) {
      for (unsigned int i = 0; i < triangleMeshes.size(); i++) {
        verticesDataSize += triangleMeshes[i].verticesDataSize;
        indicesSize += triangleMeshes[i].indices.size();
      }
      vertices.resize(verticesDataSize);
      indices.reserve(indicesSize);
    }

    int startIndex = 0;
    int currentSize = 0;

    for (unsigned int i = 0; i < triangleMeshes.size(); i++) {

      // material

      const Material *material = &triangleMeshes[i].material;
      Renderer3DMaterial r3dMaterial;
      LoadMaterials(renderer3D, material, r3dMaterial);


      // mesh

      startIndex += currentSize;
      currentSize = 0;

      if (!alreadyThere) {
        /*
        for (int e = 0; e < GetTriangleMeshElementCount(); e++) {
          memcpy(&triangleMesh[e * (tmeshsize / GetTriangleMeshElementCount()) + startIndex * 3 * 3], &triangleMeshes[i].triangleMesh[e * (triangleMeshes[i].triangleMeshSize / GetTriangleMeshElementCount())], triangleMeshes[i].triangleMeshSize / GetTriangleMeshElementCount() * sizeof(float));
        }
        */

        for (int e = 0; e < GetTriangleMeshElementCount(); e++) {
          //printf("%s: e: %i, verticesDataSize: %i, startIndex: %i, triangleMeshes[i].verticesDataSize: %i\n", geometry->GetName().c_str(), e, verticesDataSize, startIndex, triangleMeshes[i].verticesDataSize);
          memcpy(&vertices[e * (verticesDataSize / GetTriangleMeshElementCount()) + startIndex],
                 &triangleMeshes[i].vertices[e * (triangleMeshes[i].verticesDataSize / GetTriangleMeshElementCount())],
                 triangleMeshes[i].verticesDataSize / GetTriangleMeshElementCount() * sizeof(float));
        }

        //indices.insert(indices.end(), triangleMeshes[i].indices.begin(), triangleMeshes[i].indices.end());
        for (unsigned int index = 0; index < triangleMeshes[i].indices.size(); index++) {
          indices.push_back(startIndex / 3 + triangleMeshes[i].indices.at(index));
        }

      }

      currentSize = triangleMeshes[i].verticesDataSize / GetTriangleMeshElementCount();

      VertexBufferIndex vbIndex;
      if (indices.size() > 0) {
        vbIndex.startIndex = (indices.size() - triangleMeshes[i].indices.size()); // start index id
        vbIndex.size = triangleMeshes[i].indices.size(); // number of indices
      } else {
        vbIndex.startIndex = startIndex / 3; // start vertex id
        vbIndex.size = currentSize / 3; // number of vertices
      }

      vbIndex.material = r3dMaterial;
      caller->vertexBufferIndices.push_back(vbIndex);
    }

    if (indices.size() > 0) usesIndices = true; else usesIndices = false;

    if (!alreadyThere) {
      caller->vertexBuffer->GetResource()->SetTriangleMesh(vertices, verticesDataSize, indices);
      caller->vertexBuffer->GetResource()->CreateOrUpdateVertexBuffer(renderer3D, dynamicBuffer);
    }

  }

  void GraphicsGeometry_GeometryInterpreter::OnUpdateGeometry(boost::intrusive_ptr<Geometry> geometry, bool updateMaterials) {



    boost::intrusive_ptr < Resource<GeometryData> > resource = geometry->GetGeometryData();

    bool dynamicBuffer = resource->GetResource()->IsDynamic();
    const std::vector < MaterializedTriangleMesh >& triangleMeshes = resource->GetResource()->GetTriangleMeshesRef();
    Renderer3D *renderer3D = caller->GetGraphicsScene()->GetGraphicsSystem()->GetRenderer3D();

    std::vector<float> vertex_array;
    float* vertices = nullptr;
    int currentVerticesDataSize = caller->vertexBuffer->GetResource()->GetVerticesDataSize();

    std::vector<unsigned int> indices;
    int verticesDataSize = 0;
    int indicesSize = 0;
    for (unsigned int i = 0; i < triangleMeshes.size(); i++) {
      verticesDataSize += triangleMeshes[i].verticesDataSize;
      indicesSize += triangleMeshes[i].indices.size();
    }
    indices.reserve(indicesSize);
    bool newFloatData = false;
    bool updateIndices = false;
    if (verticesDataSize == currentVerticesDataSize) {
      vertices = caller->vertexBuffer->GetResource()->GetTriangleMesh();
      newFloatData = false;
      updateIndices = updateMaterials;
    } else {
      vertex_array.resize(verticesDataSize);
      vertices = &vertex_array[0];
      newFloatData = true;
      updateIndices = true;
    }

    int startIndex = 0;
    int startIndicesIndex = 0; // omfg
    int currentSize = 0;

    if (updateIndices) caller->vertexBufferIndices.clear();

    for (unsigned int i = 0; i < triangleMeshes.size(); i++) {


      // mesh

      startIndex += currentSize;
      currentSize = 0;

      for (int e = 0; e < GetTriangleMeshElementCount(); e++) {
        //printf("%s: e: %i, verticesDataSize: %i, startIndex: %i, triangleMeshes[i].verticesDataSize: %i\n", geometry->GetName().c_str(), e, verticesDataSize, startIndex, triangleMeshes[i].verticesDataSize);
        memcpy(&vertices[e * (verticesDataSize / GetTriangleMeshElementCount()) + startIndex],
               &triangleMeshes[i].vertices[e * (triangleMeshes[i].verticesDataSize / GetTriangleMeshElementCount())],
               triangleMeshes[i].verticesDataSize / GetTriangleMeshElementCount() * sizeof(float));
      }

      if (!usesIndices) { // can only set indices once (todo: make OnUpdateIndices function, or something like that)
        for (unsigned int index = 0; index < triangleMeshes[i].indices.size(); index++) {
          indices.push_back(startIndex / 3 + triangleMeshes[i].indices.at(index));
          //printf("vertex index: %i + %i = %i\n", startIndex * 3, triangleMeshes[i].indices.at(index), startIndex * 3 + triangleMeshes[i].indices.at(index));
        }
      }

      currentSize = triangleMeshes[i].verticesDataSize / GetTriangleMeshElementCount();

      if (updateIndices) {


        // material

        const Material *material = &triangleMeshes[i].material;
        Renderer3DMaterial r3dMaterial;
        LoadMaterials(renderer3D, material, r3dMaterial);


        // indices

        VertexBufferIndex vbIndex;

        // this code makes this version only work on meshes that are the same size/materialorder as their previous version
        vbIndex.material = r3dMaterial;

        if ((!usesIndices && indices.size() > 0) || usesIndices) { // include the first time using indices
          vbIndex.startIndex = startIndicesIndex;
          vbIndex.size = triangleMeshes[i].indices.size(); // number of indices
        } else {
          vbIndex.startIndex = startIndex / 3; // start vertex id
          vbIndex.size = currentSize / 3; // number of vertices
        }

        startIndicesIndex += triangleMeshes[i].indices.size();

        //printf("st %i VS %i\n", (indices.size() - triangleMeshes[i].indices.size()), startIndex / 3);
        //printf("si %i VS %i\n", triangleMeshes[i].indices.size(), currentSize / 3);

        caller->vertexBufferIndices.push_back(vbIndex);

      }

    }

    if (indices.size() > 0) usesIndices = true;

    if (newFloatData) {
      caller->vertexBuffer->GetResource()->SetTriangleMesh(vertex_array, verticesDataSize, indices);
    } else {
      caller->vertexBuffer->GetResource()->TriangleMeshWasUpdatedExternally(verticesDataSize, indices);
    }
    caller->vertexBuffer->GetResource()->CreateOrUpdateVertexBuffer(renderer3D, dynamicBuffer);
  }

  void GraphicsGeometry_GeometryInterpreter::OnUnload() {
    //printf("resetting link to vertexbuffer.. ");
    caller->vertexBuffer.reset();
    caller->vertexBufferIndices.clear();
    delete caller;
    caller = 0;
    //printf("[OK]\n");
  }

  void GraphicsGeometry_GeometryInterpreter::OnMove(const Vector3 &position) {
    caller->SetPosition(position);
  }

  void GraphicsGeometry_GeometryInterpreter::OnRotate(const Quaternion &rotation) {
    caller->SetRotation(rotation);
  }

  void GraphicsGeometry_GeometryInterpreter::GetVertexBufferQueue(std::deque<VertexBufferQueueEntry> &queue) {
    //printf("size: %i\n", size);
    VertexBufferQueueEntry queueEntry;

    queueEntry.vertexBufferIndices = &caller->vertexBufferIndices;

    queueEntry.vertexBuffer = caller->vertexBuffer;
    queueEntry.position = caller->GetPosition();
    queueEntry.rotation = caller->GetRotation();
    queue.push_back(queueEntry);
  }

  void GraphicsGeometry_GeometryInterpreter::OnSynchronize() {
    OnLoad(static_cast<Geometry*>(subjectPtr));
    //OnUpdateGeometry(static_cast<Geometry*>(subjectPtr));

  }

  void GraphicsGeometry_GeometryInterpreter::OnPoke() {
  }




  // SKYBOX INTERPRETER

  GraphicsGeometry_SkyboxInterpreter::GraphicsGeometry_SkyboxInterpreter(GraphicsGeometry *caller) : GraphicsGeometry_GeometryInterpreter(caller) {
  }

  void GraphicsGeometry_SkyboxInterpreter::OnPoke() {
  }

}
