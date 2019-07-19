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

// written by bastiaan konings schuiling 2008 - 2015
// this work is public domain. the code is undocumented, scruffy, untested, and should generally not be used for anything important.
// i do not offer support, so don't ask. to be used for inspiration :)

#include <cmath>
#include "humanoid.hpp"

#include "humanoid_utils.hpp"

#include "../playerbase.hpp"
#include "../../match.hpp"

#include "../../../main.hpp"

#include "../../AIsupport/AIfunctions.hpp"

#include "../../../utils/animationextensions/footballanimationextension.hpp"

#include "../../../managers/resourcemanagerpool.hpp"

#include "../../../scene/objectfactory.hpp"

const float bodyRotationSmoothingFactor = 1.0f;
const float bodyRotationSmoothingMaxAngle = 0.25f * pi;
const float initialReQueueDelayFrames = 32;

void FillTemporalHumanoidNodes(boost::intrusive_ptr<Node> targetNode, std::vector<TemporalHumanoidNode> &temporalHumanoidNodes) {
  //printf("%s\n", targetNode->GetName().c_str());
  TemporalHumanoidNode temporalHumanoidNode;
  temporalHumanoidNode.actualNode = targetNode;
  temporalHumanoidNode.cachedPosition = targetNode->GetPosition();
  temporalHumanoidNode.cachedOrientation = targetNode->GetRotation();
  // initial values, not sure if really needed
  temporalHumanoidNode.position.SetValue(targetNode->GetPosition(), EnvironmentManager::GetInstance().GetTime_ms());
  temporalHumanoidNode.orientation.SetValue(targetNode->GetRotation(), EnvironmentManager::GetInstance().GetTime_ms());
  temporalHumanoidNodes.push_back(temporalHumanoidNode);

  std::vector < boost::intrusive_ptr<Node> > gatherNodes;
  targetNode->GetNodes(gatherNodes);
  for (unsigned int i = 0; i < gatherNodes.size(); i++) {
    FillTemporalHumanoidNodes(gatherNodes[i], temporalHumanoidNodes);
  }

}

HumanoidBase::HumanoidBase(PlayerBase *player, Match *match, boost::intrusive_ptr<Node> humanoidSourceNode, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::shared_ptr<AnimCollection> animCollection, boost::intrusive_ptr<Node> fullbodyTargetNode, boost::intrusive_ptr < Resource<Surface> > kit, int bodyUpdatePhaseOffset) : fullbodyTargetNode(fullbodyTargetNode), match(match), player(player), anims(animCollection), buf_bodyUpdatePhaseOffset(bodyUpdatePhaseOffset) {

  interruptAnim = e_InterruptAnim_None;
  reQueueDelayFrames = 0;

  buf_LowDetailMode = false;
  fetchedbuf_previousSnapshotTime_ms = 0;

  currentAnim = new Anim();
  previousAnim = new Anim();

  decayingPositionOffset = Vector3(0);
  decayingDifficultyFactor = 0.0f;

  _cache_AgilityFactor = GetConfiguration()->GetReal("gameplay_agilityfactor", _default_AgilityFactor);
  _cache_AccelerationFactor = GetConfiguration()->GetReal("gameplay_accelerationfactor", _default_AccelerationFactor);

  allowedBodyDirVecs.push_back(Vector3(0, -1, 0));
  allowedBodyDirVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.25 * pi));
  allowedBodyDirVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.25 * pi));
  allowedBodyDirVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.75 * pi));
  allowedBodyDirVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.75 * pi));

  allowedBodyDirAngles.push_back(0 * pi);
  allowedBodyDirAngles.push_back(0.25 * pi);
  allowedBodyDirAngles.push_back(-0.25 * pi);
  allowedBodyDirAngles.push_back(0.75 * pi);
  allowedBodyDirAngles.push_back(-0.75 * pi);

  preferredDirectionVecs.push_back(Vector3(0, -1, 0));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.111 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.111 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.25 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.25 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.5 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.5 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.75 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.75 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(0.999 * pi));
  preferredDirectionVecs.push_back(Vector3(0, -1, 0).GetRotated2D(-0.999 * pi));

  preferredDirectionAngles.push_back(0 * pi);
  preferredDirectionAngles.push_back(0.111 * pi); // 20
  preferredDirectionAngles.push_back(-0.111 * pi);
  preferredDirectionAngles.push_back(0.25 * pi); // 45
  preferredDirectionAngles.push_back(-0.25 * pi);
  preferredDirectionAngles.push_back(0.5 * pi); // 90
  preferredDirectionAngles.push_back(-0.5 * pi);
  preferredDirectionAngles.push_back(0.75 * pi); // 135
  preferredDirectionAngles.push_back(-0.75 * pi);
  preferredDirectionAngles.push_back(0.999 * pi); // 180
  preferredDirectionAngles.push_back(-0.999 * pi);

  assert(match);

  float playerHeight = player->GetPlayerData()->GetHeight();
  zMultiplier = (1.0f / defaultPlayerHeight) * playerHeight;

  boost::intrusive_ptr<Node> bla(new Node(*humanoidSourceNode.get(), "", GetScene3D()));
  humanoidNode = bla;
  humanoidNode->SetLocalMode(e_LocalMode_Absolute);

  boost::intrusive_ptr < Resource<Surface> > skin;
  skin = ResourceManagerPool::getSurfaceManager()->Fetch("media/objects/players/textures/skin0" + int_to_str(player->GetPlayerData()->GetSkinColor()) + ".png", true, true);

  boost::intrusive_ptr<Node> bla2(new Node(*fullbodySourceNode.get(), int_to_str(player->GetID()), GetScene3D()));
  fullbodyNode = bla2;
  fullbodyNode->SetLocalMode(e_LocalMode_Absolute);
  fullbodyTargetNode->AddNode(fullbodyNode);

  boost::intrusive_ptr< Resource<GeometryData> > bodyGeom = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();
  std::vector < MaterializedTriangleMesh > &tmesh = bodyGeom->GetResource()->GetTriangleMeshesRef();
  for (unsigned int i = 0; i < tmesh.size(); i++) {
    if (tmesh[i].material.diffuseTexture != boost::intrusive_ptr< Resource<Surface> >()) {
      if (tmesh[i].material.diffuseTexture->GetIdentString() == "skin.jpg") {
        tmesh[i].material.diffuseTexture = skin;
        tmesh[i].material.specular_amount = 0.002f;
        tmesh[i].material.shininess = 0.2f;
      }
    }
  }

  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData();

  kitDiffuseTextureIdentString = "kit_template.png";
  SetKit(kit);


  scene3D = GetScene3D();

  FillNodeMap(humanoidNode, nodeMap);
  FillTemporalHumanoidNodes(humanoidNode, buf_TemporalHumanoidNodes);

  PrepareFullbodyModel(colorCoords);
  buf_bodyUpdatePhase = 0;


  // hairstyle

  boost::intrusive_ptr < Resource<GeometryData> > geometry = ResourceManagerPool::getGeometryManager()->Fetch("media/objects/players/hairstyles/" + player->GetPlayerData()->GetHairStyle() + ".ase", true, true);
  hairStyle = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("hairstyle", e_ObjectType_Geometry));

  scene3D->CreateSystemObjects(hairStyle);
  hairStyle->SetLocalMode(e_LocalMode_Absolute);
  hairStyle->SetGeometryData(geometry);
  fullbodyTargetNode->AddObject(hairStyle);

  boost::intrusive_ptr < Resource<Surface> > hairTexture;
  hairTexture = ResourceManagerPool::getSurfaceManager()->Fetch("media/objects/players/textures/hair/" + player->GetPlayerData()->GetHairColor() + ".png", true, true);

  std::vector < MaterializedTriangleMesh > &hairtmesh = hairStyle->GetGeometryData()->GetResource()->GetTriangleMeshesRef();

  for (unsigned int i = 0; i < hairtmesh.size(); i++) {
    if (hairtmesh[i].material.diffuseTexture != boost::intrusive_ptr<Resource <Surface> >()) {
      hairtmesh[i].material.diffuseTexture = hairTexture;
      hairtmesh[i].material.specular_amount = 0.01f;
      hairtmesh[i].material.shininess = 0.05f;
    }
  }
  hairStyle->OnUpdateGeometryData();


  ResetPosition(Vector3(0), Vector3(0));

  currentMentalImage = 0;
}

HumanoidBase::~HumanoidBase() {
  humanoidNode->Exit();
  humanoidNode.reset();
  fullbodyTargetNode->DeleteNode(fullbodyNode);
  fullbodyTargetNode->DeleteObject(hairStyle);

  buf_TemporalHumanoidNodes.clear();

  for (unsigned int i = 0; i < uniqueFullbodyMesh.size(); i++) {
    delete [] uniqueFullbodyMesh[i].data;
  }

  for (unsigned int i = 0; i < uniqueIndicesVec.size(); i++) {
    delete [] uniqueIndicesVec[i];
  }

  // the other full body mesh ref is connected to a geometry object which has taken over ownership and will clean it up

  delete currentAnim;
  delete previousAnim;
}

void HumanoidBase::PrepareFullbodyModel(std::map<Vector3, Vector3> &colorCoords) {

  // base anim with default angles - all anims' joints will be inversely rotated by the joints in this anim. this way, the fullbody mesh doesn't need to have 0 degree angles
  Animation *baseAnim = new Animation();
  baseAnim->Load("media/animations/base.anim.util");
  AnimApplyBuffer animApplyBuffer;
  animApplyBuffer.anim = baseAnim;
  animApplyBuffer.frameNum = 0;
  animApplyBuffer.smooth = false;
  animApplyBuffer.smoothFactor = 0.0f;
  animApplyBuffer.position = Vector3(0);
  animApplyBuffer.orientation = 0;
  animApplyBuffer.anim->Apply(nodeMap, animApplyBuffer.frameNum, 0, animApplyBuffer.smooth, animApplyBuffer.smoothFactor, animApplyBuffer.position, animApplyBuffer.orientation, animApplyBuffer.offsets, 0, false, true);
  std::vector < boost::intrusive_ptr<Node> > jointsVec;
  humanoidNode->GetNodes(jointsVec, true);


  // joints

  for (unsigned int i = 0; i < jointsVec.size(); i++) {
    Joint joint;
    joint.node = jointsVec[i];
    joint.origPos = jointsVec[i]->GetDerivedPosition();
    joints.push_back(joint);
  }

  boost::intrusive_ptr < Resource<GeometryData> > fullbodyGeometryData = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();
  std::vector < MaterializedTriangleMesh > &materializedTriangleMeshes = fullbodyGeometryData->GetResource()->GetTriangleMeshesRef();

  fullbodySubgeomCount = materializedTriangleMeshes.size();

  for (unsigned int subgeom = 0; subgeom < fullbodySubgeomCount; subgeom++) {

    std::vector<WeightedVertex> weightedVertexVector;
    weightedVerticesVec.push_back(weightedVertexVector);

    FloatArray meshRef;
    meshRef.data = materializedTriangleMeshes.at(subgeom).vertices;
    meshRef.size = materializedTriangleMeshes.at(subgeom).verticesDataSize;

    FloatArray uniqueMesh;

    int elementOffset = meshRef.size / GetTriangleMeshElementCount(); // was: fullbodyMeshSize

    // map is used for duplicate 'search'
    std::vector < std::vector<Vector3> > uniqueVertices;

    // generate list of unique vertices and an array linking vertexIDs with uniqueVertexIDs
    int *uniqueIndices = new int[elementOffset / 3];
    for (int v = 0; v < elementOffset; v += 3) {
      std::vector<Vector3> elementalVertex;
      for (int e = 0; e < GetTriangleMeshElementCount(); e++) {
        elementalVertex.push_back(Vector3(meshRef.data[v + e * elementOffset], meshRef.data[v + e * elementOffset + 1], meshRef.data[v + e * elementOffset + 2]));
        // test: if (e == 2) elementalVertex.at(elementalVertex.size() - 1) += 0.2f;
      }

      // see if this one already exists; if not, add
      bool duplicate = false;
      int index = 0;
      for (unsigned int i = 0; i < uniqueVertices.size(); i++) {
        if (uniqueVertices[i][0] == elementalVertex[0] &&
            uniqueVertices[i][2] == elementalVertex[2]) { // texcoord also needs to be shared
          duplicate = true;
          index = i;
          break;
        }
      }
      if (!duplicate) {
        uniqueVertices.push_back(elementalVertex);
        index = uniqueVertices.size() - 1;
      }
      uniqueIndices[v / 3] = index;
    }

    //printf("vertices: %i, unique vertices: %i\n", elementOffset / 3, uniqueVertices.size());
    //printf("meshRef.size: %i, uniqueIndices.size: %i\n", meshRef.size, elementOffset / 3);

    uniqueMesh.size = uniqueVertices.size() * 3 * GetTriangleMeshElementCount();
    uniqueMesh.data = new float[uniqueMesh.size];

    int uniqueElementOffset = uniqueMesh.size / GetTriangleMeshElementCount();

    assert((unsigned int)uniqueMesh.size == uniqueVertices.size() * GetTriangleMeshElementCount() * 3);

    for (unsigned int v = 0; v < uniqueVertices.size(); v++) {
      for (int e = 0; e < GetTriangleMeshElementCount(); e++) {
        uniqueMesh.data[v * 3 + e * uniqueElementOffset + 0] = uniqueVertices.at(v).at(e).coords[0];
        uniqueMesh.data[v * 3 + e * uniqueElementOffset + 1] = uniqueVertices.at(v).at(e).coords[1];
        uniqueMesh.data[v * 3 + e * uniqueElementOffset + 2] = uniqueVertices.at(v).at(e).coords[2];
      }
    }

    for (int v = 0; v < uniqueElementOffset; v += 3) {

      Vector3 vertexPos(uniqueMesh.data[v], uniqueMesh.data[v + 1], uniqueMesh.data[v + 2]);

      WeightedVertex weightedVertex;
      weightedVertex.vertexID = v / 3;

      if (colorCoords.find(vertexPos) == colorCoords.end()) {
        printf("color coord not found: %f, %f, %f\n", vertexPos.coords[0], vertexPos.coords[1], vertexPos.coords[2]);
      }
      assert(colorCoords.find(vertexPos) != colorCoords.end());
      const Vector3 &color = colorCoords.find(vertexPos)->second;
      uniqueMesh.data[v + 0] *= zMultiplier;
      uniqueMesh.data[v + 1] *= zMultiplier;
      uniqueMesh.data[v + 2] *= zMultiplier;

      float totalWeight = 0.0;
      WeightedBone weightedBones[3];
      for (int c = 0; c < 3; c++) {
        int jointID = floor(color.coords[c] * 0.1);
        float weight = (color.coords[c] - jointID * 10.0) / 9.0;

        weightedBones[c].jointID = jointID;
        weightedBones[c].weight = weight;

        totalWeight += weight;
      }

      // total weight has to be 1.0;
      for (int c = 0; c < 3; c++) {
        if (c == 0) {
          if (weightedBones[c].weight == 0.f) printf("offending jointID: %i (coord %i) (vertexpos %f, %f, %f)\n", weightedBones[c].jointID, c, vertexPos.coords[0], vertexPos.coords[1], vertexPos.coords[2]);
          assert(weightedBones[c].weight != 0.f);
        }
        if (weightedBones[c].weight > 0.01f) {
          weightedBones[c].weight /= totalWeight;
          weightedVertex.bones.push_back(weightedBones[c]);
        }
      }

      weightedVerticesVec.at(subgeom).push_back(weightedVertex);
    }

    uniqueFullbodyMesh.push_back(uniqueMesh);
    uniqueIndicesVec.push_back(uniqueIndices);


    // update geometry object so that it uses indices & shared vertices

    assert(materializedTriangleMeshes.at(subgeom).verticesDataSize / GetTriangleMeshElementCount() == elementOffset);
    assert(uniqueMesh.size == uniqueElementOffset * GetTriangleMeshElementCount());
    delete [] materializedTriangleMeshes.at(subgeom).vertices;
    materializedTriangleMeshes.at(subgeom).vertices = new float[uniqueMesh.size];
    memcpy(materializedTriangleMeshes.at(subgeom).vertices, uniqueMesh.data, uniqueMesh.size * sizeof(float));
    materializedTriangleMeshes.at(subgeom).verticesDataSize = uniqueMesh.size;
    materializedTriangleMeshes.at(subgeom).indices.clear();
    for (int v = 0; v < elementOffset; v += 3) {
      materializedTriangleMeshes.at(subgeom).indices.push_back(uniqueIndices[v / 3]);
    }

/*
    printf("\n");
    printf("optimized: ");
    for (int i = 0; i < elementOffset; i += 3) {
      printf("%f, ", materializedTriangleMeshes.at(subgeom).vertices[uniqueIndices[i / 3] * 3 + uniqueElementOffset * 2]);
    }
    printf("\n");
    printf("\n");
*/

  } // subgeom

  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData();

  for (unsigned int i = 0; i < joints.size(); i++) {
    joints[i].orientation = jointsVec[i]->GetDerivedRotation().GetInverse().GetNormalized();
  }

  Animation *straightAnim = new Animation();
  straightAnim->Load("media/animations/straight.anim.util");
  animApplyBuffer.anim = straightAnim;
  animApplyBuffer.anim->Apply(nodeMap, animApplyBuffer.frameNum, 0, animApplyBuffer.smooth, animApplyBuffer.smoothFactor, animApplyBuffer.position, animApplyBuffer.orientation, animApplyBuffer.offsets, 0, false, true);

  for (unsigned int i = 0; i < joints.size(); i++) {
    joints[i].position = jointsVec[i]->GetDerivedPosition();// * zMultiplier;
  }

  UpdateFullbodyModel(true);
  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData(false);

  for (unsigned int i = 0; i < joints.size(); i++) {
    joints[i].origPos = jointsVec[i]->GetDerivedPosition();
  }

  delete straightAnim;
  delete baseAnim;

  //printf("vertexcount: %i, unique vertexcount: %i\n", elementOffset / 3, uniqueElementOffset / 3);
}

