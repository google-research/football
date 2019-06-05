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

#ifndef _HPP_LOADERS_ASE
#define _HPP_LOADERS_ASE

#include "../defines.hpp"
#include "../base/utils.hpp"
#include "../managers/resourcemanager.hpp"
#include "../scene/resources/geometrydata.hpp"
#include "../scene/objects/geometry.hpp"

namespace blunted {

  struct s_Material {
    std::string maps[4];
    std::string shininess;
    std::string specular_amount;
    Vector3 self_illumination;
  };

  class ASELoader : public Loader<GeometryData> {

    public:
      ASELoader();
      virtual ~ASELoader();

      // ----- encapsulating load function
      virtual void Load(std::string filename, boost::intrusive_ptr < Resource <GeometryData> > resource);

      // ----- interpreter for the .ase treedata
      void Build(const s_tree *data, boost::intrusive_ptr < Resource <GeometryData> > resource);

      // ----- per-object interpreters
      void BuildTriangleMesh(const s_tree *data, boost::intrusive_ptr < Resource <GeometryData> > resource, std::vector <s_Material> materialList);

    protected:

      int triangleCount = 0;

  };

}

#endif
