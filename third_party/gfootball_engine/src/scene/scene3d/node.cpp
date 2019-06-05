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
      boost::intrusive_ptr<Node> copy(new Node(*gatherNodes.at(i).get(), postfix, scene3D));
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
    //printf("node::exit exiting node %s\n", GetName().c_str());
    //objects.Lock();
    int objCount = objects.size();
    for (int i = 0; i < objCount; i++) {
      //printf("node::exit exiting object %s\n", objects.at(i)->GetName().c_str());
      objects.at(i)->Exit();
    }
    objects.clear();
    //objects.Unlock();

    //nodes.Lock();
    int nodeCount = nodes.size();
    for (int i = 0; i < nodeCount; i++) {
      nodes.at(i)->Exit();
    }
    nodes.clear();
    //nodes.Unlock();
  }

  void Node::AddNode(boost::intrusive_ptr<Node> node) {
    //nodes.Lock();
    nodes.push_back(node);
    node->SetParent(this);
    //printf("adding node: %s\n", node->GetName().c_str());

    node->RecursiveUpdateSpatialData(e_SpatialDataType_Both);

    //nodes.Unlock();

    InvalidateBoundingVolume();
  }

  void Node::DeleteNode(boost::intrusive_ptr<Node> node) {
    //nodes.Lock();
    std::vector < boost::intrusive_ptr<Node> >::iterator nodeIter = find(nodes.begin(), nodes.end(), node);
    if (nodeIter != nodes.end()) {
      (*nodeIter)->Exit();
      nodes.erase(nodeIter);
    }
    //nodes.Unlock();

    InvalidateBoundingVolume();
  }

  void Node::GetNodes(std::vector < boost::intrusive_ptr<Node> > &gatherNodes, bool recurse) const {
    //nodes.Lock();
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {
      gatherNodes.push_back(nodes.at(i));
      if (recurse) nodes.at(i)->GetNodes(gatherNodes, recurse);
    }
    //nodes.Unlock();
  }

  void Node::AddObject(boost::intrusive_ptr<Object> object) {
    assert(object.get());
    //objects.Lock();
    objects.push_back(object);
    object->SetParent(this);

    object->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
    //printf("adding object: %s\n", object->GetName().c_str());

    //objects.Unlock();

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
    //objects.Lock();
    // verbose printf("deleting object %s\n", object->GetName().c_str());
    std::vector < boost::intrusive_ptr<Object> >::iterator objIter = find(objects.begin(), objects.end(), object);
    if (objIter != objects.end()) {
      // verbose printf("found!\n");
      if (exitObject) (*objIter)->Exit();
      (*objIter)->SetParent(0);
      objects.erase(objIter);
    } else Log(e_Error, "Node", "DeleteObject", "Object " + object->GetName() + " not found among node " + GetName() + "'s children!");
    //objects.Unlock();

    //aabb.Lock();
    aabb.dirty = true;
    //aabb.Unlock();
  }

  void Node::RemoveObject(boost::intrusive_ptr<Object> object) {
    DeleteObject(object, false);
  }

  void Node::GetSpatials(std::list < boost::intrusive_ptr<Spatial> > &gatherSpatials, bool recurse, int depth) const {
    //objects.Lock();
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      gatherSpatials.push_back(objects.at(i));
    }
    //objects.Unlock();

    if (recurse) {
      //nodes.Lock();
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        gatherSpatials.push_back(nodes.at(i));
        nodes.at(i)->GetSpatials(gatherSpatials, recurse, depth + 1);
      }
      //nodes.Unlock();
    }
  }

  void Node::GetObjects(std::list < boost::intrusive_ptr<Object> > &gatherObjects, bool recurse, int depth) const {
    //objects.Lock();
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      gatherObjects.push_back(objects.at(i));
    }
    //objects.Unlock();

    if (recurse) {
      //nodes.Lock();
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        nodes.at(i)->GetObjects(gatherObjects, recurse, depth + 1);
      }
      //nodes.Unlock();
    }
  }

  void Node::GetObjects(std::deque < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding, bool recurse, int depth) const {
    //objects.Lock();
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects.at(i)->GetAABB().Intersects(bounding)) gatherObjects.push_back(objects.at(i));
    }
    //objects.Unlock();

    if (recurse) {
      //nodes.Lock();
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        if (nodes.at(i)->GetAABB().Intersects(bounding)) nodes.at(i)->GetObjects(gatherObjects, bounding, recurse, depth + 1);
      }
      //nodes.Unlock();
    }
  }

  void Node::PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystemType) {
    //objects.Lock();
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      if (objects.at(i)->IsEnabled()) if (objects.at(i)->GetObjectType() == targetObjectType) objects.at(i)->Poke(targetSystemType);
    }
    //objects.Unlock();

    //nodes.Lock();
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {
      nodes.at(i)->PokeObjects(targetObjectType, targetSystemType);
    }
    //nodes.Unlock();
  }

  void Node::RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem) {

    InvalidateSpatialData();
    InvalidateBoundingVolume();

    //nodes.Lock();
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {

      nodes.at(i)->RecursiveUpdateSpatialData(spatialDataType, excludeSystem);
    }
    //nodes.Unlock();

    //objects.Lock();
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      //if (objects.at(i)->GetLocalMode() != e_LocalMode_Absolute)
      objects.at(i)->RecursiveUpdateSpatialData(spatialDataType, excludeSystem);
    }
    //objects.Unlock();
  }

  void Node::PrintTree(int recursionDepth) {
    //nodes.Lock();
    int nodesSize = nodes.size();
    for (int i = 0; i < nodesSize; i++) {
      for (int space = 0; space < recursionDepth; space++) printf("|     ");
      printf("|-----[NODE] %s\n", nodes.at(i)->GetName().c_str());
      nodes.at(i)->PrintTree(recursionDepth + 1);
    }
    //nodes.Unlock();
    //objects.Lock();
    int objectsSize = objects.size();
    for (int i = 0; i < objectsSize; i++) {
      for (int space = 0; space < recursionDepth; space++) printf("|     ");
      printf("|-----%s\n", objects.at(i)->GetName().c_str());
    }
    //objects.Unlock();
  }

  AABB Node::GetAABB() const {
    AABB tmp;

    //aabb.Lock();

    if (aabb.dirty == true) {

      tmp.Reset();
      //aabb.Unlock();

      //nodes.Lock();
      int nodesSize = nodes.size();
      for (int i = 0; i < nodesSize; i++) {
        tmp += (nodes.at(i)->GetAABB());
      }
      //nodes.Unlock();

      //objects.Lock();
      int objectsSize = objects.size();
      for (int i = 0; i < objectsSize; i++) {
        tmp += (objects.at(i)->GetAABB());
      }
      //objects.Unlock();

      //aabb.Lock();
      aabb.dirty = false;
      aabb.aabb = tmp;
    } else {
      tmp = aabb.aabb;
    }

    //aabb.Unlock();

    return tmp;
  }

}
