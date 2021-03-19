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

#include "../../../scene/objectfactory.hpp"

constexpr float bodyRotationSmoothingFactor = 1.0f;
constexpr float bodyRotationSmoothingMaxAngle = 0.25f * pi;
constexpr float initialReQueueDelayFrames = 32;

const Vector3 allowedBodyDirVecs[] = {
    Vector3(0, -1, 0),
    Vector3(0, -1, 0).GetRotated2D(-0.25 * pi),
    Vector3(0, -1, 0).GetRotated2D(0.25 * pi),
    Vector3(0, -1, 0).GetRotated2D(-0.75 * pi),
    Vector3(0, -1, 0).GetRotated2D(0.75 * pi)
};

const radian allowedBodyDirAngles[] = {
        0 * pi,
     0.25 * pi,
    -0.25 * pi,
     0.75 * pi,
    -0.75 * pi
};

const Vector3 preferredDirectionVecs[] = {
    Vector3(0, -1, 0),
    Vector3(0, -1, 0).GetRotated2D(0.111 * pi),
    Vector3(0, -1, 0).GetRotated2D(-0.111 * pi),
    Vector3(0, -1, 0).GetRotated2D(0.25 * pi),
    Vector3(0, -1, 0).GetRotated2D(-0.25 * pi),
    Vector3(0, -1, 0).GetRotated2D(0.5 * pi),
    Vector3(0, -1, 0).GetRotated2D(-0.5 * pi),
    Vector3(0, -1, 0).GetRotated2D(0.75 * pi),
    Vector3(0, -1, 0).GetRotated2D(-0.75 * pi),
    Vector3(0, -1, 0).GetRotated2D(0.999 * pi),
    Vector3(0, -1, 0).GetRotated2D(-0.999 * pi)
};

const radian preferredDirectionAngles[] = {
    0 * pi,
    0.111 * pi, // 20
    -0.111 * pi,
    0.25 * pi, // 45
    -0.25 * pi,
    0.5 * pi, // 90
    -0.5 * pi,
    0.75 * pi, // 135
    -0.75 * pi,
    0.999 * pi, // 180
    -0.999 * pi
};

HumanoidBase::HumanoidBase(PlayerBase *player, Match *match,
                           boost::intrusive_ptr<Node> humanoidSourceNode,
                           boost::intrusive_ptr<Node> fullbodySourceNode,
                           std::map<Vector3, Vector3> &colorCoords,
                           boost::shared_ptr<AnimCollection> animCollection,
                           boost::intrusive_ptr<Node> fullbodyTargetNode,
                           boost::intrusive_ptr<Resource<Surface> > kit)
    : fullbodyTargetNode(fullbodyTargetNode),
      match(match),
      player(player),
      anims(animCollection),
      zMultiplier((1.0f / defaultPlayerHeight) * player->GetPlayerData()->GetHeight()) {
  DO_VALIDATION;
  interruptAnim = e_InterruptAnim_None;
  reQueueDelayFrames = 0;
  decayingPositionOffset = Vector3(0);
  decayingDifficultyFactor = 0.0f;

  assert(match);

  boost::intrusive_ptr<Node> bla(new Node(*humanoidSourceNode.get(), "", GetScene3D()));
  humanoidNode = bla;
  humanoidNode->SetLocalMode(e_LocalMode_Absolute);

  boost::intrusive_ptr < Resource<Surface> > skin;
  skin = GetContext().surface_manager.Fetch(
      "media/objects/players/textures/skin0" +
          int_to_str(player->GetPlayerData()->GetSkinColor()) + ".png",
      true, true);

  boost::intrusive_ptr<Node> bla2(new Node(*fullbodySourceNode.get(), int_to_str(GetContext().playerCount++), GetScene3D()));
  fullbodyNode = bla2;
  fullbodyNode->SetLocalMode(e_LocalMode_Absolute);
  fullbodyTargetNode->AddNode(fullbodyNode);

  boost::intrusive_ptr< Resource<GeometryData> > bodyGeom = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();
  std::vector < MaterializedTriangleMesh > &tmesh = bodyGeom->GetResource()->GetTriangleMeshesRef();
  for (unsigned int i = 0; i < tmesh.size(); i++) {
    DO_VALIDATION;
    if (tmesh[i].material.diffuseTexture !=
        boost::intrusive_ptr<Resource<Surface> >()) {
      DO_VALIDATION;
      if (tmesh[i].material.diffuseTexture->GetIdentString() == "skin.jpg") {
        DO_VALIDATION;
        tmesh[i].material.diffuseTexture = skin;
        tmesh[i].material.specular_amount = 0.002f;
        tmesh[i].material.shininess = 0.2f;
      }
    }
  }

  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData();

  SetKit(kit);


  FillNodeMap(humanoidNode, nodeMap);

  PrepareFullbodyModel(colorCoords);


  // hairstyle

  boost::intrusive_ptr<Resource<GeometryData> > geometry =
      GetContext().geometry_manager.Fetch(
          "media/objects/players/hairstyles/" +
              player->GetPlayerData()->GetHairStyle() + ".ase",
          true, true);
  hairStyle = new Geometry("hairstyle");
  GetScene3D()->CreateSystemObjects(hairStyle);
  hairStyle->SetLocalMode(e_LocalMode_Absolute);
  hairStyle->SetGeometryData(geometry);
  fullbodyTargetNode->AddObject(hairStyle);

  boost::intrusive_ptr < Resource<Surface> > hairTexture;
  hairTexture = GetContext().surface_manager.Fetch(
      "media/objects/players/textures/hair/" +
          player->GetPlayerData()->GetHairColor() + ".png",
      true, true);

  std::vector < MaterializedTriangleMesh > &hairtmesh = hairStyle->GetGeometryData()->GetResource()->GetTriangleMeshesRef();

  for (unsigned int i = 0; i < hairtmesh.size(); i++) {
    DO_VALIDATION;
    if (hairtmesh[i].material.diffuseTexture !=
        boost::intrusive_ptr<Resource<Surface> >()) {
      DO_VALIDATION;
      hairtmesh[i].material.diffuseTexture = hairTexture;
      hairtmesh[i].material.specular_amount = 0.01f;
      hairtmesh[i].material.shininess = 0.05f;
    }
  }
  hairStyle->OnUpdateGeometryData();


  ResetPosition(Vector3(0), Vector3(0));
  mentalImageTime = 0;
}

HumanoidBase::~HumanoidBase() {
  DO_VALIDATION;
  humanoidNode->Exit();
  humanoidNode.reset();
  fullbodyTargetNode->DeleteNode(fullbodyNode);
  fullbodyTargetNode->DeleteObject(hairStyle);

  for (unsigned int i = 0; i < uniqueFullbodyMesh.size(); i++) {
    DO_VALIDATION;
    delete [] uniqueFullbodyMesh[i].data;
  }

  for (unsigned int i = 0; i < uniqueIndicesVec.size(); i++) {
    DO_VALIDATION;
    delete [] uniqueIndicesVec[i];
  }

  // the other full body mesh ref is connected to a geometry object which has taken over ownership and will clean it up
}

void HumanoidBase::Mirror() {
  // fullbodyNode - mirror for ball collision and render.
  humanoidNode->SetPosition(humanoidNode->GetPosition() * Vector3(-1, -1, 1),
                            false);
  Quaternion rotation = humanoidNode->GetRotation();
  if (!mirrored) {
    humanoidNode->SetRotation(Quaternion(rotation.elements[1], rotation.elements[0], rotation.elements[3], -rotation.elements[2]));
  } else {
    humanoidNode->SetRotation(Quaternion(rotation.elements[1], rotation.elements[0], -rotation.elements[3], rotation.elements[2]));
  }
  mirrored = !mirrored;
  startPos.Mirror();
  startAngle.Mirror();
  nextStartPos.Mirror();
  nextStartAngle.Mirror();
  spatialState.Mirror();
  previousPosition2D.Mirror();
  tripDirection.Mirror();
  decayingPositionOffset.Mirror();
  predicate_RelDesiredDirection.Mirror();
  predicate_DesiredDirection.Mirror();
  predicate_RelIncomingBodyDirection.Mirror();
  predicate_LookAt.Mirror();
  predicate_RelDesiredTripDirection.Mirror();
  predicate_RelDesiredBallDirection.Mirror();
  // movementHistory // only animation?
}

