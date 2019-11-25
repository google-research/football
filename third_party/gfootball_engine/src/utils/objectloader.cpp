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

#include "objectloader.hpp"

#include "../base/utils.hpp"
#include "../main.hpp"
#include "../scene/objectfactory.hpp"
#include "../scene/objects/geometry.hpp"
#include "../scene/objects/light.hpp"
#include "../scene/resources/geometrydata.hpp"
#include "../types/resource.hpp"

namespace blunted {

ObjectLoader::ObjectLoader() { DO_VALIDATION; }

ObjectLoader::~ObjectLoader() { DO_VALIDATION; }

boost::intrusive_ptr<Node> ObjectLoader::LoadObject(
    const std::string &filename, const Vector3 &offset) const {
  XMLLoader loader;
  const XMLTree objectTree = loader.LoadFile(filename);
  boost::intrusive_ptr<Node> result =
      LoadObjectImpl(filename, objectTree.children.begin()->second, offset);
  return result;
  }

  boost::intrusive_ptr<Node> ObjectLoader::LoadObjectImpl(const std::string &nodename, const XMLTree &objectTree, const Vector3 &offset) const {

    boost::intrusive_ptr<Node> objNode(new Node("objectnode: " + nodename));

    std::string dirpart = nodename.substr(0, nodename.find_last_of('/') + 1);

    Vector3 position;
    Quaternion rotation;

    map_XMLTree::const_iterator objectIter = objectTree.children.begin();
    while (objectIter != objectTree.children.end()) {
      DO_VALIDATION;
      std::string objectName;
      Properties properties;
      e_LocalMode localMode = e_LocalMode_Relative;

      //printf("loading %s:%s\n", nodename.c_str(), (*objectIter).first.c_str());


      // NODE (recurse)

      if (objectIter->first == "node") {
        DO_VALIDATION;
        objNode->AddNode(LoadObjectImpl(dirpart, objectIter->second, offset));
      }

      else if (objectIter->first == "name") {
        DO_VALIDATION;
        objectName = objectIter->second.value;
        //printf("node name: %s\n", objectIter->second.value.c_str());
        objNode->SetName(objectName);
      }

      else if (objectIter->first == "position") {
        DO_VALIDATION;
        position = GetVectorFromString(objectIter->second.value) + offset;
        objNode->SetPosition(position);
      }

      else if (objectIter->first == "rotation") {
        DO_VALIDATION;
        rotation = GetQuaternionFromString(objectIter->second.value);
        objNode->SetRotation(rotation);
      }

      // GEOMETRY

      else if (objectIter->first == "geometry") {
        DO_VALIDATION;
        std::string aseFilename;
        Vector3 position;
        Quaternion rotation;

        //printf("loading geom\n");

        map_XMLTree::const_iterator iter = objectIter->second.children.begin();
        while (iter != objectIter->second.children.end()) {
          DO_VALIDATION;

          if (iter->first == "filename") {
            DO_VALIDATION;
            aseFilename = iter->second.value;
            //printf("geom file: %s\n", aseFilename.c_str());
          }
          if (iter->first == "name") {
            DO_VALIDATION;
            objectName = iter->second.value;
            //printf("geom name: %s\n", objectName.c_str());
          }
          if (iter->first == "position") {
            DO_VALIDATION;
            position = GetVectorFromString(iter->second.value) + offset;
          }
          if (iter->first == "rotation") {
            DO_VALIDATION;
            rotation = GetQuaternionFromString(iter->second.value);
          }
          if (iter->first == "properties") {
            DO_VALIDATION;
            InterpretProperties(iter->second.children, properties);
          }
          if (iter->first == "localmode") {
            DO_VALIDATION;
            localMode = InterpretLocalMode(iter->second.value);
          }

          iter++;
        }
        boost::intrusive_ptr<Resource<GeometryData> > geometry =
            GetContext().geometry_manager.Fetch(dirpart + aseFilename, true);
        boost::intrusive_ptr<Geometry> object(new Geometry(objectName));
        if (properties.GetBool("dynamic")) geometry->GetResource()->SetDynamic(true);

        object->SetProperties(properties);
        GetScene3D()->CreateSystemObjects(object);
        object->SetLocalMode(localMode);
        object->SetPosition(position);
        object->SetRotation(rotation);
        object->SetGeometryData(geometry);
        objNode->AddObject(object);
      }

      // LIGHT

      else if (objectIter->first == "light") {
        DO_VALIDATION;
        Vector3 position;

        map_XMLTree::const_iterator iter = objectIter->second.children.begin();
        while (iter != objectIter->second.children.end()) {
          DO_VALIDATION;

          if (iter->first == "name") {
            DO_VALIDATION;
            objectName = iter->second.value;
          }
          if (iter->first == "position") {
            DO_VALIDATION;
            position = GetVectorFromString(iter->second.value);
          }
          if (iter->first == "properties") {
            DO_VALIDATION;
            InterpretProperties(iter->second.children, properties);
          }
          if (iter->first == "localmode") {
            DO_VALIDATION;
            localMode = InterpretLocalMode(iter->second.value);
            //if (localMode == e_localMode_
          }

          iter++;
        }

        boost::intrusive_ptr<Light> object(new Light(objectName));

        //object->SetProperties(properties);
        GetScene3D()->CreateSystemObjects(object);
        object->SetLocalMode(localMode);
        object->SetColor(GetVectorFromString(properties.Get("color")));
        object->SetRadius(properties.GetReal("radius"));
        if (properties.Get("type") == "directional") {
          DO_VALIDATION;
          object->SetType(e_LightType_Directional);
        } else {
          object->SetType(e_LightType_Point);
        }
        object->SetShadow(properties.GetBool("shadow"));
        object->SetPosition(position);
        objNode->AddObject(object);
      }
      objectIter++;
    }

    return objNode;
  }

  void ObjectLoader::InterpretProperties(const map_XMLTree &tree, Properties &properties) const {
    map_XMLTree::const_iterator propIter = tree.begin();
    while (propIter != tree.end()) {
      DO_VALIDATION;
      properties.Set(propIter->first.c_str(), propIter->second.value);
      //printf("%s %s\n", propIter->first.c_str(), propIter->second.value.c_str());
      propIter++;
    }
  }

  e_LocalMode ObjectLoader::InterpretLocalMode(const std::string &value) const {
    if (value.compare("absolute") == 0) {
      DO_VALIDATION;
      return e_LocalMode_Absolute;
    } else {
      return e_LocalMode_Relative;
    }
  }

}