void HumanoidBase::UpdateFullbodyNodes() {
  if (!GetScenarioConfig().render) {
    return;
  }

  Vector3 previousFullbodyOffset = fullbodyOffset;
  fullbodyOffset = humanoidNode->GetPosition().Get2D();
  fullbodyNode->SetPosition(fullbodyOffset);

  for (unsigned int i = 0; i < joints.size(); i++) {
    joints[i].orientation = joints[i].node->GetDerivedRotation();
    joints[i].position = joints[i].node->GetDerivedPosition() - fullbodyOffset;
  }
  if (GetScenarioConfig().render) {
    hairStyle->SetRotation(joints[2].orientation, false);
    hairStyle->SetPosition(joints[2].position * zMultiplier + fullbodyOffset, false);
    hairStyle->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  }
}

bool HumanoidBase::NeedsModelUpdate() {
  if (buf_LowDetailMode && buf_bodyUpdatePhase != 1 - buf_bodyUpdatePhaseOffset) return false; else return true;
}

void HumanoidBase::UpdateFullbodyModel(bool updateSrc) {
  if (!GetScenarioConfig().render) {
    return;
  }

  boost::intrusive_ptr < Resource<GeometryData> > fullbodyGeometryData = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();
  std::vector < MaterializedTriangleMesh > &materializedTriangleMeshes = fullbodyGeometryData->GetResource()->GetTriangleMeshesRef();

  for (unsigned int subgeom = 0; subgeom < fullbodySubgeomCount; subgeom++) {

    FloatArray &uniqueMesh = uniqueFullbodyMesh.at(subgeom);

    const std::vector<WeightedVertex> &weightedVertices = weightedVerticesVec.at(subgeom);

    int uniqueVertexCount = weightedVertices.size();

    int uniqueElementOffset = uniqueMesh.size / GetTriangleMeshElementCount();

    Vector3 origVertex;
    Vector3 origNormal;
    Vector3 origTangent;
    Vector3 origBitangent;
    Vector3 resultVertex;
    Vector3 resultNormal;
    Vector3 resultTangent;
    Vector3 resultBitangent;
    Vector3 adaptedVertex;
    Vector3 adaptedNormal;
    Vector3 adaptedTangent;
    Vector3 adaptedBitangent;

    for (int v = 0; v < uniqueVertexCount; v++) {
      memcpy(origVertex.coords,    &uniqueMesh.data[weightedVertices[v].vertexID * 3],                           3 * sizeof(float)); // was: uniqueFullbodyMeshSrc
      memcpy(origNormal.coords,    &uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset],     3 * sizeof(float));
      memcpy(origTangent.coords,   &uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 3], 3 * sizeof(float));
      memcpy(origBitangent.coords, &uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 4], 3 * sizeof(float));

      if (weightedVertices[v].bones.size() == 1) {

        resultVertex = origVertex;
        resultVertex -= joints[weightedVertices[v].bones[0].jointID].origPos * zMultiplier;
        resultVertex.Rotate(joints[weightedVertices[v].bones[0].jointID].orientation);
        resultVertex += joints[weightedVertices[v].bones[0].jointID].position * zMultiplier;

        resultNormal = origNormal;
        resultNormal.Rotate(joints[weightedVertices[v].bones[0].jointID].orientation);

        resultTangent = origTangent;
        resultTangent.Rotate(joints[weightedVertices[v].bones[0].jointID].orientation);

        resultBitangent = origBitangent;
        resultBitangent.Rotate(joints[weightedVertices[v].bones[0].jointID].orientation);

      } else {

        resultVertex.Set(0);
        resultNormal.Set(0);
        resultTangent.Set(0);
        resultBitangent.Set(0);

        for (unsigned int b = 0; b < weightedVertices[v].bones.size(); b++) {

          adaptedVertex = origVertex;
          adaptedVertex -= joints[weightedVertices[v].bones[b].jointID].origPos * zMultiplier;
          adaptedVertex.Rotate(joints[weightedVertices[v].bones[b].jointID].orientation);
          adaptedVertex += joints[weightedVertices[v].bones[b].jointID].position * zMultiplier;
          resultVertex += adaptedVertex * weightedVertices[v].bones[b].weight;

          adaptedNormal = origNormal;
          adaptedNormal.Rotate(joints[weightedVertices[v].bones[b].jointID].orientation);
          resultNormal += adaptedNormal * weightedVertices[v].bones[b].weight;

          adaptedTangent = origTangent;
          adaptedTangent.Rotate(joints[weightedVertices[v].bones[b].jointID].orientation);
          resultTangent += adaptedTangent * weightedVertices[v].bones[b].weight;

          adaptedBitangent = origBitangent;
          adaptedBitangent.Rotate(joints[weightedVertices[v].bones[b].jointID].orientation);
          resultBitangent += adaptedBitangent * weightedVertices[v].bones[b].weight;

        }

        resultNormal.FastNormalize();
        resultTangent.FastNormalize();
        resultBitangent.FastNormalize();

      }

      if (updateSrc) {
        memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3],                           resultVertex.coords,    3 * sizeof(float));
        memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset],     resultNormal.coords,    3 * sizeof(float));
        memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 3], resultTangent.coords,   3 * sizeof(float));
        memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 4], resultBitangent.coords, 3 * sizeof(float));
      }

      memcpy(&materializedTriangleMeshes[subgeom].vertices[weightedVertices[v].vertexID * 3],                           resultVertex.coords,    3 * sizeof(float));
      memcpy(&materializedTriangleMeshes[subgeom].vertices[weightedVertices[v].vertexID * 3 + uniqueElementOffset],     resultNormal.coords,    3 * sizeof(float));
      memcpy(&materializedTriangleMeshes[subgeom].vertices[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 3], resultTangent.coords,   3 * sizeof(float));
      memcpy(&materializedTriangleMeshes[subgeom].vertices[weightedVertices[v].vertexID * 3 + uniqueElementOffset * 4], resultBitangent.coords, 3 * sizeof(float));
    }

  } // subgeom

}
/* moved this to gametask upload thread, so it can be multithreaded, whilst assuring its lifetime
void HumanoidBase::UploadFullbodyModel() {
  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData(false);
}
*/

