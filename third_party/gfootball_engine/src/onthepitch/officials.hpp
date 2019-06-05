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

#ifndef _HPP_OFFICIALS
#define _HPP_OFFICIALS

#include "player/humanoid/animcollection.hpp"

class PlayerBase;
class PlayerOfficial;
class PlayerData;

class Officials {

  public:
    Officials(Match *match, boost::intrusive_ptr<Node> fullbodySourceNode, std::map<Vector3, Vector3> &colorCoords, boost::intrusive_ptr < Resource<Surface> > kit, boost::shared_ptr<AnimCollection> animCollection);
    virtual ~Officials();

    void GetPlayers(std::vector<PlayerBase*> &players);
    PlayerOfficial *GetReferee() { return referee; }

    virtual void Process();
    virtual void PreparePutBuffers(unsigned long snapshotTime_ms);
    virtual void FetchPutBuffers(unsigned long putTime_ms);
    virtual void Put();

    boost::intrusive_ptr<Geometry> GetYellowCardGeom() { return yellowCard; }
    boost::intrusive_ptr<Geometry> GetRedCardGeom() { return redCard; }

  protected:
    Match *match;

    PlayerOfficial *referee;
    PlayerOfficial *linesmen[2];
    PlayerData *playerData;

    boost::intrusive_ptr<Geometry> yellowCard;
    boost::intrusive_ptr<Geometry> redCard;

};

#endif
