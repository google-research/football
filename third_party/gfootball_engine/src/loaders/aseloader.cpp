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

#include "aseloader.hpp"

#include "../base/log.hpp"
#include "../base/math/bluntmath.hpp"
#include "../base/math/matrix3.hpp"
#include "../base/geometry/trianglemeshutils.hpp"
#include "../base/geometry/triangle.hpp"
#include "../managers/resourcemanagerpool.hpp"

#include <fstream>

namespace blunted {

  ASELoader::ASELoader() : Loader<GeometryData>(), triangleCount(0) {
  }

  ASELoader::~ASELoader() {
  }


  // ----- encapsulating load function

  // load file into resource
  void ASELoader::Load(std::string filename, boost::intrusive_ptr < Resource <GeometryData> > resource) {
    triangleCount = 0;
    s_tree *data = tree_load(filename);
    Build(data, resource);
    delete data;
    //printf("%s: %i total triangles\n", filename.c_str(), triangleCount);
  }


  // ----- interpreter for the .ase treedata

  void ASELoader::Build(const s_tree *data, boost::intrusive_ptr < Resource <GeometryData> > resource) {

    assert(data);

    assert(tree_find(data, "SCENE"));
    std::string meshname = treeentry_find(tree_find(data, "SCENE"), "SCENE_FILENAME")->values.at(0);


    // materials

    std::vector<s_Material> materialList;

    const s_tree *material_list = tree_find(data, "MATERIAL_LIST");
    assert(material_list);
    assert(material_list->entries.at(0));
    int material_count = atoi(material_list->entries.at(0)->values.at(0).c_str());
    for (int m = 0; m < material_count; m++) {
      assert(material_list->entries.at(m + 1));
      assert(material_list->entries.at(m + 1)->subtree);
      const s_tree *material_tree = material_list->entries.at(m + 1)->subtree;
      assert(material_tree);

      const s_treeentry *shine = treeentry_find(material_tree, "MATERIAL_SHINE");
      const s_treeentry *shinestrength = treeentry_find(material_tree, "MATERIAL_SHINESTRENGTH");
      const s_treeentry *self_illumination = treeentry_find(material_tree, "MATERIAL_SELFILLUM");

      const s_tree *maps[4];
      maps[0] = tree_find(material_tree, "MAP_DIFFUSE");
      maps[1] = tree_find(material_tree, "MAP_BUMP");
      maps[2] = tree_find(material_tree, "MAP_SHINE");
      maps[3] = tree_find(material_tree, "MAP_SELFILLUM");
      s_Material mat;

      for (int i = 0; i < 4; i++) {
        if (maps[i]) {
          const s_treeentry *bitmap = treeentry_find(maps[i], "BITMAP");
          assert(bitmap);
          mat.maps[i].assign(bitmap->values.at(0));
          mat.maps[i] = mat.maps[i].substr(1, mat.maps[i].length() - 2);
          //printf("%s\n", mat.maps[i].c_str());
        } else {
          if (i == 0) mat.maps[i].assign("orange.jpg"); else mat.maps[i].assign("");
        }
      }

      mat.shininess = shine->values.at(0);
      mat.specular_amount = shinestrength->values.at(0);
      mat.self_illumination.Set(atof(self_illumination->values.at(0).c_str()));

      materialList.push_back(mat);
    }

    for (unsigned int i = 0; i < data->entries.size(); i++) {
      if (data->entries[i]->name.compare("GEOMOBJECT") == 0) {
        if (data->entries[i]->subtree) if (data->entries[i]->subtree->entries.at(0)) {
          BuildTriangleMesh(data->entries[i]->subtree, resource, materialList);
        }
      }
    }
  }

  void ASELoader::BuildTriangleMesh(const s_tree *data, boost::intrusive_ptr < Resource <GeometryData> > resource, std::vector <s_Material> materialList) {
    assert(data);

    std::string name = data->entries.at(0)->values.at(0).substr(1, data->entries.at(0)->values.at(0).length() - 2);

    Vector3 maxcoords(0, 0, 0); // xyz[min .. max]
    Vector3 mincoords(0, 0, 0);

    const s_tree *tree_node_tm = tree_find(data, "NODE_TM");
    if (!tree_node_tm) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "subtree NODE_TM not found");