void HumanoidBase::Process() {

  _cache_AgilityFactor = GetConfiguration()->GetReal("gameplay_agilityfactor", _default_AgilityFactor);
  _cache_AccelerationFactor = GetConfiguration()->GetReal("gameplay_accelerationfactor", _default_AccelerationFactor);

  decayingPositionOffset *= 0.95f;
  if (decayingPositionOffset.GetLength() < 0.005) decayingPositionOffset.Set(0);
  decayingDifficultyFactor = clamp(decayingDifficultyFactor - 0.002f, 0.0f, 1.0f);


  assert(match);

  if (!currentMentalImage) {
    currentMentalImage = match->GetMentalImage(0);
  }

  CalculateSpatialState();
  spatialState.positionOffsetMovement = Vector3(0);

  currentAnim->frameNum++;
  previousAnim->frameNum++;


  if (currentAnim->frameNum == currentAnim->anim->GetFrameCount() - 1 && interruptAnim == e_InterruptAnim_None) {
    interruptAnim = e_InterruptAnim_Switch;
  }


  bool mayReQueue = false;


  // already some anim interrupt waiting?

  if (mayReQueue) {
    if (interruptAnim != e_InterruptAnim_None) {
      mayReQueue = false;
    }
  }


  // okay, see if we need to requeue

  if (mayReQueue) {
    interruptAnim = e_InterruptAnim_ReQueue;
  }

  if (interruptAnim != e_InterruptAnim_None) {

    PlayerCommandQueue commandQueue;

    if (interruptAnim == e_InterruptAnim_Trip && tripType != 0) {
      AddTripCommandToQueue(commandQueue, tripDirection, tripType);
      tripType = 0;
      commandQueue.push_back(GetBasicMovementCommand(tripDirection, spatialState.floatVelocity)); // backup, if there's no applicable trip anim
    } else {
      player->RequestCommand(commandQueue);
    }


    // iterate through the command queue and pick the first that is applicable

    bool found = false;
    for (unsigned int i = 0; i < commandQueue.size(); i++) {

      const PlayerCommand &command = commandQueue[i];

      found = SelectAnim(command, interruptAnim);
      if (found) break;
    }

    if (interruptAnim != e_InterruptAnim_ReQueue && !found) {
      printf("RED ALERT! NO APPLICABLE ANIM FOUND FOR HUMANOIDBASE! NOOOO!\n");
      printf("currentanimtype: %s\n", currentAnim->anim->GetVariable("type").c_str());
      for (unsigned int i = 0; i < commandQueue.size(); i++) {
        printf("desiredanimtype: %i\n", commandQueue[i].desiredFunctionType);
      }
    }

    if (found) {
      startPos = spatialState.position;
      startAngle = spatialState.angle;

      CalculatePredictedSituation(nextStartPos, nextStartAngle);

      animApplyBuffer.anim = currentAnim->anim;
      animApplyBuffer.smooth = true;
      animApplyBuffer.smoothFactor = (interruptAnim == e_InterruptAnim_Switch) ? 0.6f : 1.0f;

      // decaying difficulty
      float animDiff = atof(currentAnim->anim->GetVariable("animdifficultyfactor").c_str());
      if (animDiff > decayingDifficultyFactor) decayingDifficultyFactor = animDiff;
      // if we just requeued, for example, from movement to ballcontrol, there's no reason we can not immediately requeue to another ballcontrol again (next time). only apply the initial requeue delay on subsequent anims of the same type
      // (so we can have a fast ballcontrol -> ballcontrol requeue, but after that, use the initial delay)
      if (interruptAnim == e_InterruptAnim_ReQueue && previousAnim->functionType == currentAnim->functionType) {
        reQueueDelayFrames = initialReQueueDelayFrames; // don't try requeueing (some types of anims, see selectanim()) too often
      }

    }

  }
  reQueueDelayFrames = clamp(reQueueDelayFrames - 1, 0, 10000);


  interruptAnim = e_InterruptAnim_None;

  if (startPos.coords[2] != 0.f) {
    // the z coordinate not being 0 denotes something went horribly wrong :P
    Log(e_FatalError, "HumanoidBase", "Process", "BWAAAAAH FLYING PLAYERS!! height: " + real_to_str(startPos.coords[2]));
  }


  // movement/rotation smuggle

  // start with +1, because we want to influence the first frame as well
  // as for finishing, finish with frameBias = 1.0, even if the last frame is 'spiritually' the one-to-last, since the first frame of the next anim is actually 'same-tempered' as the current anim's last frame.
  // however, it works best to have all values 'done' at this one-to-last frame, so the next anim can read out these correct (new starting) values.
  float frameBias = (currentAnim->frameNum + 1) / (float)(currentAnim->anim->GetEffectiveFrameCount() + 1);

  // not sure if this is correct!
  // radian beginAngle = currentAnim->rotationSmuggle.begin;// * (1.0f - frameBias); // more influence in the beginning; act more like it was 0 later on. (yes, next one is a bias within a bias) *edit: disabled, looks better without
  // currentAnim->rotationSmuggleOffset = beginAngle * (1.0f - frameBias) +
  //                                      currentAnim->rotationSmuggle.end * frameBias;

  currentAnim->rotationSmuggleOffset = currentAnim->rotationSmuggle.begin * (1.0f - frameBias) +
                                       currentAnim->rotationSmuggle.end * frameBias;


  // next frame

  animApplyBuffer.frameNum = currentAnim->frameNum;

  if (currentAnim->positions.size() > (unsigned int)currentAnim->frameNum) {
    animApplyBuffer.position = startPos + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset + currentAnim->positions.at(currentAnim->frameNum);
    animApplyBuffer.orientation = startAngle + currentAnim->rotationSmuggleOffset;
    animApplyBuffer.noPos = true;
  } else {
    animApplyBuffer.position = startPos + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset;
    animApplyBuffer.orientation = startAngle;
    animApplyBuffer.noPos = false;
  }

  animApplyBuffer.offsets = offsets;
}

void HumanoidBase::PreparePutBuffers(unsigned long snapshotTime_ms) {

  // offsets
  CalculateGeomOffsets();

  buf_animApplyBuffer = animApplyBuffer;
  buf_animApplyBuffer.snapshotTime_ms = snapshotTime_ms;

  // display humanoids farther away from action at half FPS
  buf_LowDetailMode = false;
  if (!player->GetExternalController() && !match->GetPause()) {
    Vector3 focusPos = match->GetBall()->Predict(100).Get2D();
    if (match->GetDesignatedPossessionPlayer()) {
      focusPos = focusPos * 0.5f + match->GetDesignatedPossessionPlayer()->GetPosition() * 0.5f;
    }

    if ((spatialState.position - focusPos).GetLength() > 14.0f) buf_LowDetailMode = true;
  }

}

void HumanoidBase::FetchPutBuffers(unsigned long putTime_ms) {

  fetchedbuf_animApplyBuffer = buf_animApplyBuffer;
  assert(fetchedbuf_animApplyBuffer.anim == buf_animApplyBuffer.anim);

  fetchedbuf_LowDetailMode = buf_LowDetailMode;
  buf_bodyUpdatePhase++;
  if (buf_bodyUpdatePhase == 2) buf_bodyUpdatePhase = 0;
  fetchedbuf_bodyUpdatePhase = buf_bodyUpdatePhase;
  fetchedbuf_bodyUpdatePhaseOffset = buf_bodyUpdatePhaseOffset;
}

void HumanoidBase::Put() {

  //unsigned long timeDiff_ms = match->GetTimeSincePreviousPut_ms();//EnvironmentManager::GetInstance().GetTime_ms() - match->GetPreviousTime_ms();
  // the apply function doesn't know better than that it is displaying snapshot times, so continue this hoax into the timeDiff_ms value. then,
  // the temporalsmoother will convert it to 'realtime' once again
  unsigned long timeDiff_ms = fetchedbuf_animApplyBuffer.snapshotTime_ms - fetchedbuf_previousSnapshotTime_ms;
  timeDiff_ms = clamp(timeDiff_ms, 10, 50);
  fetchedbuf_previousSnapshotTime_ms = fetchedbuf_animApplyBuffer.snapshotTime_ms;

  for (unsigned int i = 0; i < buf_TemporalHumanoidNodes.size(); i++) {
    // first, restore the previous non-temporal-smoothed values, so the Apply() function can use them for smoothing
    buf_TemporalHumanoidNodes[i].actualNode->SetPosition(buf_TemporalHumanoidNodes[i].cachedPosition, false);
    buf_TemporalHumanoidNodes[i].actualNode->SetRotation(buf_TemporalHumanoidNodes[i].cachedOrientation, false);
  }

  if (GetScenarioConfig().render) {
    humanoidNode->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  }

  fetchedbuf_animApplyBuffer.anim->Apply(nodeMap, fetchedbuf_animApplyBuffer.frameNum, -1, fetchedbuf_animApplyBuffer.smooth, fetchedbuf_animApplyBuffer.smoothFactor, fetchedbuf_animApplyBuffer.position, fetchedbuf_animApplyBuffer.orientation, fetchedbuf_animApplyBuffer.offsets, &movementHistory, timeDiff_ms, fetchedbuf_animApplyBuffer.noPos, false);

  if (GetScenarioConfig().render) {
    humanoidNode->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  }

  // we've just set the humanoid positions for time fetchedbuf_animApplyBuffer.snapshotTime_ms. however, it's eventually going to be displayed in a historic position, for temporal smoothing.
  // thus; read out the current values we've just set, insert them in the temporal smoother, and get the historic spatial data instead (GetValue).
  for (unsigned int i = 0; i < buf_TemporalHumanoidNodes.size(); i++) {
    // save the non-historic version in cachedNode
    buf_TemporalHumanoidNodes[i].cachedPosition = buf_TemporalHumanoidNodes[i].actualNode->GetPosition();
    buf_TemporalHumanoidNodes[i].cachedOrientation = buf_TemporalHumanoidNodes[i].actualNode->GetRotation();

    // sudden realisation: by only SetValue'ing here instead of in prepareputbuffers, aren't we missing out on valuable interpolateable data?
    buf_TemporalHumanoidNodes[i].position.SetValue(buf_TemporalHumanoidNodes[i].actualNode->GetPosition(), fetchedbuf_animApplyBuffer.snapshotTime_ms);
    buf_TemporalHumanoidNodes[i].orientation.SetValue(buf_TemporalHumanoidNodes[i].actualNode->GetRotation(), fetchedbuf_animApplyBuffer.snapshotTime_ms);

    buf_TemporalHumanoidNodes[i].actualNode->SetPosition(buf_TemporalHumanoidNodes[i].position.GetValue(match->GetPreviousPutTime_ms()), false);
    buf_TemporalHumanoidNodes[i].actualNode->SetRotation(buf_TemporalHumanoidNodes[i].orientation.GetValue(match->GetPreviousPutTime_ms()), false);
  }

  humanoidNode->RecursiveUpdateSpatialData(e_SpatialDataType_Both);

  UpdateFullbodyNodes();
}

void HumanoidBase::CalculateGeomOffsets() {
}

void HumanoidBase::SetOffset(BodyPart body_part, float bias, const Quaternion &orientation, bool isRelative) {
  BiasedOffset& biasedOffset = offsets[body_part];
  biasedOffset.bias = bias;
  biasedOffset.orientation = orientation;
  biasedOffset.isRelative = isRelative;
}

int HumanoidBase::GetIdleMovementAnimID() {
  CrudeSelectionQuery query;
  query.byFunctionType = true;
  query.functionType = e_FunctionType_Movement;
  query.byIncomingVelocity = true;
  query.incomingVelocity = e_Velocity_Idle;
  query.byOutgoingVelocity = true;
  query.outgoingVelocity = e_Velocity_Idle;

  DataSet dataSet;
  anims->CrudeSelection(dataSet, query);

  SetIdlePredicate(1);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIdleVariable, this, _1, _2));

  SetIncomingBodyDirectionSimilarityPredicate(Vector3(0, -1, 0));
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIncomingBodyDirectionSimilarity, this, _1, _2));

  SetIncomingVelocitySimilarityPredicate(e_Velocity_Idle);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIncomingVelocitySimilarity, this, _1, _2));

  SetMovementSimilarityPredicate(Vector3(0, -1, 0), e_Velocity_Idle);
  SetBodyDirectionSimilarityPredicate(spatialState.position + Vector3(0, -10, 0).GetRotated2D(spatialState.angle)); // lookat
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareBodyDirectionSimilarity, this, _1, _2));

  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareMovementSimilarity, this, _1, _2));

  //printf("%s\n", anims->GetAnim(*dataSet.begin())->GetName().c_str());

  return *dataSet.begin();
}

void HumanoidBase::ResetPosition(const Vector3 &newPos, const Vector3 &focusPos) {

  startPos = newPos;
  startAngle = FixAngle((focusPos - newPos).GetNormalized(Vector3(0, -1, 0)).GetAngle2D());
  nextStartPos = startPos;
  nextStartAngle = startAngle;
  previousPosition2D = startPos;

  spatialState.position = startPos;
  spatialState.angle = startAngle;
  spatialState.directionVec = Vector3(0, -1, 0).GetRotated2D(startAngle);
  spatialState.floatVelocity = 0;
  spatialState.enumVelocity = e_Velocity_Idle;
  spatialState.movement = Vector3(0);
  spatialState.relBodyDirectionVec = Vector3(0, -1, 0);
  spatialState.relBodyAngle = 0;
  spatialState.bodyDirectionVec = Vector3(0, -1, 0);
  spatialState.bodyAngle = 0;
  spatialState.foot = e_Foot_Right;

  int idleAnimID = GetIdleMovementAnimID();
  currentAnim->id = idleAnimID;
  currentAnim->anim = anims->GetAnim(currentAnim->id);
  currentAnim->positions.clear();
  currentAnim->positions = match->GetAnimPositionCache(currentAnim->anim);
  currentAnim->frameNum = random(0, currentAnim->anim->GetEffectiveFrameCount() - 1);
  currentAnim->radiusOffset = 0.0;
  currentAnim->touchFrame = -1;
  currentAnim->originatingInterrupt = e_InterruptAnim_None;
  currentAnim->fullActionSmuggle = Vector3(0);
  currentAnim->actionSmuggle = Vector3(0);
  currentAnim->actionSmuggleOffset = Vector3(0);
  currentAnim->actionSmuggleSustain = Vector3(0);
  currentAnim->actionSmuggleSustainOffset = Vector3(0);
  currentAnim->movementSmuggle = Vector3(0);
  currentAnim->movementSmuggleOffset = Vector3(0);
  currentAnim->rotationSmuggle.begin = 0;
  currentAnim->rotationSmuggle.end = 0;
  currentAnim->rotationSmuggleOffset = 0;
  currentAnim->functionType = e_FunctionType_Movement;
  currentAnim->incomingMovement = Vector3(0);
  currentAnim->outgoingMovement = Vector3(0);
  currentAnim->positionOffset = Vector3(0);

  previousAnim->id = idleAnimID;
  previousAnim->anim = currentAnim->anim;
  previousAnim->positions.clear();
  previousAnim->positions = match->GetAnimPositionCache(previousAnim->anim);
  previousAnim->frameNum = 0;
  previousAnim->radiusOffset = 0.0;
  previousAnim->touchFrame = -1;
  previousAnim->originatingInterrupt = e_InterruptAnim_None;
  previousAnim->fullActionSmuggle = Vector3(0);
  previousAnim->actionSmuggle = Vector3(0);
  previousAnim->actionSmuggleOffset = Vector3(0);
  previousAnim->actionSmuggleSustain = Vector3(0);
  previousAnim->actionSmuggleSustainOffset = Vector3(0);
  previousAnim->movementSmuggle = Vector3(0);
  previousAnim->movementSmuggleOffset = Vector3(0);
  previousAnim->rotationSmuggle.begin = 0;
  previousAnim->rotationSmuggle.end = 0;
  previousAnim->rotationSmuggleOffset = 0;
  previousAnim->functionType = e_FunctionType_Movement;
  previousAnim->incomingMovement = Vector3(0);
  previousAnim->outgoingMovement = Vector3(0);
  previousAnim->positionOffset = Vector3(0);

  humanoidNode->SetPosition(startPos, false);

  animApplyBuffer.anim = currentAnim->anim;
  animApplyBuffer.smooth = false;
  animApplyBuffer.smoothFactor = 0.0f;
  animApplyBuffer.position = startPos;
  animApplyBuffer.orientation = startAngle;
  animApplyBuffer.offsets.clear();
  buf_animApplyBuffer = animApplyBuffer;

  interruptAnim = e_InterruptAnim_None;
  tripType = 0;

  decayingPositionOffset = Vector3(0);
  decayingDifficultyFactor = 0.0f;

  movementHistory.clear();

  // clear temporalsmoother vars

/*
  for (unsigned int i = 0; i < buf_TemporalHumanoidNodes.size(); i++) {
    buf_TemporalHumanoidNodes[i].position.Clear();
    buf_TemporalHumanoidNodes[i].orientation.Clear();
  }
  buf_animApplyBuffer_TemporalPosition.Clear();
  buf_animApplyBuffer_TemporalOrientationOffset.Clear();
  buf_animApplyBuffer_FrameNum.Clear();
*/
}