void HumanoidBase::PrepareFullbodyModel(
    std::map<Vector3, Vector3> &colorCoords) {
  DO_VALIDATION;

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
    DO_VALIDATION;
    HJoint joint;
    joint.node = jointsVec[i];
    joint.origPos = jointsVec[i]->GetDerivedPosition();
    joints.push_back(joint);
  }

  boost::intrusive_ptr < Resource<GeometryData> > fullbodyGeometryData = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();
  std::vector < MaterializedTriangleMesh > &materializedTriangleMeshes = fullbodyGeometryData->GetResource()->GetTriangleMeshesRef();

  fullbodySubgeomCount = materializedTriangleMeshes.size();

  for (unsigned int subgeom = 0; subgeom < fullbodySubgeomCount; subgeom++) {
    DO_VALIDATION;

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
      DO_VALIDATION;
      std::vector<Vector3> elementalVertex;
      for (int e = 0; e < GetTriangleMeshElementCount(); e++) {
        DO_VALIDATION;
        elementalVertex.push_back(Vector3(meshRef.data[v + e * elementOffset], meshRef.data[v + e * elementOffset + 1], meshRef.data[v + e * elementOffset + 2]));
        // test: if (e == 2) elementalVertex.at(elementalVertex.size() - 1) += 0.2f;
      }

      // see if this one already exists; if not, add
      bool duplicate = false;
      int index = 0;
      for (unsigned int i = 0; i < uniqueVertices.size(); i++) {
        DO_VALIDATION;
        if (uniqueVertices[i][0] == elementalVertex[0] &&
            uniqueVertices[i][2] == elementalVertex[2]) {
          DO_VALIDATION;  // texcoord also needs to be shared
          duplicate = true;
          index = i;
          break;
        }
      }
      if (!duplicate) {
        DO_VALIDATION;
        uniqueVertices.push_back(elementalVertex);
        index = uniqueVertices.size() - 1;
      }
      uniqueIndices[v / 3] = index;
    }

    uniqueMesh.size = uniqueVertices.size() * 3 * GetTriangleMeshElementCount();
    uniqueMesh.data = new float[uniqueMesh.size];

    int uniqueElementOffset = uniqueMesh.size / GetTriangleMeshElementCount();

    assert((unsigned int)uniqueMesh.size == uniqueVertices.size() * GetTriangleMeshElementCount() * 3);

    for (unsigned int v = 0; v < uniqueVertices.size(); v++) {
      DO_VALIDATION;
      for (int e = 0; e < GetTriangleMeshElementCount(); e++) {
        DO_VALIDATION;
        uniqueMesh.data[v * 3 + e * uniqueElementOffset + 0] = uniqueVertices.at(v).at(e).coords[0];
        uniqueMesh.data[v * 3 + e * uniqueElementOffset + 1] = uniqueVertices.at(v).at(e).coords[1];
        uniqueMesh.data[v * 3 + e * uniqueElementOffset + 2] = uniqueVertices.at(v).at(e).coords[2];
      }
    }

    for (int v = 0; v < uniqueElementOffset; v += 3) {
      DO_VALIDATION;

      Vector3 vertexPos(uniqueMesh.data[v], uniqueMesh.data[v + 1], uniqueMesh.data[v + 2]);

      WeightedVertex weightedVertex;
      weightedVertex.vertexID = v / 3;

      if (colorCoords.find(vertexPos) == colorCoords.end()) {
        DO_VALIDATION;
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
        DO_VALIDATION;
        int jointID = floor(color.coords[c] * 0.1);
        float weight = (color.coords[c] - jointID * 10.0) / 9.0;

        weightedBones[c].jointID = jointID;
        weightedBones[c].weight = weight;

        totalWeight += weight;
      }

      // total weight has to be 1.0;
      for (int c = 0; c < 3; c++) {
        DO_VALIDATION;
        if (c == 0) {
          DO_VALIDATION;
          if (weightedBones[c].weight == 0.f) printf("offending jointID: %i (coord %i) (vertexpos %f, %f, %f)\n", weightedBones[c].jointID, c, vertexPos.coords[0], vertexPos.coords[1], vertexPos.coords[2]);
          assert(weightedBones[c].weight != 0.f);
        }
        if (weightedBones[c].weight > 0.01f) {
          DO_VALIDATION;
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
      DO_VALIDATION;
      materializedTriangleMeshes.at(subgeom).indices.push_back(uniqueIndices[v / 3]);
    }

  }  // subgeom

  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData();

  for (unsigned int i = 0; i < joints.size(); i++) {
    DO_VALIDATION;
    joints[i].orientation = jointsVec[i]->GetDerivedRotation().GetInverse().GetNormalized();
  }

  Animation *straightAnim = new Animation();
  straightAnim->Load("media/animations/straight.anim.util");
  animApplyBuffer.anim = straightAnim;
  animApplyBuffer.anim->Apply(nodeMap, animApplyBuffer.frameNum, 0, animApplyBuffer.smooth, animApplyBuffer.smoothFactor, animApplyBuffer.position, animApplyBuffer.orientation, animApplyBuffer.offsets, 0, false, true);

  for (unsigned int i = 0; i < joints.size(); i++) {
    DO_VALIDATION;
    joints[i].position = jointsVec[i]->GetDerivedPosition();// * zMultiplier;
  }

  UpdateFullbodyModel(true);
  boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->OnUpdateGeometryData(false);

  for (unsigned int i = 0; i < joints.size(); i++) {
    DO_VALIDATION;
    joints[i].origPos = jointsVec[i]->GetDerivedPosition();
  }

  delete straightAnim;
  delete baseAnim;
}

void HumanoidBase::UpdateFullbodyNodes(bool mirror) {
  DO_VALIDATION;
  if (mirror) {
    DO_VALIDATION;
    Mirror();
  }

  Vector3 fullbodyOffset = humanoidNode->GetPosition().Get2D();
  fullbodyNode->SetPosition(fullbodyOffset);

  for (unsigned int i = 0; i < joints.size(); i++) {
    DO_VALIDATION;
    joints[i].orientation = joints[i].node->GetDerivedRotation();
    joints[i].position = joints[i].node->GetDerivedPosition() - fullbodyOffset;
  }
  hairStyle->SetRotation(joints[2].orientation, false);
  hairStyle->SetPosition(joints[2].position * zMultiplier + fullbodyOffset, false);
  hairStyle->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
  if (mirror) {
    DO_VALIDATION;
    Mirror();
  }
}

