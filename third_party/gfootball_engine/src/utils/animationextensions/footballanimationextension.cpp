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

#include "footballanimationextension.hpp"
#include "../animation.hpp"

#include "../../base/utils.hpp"

namespace blunted {

  FootballAnimationExtension::FootballAnimationExtension(Animation *parent) : AnimationExtension(parent) {
  }

  FootballAnimationExtension::~FootballAnimationExtension() {
    animation.clear();
  }

  void FootballAnimationExtension::Shift(int fromFrame, int offset) {
    std::map<int, FootballKeyFrame>::iterator animIter = animation.begin();
    std::map<int, FootballKeyFrame> newAnimation;

    if (offset == 1) {
      while (animIter != animation.end()) {
        FootballKeyFrame keyFrame = animIter->second;
        int frameNum = animIter->first;
        if (animIter->first >= fromFrame) frameNum++; // shift
        newAnimation.insert(std::pair<int, FootballKeyFrame>(frameNum, keyFrame));
        animIter++;
      }
    }
    if (offset == -1) {
      while (animIter != animation.end()) {
        FootballKeyFrame keyFrame = animIter->second;
        int frameNum = animIter->first;
        if (animIter->first != fromFrame) {
          if (animIter->first > fromFrame) {
            frameNum--; // shift
          }
          newAnimation.insert(std::pair<int, FootballKeyFrame>(frameNum, keyFrame));
        }
        animIter++;
      }
    }

    animation = newAnimation;
  }

  void FootballAnimationExtension::Rotate2D(radian angle) {
    std::map<int, FootballKeyFrame>::iterator animIter = animation.begin();
    while (animIter != animation.end()) {
      animIter->second.position.Rotate2D(angle);
      animIter++;
    }

    //ballDirection.Rotate2D(angle);
  }

  void FootballAnimationExtension::Mirror() {
    std::map<int, FootballKeyFrame>::iterator animIter = animation.begin();
    while (animIter != animation.end()) {
      animIter->second.position.coords[0] = -animIter->second.position.coords[0];
      animIter++;
    }

    //ballDirection.Rotate2D(angle);
  }

  bool FootballAnimationExtension::GetKeyFrame(int frame, Quaternion &orientation, Vector3 &position, float &power) const {
    std::map<int, FootballKeyFrame>::const_iterator animIter = animation.find(frame);

    if (animIter != animation.end()) {
      position = animIter->second.position;
      power = animIter->second.power;
      return true;
    } else {
      return false;
      //printf("football frame not found!\n");
    }
  }

  void FootballAnimationExtension::SetKeyFrame(int frame, const Quaternion &orientation, const Vector3 &position, float power) {
    std::map<int, FootballKeyFrame>::iterator animIter = animation.find(frame);
    if (animIter == animation.end()) {
      // keyframe does not exist yet
      FootballKeyFrame keyFrame;
      keyFrame.orientation = orientation;
      keyFrame.position = position;
      keyFrame.power = power;
      animation.insert(std::pair<int, FootballKeyFrame>(frame, keyFrame));

    } else {
      // already there
      animIter->second.orientation = orientation;
      animIter->second.position = position;
      animIter->second.power = power;
    }

  }

  void FootballAnimationExtension::DeleteKeyFrame(int frame) {
    std::map<int, FootballKeyFrame>::iterator animIter = animation.find(frame);

    if (animIter != animation.end()) {
      animation.erase(animIter);
    }
  }

  void FootballAnimationExtension::Load(std::vector<std::string> &tokenizedLine) {
    animation.clear();
    unsigned int key = 2;
    while (key < tokenizedLine.size()) {
      int frame = int(round(atoi(tokenizedLine.at(key).c_str()) * 1.0));

      Vector3 position;
      position.coords[0] = atof(tokenizedLine.at(key + 1).c_str());
      position.coords[1] = atof(tokenizedLine.at(key + 2).c_str());
      position.coords[2] = atof(tokenizedLine.at(key + 3).c_str());

      Quaternion orientation(QUATERNION_IDENTITY);
      SetKeyFrame(frame, orientation, position, 0);
      key += 4;
    }

  }

  void FootballAnimationExtension::Save(FILE *file) {
    if (animation.size() == 0) return;

    std::string line;
    line = "extension,football,";

    std::map<int, FootballKeyFrame>::iterator animIter = animation.begin();
    while (animIter != animation.end()) {
      line.append(int_to_str(animIter->first) + ","); // frame number
      line.append(real_to_str(animIter->second.position.coords[0]) + ","); // X pos
      line.append(real_to_str(animIter->second.position.coords[1]) + ","); // Y pos
      line.append(real_to_str(animIter->second.position.coords[2]) + ","); // Z pos
      animIter++;
    }

    line = line.substr(0, line.length() - 1);

    fprintf(file, "%s\n", line.c_str());
  }

  bool FootballAnimationExtension::GetFirstTouch(Vector3 &position, int &frame) {
    if (!animation.empty()) {
      position = animation.begin()->second.position;
      frame = animation.begin()->first;
      return true;
    } else {
      return false;
    }
  }

  int FootballAnimationExtension::GetTouchCount() const {
    return animation.size();
  }

  bool FootballAnimationExtension::GetTouch(unsigned int num, Vector3 &position, int &frame) {
    if (animation.size() > num) {

      std::map<int, FootballKeyFrame>::iterator iter = animation.begin();
      for (unsigned int i = 0; i < num; i++) {
        iter++;
        if (iter == animation.end()) return false;
      }

      position = iter->second.position;
      frame = iter->first;
      return true;
    } else {
      return false;
    }
  }

  bool FootballAnimationExtension::GetTouchPos(int frame, Vector3 &position) {
    std::map<int, FootballKeyFrame>::iterator iter = animation.find(frame);
    if (iter != animation.end()) {
      position = iter->second.position;
      return true;
    } else {
      return false;
    }
  }

}