void HumanoidBase::OffsetPosition(const Vector3 &offset) {


  assert(offset.coords[2] == 0.0f);

  float cheat = 1.0f;

  nextStartPos += offset * cheat;
  startPos += offset * cheat;
  spatialState.position += offset * cheat;
  spatialState.positionOffsetMovement += offset * 100.0f * cheat;
  decayingPositionOffset += offset * cheat;
  if (decayingPositionOffset.GetLength() > 0.1f) decayingPositionOffset = decayingPositionOffset.GetNormalized() * 0.1f;
  currentAnim->positionOffset += offset * cheat;
}

void HumanoidBase::TripMe(const Vector3 &tripVector, int tripType) {
  if (match->GetBallRetainer() == player) return;
  if (currentAnim->anim->GetVariableCache().incoming_special_state().compare("") == 0 && currentAnim->anim->GetVariableCache().outgoing_special_state().compare("") == 0) {
    if (this->interruptAnim == e_InterruptAnim_None && (currentAnim->functionType != e_FunctionType_Trip || (currentAnim->anim->GetVariable("triptype").compare("1") == 0 && tripType > 1))
                                                    && currentAnim->functionType != e_FunctionType_Sliding) {
      this->interruptAnim = e_InterruptAnim_Trip;
      this->tripDirection = tripVector;
      this->tripType = tripType;
    }
  }
}

void HumanoidBase::SetKit(boost::intrusive_ptr < Resource<Surface> > newKit) {
  boost::intrusive_ptr< Resource<GeometryData> > bodyGeom = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();

  std::vector < MaterializedTriangleMesh > &tmesh = bodyGeom->GetResource()->GetTriangleMeshesRef();

  if (newKit != boost::intrusive_ptr< Resource<Surface> >()) {
    for (unsigned int i = 0; i < tmesh.size(); i++) {
      if (tmesh[i].material.diffuseTexture != boost::intrusive_ptr< Resource<Surface> >()) {
        if (tmesh[i].material.diffuseTexture->GetIdentString() == kitDiffuseTextureIdentString) {
          tmesh[i].material.diffuseTexture = newKit;
          tmesh[i].material.specular_amount = 0.01f;//0.02f;//0.033f;//0.01f;
          tmesh[i].material.shininess = 0.01f;//0.005f;
        }
      }
    }
    kitDiffuseTextureIdentString = newKit->GetIdentString();
  }

  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData();
}

void HumanoidBase::ResetSituation(const Vector3 &focusPos) {
  currentMentalImage = 0;

  ResetPosition(spatialState.position, focusPos);
}

bool HumanoidBase::_HighOrBouncyBall() const {
  float ballHeight1 = match->GetBall()->Predict(10).coords[2];
  float ballHeight2 = match->GetBall()->Predict(defaultTouchOffset_ms).coords[2];
  float ballBounce = fabs(match->GetBall()->GetMovement().coords[2]);
  bool highBall = false;
  if (ballHeight1 > 0.3f || ballHeight2 > 0.3f) {
    highBall = true;
  } else if (ballBounce > 1.0f) { // low balls are also treated as 'high ball' when there's a lot of bounce going on (hard to control)
    highBall = true;
  }
  return highBall;
}

// ALERT: set sorting predicates before calling this function
void HumanoidBase::_KeepBestDirectionAnims(DataSet &dataSet, const PlayerCommand &command, bool strict, radian allowedAngle, int allowedVelocitySteps, int forcedQuadrantID) {

  assert(dataSet.size() != 0);

  int bestQuadrantID = forcedQuadrantID;
  if (bestQuadrantID == -1) {

    for (auto& anim : dataSet) {
      anims->GetAnim(anim)->order_float = GetMovementSimilarity(anim, predicate_RelDesiredDirection, predicate_DesiredVelocity, predicate_CorneringBias);
    }
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareByOrderFloat, this, _1, _2));

    // we want the best anim to be a baseanim, and compare other anims to it
    if (strict) {
      if (command.desiredFunctionType != e_FunctionType_Movement) {
        std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareBaseanimSimilarity, this, _1, _2));
      }
    }

    Animation *bestAnim = anims->GetAnim(*dataSet.begin());

    bestQuadrantID = bestAnim->GetVariableCache().quadrant_id();
  }

  const Quadrant &quadrant = anims->GetQuadrant(bestQuadrantID);

  DataSet::iterator iter = dataSet.begin();
  iter++;
  while (iter != dataSet.end()) {
    Animation *anim = anims->GetAnim(*iter);

    if (strict) {
      if (anim->GetVariableCache().quadrant_id() == bestQuadrantID) {
        iter++;
      } else {
        iter = dataSet.erase(iter);
      }
    } else {
      int quadrantID = anim->GetVariableCache().quadrant_id();
      const Quadrant &bestQuadrant = anims->GetQuadrant(bestQuadrantID);
      const Quadrant &quadrant = anims->GetQuadrant(quadrantID);

      bool predicate = true;

      if (!anim->GetVariableCache().lastditch()) { // last ditch anims may always change velo
        if (abs(GetVelocityID(quadrant.velocity, true) - GetVelocityID(bestQuadrant.velocity, true)) > allowedVelocitySteps) predicate = false;
      }
      if (fabs(quadrant.angle - bestQuadrant.angle) > allowedAngle) predicate = false;

      if (predicate) {
        iter++;
      } else {
        iter = dataSet.erase(iter);
      }
    }

  }
}

// ALERT: set sorting predicates before calling this function
void HumanoidBase::_KeepBestBodyDirectionAnims(DataSet &dataSet, const PlayerCommand &command, bool strict, radian allowedAngle) {

  // delete nonqualified bodydir quadrants

  assert(dataSet.size() != 0);
  for (auto& anim : dataSet) {
    anims->GetAnim(anim)->order_float = DirectionSimilarityRating(anim);
  }
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareByOrderFloat, this, _1, _2));

  // we want the best anim to be a baseanim, and compare other anims to it
  if (strict) {
    if (command.desiredFunctionType != e_FunctionType_Movement) {
      std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareBaseanimSimilarity, this, _1, _2));
    }
  }

  Animation *bestAnim = anims->GetAnim(*dataSet.begin());

  radian bestOutgoingBodyAngle = ForceIntoAllowedBodyDirectionAngle(bestAnim->GetOutgoingBodyAngle());
  radian bestOutgoingAngle = ForceIntoPreferredDirectionAngle(bestAnim->GetOutgoingAngle());
  radian bestLookAngle = bestOutgoingBodyAngle + bestOutgoingAngle;


  DataSet::iterator iter = dataSet.begin();
  iter++;
  while (iter != dataSet.end()) {

    Animation *anim = anims->GetAnim(*iter);

    radian animOutgoingBodyAngle = ForceIntoAllowedBodyDirectionAngle(anim->GetOutgoingBodyAngle());
    radian animOutgoingAngle = ForceIntoPreferredDirectionAngle(anim->GetOutgoingAngle());
    radian animLookAngle = animOutgoingBodyAngle + animOutgoingAngle;

    float adaptedAllowedAngle = 0.06f * pi; // between 0 and 20 deg
    if (!strict) {
      adaptedAllowedAngle = allowedAngle;
    }
    if (fabs(animLookAngle - bestLookAngle) <= adaptedAllowedAngle) {
      iter++;
    } else {
      iter = dataSet.erase(iter);
    }

  }
}

bool HumanoidBase::SelectAnim(const PlayerCommand &command, e_InterruptAnim localInterruptAnim, bool preferPassAndShot) { // returns false on no applicable anim found
  assert(command.desiredDirection.coords[2] == 0.0f);

  if (localInterruptAnim != e_InterruptAnim_ReQueue || currentAnim->frameNum > 12) CalculateFactualSpatialState();


  // CREATE A CRUDE SET OF POTENTIAL ANIMATIONS

  CrudeSelectionQuery query;
  query.byFunctionType = true;
  query.functionType = command.desiredFunctionType;

  query.byFoot = false;
  query.foot = (spatialState.foot == e_Foot_Left) ? e_Foot_Right : e_Foot_Left;

  query.byIncomingVelocity = true;
  query.incomingVelocity = spatialState.enumVelocity;
  query.incomingVelocity_Strict = true;
  query.byIncomingBodyDirection = true;
  query.incomingBodyDirection_Strict = true;
  query.incomingBodyDirection = spatialState.relBodyDirectionVec;
  query.incomingVelocity_ForceLinearity = true;
  query.incomingBodyDirection_ForceLinearity = true;

  if (command.desiredFunctionType == e_FunctionType_Trip) {
    query.byTripType = true;
    query.tripType = command.tripType;
  }
  query.properties.set("incoming_special_state", currentAnim->anim->GetVariableCache().outgoing_special_state());
  if (match->GetBallRetainer() == player) query.properties.set("incoming_retain_state", currentAnim->anim->GetVariable("outgoing_retain_state"));
  if (command.useSpecialVar1) query.properties.set_specialvar1(command.specialVar1);
  if (command.useSpecialVar2) query.properties.set_specialvar2(command.specialVar2);

  if (!currentAnim->anim->GetVariableCache().outgoing_special_state().empty()) query.incomingVelocity = e_Velocity_Idle; // standing up anims always start out idle

  DataSet dataSet;
  anims->CrudeSelection(dataSet, query);
  if (dataSet.size() == 0) {
    if (command.desiredFunctionType == e_FunctionType_Movement) {
      dataSet.push_back(GetIdleMovementAnimID()); // do with idle anim (should not happen too often, only after weird bumps when there's for example a need for a sprint anim at an impossible body angle, after a trip of whatever)
    } else return false;
  }

  // NOW SORT OUT THE RESULTING SET

  float adaptedDesiredVelocityFloat = command.desiredVelocityFloat;

  if (command.useDesiredMovement) {

    Vector3 relDesiredDirection = command.desiredDirection.GetRotated2D(-spatialState.angle);
    SetMovementSimilarityPredicate(relDesiredDirection, FloatToEnumVelocity(adaptedDesiredVelocityFloat));
    SetBodyDirectionSimilarityPredicate(command.desiredLookAt);
    if (command.desiredFunctionType == e_FunctionType_Movement) {
      _KeepBestDirectionAnims(dataSet, command);
      if (command.useDesiredLookAt) _KeepBestBodyDirectionAnims(dataSet, command);
    }

    else { // undefined animtype
      std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareMovementSimilarity, this, _1, _2));
    }

  }

  int desiredIdleLevel = 1;
  SetIdlePredicate(desiredIdleLevel);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIdleVariable, this, _1, _2));

  SetFootSimilarityPredicate(spatialState.foot);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareFootSimilarity, this, _1, _2));

  SetIncomingBodyDirectionSimilarityPredicate(spatialState.relBodyDirectionVec);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareIncomingBodyDirectionSimilarity, this, _1, _2));

  SetIncomingVelocitySimilarityPredicate(spatialState.enumVelocity);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareIncomingVelocitySimilarity, this, _1, _2));

  if (command.useDesiredTripDirection) {
    Vector3 relDesiredTripDirection = command.desiredTripDirection.GetRotated2D(-spatialState.angle);
    SetTripDirectionSimilarityPredicate(relDesiredTripDirection);
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareTripDirectionSimilarity, this, _1, _2));
  }

  if (command.desiredFunctionType != e_FunctionType_Movement) {
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareBaseanimSimilarity, this, _1, _2));
  }


  // process result

  int selectedAnimID = -1;
  std::vector<Vector3> positions_tmp;
  int touchFrame_tmp = -1;
  float radiusOffset_tmp = 0.0f;
  Vector3 touchPos_tmp;
  Vector3 fullActionSmuggle_tmp;
  Vector3 actionSmuggle_tmp;
  radian rotationSmuggle_tmp = 0;

  if (dataSet.size() == 0) {
    return false;
  }
  if (command.desiredFunctionType == e_FunctionType_Movement ||
      command.desiredFunctionType == e_FunctionType_Trip ||
      command.desiredFunctionType == e_FunctionType_Special) {
    selectedAnimID = *dataSet.begin();
    Animation *nextAnim = anims->GetAnim(selectedAnimID);
    Vector3 desiredMovement = command.desiredDirection * command.desiredVelocityFloat;
    assert(desiredMovement.coords[2] == 0.0f);
    Vector3 desiredBodyDirectionRel = Vector3(0, -1, 0);
    if (command.useDesiredLookAt) desiredBodyDirectionRel = ((command.desiredLookAt - spatialState.position).Get2D().GetRotated2D(-spatialState.angle) - nextAnim->GetTranslation()).GetNormalized(Vector3(0, -1, 0));
    Vector3 physicsVector = CalculatePhysicsVector(nextAnim, command.useDesiredMovement, desiredMovement, command.useDesiredLookAt, desiredBodyDirectionRel, positions_tmp, rotationSmuggle_tmp);
  }


  // check if we really want to requeue - only requeue movement to movement, for example, when we want to go a different direction

  if (localInterruptAnim == e_InterruptAnim_ReQueue && selectedAnimID != -1 && currentAnim->positions.size() > 1 && positions_tmp.size() > 1) {

    // don't requeue to same quadrant
    if (currentAnim->functionType == command.desiredFunctionType &&

        ((FloatToEnumVelocity(currentAnim->anim->GetOutgoingVelocity()) != e_Velocity_Idle &&
          currentAnim->anim->GetVariableCache().quadrant_id() == anims->GetAnim(selectedAnimID)->GetVariableCache().quadrant_id())
          ||
         (FloatToEnumVelocity(currentAnim->anim->GetOutgoingVelocity()) == e_Velocity_Idle &&
          fabs((ForceIntoPreferredDirectionAngle(currentAnim->anim->GetOutgoingAngle()) - ForceIntoPreferredDirectionAngle(anims->GetAnim(selectedAnimID)->GetOutgoingAngle()))) < 0.20f * pi))
       ) {

      selectedAnimID = -1;
    }
  }


  // make it so

  if (selectedAnimID != -1) {
    *previousAnim = *currentAnim;

    currentAnim->anim = anims->GetAnim(selectedAnimID);
    currentAnim->id = selectedAnimID;
    currentAnim->functionType = command.desiredFunctionType;
    currentAnim->frameNum = 0;
    currentAnim->touchFrame = touchFrame_tmp;
    currentAnim->originatingInterrupt = localInterruptAnim;
    currentAnim->radiusOffset = radiusOffset_tmp;
    currentAnim->touchPos = touchPos_tmp;
    currentAnim->rotationSmuggle.begin = clamp(ModulateIntoRange(-pi, pi, spatialState.relBodyAngleNonquantized - currentAnim->anim->GetIncomingBodyAngle()) * bodyRotationSmoothingFactor, -bodyRotationSmoothingMaxAngle, bodyRotationSmoothingMaxAngle);
    currentAnim->rotationSmuggle.end = rotationSmuggle_tmp;
    currentAnim->rotationSmuggleOffset = 0;
    currentAnim->fullActionSmuggle = fullActionSmuggle_tmp;
    currentAnim->actionSmuggle = actionSmuggle_tmp;
    currentAnim->actionSmuggleOffset = Vector3(0);
    currentAnim->actionSmuggleSustain = Vector3(0);
    currentAnim->actionSmuggleSustainOffset = Vector3(0);
    currentAnim->movementSmuggle = Vector3(0);
    currentAnim->movementSmuggleOffset = Vector3(0);
    currentAnim->incomingMovement = spatialState.movement;
    currentAnim->outgoingMovement = CalculateOutgoingMovement(positions_tmp);
    currentAnim->positions.clear();
    currentAnim->positions.assign(positions_tmp.begin(), positions_tmp.end());
    currentAnim->positionOffset = 0.0;
    currentAnim->originatingCommand = command;

    return true;
  }

  return false;
}