void HumanoidBase::UpdateFullbodyModel(bool updateSrc) {
  DO_VALIDATION;
  GetTracker()->setDisabled(true);
  if (GetGameConfig().render) {
    boost::intrusive_ptr<Resource<GeometryData> > fullbodyGeometryData =
        boost::static_pointer_cast<Geometry>(
            fullbodyNode->GetObject("fullbody"))
            ->GetGeometryData();
    std::vector<MaterializedTriangleMesh> &materializedTriangleMeshes =
        fullbodyGeometryData->GetResource()->GetTriangleMeshesRef();

    for (unsigned int subgeom = 0; subgeom < fullbodySubgeomCount; subgeom++) {
      DO_VALIDATION;

      FloatArray &uniqueMesh = uniqueFullbodyMesh.at(subgeom);

      const std::vector<WeightedVertex> &weightedVertices =
          weightedVerticesVec.at(subgeom);

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
        memcpy(origVertex.coords,
               &uniqueMesh.data[weightedVertices[v].vertexID * 3],
               3 * sizeof(float));  // was: uniqueFullbodyMeshSrc
        memcpy(
            origNormal.coords,
            &uniqueMesh
                 .data[weightedVertices[v].vertexID * 3 + uniqueElementOffset],
            3 * sizeof(float));
        memcpy(origTangent.coords,
               &uniqueMesh.data[weightedVertices[v].vertexID * 3 +
                                uniqueElementOffset * 3],
               3 * sizeof(float));
        memcpy(origBitangent.coords,
               &uniqueMesh.data[weightedVertices[v].vertexID * 3 +
                                uniqueElementOffset * 4],
               3 * sizeof(float));

        if (weightedVertices[v].bones.size() == 1) {
          DO_VALIDATION;

          resultVertex = origVertex;
          resultVertex -= joints[weightedVertices[v].bones[0].jointID].origPos *
                          zMultiplier;
          resultVertex.Rotate(
              joints[weightedVertices[v].bones[0].jointID].orientation);
          resultVertex +=
              joints[weightedVertices[v].bones[0].jointID].position *
              zMultiplier;

          resultNormal = origNormal;
          resultNormal.Rotate(
              joints[weightedVertices[v].bones[0].jointID].orientation);

          resultTangent = origTangent;
          resultTangent.Rotate(
              joints[weightedVertices[v].bones[0].jointID].orientation);

          resultBitangent = origBitangent;
          resultBitangent.Rotate(
              joints[weightedVertices[v].bones[0].jointID].orientation);

        } else {
          resultVertex.Set(0);
          resultNormal.Set(0);
          resultTangent.Set(0);
          resultBitangent.Set(0);

          for (unsigned int b = 0; b < weightedVertices[v].bones.size(); b++) {
            DO_VALIDATION;

            adaptedVertex = origVertex;
            adaptedVertex -=
                joints[weightedVertices[v].bones[b].jointID].origPos *
                zMultiplier;
            adaptedVertex.Rotate(
                joints[weightedVertices[v].bones[b].jointID].orientation);
            adaptedVertex +=
                joints[weightedVertices[v].bones[b].jointID].position *
                zMultiplier;
            resultVertex += adaptedVertex * weightedVertices[v].bones[b].weight;

            adaptedNormal = origNormal;
            adaptedNormal.Rotate(
                joints[weightedVertices[v].bones[b].jointID].orientation);
            resultNormal += adaptedNormal * weightedVertices[v].bones[b].weight;

            adaptedTangent = origTangent;
            adaptedTangent.Rotate(
                joints[weightedVertices[v].bones[b].jointID].orientation);
            resultTangent +=
                adaptedTangent * weightedVertices[v].bones[b].weight;

            adaptedBitangent = origBitangent;
            adaptedBitangent.Rotate(
                joints[weightedVertices[v].bones[b].jointID].orientation);
            resultBitangent +=
                adaptedBitangent * weightedVertices[v].bones[b].weight;
          }

          resultNormal.FastNormalize();
          resultTangent.FastNormalize();
          resultBitangent.FastNormalize();
        }

        if (updateSrc) {
          DO_VALIDATION;
          memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3],
                 resultVertex.coords, 3 * sizeof(float));
          memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3 +
                                  uniqueElementOffset],
                 resultNormal.coords, 3 * sizeof(float));
          memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3 +
                                  uniqueElementOffset * 3],
                 resultTangent.coords, 3 * sizeof(float));
          memcpy(&uniqueMesh.data[weightedVertices[v].vertexID * 3 +
                                  uniqueElementOffset * 4],
                 resultBitangent.coords, 3 * sizeof(float));
        }

        memcpy(&materializedTriangleMeshes[subgeom]
                    .vertices[weightedVertices[v].vertexID * 3],
               resultVertex.coords, 3 * sizeof(float));
        memcpy(&materializedTriangleMeshes[subgeom]
                    .vertices[weightedVertices[v].vertexID * 3 +
                              uniqueElementOffset],
               resultNormal.coords, 3 * sizeof(float));
        memcpy(&materializedTriangleMeshes[subgeom]
                    .vertices[weightedVertices[v].vertexID * 3 +
                              uniqueElementOffset * 3],
               resultTangent.coords, 3 * sizeof(float));
        memcpy(&materializedTriangleMeshes[subgeom]
                    .vertices[weightedVertices[v].vertexID * 3 +
                              uniqueElementOffset * 4],
               resultBitangent.coords, 3 * sizeof(float));
      }

    }  // subgeom
  }
  GetTracker()->setDisabled(false);
}

void HumanoidBase::Process() {
  DO_VALIDATION;

  decayingPositionOffset *= 0.95f;
  if (decayingPositionOffset.GetLength() < 0.005) decayingPositionOffset.Set(0);
  decayingDifficultyFactor = clamp(decayingDifficultyFactor - 0.002f, 0.0f, 1.0f);


  assert(match);

  CalculateSpatialState();
  spatialState.positionOffsetMovement = Vector3(0);

  currentAnim.frameNum++;
  previousAnim_frameNum++;

  if (currentAnim.frameNum == currentAnim.anim->GetFrameCount() - 1 &&
      interruptAnim == e_InterruptAnim_None) {
    DO_VALIDATION;
    interruptAnim = e_InterruptAnim_Switch;
  }

  bool mayReQueue = false;


  // already some anim interrupt waiting?

  if (mayReQueue) {
    DO_VALIDATION;
    if (interruptAnim != e_InterruptAnim_None) {
      DO_VALIDATION;
      mayReQueue = false;
    }
  }

  // okay, see if we need to requeue

  if (mayReQueue) {
    DO_VALIDATION;
    interruptAnim = e_InterruptAnim_ReQueue;
  }

  if (interruptAnim != e_InterruptAnim_None) {
    DO_VALIDATION;

    PlayerCommandQueue commandQueue;

    if (interruptAnim == e_InterruptAnim_Trip && tripType != 0) {
      DO_VALIDATION;
      AddTripCommandToQueue(commandQueue, tripDirection, tripType);
      tripType = 0;
      commandQueue.push_back(GetBasicMovementCommand(tripDirection, spatialState.floatVelocity)); // backup, if there's no applicable trip anim
    } else {
      player->RequestCommand(commandQueue);
    }

    // iterate through the command queue and pick the first that is applicable

    bool found = false;
    for (unsigned int i = 0; i < commandQueue.size(); i++) {
      DO_VALIDATION;

      const PlayerCommand &command = commandQueue[i];

      found = SelectAnim(command, interruptAnim);
      if (found) break;
    }

    if (interruptAnim != e_InterruptAnim_ReQueue && !found) {
      DO_VALIDATION;
      printf("RED ALERT! NO APPLICABLE ANIM FOUND FOR HUMANOIDBASE! NOOOO!\n");
      printf("currentanimtype: %s\n", currentAnim.anim->GetVariable("type").c_str());
      for (unsigned int i = 0; i < commandQueue.size(); i++) {
        DO_VALIDATION;
        printf("desiredanimtype: %i\n", commandQueue[i].desiredFunctionType);
      }
    }

    if (found) {
      DO_VALIDATION;
      startPos = spatialState.position;
      startAngle = spatialState.angle;

      CalculatePredictedSituation(nextStartPos, nextStartAngle);

      animApplyBuffer.anim = currentAnim.anim;
      animApplyBuffer.smooth = true;
      animApplyBuffer.smoothFactor = (interruptAnim == e_InterruptAnim_Switch) ? 0.6f : 1.0f;

      // decaying difficulty
      float animDiff = atof(currentAnim.anim->GetVariable("animdifficultyfactor").c_str());
      if (animDiff > decayingDifficultyFactor) decayingDifficultyFactor = animDiff;
      // if we just requeued, for example, from movement to ballcontrol, there's no reason we can not immediately requeue to another ballcontrol again (next time). only apply the initial requeue delay on subsequent anims of the same type
      // (so we can have a fast ballcontrol -> ballcontrol requeue, but after that, use the initial delay)
      if (interruptAnim == e_InterruptAnim_ReQueue &&
          previousAnim_functionType == currentAnim.functionType) {
        DO_VALIDATION;
        reQueueDelayFrames = initialReQueueDelayFrames; // don't try requeueing (some types of anims, see selectanim()) too often
      }
    }
  }
  reQueueDelayFrames = clamp(reQueueDelayFrames - 1, 0, 10000);


  interruptAnim = e_InterruptAnim_None;

  if (startPos.coords[2] != 0.f) {
    DO_VALIDATION;
    // the z coordinate not being 0 denotes something went horribly wrong :P
    Log(e_FatalError, "HumanoidBase", "Process", "BWAAAAAH FLYING PLAYERS!! height: " + real_to_str(startPos.coords[2]));
  }

  // movement/rotation smuggle

  // start with +1, because we want to influence the first frame as well
  // as for finishing, finish with frameBias = 1.0, even if the last frame is 'spiritually' the one-to-last, since the first frame of the next anim is actually 'same-tempered' as the current anim's last frame.
  // however, it works best to have all values 'done' at this one-to-last frame, so the next anim can read out these correct (new starting) values.
  float frameBias = (currentAnim.frameNum + 1) / (float)(currentAnim.anim->GetEffectiveFrameCount() + 1);

  // not sure if this is correct!
  // radian beginAngle = currentAnim.rotationSmuggle.begin;// * (1.0f - frameBias); // more influence in the beginning; act more like it was 0 later on. (yes, next one is a bias within a bias) *edit: disabled, looks better without
  // currentAnim.rotationSmuggleOffset = beginAngle * (1.0f - frameBias) +
  //                                      currentAnim.rotationSmuggle.end * frameBias;

  currentAnim.rotationSmuggleOffset = currentAnim.rotationSmuggle.begin * (1.0f - frameBias) +
                                       currentAnim.rotationSmuggle.end * frameBias;


  // next frame

  animApplyBuffer.frameNum = currentAnim.frameNum;

  if (currentAnim.positions.size() > (unsigned int)currentAnim.frameNum) {
    DO_VALIDATION;
    animApplyBuffer.position = startPos + currentAnim.actionSmuggleOffset + currentAnim.actionSmuggleSustainOffset + currentAnim.movementSmuggleOffset + currentAnim.positions.at(currentAnim.frameNum);
    animApplyBuffer.orientation = startAngle + currentAnim.rotationSmuggleOffset;
    animApplyBuffer.noPos = true;
  } else {
    animApplyBuffer.position = startPos + currentAnim.actionSmuggleOffset + currentAnim.actionSmuggleSustainOffset + currentAnim.movementSmuggleOffset;
    animApplyBuffer.orientation = startAngle;
    animApplyBuffer.noPos = false;
  }

  animApplyBuffer.offsets = offsets;
}

