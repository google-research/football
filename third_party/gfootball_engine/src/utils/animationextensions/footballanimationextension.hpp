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

#ifndef _HPP_FOOTBALLANIMATIONEXTENSION
#define _HPP_FOOTBALLANIMATIONEXTENSION

#include "animationextension.hpp"

namespace blunted {

  struct FootballKeyFrame {
    Quaternion orientation;
    Vector3 position;
    float power = 0.0f;
  };

  class FootballAnimationExtension : public AnimationExtension {

    public:
      FootballAnimationExtension(Animation *parent);
      virtual ~FootballAnimationExtension();

      virtual void Shift(int fromFrame, int offset);
      virtual void Rotate2D(radian angle);
      virtual void Mirror();

      virtual bool GetKeyFrame(int frame, Quaternion &orientation, Vector3 &position, float &power) const;
      virtual void SetKeyFrame(int frame, const Quaternion &orientation, const Vector3 &position = Vector3(0, 0, 0), float power = 1.0);
      virtual void DeleteKeyFrame(int frame);

      virtual void Load(std::vector<std::string> &tokenizedLine);
      virtual void Save(FILE *file);

      virtual bool GetFirstTouch(Vector3 &position, int &frame);
      int GetTouchCount() const;
      virtual bool GetTouch(unsigned int num, Vector3 &position, int &frame);
      virtual bool GetTouchPos(int frame, Vector3 &position);

    protected:
      std::map<int, FootballKeyFrame> animation;

  };

}

#endif