void HumanoidBase::CalculatePredictedSituation(Vector3 &predictedPos, radian &predictedAngle) {

  if (currentAnim->positions.size() > (unsigned int)currentAnim->frameNum) {
    assert(currentAnim->positions.size() > (unsigned int)currentAnim->anim->GetEffectiveFrameCount());
    predictedPos = spatialState.position + currentAnim->positions.at(currentAnim->anim->GetEffectiveFrameCount()) + currentAnim->actionSmuggle + currentAnim->actionSmuggleSustain + currentAnim->movementSmuggle;
  } else {
    predictedPos = spatialState.position + currentAnim->anim->GetTranslation().Get2D().GetRotated2D(spatialState.angle) + currentAnim->actionSmuggle + currentAnim->actionSmuggleSustain + currentAnim->movementSmuggle;
  }

  predictedAngle = spatialState.angle + currentAnim->anim->GetOutgoingAngle() + currentAnim->rotationSmuggle.end;
  predictedAngle = ModulateIntoRange(-pi, pi, predictedAngle);
  assert(predictedPos.coords[2] == 0.0f);
}

Vector3 HumanoidBase::CalculateOutgoingMovement(const std::vector<Vector3> &positions) const {
  if (positions.size() < 2) return 0;
  return (positions.at(positions.size() - 1) - positions.at(positions.size() - 2)) * 100.0f;
}

void HumanoidBase::CalculateSpatialState() {
  Vector3 position;
  if (currentAnim->positions.size() > (unsigned int)currentAnim->frameNum) {
    position = startPos + currentAnim->positions.at(currentAnim->frameNum) + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset;
  } else {
    Quaternion orientation;
    currentAnim->anim->GetKeyFrame(BodyPart::player, currentAnim->frameNum, orientation, position);
    position.coords[2] = 0.0f;
    position = startPos + position.GetRotated2D(startAngle) + currentAnim->actionSmuggleOffset + currentAnim->actionSmuggleSustainOffset + currentAnim->movementSmuggleOffset;
  }

  if (currentAnim->frameNum > 12) {
    spatialState.foot = currentAnim->anim->GetOutgoingFoot();
  }

  assert(startPos.coords[2] == 0.0f);
  assert(currentAnim->actionSmuggleOffset.coords[2] == 0.0f);
  assert(currentAnim->movementSmuggleOffset.coords[2] == 0.0f);
  assert(position.coords[2] == 0.0f);

  spatialState.actualMovement = (position - previousPosition2D) * 100.0f;
  float positionOffsetMovementIgnoreFactor = 0.5f;
  spatialState.physicsMovement = spatialState.actualMovement - (spatialState.actionSmuggleMovement) - (spatialState.movementSmuggleMovement) - (spatialState.positionOffsetMovement * positionOffsetMovementIgnoreFactor);
  spatialState.animMovement = spatialState.physicsMovement;
  if (currentAnim->positions.size() > 0) {
    // this way, action cheating is being omitted from the current movement, making for better requeues. however, keep in mind that
    // movementoffsets, from bumping into other players, for example, will also be ignored this way.
    const std::vector<Vector3> &origPositionCache = match->GetAnimPositionCache(currentAnim->anim);
    spatialState.animMovement = CalculateMovementAtFrame(origPositionCache, currentAnim->frameNum, 1).GetRotated2D(startAngle);
  }
  spatialState.movement = spatialState.physicsMovement; // PICK DEFAULT


  Vector3 bodyPosition;
  Quaternion bodyOrientation;
  currentAnim->anim->GetKeyFrame(body, currentAnim->frameNum, bodyOrientation, bodyPosition);
  radian x, y, z;
  bodyOrientation.GetAngles(x, y, z);

  Vector3 quatDirection; quatDirection = bodyOrientation;

  Vector3 bodyDirectionVec = Vector3(0, -1, 0).GetRotated2D(z + startAngle + currentAnim->rotationSmuggleOffset);

  spatialState.floatVelocity = spatialState.movement.GetLength();
  spatialState.enumVelocity = FloatToEnumVelocity(spatialState.floatVelocity);

  if (spatialState.enumVelocity != e_Velocity_Idle) {
    spatialState.directionVec = spatialState.movement.GetNormalized();
  } else {
    // too slow for comfort, use body direction as global direction
    spatialState.directionVec = bodyDirectionVec;
  }

  spatialState.position = position;
  spatialState.angle = ModulateIntoRange(-pi, pi, FixAngle(spatialState.directionVec.GetAngle2D()));

  if (spatialState.enumVelocity != e_Velocity_Idle) {
    Vector3 adaptedBodyDirectionVec = bodyDirectionVec.GetRotated2D(-spatialState.angle);
    // prefer straight forward, so lie about the actual direction a bit
    // this may fix bugs of body dir being non-0 somewhere during 0 anims
    // but it may also cause other bugs (going 0 to 45 all over again each time)
    //adaptedBodyDirectionVec = (adaptedBodyDirectionVec * 0.95f + Vector3(0, -1, 0) * 0.05f).GetNormalized(0);
    //ForceIntoAllowedBodyDirectionVec(Vector3(0, -1, 0)).Print();

    bool preferCorrectVeloOverCorrectAngle = true;
    radian bodyAngleRel = adaptedBodyDirectionVec.GetAngle2D(Vector3(0, -1, 0));
    if (spatialState.enumVelocity == e_Velocity_Sprint && fabs(bodyAngleRel) >= 0.125f * pi) {
      if (preferCorrectVeloOverCorrectAngle) {
        // on impossible combinations of velocity and body angle, decrease body angle
        adaptedBodyDirectionVec = Vector3(0, -1, 0).GetRotated2D(0.12f * pi * signSide(bodyAngleRel));
      } else {
        // on impossible combinations of velocity and body angle, decrease velocity
        spatialState.floatVelocity = walkSprintSwitch - 0.1f;
        spatialState.enumVelocity = FloatToEnumVelocity(spatialState.floatVelocity);
      }
    }
    else if (spatialState.enumVelocity == e_Velocity_Walk && fabs(bodyAngleRel) >= 0.5f * pi) {
      if (preferCorrectVeloOverCorrectAngle) {
        // on impossible combinations of velocity and body angle, decrease body angle
        adaptedBodyDirectionVec = Vector3(0, -1, 0).GetRotated2D(0.495f * pi * signSide(bodyAngleRel));
      } else {
        // on impossible combinations of velocity and body angle, decrease velocity
        spatialState.floatVelocity = dribbleWalkSwitch - 0.1f;
        spatialState.enumVelocity = FloatToEnumVelocity(spatialState.floatVelocity);
      }
    }

    spatialState.relBodyDirectionVecNonquantized = adaptedBodyDirectionVec;
    spatialState.relBodyDirectionVec = ForceIntoAllowedBodyDirectionVec(adaptedBodyDirectionVec);
  } else {
    spatialState.relBodyDirectionVecNonquantized = Vector3(0, -1, 0);
    spatialState.relBodyDirectionVec = Vector3(0, -1, 0);
  }
  spatialState.relBodyAngle = spatialState.relBodyDirectionVec.GetAngle2D(Vector3(0, -1, 0));
  spatialState.relBodyAngleNonquantized = spatialState.relBodyDirectionVecNonquantized.GetAngle2D(Vector3(0, -1, 0));
  spatialState.bodyDirectionVec = spatialState.relBodyDirectionVec.GetRotated2D(spatialState.angle); // rotate back, we now have it forced into allowed angle
  spatialState.bodyAngle = spatialState.bodyDirectionVec.GetAngle2D(Vector3(0, -1, 0));

  previousPosition2D = position;
}

void HumanoidBase::CalculateFactualSpatialState() {

  spatialState.foot = currentAnim->anim->GetOutgoingFoot();

  if (!currentAnim->anim->GetVariableCache().outgoing_special_state().empty()) {
    spatialState.floatVelocity = 0;
    spatialState.enumVelocity = e_Velocity_Idle;
    spatialState.movement = Vector3(0);
  }
}

void HumanoidBase::AddTripCommandToQueue(PlayerCommandQueue &commandQueue, const Vector3 &tripVector, int tripType) {
  if (tripType == 1) {
    commandQueue.push_back(GetTripCommand(tripDirection, tripType));
  } else {
    // allow both types 2 and 3, but prefer the right one
    int otherTripType = 3;
    if (tripType == 3) otherTripType = 2;
    commandQueue.push_back(GetTripCommand(tripDirection, tripType));
    commandQueue.push_back(GetTripCommand(tripDirection, otherTripType));
    commandQueue.push_back(GetTripCommand(tripDirection, 1));
  }
}

PlayerCommand HumanoidBase::GetTripCommand(const Vector3 &tripVector, int tripType) {
  PlayerCommand command;
  command.desiredFunctionType = e_FunctionType_Trip;
  command.useDesiredMovement = false;
  command.useDesiredTripDirection = true;
  command.desiredTripDirection = tripVector;
  command.desiredVelocityFloat = spatialState.floatVelocity;//e_Velocity_Sprint;
  command.useTripType = true;
  command.tripType = tripType;
  return command;
}

PlayerCommand HumanoidBase::GetBasicMovementCommand(const Vector3 &desiredDirection, float velocityFloat) {
  PlayerCommand command;
  command.desiredFunctionType = e_FunctionType_Movement;
  command.useDesiredMovement = true;
  command.useDesiredLookAt = true;
  command.desiredDirection = spatialState.directionVec;
  command.desiredVelocityFloat = velocityFloat;
  command.desiredLookAt = spatialState.position + command.desiredDirection * 10.0f;
  return command;
}

void HumanoidBase::SetFootSimilarityPredicate(e_Foot desiredFoot) const {
  predicate_DesiredFoot = desiredFoot;
}

bool HumanoidBase::CompareFootSimilarity(int animIndex1, int animIndex2) const {
  int one = 1;
  int two = 1;
  if (anims->GetAnim(animIndex1)->GetCurrentFoot() == predicate_DesiredFoot) one = 0;
  if (anims->GetAnim(animIndex2)->GetCurrentFoot() == predicate_DesiredFoot) two = 0;
  if (FloatToEnumVelocity(anims->GetAnim(animIndex1)->GetIncomingVelocity()) == e_Velocity_Idle) one = 0;
  if (FloatToEnumVelocity(anims->GetAnim(animIndex2)->GetIncomingVelocity()) == e_Velocity_Idle) two = 0;
  return one < two;
}

void HumanoidBase::SetIncomingVelocitySimilarityPredicate(e_Velocity velocity) const {
  predicate_IncomingVelocity = velocity;
}