void HumanoidBase::PreparePutBuffers() {
  DO_VALIDATION;
  CalculateGeomOffsets();
}

void HumanoidBase::FetchPutBuffers() {
  DO_VALIDATION;
  animApplyBuffer.anim->Apply(nodeMap, animApplyBuffer.frameNum, -1, animApplyBuffer.smooth, animApplyBuffer.smoothFactor, animApplyBuffer.position, animApplyBuffer.orientation, animApplyBuffer.offsets, &movementHistory, 10, animApplyBuffer.noPos, false);
  humanoidNode->RecursiveUpdateSpatialData(e_SpatialDataType_Both);
}

void HumanoidBase::Put(bool mirror) {
  DO_VALIDATION;
  UpdateFullbodyNodes(mirror);
}

void HumanoidBase::CalculateGeomOffsets() { DO_VALIDATION; }

void HumanoidBase::SetOffset(BodyPart body_part, float bias,
                             const Quaternion &orientation, bool isRelative) {
  DO_VALIDATION;
  BiasedOffset& biasedOffset = offsets[body_part];
  biasedOffset.bias = bias;
  biasedOffset.orientation = orientation;
}

int HumanoidBase::GetIdleMovementAnimID() {
  DO_VALIDATION;
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

void HumanoidBase::ResetPosition(const Vector3 &newPos,
                                 const Vector3 &focusPos) {
  DO_VALIDATION;

  startPos = newPos;
  startAngle = FixAngle((focusPos - newPos).GetNormalized(Vector3(0, -1, 0)).GetAngle2D());
  nextStartPos = startPos;
  nextStartAngle = startAngle;
  previousPosition2D = startPos;

  spatialState.position = startPos;
  spatialState.angle = startAngle;
  spatialState.directionVec = Vector3(0, -1, 0).GetRotated2D(startAngle);
  spatialState.floatVelocity = 0;

  spatialState.actualMovement = Vector(0);
  spatialState.physicsMovement = Vector(0);
  spatialState.animMovement = Vector(0);
  spatialState.movement = Vector(0);
  spatialState.actionSmuggleMovement = Vector(0);
  spatialState.movementSmuggleMovement = Vector(0);
  spatialState.positionOffsetMovement = Vector(0);
  spatialState.relBodyAngleNonquantized = 0;

  spatialState.enumVelocity = e_Velocity_Idle;
  spatialState.floatVelocity = 0;
  spatialState.movement = Vector3(0);
  spatialState.relBodyDirectionVec = Vector3(0, -1, 0);
  spatialState.relBodyAngle = 0;
  spatialState.bodyDirectionVec = Vector3(0, -1, 0);
  spatialState.bodyAngle = 0;
  spatialState.foot = e_Foot_Right;

  int idleAnimID = GetIdleMovementAnimID();
  currentAnim.id = idleAnimID;
  currentAnim.anim = anims->GetAnim(currentAnim.id);
  currentAnim.positions.clear();
  currentAnim.positions = match->GetAnimPositionCache(currentAnim.anim);
  currentAnim.frameNum =
      boostrandom(0, currentAnim.anim->GetEffectiveFrameCount() - 1);
  currentAnim.touchFrame = -1;
  currentAnim.originatingInterrupt = e_InterruptAnim_None;
  currentAnim.actionSmuggle = Vector3(0);
  currentAnim.actionSmuggleOffset = Vector3(0);
  currentAnim.actionSmuggleSustain = Vector3(0);
  currentAnim.actionSmuggleSustainOffset = Vector3(0);
  currentAnim.movementSmuggle = Vector3(0);
  currentAnim.movementSmuggleOffset = Vector3(0);
  currentAnim.rotationSmuggle.begin = 0;
  currentAnim.rotationSmuggle.end = 0;
  currentAnim.rotationSmuggleOffset = 0;
  currentAnim.functionType = e_FunctionType_Movement;
  currentAnim.incomingMovement = Vector3(0);
  currentAnim.outgoingMovement = Vector3(0);
  currentAnim.positionOffset = Vector3(0);

  previousAnim_frameNum = 0;
  previousAnim_functionType = e_FunctionType_Movement;

  humanoidNode->SetPosition(startPos, false);

  animApplyBuffer.anim = currentAnim.anim;
  animApplyBuffer.smooth = false;
  animApplyBuffer.smoothFactor = 0.0f;
  animApplyBuffer.position = startPos;
  animApplyBuffer.orientation = startAngle;
  animApplyBuffer.offsets.clear();

  interruptAnim = e_InterruptAnim_None;
  tripType = 0;

  decayingPositionOffset = Vector3(0);
  decayingDifficultyFactor = 0.0f;

  movementHistory.clear();
  reQueueDelayFrames = 0;
  tripDirection = Vector3(0);
  for (int x = 0; x < body_part_max - 1; x++) {
    nodeMap[x]->SetRotation(Quaternion(), true);
  }
}

void HumanoidBase::OffsetPosition(const Vector3 &offset) {
  DO_VALIDATION;


  assert(offset.coords[2] == 0.0f);

  nextStartPos += offset;
  startPos += offset;
  spatialState.position += offset;
  spatialState.positionOffsetMovement += offset * 100.0f;
  decayingPositionOffset += offset;
  if (decayingPositionOffset.GetLength() > 0.1f) decayingPositionOffset = decayingPositionOffset.GetNormalized() * 0.1f;
  currentAnim.positionOffset += offset;
}

void HumanoidBase::TripMe(const Vector3 &tripVector, int tripType) {
  DO_VALIDATION;
  if (match->GetBallRetainer() == player) return;
  if (currentAnim.anim->GetVariableCache().incoming_special_state().compare(
          "") == 0 &&
      currentAnim.anim->GetVariableCache().outgoing_special_state().compare(
          "") == 0) {
    DO_VALIDATION;
    if (this->interruptAnim == e_InterruptAnim_None &&
        (currentAnim.functionType != e_FunctionType_Trip ||
         (currentAnim.anim->GetVariable("triptype").compare("1") == 0 &&
          tripType > 1)) &&
        currentAnim.functionType != e_FunctionType_Sliding) {
      DO_VALIDATION;
      this->interruptAnim = e_InterruptAnim_Trip;
      this->tripDirection = tripVector;
      this->tripType = tripType;
    }
  }
}

void HumanoidBase::SetKit(boost::intrusive_ptr<Resource<Surface> > newKit) {
  DO_VALIDATION;
  boost::intrusive_ptr< Resource<GeometryData> > bodyGeom = boost::static_pointer_cast<Geometry>(fullbodyNode->GetObject("fullbody"))->GetGeometryData();

  std::vector < MaterializedTriangleMesh > &tmesh = bodyGeom->GetResource()->GetTriangleMeshesRef();

  if (newKit != boost::intrusive_ptr<Resource<Surface> >()) {
    DO_VALIDATION;
    for (unsigned int i = 0; i < tmesh.size(); i++) {
      DO_VALIDATION;
      if (tmesh[i].material.diffuseTexture !=
          boost::intrusive_ptr<Resource<Surface> >()) {
        DO_VALIDATION;
        if (tmesh[i].material.diffuseTexture->GetIdentString() ==
            kitDiffuseTextureIdentString) {
          DO_VALIDATION;
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
  DO_VALIDATION;
  mentalImageTime = 0;
  ResetPosition(spatialState.position, focusPos);
}

bool HumanoidBase::_HighOrBouncyBall() const {
  float ballHeight1 = match->GetBall()->Predict(10).coords[2];
  float ballHeight2 = match->GetBall()->Predict(defaultTouchOffset_ms).coords[2];
  float ballBounce = fabs(match->GetBall()->GetMovement().coords[2]);
  bool highBall = false;
  if (ballHeight1 > 0.3f || ballHeight2 > 0.3f) {
    DO_VALIDATION;
    highBall = true;
  } else if (ballBounce > 1.0f) {
    DO_VALIDATION;  // low balls are also treated as 'high ball' when there's a
                    // lot of bounce going on (hard to control)
    highBall = true;
  }
  return highBall;
}

// ALERT: set sorting predicates before calling this function
void HumanoidBase::_KeepBestDirectionAnims(DataSet &dataSet,
                                           const PlayerCommand &command,
                                           bool strict, radian allowedAngle,
                                           int allowedVelocitySteps,
                                           int forcedQuadrantID) {
  DO_VALIDATION;

  assert(dataSet.size() != 0);

  int bestQuadrantID = forcedQuadrantID;
  if (bestQuadrantID == -1) {
    DO_VALIDATION;

    for (auto &anim : dataSet) {
      DO_VALIDATION;
      anims->GetAnim(anim)->order_float = GetMovementSimilarity(anim, predicate_RelDesiredDirection, predicate_DesiredVelocity, predicate_CorneringBias);
    }
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareByOrderFloat, this, _1, _2));

    // we want the best anim to be a baseanim, and compare other anims to it
    if (strict) {
      DO_VALIDATION;
      if (command.desiredFunctionType != e_FunctionType_Movement) {
        DO_VALIDATION;
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
    DO_VALIDATION;
    Animation *anim = anims->GetAnim(*iter);

    if (strict) {
      DO_VALIDATION;
      if (anim->GetVariableCache().quadrant_id() == bestQuadrantID) {
        DO_VALIDATION;
        iter++;
      } else {
        iter = dataSet.erase(iter);
      }
    } else {
      int quadrantID = anim->GetVariableCache().quadrant_id();
      const Quadrant &bestQuadrant = anims->GetQuadrant(bestQuadrantID);
      const Quadrant &quadrant = anims->GetQuadrant(quadrantID);

      bool predicate = true;

      if (!anim->GetVariableCache().lastditch()) {
        DO_VALIDATION;  // last ditch anims may always change velo
        if (abs(GetVelocityID(quadrant.velocity, true) - GetVelocityID(bestQuadrant.velocity, true)) > allowedVelocitySteps) predicate = false;
      }
      if (fabs(quadrant.angle - bestQuadrant.angle) > allowedAngle) predicate = false;

      if (predicate) {
        DO_VALIDATION;
        iter++;
      } else {
        iter = dataSet.erase(iter);
      }
    }
  }
}

// ALERT: set sorting predicates before calling this function
void HumanoidBase::_KeepBestBodyDirectionAnims(DataSet &dataSet,
                                               const PlayerCommand &command,
                                               bool strict,
                                               radian allowedAngle) {
  DO_VALIDATION;

  // delete nonqualified bodydir quadrants

  assert(dataSet.size() != 0);
  for (auto &anim : dataSet) {
    DO_VALIDATION;
    anims->GetAnim(anim)->order_float = DirectionSimilarityRating(anim);
  }
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareByOrderFloat, this, _1, _2));

  // we want the best anim to be a baseanim, and compare other anims to it
  if (strict) {
    DO_VALIDATION;
    if (command.desiredFunctionType != e_FunctionType_Movement) {
      DO_VALIDATION;
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
    DO_VALIDATION;

    Animation *anim = anims->GetAnim(*iter);

    radian animOutgoingBodyAngle = ForceIntoAllowedBodyDirectionAngle(anim->GetOutgoingBodyAngle());
    radian animOutgoingAngle = ForceIntoPreferredDirectionAngle(anim->GetOutgoingAngle());
    radian animLookAngle = animOutgoingBodyAngle + animOutgoingAngle;

    float adaptedAllowedAngle = 0.06f * pi; // between 0 and 20 deg
    if (!strict) {
      DO_VALIDATION;
      adaptedAllowedAngle = allowedAngle;
    }
    if (fabs(animLookAngle - bestLookAngle) <= adaptedAllowedAngle) {
      DO_VALIDATION;
      iter++;
    } else {
      iter = dataSet.erase(iter);
    }
  }
}

bool HumanoidBase::SelectAnim(const PlayerCommand &command,
                              e_InterruptAnim localInterruptAnim,
                              bool preferPassAndShot) {
  DO_VALIDATION;  // returns false on no applicable anim found
  assert(command.desiredDirection.coords[2] == 0.0f);

  if (localInterruptAnim != e_InterruptAnim_ReQueue || currentAnim.frameNum > 12) CalculateFactualSpatialState();


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
    DO_VALIDATION;
    query.byTripType = true;
    query.tripType = command.tripType;
  }
  query.properties.set("incoming_special_state", currentAnim.anim->GetVariableCache().outgoing_special_state());
  if (match->GetBallRetainer() == player) query.properties.set("incoming_retain_state", currentAnim.anim->GetVariable("outgoing_retain_state"));
  if (command.useSpecialVar1) query.properties.set_specialvar1(command.specialVar1);
  if (command.useSpecialVar2) query.properties.set_specialvar2(command.specialVar2);

  if (!currentAnim.anim->GetVariableCache().outgoing_special_state().empty()) query.incomingVelocity = e_Velocity_Idle; // standing up anims always start out idle

  DataSet dataSet;
  anims->CrudeSelection(dataSet, query);
  if (dataSet.size() == 0) {
    DO_VALIDATION;
    if (command.desiredFunctionType == e_FunctionType_Movement) {
      DO_VALIDATION;
      dataSet.push_back(GetIdleMovementAnimID()); // do with idle anim (should not happen too often, only after weird bumps when there's for example a need for a sprint anim at an impossible body angle, after a trip of whatever)
    } else
      return false;
  }

  // NOW SORT OUT THE RESULTING SET

  float adaptedDesiredVelocityFloat = command.desiredVelocityFloat;

  if (command.useDesiredMovement) {
    DO_VALIDATION;

    Vector3 relDesiredDirection = command.desiredDirection.GetRotated2D(-spatialState.angle);
    SetMovementSimilarityPredicate(relDesiredDirection, FloatToEnumVelocity(adaptedDesiredVelocityFloat));
    SetBodyDirectionSimilarityPredicate(command.desiredLookAt);
    if (command.desiredFunctionType == e_FunctionType_Movement) {
      DO_VALIDATION;
      _KeepBestDirectionAnims(dataSet, command);
      if (command.useDesiredLookAt) _KeepBestBodyDirectionAnims(dataSet, command);
    }

    else {  // undefined animtype
      std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareMovementSimilarity, this, _1, _2));
    }
  }

  int desiredIdleLevel = 1;
  SetIdlePredicate(desiredIdleLevel);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&Humanoid::CompareIdleVariable, this, _1, _2));

  SetFootSimilarityPredicate(spatialState.foot);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareFootSimilarity, this, spatialState.foot, _1, _2));

  SetIncomingBodyDirectionSimilarityPredicate(spatialState.relBodyDirectionVec);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareIncomingBodyDirectionSimilarity, this, _1, _2));

  SetIncomingVelocitySimilarityPredicate(spatialState.enumVelocity);
  std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareIncomingVelocitySimilarity, this, _1, _2));

  if (command.useDesiredTripDirection) {
    DO_VALIDATION;
    Vector3 relDesiredTripDirection = command.desiredTripDirection.GetRotated2D(-spatialState.angle);
    SetTripDirectionSimilarityPredicate(relDesiredTripDirection);
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareTripDirectionSimilarity, this, _1, _2));
  }

  if (command.desiredFunctionType != e_FunctionType_Movement) {
    DO_VALIDATION;
    std::stable_sort(dataSet.begin(), dataSet.end(), boost::bind(&HumanoidBase::CompareBaseanimSimilarity, this, _1, _2));
  }

  // process result

  int selectedAnimID = -1;
  std::vector<Vector3> positions_tmp;
  int touchFrame_tmp = -1;
  Vector3 touchPos_tmp;
  Vector3 actionSmuggle_tmp;
  radian rotationSmuggle_tmp = 0;

  if (dataSet.size() == 0) {
    DO_VALIDATION;
    return false;
  }
  if (command.desiredFunctionType == e_FunctionType_Movement ||
      command.desiredFunctionType == e_FunctionType_Trip ||
      command.desiredFunctionType == e_FunctionType_Special) {
    DO_VALIDATION;
    selectedAnimID = *dataSet.begin();
    Animation *nextAnim = anims->GetAnim(selectedAnimID);
    Vector3 desiredMovement = command.desiredDirection * command.desiredVelocityFloat;
    assert(desiredMovement.coords[2] == 0.0f);
    Vector3 desiredBodyDirectionRel = Vector3(0, -1, 0);
    if (command.useDesiredLookAt) desiredBodyDirectionRel = ((command.desiredLookAt - spatialState.position).Get2D().GetRotated2D(-spatialState.angle) - nextAnim->GetTranslation()).GetNormalized(Vector3(0, -1, 0));
    Vector3 physicsVector = CalculatePhysicsVector(nextAnim, command.useDesiredMovement, desiredMovement, command.useDesiredLookAt, desiredBodyDirectionRel, positions_tmp, rotationSmuggle_tmp);
  }

  // check if we really want to requeue - only requeue movement to movement, for example, when we want to go a different direction

  if (localInterruptAnim == e_InterruptAnim_ReQueue && selectedAnimID != -1 &&
      currentAnim.positions.size() > 1 && positions_tmp.size() > 1) {
    DO_VALIDATION;

    // don't requeue to same quadrant
    if (currentAnim.functionType == command.desiredFunctionType &&

        ((FloatToEnumVelocity(currentAnim.anim->GetOutgoingVelocity()) !=
              e_Velocity_Idle &&
          currentAnim.anim->GetVariableCache().quadrant_id() ==
              anims->GetAnim(selectedAnimID)
                  ->GetVariableCache()
                  .quadrant_id()) ||
         (FloatToEnumVelocity(currentAnim.anim->GetOutgoingVelocity()) ==
              e_Velocity_Idle &&
          fabs((ForceIntoPreferredDirectionAngle(
                    currentAnim.anim->GetOutgoingAngle()) -
                ForceIntoPreferredDirectionAngle(
                    anims->GetAnim(selectedAnimID)->GetOutgoingAngle()))) <
              0.20f * pi))) {
      DO_VALIDATION;

      selectedAnimID = -1;
    }
  }

  // make it so

  if (selectedAnimID != -1) {
    DO_VALIDATION;
    previousAnim_frameNum = currentAnim.frameNum;
    previousAnim_functionType = currentAnim.functionType;
    currentAnim.anim = anims->GetAnim(selectedAnimID);
    currentAnim.id = selectedAnimID;
    currentAnim.functionType = command.desiredFunctionType;
    currentAnim.frameNum = 0;
    currentAnim.touchFrame = touchFrame_tmp;
    currentAnim.originatingInterrupt = localInterruptAnim;
    currentAnim.touchPos = touchPos_tmp;
    currentAnim.rotationSmuggle.begin = clamp(ModulateIntoRange(-pi, pi, spatialState.relBodyAngleNonquantized - currentAnim.anim->GetIncomingBodyAngle()) * bodyRotationSmoothingFactor, -bodyRotationSmoothingMaxAngle, bodyRotationSmoothingMaxAngle);
    currentAnim.rotationSmuggle.end = rotationSmuggle_tmp;
    currentAnim.rotationSmuggleOffset = 0;
    currentAnim.actionSmuggle = actionSmuggle_tmp;
    currentAnim.actionSmuggleOffset = Vector3(0);
    currentAnim.actionSmuggleSustain = Vector3(0);
    currentAnim.actionSmuggleSustainOffset = Vector3(0);
    currentAnim.movementSmuggle = Vector3(0);
    currentAnim.movementSmuggleOffset = Vector3(0);
    currentAnim.incomingMovement = spatialState.movement;
    currentAnim.outgoingMovement = CalculateOutgoingMovement(positions_tmp);
    currentAnim.positions.clear();
    currentAnim.positions.assign(positions_tmp.begin(), positions_tmp.end());
    currentAnim.positionOffset = 0.0;
    currentAnim.originatingCommand = command;

    return true;
  }

  return false;
}