    const s_tree *tree_mesh = tree_find(data, "MESH");
    if (!tree_mesh) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "subtree MESH not found");


    // counts

    const s_treeentry *entry_mesh_numvertex = treeentry_find(tree_mesh, "MESH_NUMVERTEX");
    if (!entry_mesh_numvertex) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "tree entry MESH_NUMVERTEX not found");
    int numvertex = atoi(entry_mesh_numvertex->values.at(0).c_str());

    const s_treeentry *entry_mesh_numtvertex = treeentry_find(tree_mesh, "MESH_NUMTVERTEX");
    if (!entry_mesh_numtvertex) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "tree entry MESH_NUMTVERTEX not found");
    int numtvertex = atoi(entry_mesh_numtvertex->values.at(0).c_str());

    const s_treeentry *entry_mesh_numfaces = treeentry_find(tree_mesh, "MESH_NUMFACES");
    if (!entry_mesh_numfaces) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "tree entry MESH_NUMFACES not found");
    int numfaces = atoi(entry_mesh_numfaces->values.at(0).c_str());

    const s_treeentry *entry_mesh_numtfaces = NULL;
    int numtfaces = 0;
    if (numtvertex > 0) {
      entry_mesh_numtfaces = treeentry_find(tree_mesh, "MESH_NUMTVFACES"); // notice the 3ds .ase export typo ('v')
      if (!entry_mesh_numtfaces) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "tree entry MESH_NUMTFACES not found");
      numtfaces = atoi(entry_mesh_numtfaces->values.at(0).c_str());
    }


    // data

    const s_tree *tree_mesh_vertex_list = tree_find(tree_mesh, "MESH_VERTEX_LIST");
    if (!tree_mesh_vertex_list) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "subtree MESH_VERTEX_LIST not found");

    const s_tree *tree_mesh_face_list = tree_find(tree_mesh, "MESH_FACE_LIST");
    if (!tree_mesh_face_list) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "subtree MESH_FACE_LIST not found");

    const s_tree *tree_mesh_tvertex_list = NULL;
    if (numtvertex > 0) {
      tree_mesh_tvertex_list = tree_find(tree_mesh, "MESH_TVERTLIST");
      if (!tree_mesh_tvertex_list) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "subtree MESH_TVERTLIST not found");
    }

    const s_tree *tree_mesh_tface_list = NULL;
    if (numtfaces > 0) {
      tree_mesh_tface_list = tree_find(tree_mesh, "MESH_TFACELIST");
      if (!tree_mesh_tface_list) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "subtree MESH_TFACELIST not found");
    }

    const s_tree *tree_mesh_normals = tree_find(tree_mesh, "MESH_NORMALS");
    if (!tree_mesh_normals) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "subtree MESH_NORMALS not found");


    /* vertices
      MESH_VERTEX_LIST {
        MESH_VERTEX 0 0.000 500.000 0.000
        ..
    */

    Vector3 vertex_cache[numvertex];

    if (tree_mesh_vertex_list->entries.size() != (unsigned int)numvertex) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "numvertex and tree_mesh_vertex_list->entries.size() differ! Loader corrupt?");

    for (unsigned int i = 0; i < tree_mesh_vertex_list->entries.size(); i++) {
      const s_treeentry *entry = tree_mesh_vertex_list->entries[i]; // less typing

      if (atoi(entry->values.at(0).c_str()) >= numvertex) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "vertex index larger than numvertex! Loader corrupt?");
      vertex_cache[atoi(entry->values.at(0).c_str())].coords[0] = atof(entry->values.at(1).c_str());
      vertex_cache[atoi(entry->values.at(0).c_str())].coords[1] = atof(entry->values.at(2).c_str());
      vertex_cache[atoi(entry->values.at(0).c_str())].coords[2] = atof(entry->values.at(3).c_str());

      if (vertex_cache[atoi(entry->values.at(0).c_str())].coords[0] > maxcoords.coords[0]) maxcoords.coords[0] = vertex_cache[atoi(entry->values.at(0).c_str())].coords[0];
      if (vertex_cache[atoi(entry->values.at(0).c_str())].coords[0] < mincoords.coords[0]) mincoords.coords[0] = vertex_cache[atoi(entry->values.at(0).c_str())].coords[0];
      if (vertex_cache[atoi(entry->values.at(0).c_str())].coords[1] > maxcoords.coords[1]) maxcoords.coords[1] = vertex_cache[atoi(entry->values.at(0).c_str())].coords[1];
      if (vertex_cache[atoi(entry->values.at(0).c_str())].coords[1] < mincoords.coords[1]) mincoords.coords[1] = vertex_cache[atoi(entry->values.at(0).c_str())].coords[1];
      if (vertex_cache[atoi(entry->values.at(0).c_str())].coords[2] > maxcoords.coords[2]) maxcoords.coords[2] = vertex_cache[atoi(entry->values.at(0).c_str())].coords[2];
      if (vertex_cache[atoi(entry->values.at(0).c_str())].coords[2] < mincoords.coords[2]) mincoords.coords[2] = vertex_cache[atoi(entry->values.at(0).c_str())].coords[2];
    }

    /* faces
      MESH_FACE_LIST {
        MESH_FACE 0: A: 0 B: 2 C: 1 AB: 0 BC: 1 CA: 0 *MESH_SMOOTHING 1 *MESH_MTLID 1
        MESH_FACE 1: A: 0 B: 3 C: 2 AB: 0 BC: 1 CA: 0 *MESH_SMOOTHING 1 *MESH_MTLID 1
        ..
    */

    if (tree_mesh_face_list->entries.size() != (unsigned int)numfaces) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "numfaces and tree_mesh_face_list->entries.size() differ! Loader corrupt?");

    std::vector<Triangle*> triangles;

    for (unsigned int i = 0; i < tree_mesh_face_list->entries.size(); i++) {
      Triangle *triangle = new Triangle();
      s_treeentry *entry = tree_mesh_face_list->entries[i]; // less typing

      if (atoi(entry->values.at(0).c_str()) >= numfaces) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "Face index larger than numfaces! Loader corrupt?");
      triangle->SetVertex(0, vertex_cache[atoi(entry->values.at(2).c_str())]);
      triangle->SetVertex(1, vertex_cache[atoi(entry->values.at(4).c_str())]);
      triangle->SetVertex(2, vertex_cache[atoi(entry->values.at(6).c_str())]);

      triangles.push_back(triangle);
    }

    /* normals
      MESH_NORMALS {
        MESH_FACENORMAL 0 0.000 -0.000 -1.000
          MESH_VERTEXNORMAL 0 0.000 0.000 -1.000
          MESH_VERTEXNORMAL 2 0.000 -0.000 -1.000
          MESH_VERTEXNORMAL 1 0.000 0.000 -1.000
        MESH_FACENORMAL 1 0.000 -0.000 -1.000
          MESH_VERTEXNORMAL 0 0.000 0.000 -1.000
          ..
    */

    // normal rotation matrix
    Matrix3 rotation_matrix;

    rotation_matrix.elements[0] = atof(tree_node_tm->entries.at(4)->values.at(0).c_str());
    rotation_matrix.elements[1] = atof(tree_node_tm->entries.at(4)->values.at(1).c_str());
    rotation_matrix.elements[2] = atof(tree_node_tm->entries.at(4)->values.at(2).c_str());

    rotation_matrix.elements[3] = atof(tree_node_tm->entries.at(5)->values.at(0).c_str());
    rotation_matrix.elements[4] = atof(tree_node_tm->entries.at(5)->values.at(1).c_str());
    rotation_matrix.elements[5] = atof(tree_node_tm->entries.at(5)->values.at(2).c_str());

    rotation_matrix.elements[6] = atof(tree_node_tm->entries.at(6)->values.at(0).c_str());
    rotation_matrix.elements[7] = atof(tree_node_tm->entries.at(6)->values.at(1).c_str());
    rotation_matrix.elements[8] = atof(tree_node_tm->entries.at(6)->values.at(2).c_str());

    normalize(&rotation_matrix.elements[0]);
    normalize(&rotation_matrix.elements[3]);
    normalize(&rotation_matrix.elements[6]);

    if (tree_mesh_normals->entries.size() / 4 != (unsigned int)numfaces) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "numfaces and tree_mesh_normals->entries.size() * 4 differ! Loader corrupt?");

    // face normals
    for (unsigned int i = 0; i < tree_mesh_normals->entries.size(); i += 4) {

      s_treeentry *entry_normal = tree_mesh_normals->entries[i]; // less typing

      // vertex normals
      for (int v = 0; v < 3; v++) {
        s_treeentry *entry_vertexnormal = tree_mesh_normals->entries.at(i + v + 1); // less typing

        Vector3 normal3(atof(entry_vertexnormal->values.at(1).c_str()), atof(entry_vertexnormal->values.at(2).c_str()), atof(entry_vertexnormal->values.at(3).c_str()));
        normal3 *= rotation_matrix;
        normal3.Normalize();

        triangles.at(atoi(entry_normal->values.at(0).c_str()))->SetNormal(v, normal3.coords[0], normal3.coords[1], normal3.coords[2]);
      }

      // D.I.Y. normals *disables smooth normals*
      /*
      Vector3 v0 = triangles.at(atoi(entry_normal->values.at(0).c_str()))->GetVertex(0);
      Vector3 v1 = triangles.at(atoi(entry_normal->values.at(0).c_str()))->GetVertex(1);
      Vector3 v2 = triangles.at(atoi(entry_normal->values.at(0).c_str()))->GetVertex(2);
      Vector3 normal4 = Vector3(v2 - v0).GetCrossProduct(Vector3(v2 - v1));
      normal4.Normalize();
      triangles.at(atoi(entry_normal->values.at(0).c_str()))->SetNormal(0, normal4);
      triangles.at(atoi(entry_normal->values.at(0).c_str()))->SetNormal(1, normal4);
      triangles.at(atoi(entry_normal->values.at(0).c_str()))->SetNormal(2, normal4);
      */
    }

    /* texture vertices
      MESH_TVERT_LIST {
        MESH_TVERT 0 0.00 0.00 0.00
        ..
    */

    if (numtvertex > 0) {
      Vector3 tvertex_cache[numtvertex];

      if (tree_mesh_tvertex_list->entries.size() != (unsigned int)numtvertex) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "numtvertex and tree_mesh_tvertex_list->entries.size() differ! Loader corrupt?");

      for (unsigned int i = 0; i < tree_mesh_tvertex_list->entries.size(); i++) {
        s_treeentry *entry = tree_mesh_tvertex_list->entries[i]; // less typing

        if (atoi(entry->values.at(0).c_str()) >= numtvertex) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "tvertex index larger than numtvertex! Loader corrupt?");
        tvertex_cache[atoi(entry->values.at(0).c_str())].coords[0] = atof(entry->values.at(1).c_str());
        tvertex_cache[atoi(entry->values.at(0).c_str())].coords[1] = -atof(entry->values.at(2).c_str());
        tvertex_cache[atoi(entry->values.at(0).c_str())].coords[2] = atof(entry->values.at(3).c_str());
      }

      /* texture faces
        MESH_TFACE_LIST {
          MESH_TFACE 0 9 11 10
          ..
      */

      if (tree_mesh_tface_list->entries.size() != (unsigned int)numtfaces) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "numtfaces and tree_mesh_tface_list->entries.size() differ! Loader corrupt?");

      for (unsigned int i = 0; i < tree_mesh_tface_list->entries.size(); i++) {
        s_treeentry *entry = tree_mesh_tface_list->entries[i]; // less typing

        if (atoi(entry->values.at(0).c_str()) >= numtfaces) Log(e_FatalError, "ASELoader", "BuildTriangleMesh", "tface index larger than numtfaces! Loader corrupt?");
        for (int x = 0; x < 3; x++) {
          // use texture units 0, 2 and 3 for texture, bump and specular map respectively
          int texunit = x;
          if (texunit > 0) texunit++;
          triangles.at(atoi(entry->values.at(0).c_str()))->SetTextureVertex(texunit, 0, tvertex_cache[atoi(entry->values.at(1).c_str())].coords[0], tvertex_cache[atoi(entry->values.at(1).c_str())].coords[1], tvertex_cache[atoi(entry->values.at(1).c_str())].coords[2]);
          triangles.at(atoi(entry->values.at(0).c_str()))->SetTextureVertex(texunit, 1, tvertex_cache[atoi(entry->values.at(2).c_str())].coords[0], tvertex_cache[atoi(entry->values.at(2).c_str())].coords[1], tvertex_cache[atoi(entry->values.at(2).c_str())].coords[2]);
          triangles.at(atoi(entry->values.at(0).c_str()))->SetTextureVertex(texunit, 2, tvertex_cache[atoi(entry->values.at(3).c_str())].coords[0], tvertex_cache[atoi(entry->values.at(3).c_str())].coords[1], tvertex_cache[atoi(entry->values.at(3).c_str())].coords[2]);
        }
      }
    }


    int tmeshsize = triangles.size() * 3 * 3 * GetTriangleMeshElementCount();
    float *rawTriangleMesh = new float[tmeshsize];

    for (unsigned int t = 0; t < triangles.size(); t++) {
      triangles[t]->CalculateTangents();
      for (int v = 0; v < 3; v++) {
        memcpy(&rawTriangleMesh[0 * triangles.size() * 9 + t * 9 + v * 3], triangles[t]->GetVertex(v).coords, sizeof(float) * 3);
        memcpy(&rawTriangleMesh[1 * triangles.size() * 9 + t * 9 + v * 3], triangles[t]->GetNormal(v).coords, sizeof(float) * 3);
        memcpy(&rawTriangleMesh[2 * triangles.size() * 9 + t * 9 + v * 3], triangles[t]->GetTextureVertex(v).coords, sizeof(float) * 3);
        memcpy(&rawTriangleMesh[3 * triangles.size() * 9 + t * 9 + v * 3], triangles[t]->GetTangent(v).coords, sizeof(float) * 3);
        memcpy(&rawTriangleMesh[4 * triangles.size() * 9 + t * 9 + v * 3], triangles[t]->GetBiTangent(v).coords, sizeof(float) * 3);
      }
    }

    triangleCount += triangles.size();

    for (unsigned int t = 0; t < triangles.size(); t++) {
      delete triangles.at(t);
    }


    // material

    Material material;

    signed int material_reference = -1;
    assert(data);
    const s_treeentry *entry_material_ref = treeentry_find(data, "MATERIAL_REF");
    if (entry_material_ref) material_reference = atoi(entry_material_ref->values.at(0).c_str());

    std::string matname;
    if (material_reference == -1) {
      matname.assign("standard material");

    } else {


      // use texture filename as material name
      matname.assign(materialList.at(material_reference).maps[0]);
      material.diffuseTexture = ResourceManagerPool::getSurfaceManager()->Fetch(matname, true, true);
      matname.assign(materialList.at(material_reference).maps[1]);
      if (matname.length() > 0) {
        material.normalTexture = ResourceManagerPool::getSurfaceManager()->Fetch(matname, true, true);
      }
      matname.assign(materialList.at(material_reference).maps[2]);
      if (matname.length() > 0) {
        material.specularTexture = ResourceManagerPool::getSurfaceManager()->Fetch(matname, true, true);
      }
      matname.assign(materialList.at(material_reference).maps[3]);
      if (matname.length() > 0) {
        material.illuminationTexture = ResourceManagerPool::getSurfaceManager()->Fetch(matname, true, true);
      }

      material.shininess = atof(materialList.at(material_reference).shininess.c_str());
      material.specular_amount = atof(materialList.at(material_reference).specular_amount.c_str());
      material.self_illumination = materialList.at(material_reference).self_illumination;

    }

    std::vector<unsigned int> indices;
    resource->GetResource()->AddTriangleMesh(material, rawTriangleMesh, tmeshsize, indices);
  }

}