bool HumanoidBase::CompareIncomingVelocitySimilarity(int animIndex1, int animIndex2) const {
  /* old version
  float rating1 = fabs(clamp(RangeVelocity(anims->GetAnim(animIndex1)->GetIncomingVelocity()) - EnumToFloatVelocity(predicate_IncomingVelocity), -sprintVelocity, sprintVelocity));
  float rating2 = fabs(clamp(RangeVelocity(anims->GetAnim(animIndex2)->GetIncomingVelocity()) - EnumToFloatVelocity(predicate_IncomingVelocity), -sprintVelocity, sprintVelocity));
  */

  int currentVelocityID = GetVelocityID(predicate_IncomingVelocity);

  // rate difference anim incoming / actual incoming
  int anim1_incomingVelocityID = GetVelocityID(FloatToEnumVelocity(anims->GetAnim(animIndex1)->GetIncomingVelocity()));
  int anim2_incomingVelocityID = GetVelocityID(FloatToEnumVelocity(anims->GetAnim(animIndex2)->GetIncomingVelocity()));
  float rating1 = fabs(clamp(anim1_incomingVelocityID - currentVelocityID, -3, 3));
  float rating2 = fabs(clamp(anim2_incomingVelocityID - currentVelocityID, -3, 3));

  // also add a penalty for anim incoming velocities which aren't between actual incoming and anim outgoing
  int anim1_outgoingVelocityID = GetVelocityID(FloatToEnumVelocity(anims->GetAnim(animIndex1)->GetOutgoingVelocity()));
  int anim2_outgoingVelocityID = GetVelocityID(FloatToEnumVelocity(anims->GetAnim(animIndex2)->GetOutgoingVelocity()));
  if (anim1_incomingVelocityID > std::max(currentVelocityID, anim1_outgoingVelocityID)) rating1 += 0.5f;
  if (anim1_incomingVelocityID < std::min(currentVelocityID, anim1_outgoingVelocityID)) rating1 += 0.5f;
  if (anim2_incomingVelocityID > std::max(currentVelocityID, anim2_outgoingVelocityID)) rating2 += 0.5f;
  if (anim2_incomingVelocityID < std::min(currentVelocityID, anim2_outgoingVelocityID)) rating2 += 0.5f;

  return rating1 < rating2;
}

void HumanoidBase::SetMovementSimilarityPredicate(const Vector3 &relDesiredDirection, e_Velocity desiredVelocity) const {
  predicate_RelDesiredDirection = relDesiredDirection;
  predicate_DesiredVelocity = desiredVelocity;
  //if (relDesiredDirection.GetDotProduct(Vector3(0, -1, 0)) < 0) predicate_DesiredVelocity = e_Velocity_Idle;
  // this isn't working all too well: if targetmovement is set to 0 (aka corneringbias towards 1), both dribble @ 0 deg and dribble @ 90 deg will be the same distance (from 0), so still no preference for braking straight
  predicate_CorneringBias = CalculateBiasForFastCornering(Vector3(0, -1.0f * spatialState.floatVelocity, 0), relDesiredDirection * EnumToFloatVelocity(desiredVelocity), 1.0f, 0.9f); // anim space values!
}

float HumanoidBase::GetMovementSimilarity(int animIndex, const Vector3 &relDesiredDirection, e_Velocity desiredVelocity, float corneringBias) const {

  Vector3 desiredMovement = relDesiredDirection * EnumToFloatVelocity(desiredVelocity);

  Vector3 outgoingDirection = ForceIntoPreferredDirectionVec(anims->GetAnim(animIndex)->GetOutgoingDirection());
  float outgoingVelocity = RangeVelocity(anims->GetAnim(animIndex)->GetOutgoingVelocity());
  Vector3 outgoingMovement = outgoingDirection * outgoingVelocity;


  // anims that end at lower velocities have an advantage: they don't get dragged into the currentmovement that much
  // thus: have a bias that is higher at higher outgoing velocities, which means anim outgoingmovement gets more % of current movement and less % of their own
  // disabled for now - the new physics system disregards most of the anims movement anyway :)
  // *enabled again: altered the physics system so that it will regard anim movement more, so this became useful again for proper hard cornering

  // alternative to this system (drags back movement)
  //outgoingMovement += -desiredMovement.GetNormalized(0) * outgoingMovement.GetLength() * 0.5f;
  //desiredMovement = desiredMovement.GetNormalized(0) * clamp(desiredMovement.GetLength() - 0.8f, idleVelocity, sprintVelocity);

  desiredMovement = desiredMovement * (1.0f - corneringBias);

  float value = (desiredMovement - outgoingMovement).GetLength();

  value -= fabs(relDesiredDirection.GetDotProduct(outgoingDirection)) * 4.0f; // prefer straight lines (towards/away from desired outgoing)

/* disabled quantization:
  // maximum quantization (20 degree angle @ dribble velocity, smallest distance to be measured)
  // faulty calculation (cornering distance instead of straight line) for optimization, makes little difference anyway for 20 degrees
  float maxQuant = (dribbleVelocity * 2.0f * pi) / (360.0f / 20.0f);// * (1.0 - velocityBias * 0.5) // == ~1.22 at the moment of writing
  //float maxQuant = sqrt(2.0f * pow(dribbleVelocity, 2.0f) * (1.0f - cos(pi / 180.0f * 20.0f))); // the actual correct calculation (straight line)

  // add safety
  maxQuant *= 0.8f;

  // quantize
  value /= maxQuant;
  value = round(value);
  value *= maxQuant;
*/

  // never turn the wrong way around ** BUGGY, causes weird acceleration when we want idle velo **
  //if (fabs(outgoingDirection.GetAngle2D() - relDesiredDirection.GetAngle2D()) > pi) value += 100000;

  return value;
}

bool HumanoidBase::CompareMovementSimilarity(int animIndex1, int animIndex2) const {
  float rating1 = GetMovementSimilarity(animIndex1, predicate_RelDesiredDirection, predicate_DesiredVelocity, predicate_CorneringBias);
  float rating2 = GetMovementSimilarity(animIndex2, predicate_RelDesiredDirection, predicate_DesiredVelocity, predicate_CorneringBias);
  return rating1 < rating2;
}

bool HumanoidBase::CompareByOrderFloat(int animIndex1, int animIndex2) const {
  float rating1 = anims->GetAnim(animIndex1)->order_float;
  float rating2 = anims->GetAnim(animIndex2)->order_float;
  return rating1 < rating2;
}

void HumanoidBase::SetIncomingBodyDirectionSimilarityPredicate(const Vector3 &relIncomingBodyDirection) const {
  predicate_RelIncomingBodyDirection = relIncomingBodyDirection;
}

bool HumanoidBase::CompareIncomingBodyDirectionSimilarity(int animIndex1, int animIndex2) const {
  float rating1 = fabs(ForceIntoAllowedBodyDirectionVec(anims->GetAnim(animIndex1)->GetIncomingBodyDirection()).GetAngle2D(ForceIntoAllowedBodyDirectionVec(predicate_RelIncomingBodyDirection))) / pi;
  float rating2 = fabs(ForceIntoAllowedBodyDirectionVec(anims->GetAnim(animIndex2)->GetIncomingBodyDirection()).GetAngle2D(ForceIntoAllowedBodyDirectionVec(predicate_RelIncomingBodyDirection))) / pi;
  if (FloatToEnumVelocity(anims->GetAnim(animIndex1)->GetIncomingVelocity()) == e_Velocity_Idle) rating1 = 0;//-1;
  if (FloatToEnumVelocity(anims->GetAnim(animIndex2)->GetIncomingVelocity()) == e_Velocity_Idle) rating2 = 0;//-1;

  return rating1 < rating2;
}

void HumanoidBase::SetBodyDirectionSimilarityPredicate(const Vector3 &lookAt) const {
  predicate_LookAt = lookAt;
}

real HumanoidBase::DirectionSimilarityRating(int animIndex) const {
  Animation *a1 = anims->GetAnim(animIndex);
  Vector3 relDesiredBodyDirection1 = ((predicate_LookAt - spatialState.position).GetRotated2D(-spatialState.angle) - a1->GetTranslation()).GetNormalized(Vector3(0, -1, 0));
  radian maxAngleSmuggle = 0.1f * pi;
  radian outgoingAngle1 = a1->GetOutgoingDirection().GetRotated2D( clamp(predicate_RelDesiredDirection.GetAngle2D(a1->GetOutgoingDirection()), -maxAngleSmuggle, maxAngleSmuggle) ).GetAngle2D(Vector3(0, -1, 0));
  Vector3 predictedOutgoingBodyDirection1 = a1->GetOutgoingBodyDirection().GetRotated2D(outgoingAngle1);
  radian rating1 = fabs(predictedOutgoingBodyDirection1.GetAngle2D(relDesiredBodyDirection1));
  // penalty for body angles (as opposed to straight forward), to get a slight preference for forward angles
  rating1 += fabs(a1->GetOutgoingBodyAngle()) * 0.05f;
  return rating1;
}

bool HumanoidBase::CompareBodyDirectionSimilarity(int animIndex1, int animIndex2) const {
  return DirectionSimilarityRating(animIndex1) < DirectionSimilarityRating(animIndex2);
}

void HumanoidBase::SetTripDirectionSimilarityPredicate(const Vector3 &relDesiredTripDirection) const {
  predicate_RelDesiredTripDirection = relDesiredTripDirection;
}

bool HumanoidBase::CompareTripDirectionSimilarity(int animIndex1, int animIndex2) const {
  float rating1 = -GetVectorFromString(anims->GetAnim(animIndex1)->GetVariable("bumpdirection")).GetDotProduct(predicate_RelDesiredTripDirection);
  float rating2 = -GetVectorFromString(anims->GetAnim(animIndex2)->GetVariable("bumpdirection")).GetDotProduct(predicate_RelDesiredTripDirection);
  return rating1 < rating2;
}

bool HumanoidBase::CompareBaseanimSimilarity(int animIndex1, int animIndex2) const {
  bool isBase1 = anims->GetAnim(animIndex1)->GetVariableCache().baseanim();
  bool isBase2 = anims->GetAnim(animIndex2)->GetVariableCache().baseanim();

  if (isBase1 == true && isBase2 == false) return true;
  return false;
}

bool HumanoidBase::CompareCatchOrDeflect(int animIndex1, int animIndex2) const {
  bool catch1 = (anims->GetAnim(animIndex1)->GetVariable("outgoing_retain_state").compare("") != 0);
  bool catch2 = (anims->GetAnim(animIndex2)->GetVariable("outgoing_retain_state").compare("") != 0);

  if (catch1 == true && catch2 == false) return true;
  return false;
}

void HumanoidBase::SetIdlePredicate(float desiredValue) const {
  predicate_idle = desiredValue;
}


bool HumanoidBase::CompareIdleVariable(int animIndex1, int animIndex2) const {
  return fabs(anims->GetAnim(animIndex1)->GetVariableCache().idlelevel() - predicate_idle) <
         fabs(anims->GetAnim(animIndex2)->GetVariableCache().idlelevel() - predicate_idle);
}

bool HumanoidBase::ComparePriorityVariable(int animIndex1, int animIndex2) const {
  return fabs(atof(anims->GetAnim(animIndex1)->GetVariable("priority").c_str())) <
         fabs(atof(anims->GetAnim(animIndex2)->GetVariable("priority").c_str()));
}