void HumanoidBase::CalculatePredictedSituation(Vector3 &predictedPos,
                                               radian &predictedAngle) {
  DO_VALIDATION;

  if (currentAnim.positions.size() > (unsigned int)currentAnim.frameNum) {
    DO_VALIDATION;
    assert(currentAnim.positions.size() > (unsigned int)currentAnim.anim->GetEffectiveFrameCount());
    predictedPos = spatialState.position + currentAnim.positions.at(currentAnim.anim->GetEffectiveFrameCount()) + currentAnim.actionSmuggle + currentAnim.actionSmuggleSustain + currentAnim.movementSmuggle;
  } else {
    predictedPos = spatialState.position + currentAnim.anim->GetTranslation().Get2D().GetRotated2D(spatialState.angle) + currentAnim.actionSmuggle + currentAnim.actionSmuggleSustain + currentAnim.movementSmuggle;
  }

  predictedAngle = spatialState.angle + currentAnim.anim->GetOutgoingAngle() + currentAnim.rotationSmuggle.end;
  predictedAngle = ModulateIntoRange(-pi, pi, predictedAngle);
  assert(predictedPos.coords[2] == 0.0f);
}

Vector3 HumanoidBase::CalculateOutgoingMovement(const std::vector<Vector3> &positions) const {
  if (positions.size() < 2) return 0;
  return (positions.at(positions.size() - 1) - positions.at(positions.size() - 2)) * 100.0f;
}

