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

#ifndef _HPP_SCENE3D_NODE
#define _HPP_SCENE3D_NODE

#include "../../scene/object.hpp"

#include "../../types/spatial.hpp"

#include "../../base/geometry/aabb.hpp"

namespace blunted {

  class Scene3D;

  class Node : public Spatial {

    public:
      Node(const std::string &name);
      Node(const Node &source, const std::string &postfix, boost::shared_ptr<Scene3D> scene3D);
      virtual ~Node();

      virtual void Exit();

      void AddNode(boost::intrusive_ptr<Node> node);
      void DeleteNode(boost::intrusive_ptr<Node> node);
      void GetNodes(std::vector<boost::intrusive_ptr<Node> > &gatherNodes,
                    bool recurse = false) const;

      void AddObject(boost::intrusive_ptr<Object> object);
      boost::intrusive_ptr<Object> GetObject(const std::string &name);
      void DeleteObject(boost::intrusive_ptr<Object> object,
                        bool exitObject = true);

      void GetObjects(std::list<boost::intrusive_ptr<Object> > &gatherObjects,
                      bool recurse = true, int depth = 0) const;

      void GetObjects(std::deque < boost::intrusive_ptr<Object> > &gatherObjects, const vector_Planes &bounding, bool recurse = true, int depth = 0) const;

      template <class T>
      inline void GetObjects(e_ObjectType targetObjectType, std::list < boost::intrusive_ptr<T> > &gatherObjects, bool recurse = true, int depth = 0) const {
        //objects.Lock();
        int objectsSize = objects.size();
        for (int i = 0; i < objectsSize; i++) {
          if (objects[i]->GetObjectType() == targetObjectType) {
            gatherObjects.push_back(static_pointer_cast<T>(objects[i]));
          }
        }
        //objects.Unlock();

        if (recurse) {
          //nodes.Lock();
          int nodesSize = nodes.size();
          for (int i = 0; i < nodesSize; i++) {
            nodes[i]->GetObjects<T>(targetObjectType, gatherObjects, recurse, depth + 1);
          }
          //nodes.Unlock();
        }
      }

      template <class T>
      inline void GetObjects(e_ObjectType targetObjectType, std::deque < boost::intrusive_ptr<T> > &gatherObjects, const vector_Planes &bounding, bool recurse = true, int depth = 0) const {
        //objects.Lock();
        int objectsSize = objects.size();
        for (int i = 0; i < objectsSize; i++) {
          if (objects[i]->GetObjectType() == targetObjectType) {
            if (objects[i]->GetAABB().Intersects(bounding)) gatherObjects.push_back(static_pointer_cast<T>(objects[i]));
          }
        }
        //objects.Unlock();

        if (recurse) {
          //nodes.Lock();
          int nodesSize = nodes.size();
          for (int i = 0; i < nodesSize; i++) {
            if (nodes[i]->GetAABB().Intersects(bounding)) nodes[i]->GetObjects<T>(targetObjectType, gatherObjects, bounding, recurse, depth + 1);
          }
          //nodes.Unlock();
        }
      }

      void PokeObjects(e_ObjectType targetObjectType, e_SystemType targetSystem);

      virtual AABB GetAABB() const;

      virtual void RecursiveUpdateSpatialData(e_SpatialDataType spatialDataType, e_SystemType excludeSystem = e_SystemType_None);

    protected:
      mutable std::vector < boost::intrusive_ptr<Node> > nodes;
      mutable std::vector < boost::intrusive_ptr<Object> > objects;

  };

}

#endif