Vector3 HumanoidBase::CalculatePhysicsVector(Animation *anim, bool useDesiredMovement, const Vector3 &desiredMovement, bool useDesiredBodyDirection, const Vector3 &desiredBodyDirectionRel, std::vector<Vector3> &positions_ret, radian &rotationOffset_ret) const {

  positions_ret.clear();

  int animTouchFrame = atoi(anim->GetVariable("touchframe").c_str());
  bool touch = (animTouchFrame > 0);

  float stat_agility = player->GetStat(physical_agility);
  float stat_acceleration = player->GetStat(physical_acceleration);
  float stat_velocity = player->GetStat(physical_velocity);
  float stat_dribble = player->GetStat(technical_dribble);

  float incomingSwitchBias = 0.0f; // anything other than 0.0 may result in unpuristic behavior
  float outgoingSwitchBias = 0.0f;

  e_DefString animType = anim->GetAnimType();

  if (animType == e_DefString_BallControl) {
    outgoingSwitchBias = 0.0f;
  } else if (animType== e_DefString_Trap) {
    outgoingSwitchBias = 0.0f;
  } else if (animType== e_DefString_Interfere) {
    outgoingSwitchBias = 0.0f;
  } else if (animType== e_DefString_Deflect) {
    outgoingSwitchBias = 1.0f;
  } else if (animType== e_DefString_Sliding) {
    outgoingSwitchBias = 0.0f;
  } else if (animType== e_DefString_Special) {
    outgoingSwitchBias = 1.0f;
  } else if (animType== e_DefString_Trip) {
    outgoingSwitchBias = 0.5f; // direction partly predecided by collision function in match class
  } else if (touch) {
    outgoingSwitchBias = 1.0f;
  }

  if (anim->GetVariableCache().incoming_special_state().compare("") != 0 ||
      anim->GetVariableCache().outgoing_special_state().compare("") != 0) outgoingSwitchBias = 1.0f;

  Vector3 animIncomingMovement = Vector3(0, -1, 0).GetRotated2D(spatialState.angle) * RangeVelocity(anim->GetIncomingVelocity());
  Vector3 adaptedCurrentMovement = animIncomingMovement * incomingSwitchBias + spatialState.movement * (1.0f - incomingSwitchBias);

  Vector3 predictedOutgoingMovement = anim->GetOutgoingMovement().GetRotated2D(spatialState.angle);
  Vector3 velocifiedDesiredMovement = (useDesiredMovement) ? desiredMovement : predictedOutgoingMovement;

  assert(desiredMovement.coords[2] == 0.0f);

  Vector3 adaptedDesiredMovement = predictedOutgoingMovement * outgoingSwitchBias + velocifiedDesiredMovement * (1.0f - outgoingSwitchBias);
  assert(predictedOutgoingMovement.coords[2] == 0.0f);
  assert(velocifiedDesiredMovement.coords[2] == 0.0f);
  assert(adaptedDesiredMovement.coords[2] == 0.0f);

  float maxVelocity = player->GetMaxVelocity();
  if (touch) maxVelocity *= 0.92f;

  Vector3 resultingMovement;

  const int timeStep_ms = 10;

  bool isBaseAnim = anim->GetVariableCache().baseanim();

  float difficultyFactor = atof(anim->GetVariable("animdifficultyfactor").c_str());
  float difficultyPenaltyFactor = std::pow(
      clamp((difficultyFactor - 0.0f) *
                (1.0f - (stat_agility * 0.2f + stat_acceleration * 0.2f)) *
                2.0f,
            0.0f, 1.0f),
      0.7f);

  float powerFactor = 1.0f - clamp(pow(player->GetLastTouchBias(1000), 0.8f) * (0.8f - stat_dribble * 0.3f), 0.0f, 0.4f);
  powerFactor *= 1.0f - clamp(decayingPositionOffset.GetLength() * (10.0f - player->GetStat(physical_balance) * 5.0f) - 0.1f, 0.0f, 0.3f);

  Vector3 temporalMovement = adaptedCurrentMovement;

  assert(adaptedCurrentMovement.coords[2] == 0.0f);
  assert(adaptedDesiredMovement.coords[2] == 0.0f);


  // orig anim positions

  const std::vector<Vector3> &origPositionCache = match->GetAnimPositionCache(anim);

  Vector3 currentPosition;

  // amount of pure physics that 'shines through' pure anim
  float physicsBias = 1.0f;
  // angle deviation away from anim
  radian maxAngleMod_underAnimAngle = 0.125f * pi;
  radian maxAngleMod_overAnimAngle = 0.125f * pi;
  radian maxAngleMod_straightAnimAngle = 0.125f * pi;
  if (touch) {
    float bonus = 1.0f - std::pow(NormalizedClamp((adaptedCurrentMovement +
                                                   predictedOutgoingMovement)
                                                          .GetLength() *
                                                      0.5f,
                                                  0, sprintVelocity),
                                  0.8f) *
                             0.8f;
    bonus *= 0.6f + 0.4f * player->GetStat(technical_ballcontrol);
    maxAngleMod_underAnimAngle = 0.2f * pi * bonus;
    maxAngleMod_overAnimAngle = 0;
    maxAngleMod_straightAnimAngle = 0.1f * pi * bonus;
  }
  if (animType== e_DefString_Sliding) {
    maxAngleMod_underAnimAngle = 0.5f * pi;
    maxAngleMod_overAnimAngle = 0.5f * pi;
    maxAngleMod_straightAnimAngle = 0.5f * pi;
  }

  if (animType == e_DefString_Movement)    { physicsBias *= 1.0f; }

  if (animType == e_DefString_BallControl) { physicsBias *= 1.0f; }
  if (animType== e_DefString_Trap)        { physicsBias *= 1.0f; }

  if (animType== e_DefString_ShortPass)   { physicsBias *= 0.0f; }
  if (animType== e_DefString_HighPass)    { physicsBias *= 0.0f; }
  if (animType== e_DefString_Shot)        { physicsBias *= 0.0f; }

  if (animType== e_DefString_Interfere)   { physicsBias *= 0.5f; }
  if (animType== e_DefString_Deflect)     { physicsBias *= 0.0f; }

  if (animType== e_DefString_Sliding)     { physicsBias *= 1.0f; }
  if (animType== e_DefString_Trip)        { if (anim->GetVariable("triptype").compare("1") == 0) physicsBias *= 0.5f; else physicsBias *= 0.0f; }

  if (animType== e_DefString_Special)     { physicsBias *= 0.0f; }
  if (anim->GetVariableCache().incoming_special_state().compare("") != 0)
                                            { physicsBias *= 0.0f; }

  bool mod_AllowRotation = true;
  bool mod_CorneringBraking = false;
  bool mod_PointinessCurve = true;
  bool mod_MaximumAccelDecel = false;
  bool mod_BrakeOnTouch = false; // may be too pointy for anims near 90 degree
  bool mod_MaxCornering = true;
  bool mod_MaxChange = true;
  bool mod_AirResistance = true;
  bool mod_CheatBodyDirection = false;

  float accelerationMultiplier = 0.5f + _cache_AccelerationFactor;


  // rotate anim towards desired angle

  radian toDesiredAngle_capped = 0;
  if (mod_AllowRotation && physicsBias > 0.0f) {
    Vector3 animOutgoingVector = predictedOutgoingMovement.GetNormalized(0);
    if (FloatToEnumVelocity(predictedOutgoingMovement.GetLength()) == e_Velocity_Idle) animOutgoingVector = anim->GetOutgoingDirection().GetRotated2D(spatialState.angle);
    Vector3 desiredVector = adaptedDesiredMovement.GetNormalized(0);
    if (FloatToEnumVelocity(adaptedDesiredMovement.GetLength()) == e_Velocity_Idle) desiredVector = desiredBodyDirectionRel.GetRotated2D(spatialState.angle);
    radian toDesiredAngle = desiredVector.GetAngle2D(animOutgoingVector);
    if (fabs(toDesiredAngle) <= 0.5f * pi || animType== e_DefString_Sliding) { // if we want > x degrees, just skip it to next anim, it'll only look weird otherwise

      radian animChange = animOutgoingVector.GetAngle2D(spatialState.directionVec);
      if (fabs(animChange) > 0.06f * pi) {
        int sign = signSide(animChange);
        if (signSide(toDesiredAngle) == sign) {
          toDesiredAngle_capped = clamp(toDesiredAngle, -maxAngleMod_overAnimAngle, maxAngleMod_overAnimAngle);
        } else {
          toDesiredAngle_capped = clamp(toDesiredAngle, -maxAngleMod_underAnimAngle, maxAngleMod_underAnimAngle);
        }
      } else {
        // straight ahead anim, has no specific side.
        toDesiredAngle_capped = clamp(toDesiredAngle, -maxAngleMod_straightAnimAngle, maxAngleMod_straightAnimAngle);
      }
      //printf("animChange: %f radians, to desired angle: %f radians, sign: %i, sign side of to desired angle: %i, resulting capped angle: %f radians\n", animChange, toDesiredAngle, sign, signSide(toDesiredAngle), toDesiredAngle_capped);
    }
  }


  float maximumOutgoingVelocity = sprintVelocity;
  // brake on cornering
  if (mod_CorneringBraking) {

    float brakeBias = 0.8f;
    brakeBias *= (touch) ? 1.0f : 0.8f;
    brakeBias *= (1.0f - stat_agility * 0.2f);

    Vector3 animOutgoingMovement = anim->GetOutgoingMovement();
    animOutgoingMovement.Rotate2D(toDesiredAngle_capped);
    brakeBias *= std::pow(NormalizedClamp(spatialState.floatVelocity,
                                          idleVelocity, sprintVelocity - 0.5f),
                          0.8f);  // 0.5f);
    float maxVelo =
        sprintVelocity *
        ((1.0f - brakeBias) +
         ((1.0f -
           std::pow(fabs(animOutgoingMovement.GetNormalized(0).GetAngle2D(
                             Vector3(0, -1, 0)) /
                         pi),
                    0.5f)) *
          brakeBias));
    maximumOutgoingVelocity = maxVelo;
  }


  // --- loop da loop ------------------------------------------------------------------------------------------------------------------------------------------
  for (int time_ms = 0; time_ms < anim->GetFrameCount() * 10; time_ms += timeStep_ms) {

    // start with +1, because we want to influence the first frame as well
    // as for finishing, finish with frameBias = 1.0, even if the last frame is 'spiritually' the one-to-last, since the first frame of the next anim is actually 'same-tempered' as the current anim's last frame.
    // however, it works best to have all values 'done' at this one-to-last frame, so the next anim can read out these correct (new starting) values.
    float frameBias = (time_ms + 10) / (float)((anim->GetEffectiveFrameCount() + 1) * 10);

    float lagExp = 1.0f;
    if (mod_PointinessCurve && physicsBias > 0.0f && (animType == e_DefString_BallControl || animType == e_DefString_Movement)) {
      lagExp = 1.4f - _cache_AgilityFactor * 0.8f;
      lagExp *= 1.2f - stat_agility * 0.4f;
      if (touch) {
        lagExp += -0.1f + clamp(difficultyFactor * 0.4f, 0.0f, 0.5f);
      } else {
        lagExp += -0.2f + clamp(difficultyFactor * 0.2f, 0.0f, 0.2f);
      }

      lagExp = clamp(lagExp, 0.25f, 4.0f);
      if (touch && time_ms < animTouchFrame * 10) lagExp = std::max(lagExp, 0.7f); // else, we could 'miss' the ball because we're already turned around too much

      lagExp = lagExp * physicsBias + 1.0f * (1.0f - physicsBias);
    }
    float adaptedFrameBias = std::pow(frameBias, lagExp);
    Vector3 animMovement = CalculateMovementAtFrame(origPositionCache, anim->GetEffectiveFrameCount() * adaptedFrameBias, 1).GetRotated2D(spatialState.angle);

    float animVelo = animMovement.GetLength();
    Vector3 adaptedAnimMovement = animMovement;
    float adaptedAnimVelo = animVelo;


    // adapt sprint velocity to player's max velocity stat

    if (animVelo > walkSprintSwitch && (animType == e_DefString_Movement || animType == e_DefString_BallControl || animType== e_DefString_Trap)) {

      if (maxVelocity > animVelo) { // only speed up, don't slow down. may be faster parts (jumps and such) within anim, allow this
        adaptedAnimVelo = StretchSprintTo(animVelo, animSprintVelocity, maxVelocity);
        adaptedAnimMovement = adaptedAnimMovement.GetNormalized(0) * adaptedAnimVelo;
      }

    }


    float maxSlower = 1.6f; // rationale: don't want to end up below dribbleVelocity - idleDribbleSwitch (= change velocity)
    if (touch) maxSlower = 1.2f;
    float maxFaster = 0.0f;
    if (touch) maxFaster = 0.0f;
    if (temporalMovement.GetLength() > adaptedAnimVelo) maxFaster = std::min(0.0f + 1.0f * (1.0f - frameBias), std::max(maxFaster, temporalMovement.GetLength() - adaptedAnimVelo)); // ..already going faster.. well okay, allow this
    if (maxFaster > 0) maxFaster *= std::max(0.0f, adaptedAnimMovement.GetNormalizedMax(1.0f).GetDotProduct(adaptedDesiredMovement.GetNormalized(0))); // only go faster if it's in the right direction
    if (animType== e_DefString_Sliding) maxFaster = 100;
    float desiredVelocity = adaptedDesiredMovement.GetLength();
    adaptedAnimVelo = clamp(desiredVelocity, adaptedAnimVelo - maxSlower, adaptedAnimVelo + maxFaster);
    adaptedAnimMovement = adaptedAnimMovement.GetNormalized(0) * adaptedAnimVelo;

    if (mod_CorneringBraking) {
      float frameBiasedMaximumOutgoingVelocity = sprintVelocity * (1.0f - frameBias) + maximumOutgoingVelocity * frameBias;
      if (adaptedAnimVelo > frameBiasedMaximumOutgoingVelocity) {
        adaptedAnimVelo = frameBiasedMaximumOutgoingVelocity;
        adaptedAnimMovement = adaptedAnimMovement.GetNormalized(0) * adaptedAnimVelo;
      }
    }

    if (mod_MaximumAccelDecel) {
      // this is basically meant to enforce transitions to be smoother, disallowing bizarre steps. however, with low enough max values, it can also serve as a physics slowness thing.
      // in that regard, the maxaccel part is somewhat similar to the air resistance mod below. they can live together; this one can serve as a constant maximum, and the air resistance as a velocity-based maximum.
      // update: decided to not let them live together, this should now purely be used for capping transition speed
      float maxAccelMPS = 20.0f;
      float maxDecelMPS = 20.0f;
      float currentVelo = temporalMovement.GetLength();
      float veloChangeMPS = (adaptedAnimVelo - currentVelo) / ((float)timeStep_ms * 0.001f);
      if (veloChangeMPS < -maxDecelMPS || veloChangeMPS > maxAccelMPS) {
        adaptedAnimVelo = currentVelo + clamp(veloChangeMPS, -maxDecelMPS, maxAccelMPS) * ((float)timeStep_ms * 0.001f);
        adaptedAnimMovement = adaptedAnimMovement.GetNormalized(0) * adaptedAnimVelo;
      }
    }

    Vector3 resultingPhysicsMovement = adaptedAnimMovement;

    // angle
    resultingPhysicsMovement = resultingPhysicsMovement.GetRotated2D(toDesiredAngle_capped * frameBias);// * pow(frameBias, 0.6f));


    // --- stay true to anim? -----------------------------------------------------------------------------------------------------------------------

    resultingPhysicsMovement = resultingPhysicsMovement * physicsBias + animMovement * (1.0f - physicsBias);


    // that's it, we now know where we want to go in life

    Vector3 toDesired = (resultingPhysicsMovement - temporalMovement);


    // --- end --------------------------------------------------------------------------------------------------------------------------------------


    assert(toDesired.coords[2] == 0.0f);


    float penaltyBreakFactor = 0.0f;
    if (mod_BrakeOnTouch) {
      // slow down after touching ball
      // (precalc at touchframe, because temporalMovement will change because of this, so if we don't precalc then changing numBrakeFrames will change the amount of effect)
      int numBrakeFrames = 15;
      if (touch && time_ms >= animTouchFrame * 10 && time_ms < (animTouchFrame + numBrakeFrames) * 10) {
        int brakeFramesInto = (time_ms - (animTouchFrame * 10)) / 10;
        float brakeFrameFactor =
            std::pow(1.0f - (brakeFramesInto / (float)numBrakeFrames), 0.5f);

        float touchBrakeFactor = 0.3f;

        float touchDifficultyFactor = clamp((difficultyFactor + 0.7f) * (1.0f - stat_dribble * 0.4f), 0.0, 1.0f);
        touchDifficultyFactor *=
            1.0f -
            std::pow(
                fabs(anim->GetOutgoingAngle()) / pi,
                0.75f);  // don't help with braking (when going nearer 180 deg)

        float veloFactor = NormalizedClamp(temporalMovement.GetLength(), walkVelocity, sprintVelocity);

        penaltyBreakFactor = touchDifficultyFactor * veloFactor * brakeFrameFactor * touchBrakeFactor;
      }
    }

    /* part of new method, but needs some debugging/unittesting
    // increase over multiple frames for smoother effect, and so we get a proper effect even if maxChange isn't very high
    int numBrakeFrames = 5;
    if (touch && time_ms >= animTouchFrame * 10 && time_ms < (animTouchFrame + numBrakeFrames) * 10) {
      int framesInto = (time_ms - (animTouchFrame * 10)) / 10;
      toDesired += -temporalMovement.GetNormalized(0) * (ballTouchSlowdownAmount / (float)(numBrakeFrames - framesInto));
    }
    */

    if (mod_MaxCornering) {
      Vector3 predictedMovement = temporalMovement + toDesired;
      float startVelo = idleDribbleSwitch;
      if (temporalMovement.GetLength() > startVelo && predictedMovement.GetLength() > startVelo) {
        radian angle = predictedMovement.GetNormalized().GetAngle2D(temporalMovement.GetNormalized());
        float maxAngleFactor = 1.0f * (timeStep_ms / 1000.0f);
        maxAngleFactor *= (0.7f + 0.3f * stat_agility);
        if (!touch) maxAngleFactor *= 1.5f;
        radian maxAngle = maxAngleFactor * pi;
        float veloFactor = std::pow(
            NormalizedClamp(temporalMovement.GetLength(), 0, sprintVelocity),
            1.0f);
        maxAngle /= (veloFactor + 0.01f);

        if (fabs(angle) > maxAngle) {

          int mode = 1; // 0: restrict max angle, 1: restrict velocity

          if (mode == 0) {
            Vector3 restrictedPredictedMovement = predictedMovement.GetRotated2D((fabs(angle) - maxAngle) * -signSide(angle));
            Vector3 newToDesired = restrictedPredictedMovement - temporalMovement;
            toDesired = newToDesired;
          } else if (mode == 1) {
            radian overAngle = fabs(angle) - maxAngle; // > 0
            toDesired += -temporalMovement * clamp(overAngle / pi * 3.0f, 0.0f, 1.0f);// was: 3
          }

        }
      }
    }

    if (mod_MaxChange) {
      float maxChange = 0.03f;
      if (animType== e_DefString_Trip) maxChange *= 0.7f;
      if (animType== e_DefString_Sliding) maxChange = 0.1f;
      // no power first few frames, so transitions are smoother
      //maxChange *= 0.3f + 0.7f * curve(NormalizedClamp(time_ms, 0.0f, 80.0f), 1.0f);
      float veloFactor = std::pow(
          NormalizedClamp(temporalMovement.GetLength(), 0, sprintVelocity),
          1.5f);
      float firstStepFactor = veloFactor;
      if (animType == e_DefString_Movement) firstStepFactor *= 0.4f;
      maxChange *= (1.0f - firstStepFactor) + firstStepFactor * curve(NormalizedClamp(time_ms, 0.0f, 160.0f), 1.0f);

      maxChange *= 1.2f - veloFactor * 0.4f;

      maxChange *= 0.75f + _cache_AgilityFactor * 0.5f;

      // lose power 'around' touch

      // if (touch && time_ms >= animTouchFrame * 10) {
      //   int influenceFrames = 16; // number of frames to slow down on each 'side' of the balltouch
      //   //float frameBias = NormalizedClamp(fabs((animTouchFrame * 10) - time_ms), 0, influenceFrames * 10) * 0.7f + 0.3f;
      //   float frameBias = NormalizedClamp(time_ms - (animTouchFrame * 10), 0, influenceFrames * 10) * 1.0f;// + 0.1f;
      //   frameBias = curve(frameBias, 1.0f);
      //   maxChange *= clamp(frameBias, 0.1f, 1.0f);
      // }

      maxChange *= powerFactor;

      float desiredLength = toDesired.GetLength();
      float maxAddition = maxChange * timeStep_ms;

      toDesired.NormalizeMax(std::min(desiredLength, maxAddition));
    }


    // air resistance

    if (mod_AirResistance && animType != e_DefString_Sliding && animType != e_DefString_Deflect) {
      float veloExp = 1.8f;
      float accelPower = 11.0f * accelerationMultiplier;
      float falloffStartVelo = idleDribbleSwitch;

      if ((temporalMovement + toDesired).GetLength() > falloffStartVelo) {

        // less accelpower on tough anims
        accelPower *= 1.0f - difficultyPenaltyFactor * 0.4f;

        // wolfram alpha to show difference in sin before/after exp order: 10 * (sin(((1.0 - x ^ 2.0) - 0.5) * pi) * 0.5 + 0.5), 10 * ((1.0 - (sin((x - 0.5) * pi) * 0.5 + 0.5)) ^ 2.0) | from x = 0.0 to 1.0

        float veloAirResistanceFactor = clamp(
            std::pow(clamp((temporalMovement.GetLength() - falloffStartVelo) /
                               (player->GetMaxVelocity() - falloffStartVelo),
                           0.0f, 1.0f),
                     veloExp),
            0.0f, 1.0f);

        // simulate footsteps - powered in the middle
        //veloAirResistanceFactor = 1.0f - ((1.0f - veloAirResistanceFactor) * pow(sin(frameBias * pi), 2.0f));

        // circular version
        Vector3 forwardVector;
        if ((temporalMovement + toDesired).GetLength() > temporalMovement.GetLength()) { // outside the 'velocity circle'
          Vector3 destination = temporalMovement + toDesired;
          float velo = temporalMovement.GetLength();
          float accel = destination.GetLength() - velo;
          forwardVector = destination.GetNormalized(0) * accel;
        }

        float accelerationAddition = forwardVector.GetLength();
        float maxAccelerationMPS = accelPower * (1.0f - veloAirResistanceFactor) * (stat_acceleration * 0.3f + 0.7f);
        float maxAccelerationAddition = maxAccelerationMPS * (timeStep_ms / 1000.0f);
        if (accelerationAddition > maxAccelerationAddition) {
          float remainingFactor = maxAccelerationAddition / accelerationAddition;
          toDesired = toDesired - forwardVector * (1.0f - remainingFactor);
        }

      }

    }


    // MAKE IT SEW! http://static.wixstatic.com/media/fc58ad_c0ef2d69d98f4f7ba8e7e488f0e28ece.jpg

    Vector3 tmpTemporalMovement = temporalMovement + toDesired;

    // make sure outgoing velocity is of the same idleness as the anim
    if (time_ms >= (anim->GetFrameCount() - 2) * 10) {

      bool hardQuantize = true;
      if (!hardQuantize && anim->GetVariableCache().outgoing_special_state().compare("") != 0) hardQuantize = true;

      if (!hardQuantize) {
        // soft version
        if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) == e_Velocity_Idle && FloatToEnumVelocity(tmpTemporalMovement.GetLength()) != e_Velocity_Idle) tmpTemporalMovement.NormalizeTo(idleDribbleSwitch - 0.01f);
        else if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) != e_Velocity_Idle && FloatToEnumVelocity(tmpTemporalMovement.GetLength()) == e_Velocity_Idle) tmpTemporalMovement = anim->GetOutgoingMovement().GetRotated2D(spatialState.angle).GetNormalizedTo(idleDribbleSwitch + 0.01f);
      } else {
        // hard version
        if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) == e_Velocity_Idle && FloatToEnumVelocity(tmpTemporalMovement.GetLength()) != e_Velocity_Idle) tmpTemporalMovement = 0;
        else if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) != e_Velocity_Idle && FloatToEnumVelocity(tmpTemporalMovement.GetLength()) == e_Velocity_Idle) tmpTemporalMovement = anim->GetOutgoingMovement().GetRotated2D(spatialState.angle).GetNormalizedTo(dribbleVelocity);
      }
    }

    assert(tmpTemporalMovement.coords[2] == 0.0f);
    temporalMovement = tmpTemporalMovement;

    if (time_ms >= (anim->GetFrameCount() - 2) * 10) penaltyBreakFactor = 0.0f;
    currentPosition += temporalMovement * (1.0f - penaltyBreakFactor) * (timeStep_ms / 1000.0f);
    assert(currentPosition.coords[2] == 0.0f);

    if (time_ms % 10 == 0) {
      positions_ret.push_back(currentPosition);
    }

    /*
    // dynamic timestep: more precision at high velocities
    if ((int)time_ms % 10 == 0) {
      //if (temporalMovement.GetLength() > walkVelocity) timeStep_ms = 5; else timeStep_ms = 10;
      timeStep_ms = 10;
    }
    */
  }

  assert(positions_ret.size() >= (unsigned int)anim->GetFrameCount());
  resultingMovement = temporalMovement;


  if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) != e_Velocity_Idle && FloatToEnumVelocity(resultingMovement.GetLength()) != e_Velocity_Idle) {
    rotationOffset_ret = resultingMovement.GetRotated2D(-spatialState.angle).GetAngle2D(anim->GetOutgoingMovement());
  } else {
    rotationOffset_ret = toDesiredAngle_capped * physicsBias;
  }


  // body direction assist
  if (mod_CheatBodyDirection && useDesiredBodyDirection && animType == e_DefString_Movement) {

    float angleFactor = 0.5f;
    radian maxAngle = 0.25f * pi;

    radian predictedAngleRel = anim->GetOutgoingAngle() + anim->GetOutgoingBodyAngle() + rotationOffset_ret;
    radian desiredRotationOffset = desiredBodyDirectionRel.GetRotated2D(-predictedAngleRel).GetAngle2D(Vector3(0, -1, 0));

    if (fabs(desiredRotationOffset) < 0.5f * pi) { // else: too much

      float outgoingVelocityFactorInv = 1.0f - NormalizedClamp(resultingMovement.GetLength(), idleDribbleSwitch, sprintVelocity - 1.0f) * 1.0f;
      float animLengthFactor = NormalizedClamp(anim->GetFrameCount(), 0, 25);
      radian maximizedRotationOffset = clamp(desiredRotationOffset, outgoingVelocityFactorInv * animLengthFactor * angleFactor * -maxAngle,
                                                                    outgoingVelocityFactorInv * animLengthFactor * angleFactor *  maxAngle);

      rotationOffset_ret += maximizedRotationOffset;
    }
  }

  assert(resultingMovement.coords[2] == 0.0f);

  return resultingMovement;
}