void HumanoidBase::CalculateSpatialState() {
  DO_VALIDATION;
  Vector3 position;
  if (currentAnim.positions.size() > (unsigned int)currentAnim.frameNum) {
    DO_VALIDATION;
    position = startPos + currentAnim.positions.at(currentAnim.frameNum) + currentAnim.actionSmuggleOffset + currentAnim.actionSmuggleSustainOffset + currentAnim.movementSmuggleOffset;
  } else {
    Quaternion orientation;
    currentAnim.anim->GetKeyFrame(BodyPart::player, currentAnim.frameNum, orientation, position);
    position.coords[2] = 0.0f;
    position = startPos + position.GetRotated2D(startAngle) + currentAnim.actionSmuggleOffset + currentAnim.actionSmuggleSustainOffset + currentAnim.movementSmuggleOffset;
  }

  if (currentAnim.frameNum > 12) {
    DO_VALIDATION;
    spatialState.foot = currentAnim.anim->GetOutgoingFoot();
  }

  assert(startPos.coords[2] == 0.0f);
  assert(currentAnim.actionSmuggleOffset.coords[2] == 0.0f);
  assert(currentAnim.movementSmuggleOffset.coords[2] == 0.0f);
  assert(position.coords[2] == 0.0f);

  spatialState.actualMovement = (position - previousPosition2D) * 100.0f;
  const float positionOffsetMovementIgnoreFactor = 0.5f;
  spatialState.physicsMovement = spatialState.actualMovement - (spatialState.actionSmuggleMovement) - (spatialState.movementSmuggleMovement) - (spatialState.positionOffsetMovement * positionOffsetMovementIgnoreFactor);
  spatialState.animMovement = spatialState.physicsMovement;
  if (currentAnim.positions.size() > 0) {
    DO_VALIDATION;
    // this way, action cheating is being omitted from the current movement, making for better requeues. however, keep in mind that
    // movementoffsets, from bumping into other players, for example, will also be ignored this way.
    const std::vector<Vector3> &origPositionCache = match->GetAnimPositionCache(currentAnim.anim);
    spatialState.animMovement = CalculateMovementAtFrame(origPositionCache, currentAnim.frameNum, 1).GetRotated2D(startAngle);
  }
  spatialState.movement = spatialState.physicsMovement; // PICK DEFAULT


  Vector3 bodyPosition;
  Quaternion bodyOrientation;
  currentAnim.anim->GetKeyFrame(body, currentAnim.frameNum, bodyOrientation, bodyPosition);
  real x, y, z;
  bodyOrientation.GetAngles(x, y, z);

  Vector3 quatDirection; quatDirection = bodyOrientation;

  Vector3 bodyDirectionVec = Vector3(0, -1, 0).GetRotated2D(z + startAngle + currentAnim.rotationSmuggleOffset);

  spatialState.floatVelocity = spatialState.movement.GetLength();
  spatialState.enumVelocity = FloatToEnumVelocity(spatialState.floatVelocity);

  if (spatialState.enumVelocity != e_Velocity_Idle) {
    DO_VALIDATION;
    spatialState.directionVec = spatialState.movement.GetNormalized();
  } else {
    // too slow for comfort, use body direction as global direction
    spatialState.directionVec = bodyDirectionVec;
  }

  spatialState.position = position;
  spatialState.angle = ModulateIntoRange(-pi, pi, FixAngle(spatialState.directionVec.GetAngle2D()));

  if (spatialState.enumVelocity != e_Velocity_Idle) {
    DO_VALIDATION;
    Vector3 adaptedBodyDirectionVec = bodyDirectionVec.GetRotated2D(-spatialState.angle);
    // prefer straight forward, so lie about the actual direction a bit
    // this may fix bugs of body dir being non-0 somewhere during 0 anims
    // but it may also cause other bugs (going 0 to 45 all over again each time)
    //adaptedBodyDirectionVec = (adaptedBodyDirectionVec * 0.95f + Vector3(0, -1, 0) * 0.05f).GetNormalized(0);
    //ForceIntoAllowedBodyDirectionVec(Vector3(0, -1, 0)).Print();

    bool preferCorrectVeloOverCorrectAngle = true;
    radian bodyAngleRel = adaptedBodyDirectionVec.GetAngle2D(Vector3(0, -1, 0));
    if (spatialState.enumVelocity == e_Velocity_Sprint &&
        fabs(bodyAngleRel) >= 0.125f * pi) {
      DO_VALIDATION;
      if (preferCorrectVeloOverCorrectAngle) {
        DO_VALIDATION;
        // on impossible combinations of velocity and body angle, decrease body angle
        adaptedBodyDirectionVec = Vector3(0, -1, 0).GetRotated2D(0.12f * pi * signSide(bodyAngleRel));
      } else {
        // on impossible combinations of velocity and body angle, decrease velocity
        spatialState.floatVelocity = walkSprintSwitch - 0.1f;
        spatialState.enumVelocity = FloatToEnumVelocity(spatialState.floatVelocity);
      }
    } else if (spatialState.enumVelocity == e_Velocity_Walk &&
               fabs(bodyAngleRel) >= 0.5f * pi) {
      DO_VALIDATION;
      if (preferCorrectVeloOverCorrectAngle) {
        DO_VALIDATION;
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
  DO_VALIDATION;

  spatialState.foot = currentAnim.anim->GetOutgoingFoot();

  if (!currentAnim.anim->GetVariableCache().outgoing_special_state().empty()) {
    DO_VALIDATION;
    spatialState.floatVelocity = 0;
    spatialState.enumVelocity = e_Velocity_Idle;
    spatialState.movement = Vector3(0);
  }
}

void HumanoidBase::AddTripCommandToQueue(PlayerCommandQueue &commandQueue,
                                         const Vector3 &tripVector,
                                         int tripType) {
  DO_VALIDATION;
  if (tripType == 1) {
    DO_VALIDATION;
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

PlayerCommand HumanoidBase::GetTripCommand(const Vector3 &tripVector,
                                           int tripType) {
  DO_VALIDATION;
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

PlayerCommand HumanoidBase::GetBasicMovementCommand(
    const Vector3 &desiredDirection, float velocityFloat) {
  DO_VALIDATION;
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

bool HumanoidBase::CompareFootSimilarity(e_Foot foot, int animIndex1, int animIndex2) const {
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
    DO_VALIDATION;
    outgoingSwitchBias = 0.0f;
  } else if (animType == e_DefString_Trap) {
    DO_VALIDATION;
    outgoingSwitchBias = 0.0f;
  } else if (animType == e_DefString_Interfere) {
    DO_VALIDATION;
    outgoingSwitchBias = 0.0f;
  } else if (animType == e_DefString_Deflect) {
    DO_VALIDATION;
    outgoingSwitchBias = 1.0f;
  } else if (animType == e_DefString_Sliding) {
    DO_VALIDATION;
    outgoingSwitchBias = 0.0f;
  } else if (animType == e_DefString_Special) {
    DO_VALIDATION;
    outgoingSwitchBias = 1.0f;
  } else if (animType == e_DefString_Trip) {
    DO_VALIDATION;
    outgoingSwitchBias = 0.5f; // direction partly predecided by collision function in match class
  } else if (touch) {
    DO_VALIDATION;
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
    DO_VALIDATION;
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
  if (animType == e_DefString_Sliding) {
    DO_VALIDATION;
    maxAngleMod_underAnimAngle = 0.5f * pi;
    maxAngleMod_overAnimAngle = 0.5f * pi;
    maxAngleMod_straightAnimAngle = 0.5f * pi;
  }

  if (animType == e_DefString_Movement)    { physicsBias *= 1.0f; }

  if (animType == e_DefString_BallControl) {
    DO_VALIDATION;
    physicsBias *= 1.0f;
  }
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

  float accelerationMultiplier = 0.5f + _default_AccelerationFactor;

  // rotate anim towards desired angle

  radian toDesiredAngle_capped = 0;
  if (mod_AllowRotation && physicsBias > 0.0f) {
    DO_VALIDATION;
    Vector3 animOutgoingVector = predictedOutgoingMovement.GetNormalized(0);
    if (FloatToEnumVelocity(predictedOutgoingMovement.GetLength()) == e_Velocity_Idle) animOutgoingVector = anim->GetOutgoingDirection().GetRotated2D(spatialState.angle);
    Vector3 desiredVector = adaptedDesiredMovement.GetNormalized(0);
    if (FloatToEnumVelocity(adaptedDesiredMovement.GetLength()) == e_Velocity_Idle) desiredVector = desiredBodyDirectionRel.GetRotated2D(spatialState.angle);
    radian toDesiredAngle = desiredVector.GetAngle2D(animOutgoingVector);
    if (fabs(toDesiredAngle) <= 0.5f * pi || animType == e_DefString_Sliding) {
      DO_VALIDATION;  // if we want > x degrees, just skip it to next anim,
                      // it'll only look weird otherwise

      radian animChange = animOutgoingVector.GetAngle2D(spatialState.directionVec);
      if (fabs(animChange) > 0.06f * pi) {
        DO_VALIDATION;
        int sign = signSide(animChange);
        if (signSide(toDesiredAngle) == sign) {
          DO_VALIDATION;
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
    DO_VALIDATION;

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
  for (int time_ms = 0; time_ms < anim->GetFrameCount() * 10;
       time_ms += timeStep_ms) {
    DO_VALIDATION;

    // start with +1, because we want to influence the first frame as well
    // as for finishing, finish with frameBias = 1.0, even if the last frame is 'spiritually' the one-to-last, since the first frame of the next anim is actually 'same-tempered' as the current anim's last frame.
    // however, it works best to have all values 'done' at this one-to-last frame, so the next anim can read out these correct (new starting) values.
    float frameBias = (time_ms + 10) / (float)((anim->GetEffectiveFrameCount() + 1) * 10);

    float lagExp = 1.0f;
    if (mod_PointinessCurve && physicsBias > 0.0f &&
        (animType == e_DefString_BallControl ||
         animType == e_DefString_Movement)) {
      DO_VALIDATION;
      lagExp = 1.4f - _default_AgilityFactor * 0.8f;
      lagExp *= 1.2f - stat_agility * 0.4f;
      if (touch) {
        DO_VALIDATION;
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

    if (animVelo > walkSprintSwitch &&
        (animType == e_DefString_Movement ||
         animType == e_DefString_BallControl || animType == e_DefString_Trap)) {
      DO_VALIDATION;

      if (maxVelocity > animVelo) {
        DO_VALIDATION;  // only speed up, don't slow down. may be faster parts
                        // (jumps and such) within anim, allow this
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
      DO_VALIDATION;
      float frameBiasedMaximumOutgoingVelocity = sprintVelocity * (1.0f - frameBias) + maximumOutgoingVelocity * frameBias;
      if (adaptedAnimVelo > frameBiasedMaximumOutgoingVelocity) {
        DO_VALIDATION;
        adaptedAnimVelo = frameBiasedMaximumOutgoingVelocity;
        adaptedAnimMovement = adaptedAnimMovement.GetNormalized(0) * adaptedAnimVelo;
      }
    }

    if (mod_MaximumAccelDecel) {
      DO_VALIDATION;
      // this is basically meant to enforce transitions to be smoother, disallowing bizarre steps. however, with low enough max values, it can also serve as a physics slowness thing.
      // in that regard, the maxaccel part is somewhat similar to the air resistance mod below. they can live together; this one can serve as a constant maximum, and the air resistance as a velocity-based maximum.
      // update: decided to not let them live together, this should now purely be used for capping transition speed
      float maxAccelMPS = 20.0f;
      float maxDecelMPS = 20.0f;
      float currentVelo = temporalMovement.GetLength();
      float veloChangeMPS = (adaptedAnimVelo - currentVelo) / ((float)timeStep_ms * 0.001f);
      if (veloChangeMPS < -maxDecelMPS || veloChangeMPS > maxAccelMPS) {
        DO_VALIDATION;
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
      DO_VALIDATION;
      // slow down after touching ball
      // (precalc at touchframe, because temporalMovement will change because of this, so if we don't precalc then changing numBrakeFrames will change the amount of effect)
      int numBrakeFrames = 15;
      if (touch && time_ms >= animTouchFrame * 10 &&
          time_ms < (animTouchFrame + numBrakeFrames) * 10) {
        DO_VALIDATION;
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
    // increase over multiple frames for smoother effect, and so we get a proper
    effect even if maxChange isn't very high int numBrakeFrames = 5; if (touch
    && time_ms >= animTouchFrame * 10 && time_ms < (animTouchFrame +
    numBrakeFrames) * 10) { DO_VALIDATION; int framesInto = (time_ms -
    (animTouchFrame * 10)) / 10; toDesired += -temporalMovement.GetNormalized(0)
    * (ballTouchSlowdownAmount / (float)(numBrakeFrames - framesInto));
    }
    */

    if (mod_MaxCornering) {
      DO_VALIDATION;
      Vector3 predictedMovement = temporalMovement + toDesired;
      float startVelo = idleDribbleSwitch;
      if (temporalMovement.GetLength() > startVelo &&
          predictedMovement.GetLength() > startVelo) {
        DO_VALIDATION;
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
          DO_VALIDATION;

          int mode = 1; // 0: restrict max angle, 1: restrict velocity

          if (mode == 0) {
            DO_VALIDATION;
            Vector3 restrictedPredictedMovement = predictedMovement.GetRotated2D((fabs(angle) - maxAngle) * -signSide(angle));
            Vector3 newToDesired = restrictedPredictedMovement - temporalMovement;
            toDesired = newToDesired;
          } else if (mode == 1) {
            DO_VALIDATION;
            radian overAngle = fabs(angle) - maxAngle; // > 0
            toDesired += -temporalMovement * clamp(overAngle / pi * 3.0f, 0.0f, 1.0f);// was: 3
          }
        }
      }
    }

    if (mod_MaxChange) {
      DO_VALIDATION;
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

      maxChange *= 0.75f + _default_AgilityFactor * 0.5f;

      // lose power 'around' touch

      // if (touch && time_ms >= animTouchFrame * 10) { DO_VALIDATION;
      //   int influenceFrames = 16; // number of frames to slow down on each
      //   'side' of the balltouch
      //   //float frameBias = NormalizedClamp(fabs((animTouchFrame * 10) -
      //   time_ms), 0, influenceFrames * 10) * 0.7f + 0.3f; float frameBias =
      //   NormalizedClamp(time_ms - (animTouchFrame * 10), 0, influenceFrames *
      //   10) * 1.0f;// + 0.1f; frameBias = curve(frameBias, 1.0f); maxChange
      //   *= clamp(frameBias, 0.1f, 1.0f);
      // }

      maxChange *= powerFactor;

      float desiredLength = toDesired.GetLength();
      float maxAddition = maxChange * timeStep_ms;

      toDesired.NormalizeMax(std::min(desiredLength, maxAddition));
    }

    // air resistance

    if (mod_AirResistance && animType != e_DefString_Sliding &&
        animType != e_DefString_Deflect) {
      DO_VALIDATION;
      float veloExp = 1.8f;
      float accelPower = 11.0f * accelerationMultiplier;
      float falloffStartVelo = idleDribbleSwitch;

      if ((temporalMovement + toDesired).GetLength() > falloffStartVelo) {
        DO_VALIDATION;

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
        if ((temporalMovement + toDesired).GetLength() >
            temporalMovement.GetLength()) {
          DO_VALIDATION;  // outside the 'velocity circle'
          Vector3 destination = temporalMovement + toDesired;
          float velo = temporalMovement.GetLength();
          float accel = destination.GetLength() - velo;
          forwardVector = destination.GetNormalized(0) * accel;
        }

        float accelerationAddition = forwardVector.GetLength();
        float maxAccelerationMPS = accelPower * (1.0f - veloAirResistanceFactor) * (stat_acceleration * 0.3f + 0.7f);
        float maxAccelerationAddition = maxAccelerationMPS * (timeStep_ms / 1000.0f);
        if (accelerationAddition > maxAccelerationAddition) {
          DO_VALIDATION;
          float remainingFactor = maxAccelerationAddition / accelerationAddition;
          toDesired = toDesired - forwardVector * (1.0f - remainingFactor);
        }
      }
    }

    // MAKE IT SEW! http://static.wixstatic.com/media/fc58ad_c0ef2d69d98f4f7ba8e7e488f0e28ece.jpg

    Vector3 tmpTemporalMovement = temporalMovement + toDesired;

    // make sure outgoing velocity is of the same idleness as the anim
    if (time_ms >= (anim->GetFrameCount() - 2) * 10) {
      DO_VALIDATION;

      bool hardQuantize = true;
      if (!hardQuantize && anim->GetVariableCache().outgoing_special_state().compare("") != 0) hardQuantize = true;

      if (!hardQuantize) {
        DO_VALIDATION;
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
      DO_VALIDATION;
      positions_ret.push_back(currentPosition);
    }

    /*
    // dynamic timestep: more precision at high velocities
    if ((int)time_ms % 10 == 0) { DO_VALIDATION;
      //if (temporalMovement.GetLength() > walkVelocity) timeStep_ms = 5; else
    timeStep_ms = 10; timeStep_ms = 10;
    }
    */
  }

  assert(positions_ret.size() >= (unsigned int)anim->GetFrameCount());
  resultingMovement = temporalMovement;

  if (FloatToEnumVelocity(anim->GetOutgoingVelocity()) != e_Velocity_Idle &&
      FloatToEnumVelocity(resultingMovement.GetLength()) != e_Velocity_Idle) {
    DO_VALIDATION;
    rotationOffset_ret = resultingMovement.GetRotated2D(-spatialState.angle).GetAngle2D(anim->GetOutgoingMovement());
  } else {
    rotationOffset_ret = toDesiredAngle_capped * physicsBias;
  }

  // body direction assist
  if (mod_CheatBodyDirection && useDesiredBodyDirection &&
      animType == e_DefString_Movement) {
    DO_VALIDATION;

    float angleFactor = 0.5f;
    radian maxAngle = 0.25f * pi;

    radian predictedAngleRel = anim->GetOutgoingAngle() + anim->GetOutgoingBodyAngle() + rotationOffset_ret;
    radian desiredRotationOffset = desiredBodyDirectionRel.GetRotated2D(-predictedAngleRel).GetAngle2D(Vector3(0, -1, 0));

    if (fabs(desiredRotationOffset) < 0.5f * pi) {
      DO_VALIDATION;  // else: too much

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
  Vector3 best;
  for (const Vector3 &vec : allowedBodyDirVecs) {
    DO_VALIDATION;
    float nDotL = vec.GetDotProduct(src);
    if (nDotL > bestDot) {
      DO_VALIDATION;
      bestDot = nDotL;
      best = vec;
    }
  }

  return best;
}

radian HumanoidBase::ForceIntoAllowedBodyDirectionAngle(radian angle) const {

  float bestAngleDiff = 10000.0;
  radian bestValue = 0;
  for (auto v : allowedBodyDirAngles) {
    DO_VALIDATION;
    float diff = fabs(v - angle);
    if (diff < bestAngleDiff) {
      DO_VALIDATION;
      bestAngleDiff = diff;
      bestValue = v;
    }
  }
  return bestValue;
}

Vector3 HumanoidBase::ForceIntoPreferredDirectionVec(const Vector3 &src) const {

  float bestDot = -1.0f;
  Vector3 bestValue;
  for (auto &v : preferredDirectionVecs) {
    DO_VALIDATION;
    float nDotL = v.GetDotProduct(src);
    if (nDotL > bestDot) {
      DO_VALIDATION;
      bestDot = nDotL;
      bestValue = v;
    }
  }
  return bestValue;
}

radian HumanoidBase::ForceIntoPreferredDirectionAngle(radian angle) const {

  float bestAngleDiff = 10000.0;
  radian bestValue;
  for (auto v : preferredDirectionAngles) {
    DO_VALIDATION;
    float diff = fabs(v - angle);
    if (diff < bestAngleDiff) {
      DO_VALIDATION;
      bestAngleDiff = diff;
      bestValue = v;
    }
  }
  return bestValue;
}

void HumanoidBase::ProcessState(EnvState *state) {
  DO_VALIDATION;
  humanoidNode->ProcessState(state);
  animApplyBuffer.ProcessState(state);
  offsets.ProcessState(state);
  currentAnim.ProcessState(state);
  state->process(previousAnim_frameNum);
  state->process(previousAnim_functionType);
  state->process(startPos);
  state->process(startAngle);
  state->process(nextStartPos);
  state->process(nextStartAngle);
  spatialState.ProcessState(state);
  state->process(previousPosition2D);
  state->process(interruptAnim);
  state->process(reQueueDelayFrames);
  state->process(tripType);
  state->process(tripDirection);
  state->process(decayingPositionOffset);
  state->process(decayingDifficultyFactor);
  int s = movementHistory.size();
  state->process(s);
  movementHistory.resize(s);
  for (auto &i : movementHistory) {
    DO_VALIDATION;
    i.ProcessState(state);
  }
  state->process(mentalImageTime);
}
