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

#include "menuscene.hpp"

#include <cmath>

#include "../managers/resourcemanagerpool.hpp"
#include "../scene/objectfactory.hpp"

#include "../main.hpp"

MenuScene::MenuScene() {
  seamless = false;

  containerNode = boost::intrusive_ptr<Node>(new Node("menuSceneNode"));
  GetScene3D()->AddNode(containerNode);


  currentPosition = Vector3(0.0f, 0.0f, 1.0f);
  currentOrientation = Quaternion(QUATERNION_IDENTITY);
  SetTargetLocation(currentPosition, currentOrientation);


  // camera
  camera = static_pointer_cast<Camera>(ObjectFactory::GetInstance().CreateObject("camera_MenuScene", e_ObjectType_Camera));
  GetScene3D()->CreateSystemObjects(camera);
  camera->Init();
  camera->SetFOV(90);

  camera->SetPosition(Vector3(0.0f, 0.0f, 1.0f));
  camera->SetCapping(0.5f, 2.0f);
  Quaternion orientation(QUATERNION_IDENTITY);
  camera->SetRotation(orientation);
  containerNode->AddObject(camera);


  // light
  float mainLightBrightness = 0.0f;
  float hoverLightBrightness = 2.0f;

  for (int i = 0; i < 3; i++) {
    hoverLights[i] = static_pointer_cast<Light>(ObjectFactory::GetInstance().CreateObject("light_MenuScene_hover" + int_to_str(i), e_ObjectType_Light));
    GetScene3D()->CreateSystemObjects(hoverLights[i]);
    hoverLights[i]->SetShadow(false);
    hoverLights[i]->SetType(e_LightType_Point);
    hoverLights[i]->SetRadius(2.0f);
    containerNode->AddObject(hoverLights[i]);
  }

  hoverLights[0]->SetColor(Vector3(1.0f, 0.3f, 0.3f) * hoverLightBrightness);
  hoverLights[1]->SetColor(Vector3(0.3f, 1.0f, 0.3f) * hoverLightBrightness);
  hoverLights[2]->SetColor(Vector3(0.3f, 0.3f, 1.0f) * hoverLightBrightness);


  // geometry
  boost::intrusive_ptr < Resource<GeometryData> > geometryData = ResourceManagerPool::getGeometryManager()->Fetch("media/objects/menu/background01.ase", true);
  geom = static_pointer_cast<Geometry>(ObjectFactory::GetInstance().CreateObject("geometry_menuscene", e_ObjectType_Geometry));
  GetScene3D()->CreateSystemObjects(geom);
  geom->SetGeometryData(geometryData);
  geom->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
  containerNode->AddObject(geom);


  // for usage in destructor
  scene3D = GetScene3D();
}

MenuScene::~MenuScene() {
  scene3D->DeleteNode(containerNode);
}

void MenuScene::Get() {
}

void MenuScene::Process() {
  // calculate position

  unsigned long time_ms = EnvironmentManager::GetInstance().GetTime_ms();

  if (targetLocation.timeStamp_ms >= time_ms) {
    float bias = (time_ms - sourceLocation.timeStamp_ms) / (float)(targetLocation.timeStamp_ms - sourceLocation.timeStamp_ms);
    bias = std::pow(bias, 0.8f);
    bias = curve(bias, 1.0f);

    currentPosition = sourceLocation.position * (1.0f - bias) + targetLocation.position * bias;
    currentOrientation = sourceLocation.orientation.GetLerped(bias, targetLocation.orientation);
  }

  // make sure we stay in the 'main quad'
  if (seamless) {
    Vector3 offset;
    if (currentPosition.coords[0] > 5.0f) {
      offset.coords[0] = -10.0f;
    } else if (currentPosition.coords[0] < -5.0f) {
      offset.coords[0] = 10.0f;
    }
    if (currentPosition.coords[1] > 5.0f) {
      offset.coords[1] = -10.0f;
    } else if (currentPosition.coords[1] < -5.0f) {
      offset.coords[1] = 10.0f;
    }
    currentPosition += offset;
    sourceLocation.position += offset;
    targetLocation.position += offset;
  }

  // move around a little
  float randomPositionIntensity = 0.3f;
  Vector3 randomPositionNoise;
  randomPositionNoise.coords[0] =
      std::sin(time_ms / 7420.0f) * 0.5f + std::cos(time_ms / 3150.0f) * 0.3f;
  randomPositionNoise.coords[1] =
      std::cos(time_ms / 8250.0f) * 0.5f + std::sin(time_ms / 2420.0f) * 0.3f;

  camera->SetPosition(currentPosition + randomPositionNoise * randomPositionIntensity);
  camera->SetRotation(currentOrientation);


  hoverLightPosition = currentPosition.Get2D() + Vector3(0.0f, 0.0f, 0.5f);
  for (int i = 0; i < 3; i++) {
    float separationFactor = 0.7f;
    Vector3 lightOffset = Vector3(0.0f, -1.0f, 0.0f) * separationFactor;
    float timeOffset = std::sin(time_ms / 4000.0f) * 5.0f;
    lightOffset.Rotate2D(2.0f * pi * (i / 3.0f) + timeOffset);
    hoverLights[i]->SetPosition(hoverLightPosition + lightOffset);
  }

}

void MenuScene::Put() {
}

void MenuScene::RandomizeTargetLocation() {
  Vector3 dir = Vector3(0.0f, -1.0f, 0.0f).GetRotated2D(random(-1.0f * pi, 1.0f * pi));
  dir *= 0.5f;

  Vector3 targetPos;
  if (seamless) {
    targetPos = currentPosition + dir;
  } else {
    // always aim for the middle, don't stray
    Vector3 targetPos1 = currentPosition + dir;
    Vector3 targetPos2 = currentPosition - dir;
    targetPos = targetPos1;
    if (targetPos1.GetLength() > targetPos2.GetLength()) targetPos = targetPos2;
  }

  radian angle = random(-0.1f * pi, 0.1f * pi);

  SetTargetLocation(targetPos, angle);
}

void MenuScene::SetTargetLocation(const Vector3 &position, radian angle) {
  Quaternion orientation;
  orientation.SetAngleAxis(angle, Vector3(random(-0.2f, 0.2f), random(-0.2f, 0.2f), -1.0f).GetNormalized());
  SetTargetLocation(position, orientation);
}

void MenuScene::SetTargetLocation(const Vector3 &position, const Quaternion &orientation) {
  sourceLocation.timeStamp_ms = EnvironmentManager::GetInstance().GetTime_ms();
  sourceLocation.position = currentPosition;
  sourceLocation.orientation = currentOrientation;

  targetLocation.timeStamp_ms = sourceLocation.timeStamp_ms + 1000;
  targetLocation.position = position;
  targetLocation.orientation = orientation;
}