Vector3 HumanoidBase::ForceIntoAllowedBodyDirectionVec(const Vector3 &src) const {

  // check what allowed dir this vector is closest to
  float bestDot = -1.0f;
  int bestIndex = 0;
  for (unsigned int i = 0; i < allowedBodyDirVecs.size(); i++) {
    float nDotL = allowedBodyDirVecs[i].GetDotProduct(src);
    if (nDotL > bestDot) {
      bestDot = nDotL;
      bestIndex = i;
    }
  }

  return allowedBodyDirVecs.at(bestIndex);
}

radian HumanoidBase::ForceIntoAllowedBodyDirectionAngle(radian angle) const {

  float bestAngleDiff = 10000.0;
  int bestIndex = 0;
  for (unsigned int i = 0; i < allowedBodyDirAngles.size(); i++) {
    float diff = fabs(allowedBodyDirAngles[i] - angle);
    if (diff < bestAngleDiff) {
      bestAngleDiff = diff;
      bestIndex = i;
    }
  }

  return allowedBodyDirAngles.at(bestIndex);
}

Vector3 HumanoidBase::ForceIntoPreferredDirectionVec(const Vector3 &src) const {

  float bestDot = -1.0f;
  int bestIndex = 0;
  for (unsigned int i = 0; i < preferredDirectionVecs.size(); i++) {
    float nDotL = preferredDirectionVecs[i].GetDotProduct(src);
    if (nDotL > bestDot) {
      bestDot = nDotL;
      bestIndex = i;
    }
  }

  return preferredDirectionVecs.at(bestIndex);
}

radian HumanoidBase::ForceIntoPreferredDirectionAngle(radian angle) const {

  float bestAngleDiff = 10000.0;
  int bestIndex = 0;
  for (unsigned int i = 0; i < preferredDirectionAngles.size(); i++) {
    float diff = fabs(preferredDirectionAngles[i] - angle);
    if (diff < bestAngleDiff) {
      bestAngleDiff = diff;
      bestIndex = i;
    }
  }

  return preferredDirectionAngles.at(bestIndex);
}
