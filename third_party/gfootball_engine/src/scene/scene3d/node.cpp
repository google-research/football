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

#include "node.hpp"

#include "../../main.hpp"
#include "../../scene/objectfactory.hpp"
#include "scene3d.hpp"

namespace blunted {

Node::Node(const std::string &name) : Spatial(name) {
  DO_VALIDATION;
  aabb.aabb.Reset();
  aabb.dirty = false;
}

Node::Node(const Node &source, const std::string &postfix,
           boost::shared_ptr<Scene3D> scene3D)
    : Spatial(source) {
  DO_VALIDATION;
  SetName(source.GetName() + postfix);
  std::vector<boost::intrusive_ptr<Node> > gatherNodes;
  source.GetNodes(gatherNodes);
  for (int i = 0; i < (signed int)gatherNodes.size(); i++) {
    DO_VALIDATION;
    boost::intrusive_ptr<Node> copy(
        new Node(*gatherNodes[i].get(), postfix, scene3D));
    AddNode(copy);
  }

  std::list<boost::intrusive_ptr<Object> > gatherObjects;
  source.GetObjects(gatherObjects, false);
  std::list<boost::intrusive_ptr<Object> >::iterator objectIter =
      gatherObjects.begin();
  while (objectIter != gatherObjects.end()) {
    DO_VALIDATION;
    boost::intrusive_ptr<Object> objCopy =
        GetContext().object_factory.CopyObject((*objectIter), postfix);
    scene3D->CreateSystemObjects(objCopy);
    objCopy->Synchronize();
    AddObject(objCopy);

    objectIter++;
  }
}

Node::~Node() { DO_VALIDATION; }

void Node::Exit() {
  DO_VALIDATION;
  int objCount = objects.size();
  for (int i = 0; i < objCount; i++) {
    DO_VALIDATION;
    objects[i]->Exit();
  }
  objects.clear();
  int nodeCount = nodes.size();
  for (int i = 0; i < nodeCount; i++) {
    DO_VALIDATION;
    nodes[i]->Exit();
  }
  nodes.clear();
}

void Node::AddNode(boost::intrusive_ptr<Node> node) {
  DO_VALIDATION;
  nodes.push_back(node);
  node->SetParent(this);
  node->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  InvalidateBoundingVolume();
}

void Node::DeleteNode(boost::intrusive_ptr<Node> node) {
  DO_VALIDATION;
  std::vector<boost::intrusive_ptr<Node> >::iterator nodeIter =
      find(nodes.begin(), nodes.end(), node);
  if (nodeIter != nodes.end()) {
    DO_VALIDATION;
    (*nodeIter)->Exit();
    nodes.erase(nodeIter);
  }
  InvalidateBoundingVolume();
}

  void Node::GetNodes(std::vector < boost::intrusive_ptr<Node> > &gatherNodes, bool recurse) const {
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {
      DO_VALIDATION;
      gatherNodes.push_back(nodes[i]);
      if (recurse) nodes[i]->GetNodes(gatherNodes, recurse);
    }
  }

  void Node::AddObject(boost::intrusive_ptr<Object> object) {
    DO_VALIDATION;
    assert(object.get());
    objects.push_back(object);
    object->SetParent(this);

    object->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
    InvalidateBoundingVolume();
  }

  boost::intrusive_ptr<Object> Node::GetObject(const std::string &name) {
    DO_VALIDATION;
    std::vector < boost::intrusive_ptr<Object> >::iterator objIter = objects.begin();
    while (objIter != objects.end()) {
      DO_VALIDATION;
      if ((*objIter)->GetName() == name) {
        DO_VALIDATION;
        return (*objIter);
      } else {
        objIter++;
      }
    }
    return boost::intrusive_ptr<Object>();
  }

  void Node::DeleteObject(boost::intrusive_ptr<Object> object,
                          bool exitObject) {
    DO_VALIDATION;
    std::vector < boost::intrusive_ptr<Object> >::iterator objIter = find(objects.begin(), objects.end(), object);
    if (objIter != objects.end()) {
      DO_VALIDATION;
      if (exitObject) (*objIter)->Exit();
      (*objIter)->SetParent(0);
      objects.erase(objIter);
    } else
      Log(e_Error, "Node", "DeleteObject",
          "Object " + object->GetName() + " not found among node " + GetName() +
              "'s children!");
    aabb.dirty = true;
  }

  void Node::GetObjects(std::list < boost::intrusive_ptr<Object> > &gatherObjects, bool recurse, int depth) const {
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      DO_VALIDATION;
      gatherObjects.push_back(objects[i]);
    }
    if (recurse) {
      DO_VALIDATION;
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        DO_VALIDATION;
        nodes[i]->GetObjects(gatherObjects, recurse, depth + 1);
      }
    }
  }

  void Node::GetObjects(std::deque < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding, bool recurse, int depth) const {
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      DO_VALIDATION;
      if (objects[i]->GetAABB().Intersects(bounding)) gatherObjects.push_back(objects[i]);
    }
    if (recurse) {
      DO_VALIDATION;
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        DO_VALIDATION;
        if (nodes[i]->GetAABB().Intersects(bounding)) nodes[i]->GetObjects(gatherObjects, bounding, recurse, depth + 1);
      }
    }
  }

  void Node::ProcessState(EnvState* state) {
    state->process(position);
    state->process(rotation);
    state->process(scale);
    state->process((void*) &localMode, sizeof(localMode));
    _dirty_DerivedPosition = true;
    _dirty_DerivedRotation = true;
    _dirty_DerivedScale = true;
    aabb.dirty = true;
    for (auto& node : nodes) {
      node->ProcessState(state);
    }

  }

  void Node::PokeObjects(e_ObjectType targetObjectType,
                         e_SystemType targetSystemType) {
    DO_VALIDATION;
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      DO_VALIDATION;
      if (objects[i]->IsEnabled()) if (objects[i]->GetObjectType() == targetObjectType) objects[i]->Poke(targetSystemType);
    }
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {
      DO_VALIDATION;
      nodes[i]->PokeObjects(targetObjectType, targetSystemType);
    }
  }

  void Node::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType,
                                        e_SystemType excludeSystem) {
    DO_VALIDATION;

    InvalidateSpatialData();
    InvalidateBoundingVolume();
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {
      DO_VALIDATION;

      nodes[i]->RecursiveUpdateSpatialData(spatialDataType, excludeSystem);
    }
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      DO_VALIDATION;
      objects[i]->RecursiveUpdateSpatialData(spatialDataType, excludeSystem);
    }
  }

  AABB Node::GetAABB() const {
    AABB tmp;
    if (aabb.dirty == true) {
      DO_VALIDATION;
      tmp.Reset();
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        DO_VALIDATION;
        tmp += (nodes[i]->GetAABB());
      }
      int objectsSize = objects.size();
      for (int i = 0; i < objectsSize; i++) {
        DO_VALIDATION;
        tmp += (objects[i]->GetAABB());
      }
      aabb.dirty = false;
      aabb.aabb = tmp;
    } else {
      tmp = aabb.aabb;
    }
    return tmp;
  }

}
