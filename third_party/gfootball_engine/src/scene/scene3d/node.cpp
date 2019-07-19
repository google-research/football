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

#include "scene3d.hpp"

#include "../../scene/objectfactory.hpp"

namespace blunted {

  Node::Node(const std::string &name) : Spatial(name) {
    aabb.aabb.Reset();
    aabb.dirty = false;
  }

  Node::Node(const Node &source, const std::string &postfix, boost::shared_ptr<Scene3D> scene3D) : Spatial(source) {
    SetName(source.GetName() + postfix);
    std::vector < boost::intrusive_ptr<Node> > gatherNodes;
    source.GetNodes(gatherNodes);
    for (int i = 0; i < (signed int)gatherNodes.size(); i++) {
      boost::intrusive_ptr<Node> copy(new Node(*gatherNodes[i].get(), postfix, scene3D));
      AddNode(copy);
    }

    std::list < boost::intrusive_ptr<Object> > gatherObjects;
    source.GetObjects(gatherObjects, false);
    std::list < boost::intrusive_ptr<Object> >::iterator objectIter = gatherObjects.begin();
    while (objectIter != gatherObjects.end()) {
      boost::intrusive_ptr<Object> objCopy = ObjectFactory::GetInstance().CopyObject((*objectIter), postfix);
      scene3D->CreateSystemObjects(objCopy);
      objCopy->Synchronize();
      AddObject(objCopy);

      objectIter++;
    }
  }

  Node::~Node() {
  }

  void Node::Exit() {
    int objCount = objects.size();
    for (int i = 0; i < objCount; i++) {
      objects[i]->Exit();
    }
    objects.clear();
    int nodeCount = nodes.size();
    for (int i = 0; i < nodeCount; i++) {
      nodes[i]->Exit();
    }
    nodes.clear();
  }

  void Node::AddNode(boost::intrusive_ptr<Node> node) {
    nodes.push_back(node);
    node->SetParent(this);
    node->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
    InvalidateBoundingVolume();
  }

  void Node::DeleteNode(boost::intrusive_ptr<Node> node) {
    std::vector < boost::intrusive_ptr<Node> >::iterator nodeIter = find(nodes.begin(), nodes.end(), node);
    if (nodeIter != nodes.end()) {
      (*nodeIter)->Exit();
      nodes.erase(nodeIter);
    }
    InvalidateBoundingVolume();
  }

  void Node::GetNodes(std::vector < boost::intrusive_ptr<Node> > &gatherNodes, bool recurse) const {
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {
      gatherNodes.push_back(nodes[i]);
      if (recurse) nodes[i]->GetNodes(gatherNodes, recurse);
    }
  }

  void Node::AddObject(boost::intrusive_ptr<Object> object) {
    assert(object.get());
    objects.push_back(object);
    object->SetParent(this);

    object->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
    InvalidateBoundingVolume();
  }

  boost::intrusive_ptr<Object> Node::GetObject(const std::string &name) {
    //boost::mutex::scoped_lock blah(objects.mutex);
    std::vector < boost::intrusive_ptr<Object> >::iterator objIter = objects.begin();
    while (objIter != objects.end()) {
      if ((*objIter)->GetName() == name) {
        return (*objIter);
      } else {
        objIter++;
      }
    }
    return boost::intrusive_ptr<Object>();
  }

  void Node::DeleteObject(boost::intrusive_ptr<Object> object, bool exitObject) {
    std::vector < boost::intrusive_ptr<Object> >::iterator objIter = find(objects.begin(), objects.end(), object);
    if (objIter != objects.end()) {
      if (exitObject) (*objIter)->Exit();
      (*objIter)->SetParent(0);
      objects.erase(objIter);
    } else Log(e_Error, "Node", "DeleteObject", "Object " + object->GetName() + " not found among node " + GetName() + "'s children!");
    aabb.dirty = true;
  }

  void Node::GetObjects(std::list < boost::intrusive_ptr<Object> > &gatherObjects, bool recurse, int depth) const {
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      gatherObjects.push_back(objects[i]);
    }
    if (recurse) {
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        nodes[i]->GetObjects(gatherObjects, recurse, depth + 1);
      }
    }
  }

  void Node::GetObjects(std::deque < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding, bool recurse, int depth) const {
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects[i]->GetAABB().Intersects(bounding)) gatherObjects.push_back(objects[i]);
    }
    if (recurse) {
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        if (nodes[i]->GetAABB().Intersects(bounding)) nodes[i]->GetObjects(gatherObjects, bounding, recurse, depth + 1);
      }
    }
  }

  void Node::PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystemType) {
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects[i]->IsEnabled()) if (objects[i]->GetObjectType() == targetObjectType) objects[i]->Poke(targetSystemType);
    }
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {
      nodes[i]->PokeObjects(targetObjectType, targetSystemType);
    }
  }

  void Node::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {

    InvalidateSpatialData();
    InvalidateBoundingVolume();
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {

      nodes[i]->RecursiveUpdateSpatialData(spatialDataType, excludeSystem);
    }
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      objects[i]->RecursiveUpdateSpatialData(spatialDataType, excludeSystem);
    }
  }

  AABB Node::GetAABB() const {
    AABB tmp;
    if (aabb.dirty == true) {
      tmp.Reset();
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        tmp += (nodes[i]->GetAABB());
      }
      int objectsSize = objects.size();
      for (int i = 0; i < objectsSize; i++) {
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
