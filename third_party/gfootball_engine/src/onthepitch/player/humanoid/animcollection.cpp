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

#include "animcollection.hpp"

#include <cmath>

#include "../../../utils/directoryparser.hpp"

#include "../../../utils/animationextensions/footballanimationextension.hpp"

#include "../../../managers/resourcemanagerpool.hpp"

#include "../../../utils/objectloader.hpp"
#include "../../../scene/objectfactory.hpp"

#include "humanoid_utils.hpp"

#include "humanoid.hpp"

#include "../../../main.hpp"

void FillNodeMap(boost::intrusive_ptr<Node> targetNode, NodeMap &nodeMap) {
  nodeMap[BodyPartFromString(targetNode->GetName())] = targetNode;

  std::vector < boost::intrusive_ptr<Node> > gatherNodes;
  targetNode->GetNodes(gatherNodes);
  for (unsigned int i = 0; i < gatherNodes.size(); i++) {
    FillNodeMap(gatherNodes[i], nodeMap);
  }

}

AnimCollection::AnimCollection(boost::shared_ptr<Scene3D> scene3D) : scene3D(scene3D) {
  maxIncomingBallDirectionDeviation = 0.25f * pi;
  maxOutgoingBallDirectionDeviation = 0.25f * pi;


  // quadrants denote the quantized outgoing movement - there are only certain possibilities:
  // idle - dribble - walk - run, in combination with angles: 0 - 20 - 45 - 90 - 135 - 180 (and their negatives, where applicable)
  // so every anim falls into one of these quadrants.

  // idle
  Quadrant quadrant;
  quadrant.id = 0;
  quadrant.velocity = e_Velocity_Idle;
  quadrant.angle = 0;
  quadrant.position = Vector3(0, 0, 0);
  quadrants.push_back(quadrant);

  int id = 1;
  for (int velocityID = 1; velocityID < 4; velocityID++) {

    e_Velocity velocity;
    if (velocityID == 1) velocity = e_Velocity_Dribble;
    else if (velocityID == 2) velocity = e_Velocity_Walk;
    else if (velocityID == 3) velocity = e_Velocity_Sprint;

    for (int angleID = 0; angleID < 11; angleID++) {

      radian angle;
      if      (angleID == 0)  angle = pi / 180.0f *    0.0f;
      else if (angleID == 1)  angle = pi / 180.0f *   20.0f;
      else if (angleID == 2)  angle = pi / 180.0f *   45.0f;
      else if (angleID == 3)  angle = pi / 180.0f *   90.0f;
      else if (angleID == 4)  angle = pi / 180.0f *  135.0f;
      else if (angleID == 5)  angle = pi / 180.0f *  179.0f;
      else if (angleID == 6)  angle = pi / 180.0f *  -20.0f;
      else if (angleID == 7)  angle = pi / 180.0f *  -45.0f;
      else if (angleID == 8)  angle = pi / 180.0f *  -90.0f;
      else if (angleID == 9)  angle = pi / 180.0f * -135.0f;
      //else if (angleID == 10) angle = pi / 180.0f * -179.0f;

      Quadrant quadrant;
      quadrant.id = id;
      quadrant.velocity = velocity;
      quadrant.angle = angle;
      quadrant.position = Vector3(0, -1, 0).GetRotated2D(angle) * EnumToFloatVelocity(velocity);
      quadrants.push_back(quadrant);

      id++;
    }
  }

}

AnimCollection::~AnimCollection() {
  Clear();
}

void AnimCollection::Clear() {
  std::vector < Animation* >::iterator animIter = animations.begin();
  while (animIter != animations.end()) {
    delete *animIter;
    animIter++;
  }
  animations.clear();
}

radian GetAngle(int directionID) {
  radian angle = 0.0f;
  switch (directionID) {
    case 0:
      angle = 0.0f;
      break;
    case 1:
      angle = 0.25f * pi;
      break;
    case 2:
      angle = -0.25f * pi;
      break;
    case 3:
      angle = 0.50f * pi;
      break;
    case 4:
      angle = -0.50f * pi;
      break;
    case 5:
      angle = 0.75f * pi;
      break;
    case 6:
      angle = -0.75f * pi;
      break;
    case 7:
      angle = 0.99f * pi;
      break;
    case 8:
      angle = -0.99f * pi;
      break;
    default:
      break;

  }
  return angle;
}

void GenerateAutoAnims(const std::vector<Animation*> &templates, std::vector<Animation*> &autoAnims) {

  const float leanAmount = 0.001f;
  const int frameCount = 25;
  const float margin = 0.01f;

  for (unsigned int t1 = 0; t1 < templates.size(); t1++) {
    for (unsigned int t2 = 0; t2 < templates.size(); t2++) {

      Animation *anim1 = templates.at(t1);
      Animation *anim2 = templates.at(t2);

      for (unsigned int direction = 0; direction < 9; direction++) {

        radian angle = GetAngle(direction);
        float incomingVelocityT1 = anim1->GetIncomingVelocity();
        float outgoingVelocityT2 = anim2->GetOutgoingVelocity();
        radian incomingBodyAngleT1 = anim1->GetIncomingBodyAngle();
        radian outgoingBodyAngleT2 = anim2->GetOutgoingBodyAngle();

        bool legalAnim = true;

        int incomingVeloID = GetVelocityID(FloatToEnumVelocity(incomingVelocityT1));
        int outgoingVeloID = GetVelocityID(FloatToEnumVelocity(outgoingVelocityT2));
        float averageVeloFactor = NormalizedClamp(incomingVeloID + outgoingVeloID, 0, 6);

        // max acceleration
        int veloIDDiff = outgoingVeloID - incomingVeloID;
        if (veloIDDiff > 1) legalAnim = false;

        // max deceleration
        // use dot product to make sure "running 180 degrees" isn't considered "keeping the same velocity" and therefore legal.
        float dot = Vector3(0, -1, 0).GetDotProduct(Vector3(0, -1, 0).GetRotated2D(angle));
        int veloIDDiff_dotted =
            std::round(outgoingVeloID * dot - incomingVeloID);
        if (veloIDDiff_dotted < -3) legalAnim = false;

        if (incomingVeloID == 3 && outgoingVeloID == 1 && (fabs(outgoingBodyAngleT2) > 0.25f * pi + margin || fabs(angle) > 0.25f * pi + margin)) legalAnim = false; // no sprint to dribble with backwards body angle
        //if (incomingVeloID == 3 && outgoingVeloID == 0 && fabs(angle) > 0.25f * pi) legalAnim = false; // no sprint to idle with big angle

        // experimental block
        if (incomingVeloID > 0 && outgoingVeloID > 0 && fabs(angle) > 0.50f * pi + margin) legalAnim = false;
        //if (incomingVeloID > 0 && outgoingVeloID == 0 && fabs(outgoingBodyAngleT2) > 0.50f * pi + margin) legalAnim = false;
        // if (incomingVeloID == 3 && outgoingVeloID == 2 && (fabs(outgoingBodyAngleT2) > 0.3f * pi || fabs(angle) > 0.25f * pi)) legalAnim = false;
        //zzif (incomingVeloID == 3 && outgoingVeloID == 0) legalAnim = false;
        //if (incomingVeloID == 3 && outgoingVeloID == 2 && fabs(angle) > 0.25f * pi) legalAnim = false;
        //if (fabs(outgoingBodyAngleT2 - incomingBodyAngleT1 + angle) > 0.5f * pi) legalAnim = false;

        //if (incomingVeloID + outgoingVeloID > 5 && fabs(angle) > 0.25f * pi) legalAnim = false; // sprint -> sprint
        //if (incomingVeloID + outgoingVeloID > 4 && fabs(angle) > 0.5f * pi) legalAnim = false; // walk -> sprint && sprint -> walk
        if (incomingVeloID + outgoingVeloID > 5 && fabs(angle) > 0.25f * pi + margin) legalAnim = false; // sprint -> sprint
        if (incomingVeloID + outgoingVeloID > 4 && fabs(angle) > 0.25f * pi + margin) legalAnim = false; // walk -> sprint && sprint -> walk
        //if (incomingVeloID + outgoingVeloID > 3 && fabs(angle) > 0.50f * pi + margin) legalAnim = false; // walk -> walk && sprint -> dribble && dribble -> sprint
        //if (incomingVeloID + outgoingVeloID > 2 && fabs(angle) > 0.75f * pi + margin) legalAnim = false; // dribble -> walk && walk -> dribble
        //if (incomingVeloID > 0 && outgoingVeloID > 0 && fabs(angle) > 0.5f * pi + margin) legalAnim = false; // dribble -> walk && walk -> dribble

        radian bodyAngleDelta = ModulateIntoRange(-pi, pi, outgoingBodyAngleT2 - incomingBodyAngleT1);
        if (fabs(angle + bodyAngleDelta) > 1.0f * pi + margin) legalAnim = false; // don't rotate over 180 degrees (we've got an anim for that the shorter away around anyway)
        //if (fabs(angle + bodyAngleDelta) > 0.5f * pi + margin) legalAnim = false; // just don't turn too fast

        //if (fabs(bodyAngleDelta) > 0.9f * pi) legalAnim = false; // body rotation limit: if we allow 180 degrees, the slerp doesn't know whether it should go CW or CCW (seems to be fixed by the ModulateIntoRange above - why?)
        //if (fabs(veloIDDiff) > 2 && fabs(angle + bodyAngleDelta) > 0.25f * pi) legalAnim = false; // don't brake AND turn all too much
        //|__[R]__ autogen [v2 b-135] => [v2 b45 a-179]_mirror
        //if (incomingVeloID + outgoingVeloID > 3 && fabs(angle) > 0.50f * pi) legalAnim = false;

/* too uncontrollable
        Vector3 movementChangeVec = (Vector3(0, -1, 0) * incomingVelocityT1) - (Vector3(0, -1, 0).GetRotated2D(angle) * outgoingVelocityT2);
        Vector3 bodyDirChangeVec = (Vector3(0, -1, 0).GetRotated2D(incomingBodyAngleT1)) - (Vector3(0, -1, 0).GetRotated2D(angle + outgoingBodyAngleT2));
        Vector3 totalChangeVec = movementChangeVec + bodyDirChangeVec * 1.0f;
        if (totalChangeVec.GetLength() > sprintVelocity) legalAnim = false;
*/

        float animSpeedFactor = 1.0f;

        if (legalAnim == true) {

          Animation *gen = new Animation(*templates.at(t1));
          gen->SetName("autogen [v" + int_to_str(GetVelocityID(FloatToEnumVelocity(incomingVelocityT1))) + " b" + int_to_str(incomingBodyAngleT1 / pi * 180) + "] => [v" + int_to_str(GetVelocityID(FloatToEnumVelocity(outgoingVelocityT2))) + " b" + int_to_str(outgoingBodyAngleT2 / pi * 180) + " a" + int_to_str(angle / pi * 180) + "]");
          gen->SetVariable("priority", "1");

          std::vector<NodeAnimation*> &nodeAnimsT1 = anim1->GetNodeAnimations();
          std::vector<NodeAnimation*> &nodeAnimsT2 = anim2->GetNodeAnimations();

          for (unsigned int n = 0; n < nodeAnimsT1.size(); n++) {

            gen->GetNodeAnimations().at(n)->animation.clear();

            // NodeAnimation *nodeAnimT1 = nodeAnimsT1.at(n);
            // NodeAnimation *nodeAnimT2 = nodeAnimsT2.at(n);
            const KeyFrames &animationT1 = nodeAnimsT1.at(n)->animation;
            const KeyFrames &animationT2 = nodeAnimsT2.at(n)->animation;

            Vector3 cumulativePosition;
            int prevFrame = 0;
            Vector3 outgoingMovement = anim2->GetOutgoingMovement().GetRotated2D(angle);

            Vector3 movementChangeMPS = (outgoingMovement - anim1->GetIncomingMovement()) * (100.0f / frameCount);


            // COLLECT KEYFRAMES FOR THIS NODEANIM

            std::list<int> keyFrames; // frames at which one of the two anims has a keyframe
            // first, add all keyframes, even if duplicate
            for (auto i : animationT1.d) {
              keyFrames.push_back(i.first);
            }
            for (auto i : animationT2.d) {
              keyFrames.push_back(i.first);
            }
            if (n == 0) {
              // make sure there's 2 position keyframes close to each other at the start and at the end, so we will have the right ingoing and outgoing velocities
              keyFrames.push_back(1);
              keyFrames.push_back(23);
              //for (int i = 2; i < frameCount - 2; i += 2) keyFrames.push_back(i); // smoother
              for (int i = 2; i < frameCount - 2; i += 4) keyFrames.push_back(i); // smoother
            }
            keyFrames.sort();
            keyFrames.unique(); // delete duplicates


            // ITERATE AND INTERPOLATE KEYFRAMES

            std::list<int>::iterator keyIter = keyFrames.begin();
            while (keyIter != keyFrames.end()) {
              int frame = *keyIter;
              float targetFrame = frame * (1.0f / animSpeedFactor);

              Quaternion orientationT1 = QUATERNION_IDENTITY;
              Quaternion orientationT2 = QUATERNION_IDENTITY;
              Vector3 positionT1, positionT2;
              templates.at(t1)->GetInterpolatedValues(animationT1, frame, orientationT1, positionT1);
              templates.at(t2)->GetInterpolatedValues(animationT2, frame, orientationT2, positionT2);

              //float bias = frame / ((float)frameCount - 1);
              float origBias = clamp(frame - 1.0f, 0.0f, frameCount - 3.0f) / ((float)frameCount - 3.0f); // version that ignores first and last 2 frames, so incoming/outgoing velo/angle will be correct
              // bias = pow(bias, 0.7f + pow(averageVeloFactor, 2.0) * 0.2f); // move a bit earlier
              // bias = curve(bias, 1.0f - pow(averageVeloFactor, 2.0) * 0.8f); // concentrate change in the middle of the anim
              float bias = origBias;
              //bias = pow(bias, 0.7f); // move a bit earlier
              //bias = pow(bias, 0.2f + 0.6f * averageVeloFactor);
              //if (frame == frameCount - 2) printf("before: %f, ", bias);
              bias = std::pow(
                  bias,
                  1.0f * (0.3f + 0.4f * averageVeloFactor +
                          0.3f * NormalizedClamp(movementChangeMPS.GetLength(),
                                                 0.0f, 20.0f)));
              //if (frame == frameCount - 2) printf("middle: %f, ", bias);
              bias = curve(bias, 0.7f); //0.3f// concentrate change in the middle of the anim
              //if (frame == frameCount - 2) printf("after: %f\n", bias);
              Quaternion orientation = orientationT1.GetSlerped(bias, orientationT2);

              if (n == 1) { // body
                // body orientation
                Quaternion angleQuat;
                angleQuat.SetAngleAxis(
                    angle * std::pow((bias * 0.7f + origBias * 0.3f), 1.0f),
                    Vector3(0, 0, 1));
                orientation = angleQuat * orientation;

                // leaning
                //movementChangeMPS *= (incomingVeloID + outgoingVeloID) / 6.0f;
                movementChangeMPS *= 0.5f + 0.5f * ((anim1->GetIncomingMovement() * (1.0f - bias) + outgoingMovement * bias).GetLength() / sprintVelocity); // high velo = more leaning into the wind and such :p
                Quaternion leanQuat;
                leanQuat.SetAngleAxis(
                    movementChangeMPS.GetLength() * leanAmount *
                        (0.5f + 0.5f * std::sin(origBias * pi)),
                    Vector3(0, 1, 0).GetRotated2D(
                        movementChangeMPS.GetNormalized(0).GetAngle2D()));
                orientation = leanQuat * orientation;
              }

              float height = 0.0f;
              if (n == 0) { // player
                //if (outgoingVeloID == 0 && incomingVeloID != 0) printf("frame: %i, bias: %f\n", frame, bias);
                cumulativePosition += anim1->GetIncomingMovement() * ((frame - prevFrame) * 0.01f) * (1.0f - bias) +
                                      outgoingMovement * ((frame - prevFrame) * 0.01f) * (bias);
                height = positionT1.coords[2] * (1.0f - bias) + positionT2.coords[2] * bias;
              }

              gen->SetKeyFrame(nodeAnimsT1.at(n)->nodeName,
                               int(std::floor(targetFrame)), orientation,
                               cumulativePosition *(1.0f / animSpeedFactor) +
                                   Vector3(0, 0, height));

              prevFrame = frame;
              keyIter++;
            }
          }

          gen->DirtyCache();

          assert(gen->GetIncomingVelocity() == anim1->GetIncomingVelocity());
          assert(gen->GetOutgoingVelocity() == anim2->GetOutgoingVelocity());

          // enable this to save all the autogenerated anims to files, so that we can use them as a base for manual anims. don't forget to disable again after usage ;)
          //gen->Save("media/animations/debug_luxury/autogen v" + int_to_str(GetVelocityID(FloatToEnumVelocity(incomingVelocityT1))) + " b" + int_to_str(round(incomingBodyAngleT1 / pi * 180)) + " _to_ v" + int_to_str(GetVelocityID(FloatToEnumVelocity(outgoingVelocityT2))) + " b" + int_to_str(round(outgoingBodyAngleT2 / pi * 180)) + " a" + int_to_str(angle / pi * 180) + ".anim");

          autoAnims.push_back(gen);

        } // == legalAnim

      }
    }
  }
}

void AnimCollection::Load(boost::filesystem::path directory) {


  // load utility player to get things like foot position in the frames around the balltouch etc.

  ObjectLoader loader;
  boost::intrusive_ptr<Node> playerNode;
  playerNode = loader.LoadObject(scene3D, "media/objects/players/player.object");
  playerNode->SetName("player");
  playerNode->SetLocalMode(e_LocalMode_Absolute);

  std::list < boost::intrusive_ptr<Object> > bodyParts;
  playerNode->GetObjects(e_ObjectType_Geometry, bodyParts, true);

  NodeMap nodeMap;
  FillNodeMap(playerNode, nodeMap);


  // base anim with default angles - all anims' joints will be inversely rotated by the joints in this anim. this way, the fullbody mesh doesn't need to have 0 degree angles

/*
  Animation *baseAnim = new Animation();
  baseAnim->Load("media/animations/base.anim.util");
*/


  // auto generated anims

  DirectoryParser parser;
  std::vector<std::string> files;
  parser.Parse(directory / "/templates", "anim", files);
  sort(files.begin(), files.end());

  std::vector<Animation*> templates;
  for (unsigned int i = 0; i < files.size(); i++) {
    Animation *animTemplate = new Animation();
    animTemplate->Load(files[i]);
    templates.push_back(animTemplate);
  }

  std::vector<Animation*> autoAnims;
  GenerateAutoAnims(templates, autoAnims);
  std::vector < Animation* >::iterator animIter = templates.begin();
  while (animIter != templates.end()) {
    delete *animIter;
    animIter++;
  }
  templates.clear();

  for (unsigned int i = 0; i < autoAnims.size(); i++) {
    Animation *animation = new Animation(*autoAnims[i]);
    boost::shared_ptr<FootballAnimationExtension> extension(new FootballAnimationExtension(animation));
    animation->AddExtension("football", extension);
    animation->Mirror();
    _PrepareAnim(animation, playerNode, bodyParts, nodeMap, false);

    animation = autoAnims[i];
    extension.reset(new FootballAnimationExtension(animation));
    animation->AddExtension("football", extension);
    _PrepareAnim(animation, playerNode, bodyParts, nodeMap, false);
  }


  // load all other animations

  files.clear();
  parser.Parse(directory, "anim", files);
  sort(files.begin(), files.end());

  bool omitLuxuryAnims = true;

  for (unsigned int i = 0; i < files.size(); i++) {

    //printf("%s\n", files[i].c_str());

    if ((omitLuxuryAnims && files[i].find("luxury") != std::string::npos) || files[i].find("templates") != std::string::npos) {
      //printf ("ignoring\n");

    } else {

      for (int mirror = 0; mirror < 2; mirror++) {
        Animation *animation = new Animation();
        boost::shared_ptr<FootballAnimationExtension> extension(new FootballAnimationExtension(animation));
        animation->AddExtension("football", extension);
        animation->Load(files[i]);
        if (mirror == 1) animation->Mirror();

        _PrepareAnim(animation, playerNode, bodyParts, nodeMap, false);

        /* disabled: too many side effects, should just make the most important of these manually

        // duplicate dribble anims with > 45 degree body directions (either in or out) and convert duplicate to walking speed
        // this is because of the decision to allow 135 degree body directions ('walking backward') on walking velocities.
        // more correct (to get proper leg movement for walking velocities) would be to create separate anims for these, but i'm feeling lazy
        if (animation->GetAnimType().compare(e_DefString_Movement) == 0) {
          if (fabs(animation->GetIncomingBodyAngle()) > 0.5 * pi || fabs(animation->GetOutgoingBodyAngle()) > 0.5 * pi) {
            if (FloatToEnumVelocity(animation->GetIncomingVelocity()) == e_Velocity_Dribble || FloatToEnumVelocity(animation->GetOutgoingVelocity()) == e_Velocity_Dribble) {

              Animation *animation2 = new Animation();
              boost::shared_ptr<FootballAnimationExtension> extension(new FootballAnimationExtension(animation));
              animation2->AddExtension("football", extension);
              animation2->Load(files[i], mirror == 0 ? false : true);

              _PrepareAnim(animation2, playerNode, bodyParts, nodeMap, true);

            }
          }
        }*/
      }

    }

  }

  //delete baseAnim;

  playerNode->Exit();
}

const std::vector < Animation* > &AnimCollection::GetAnimations() const {
  return animations;
}


void AnimCollection::CrudeSelection(DataSet &dataSet, const CrudeSelectionQuery &query) {

  // makes a crude selection to later refine


  int animSize = animations.size();

  for (int i = 0; i < animSize; i++) {

    e_DefString animType = animations[i]->GetAnimType();

    // select by TYPE

    if (query.byFunctionType == true) {
      if (_CheckFunctionType(animType, query.functionType) == false) continue;
    }

    // select by INCOMING VELOCITY

    if (query.byIncomingVelocity == true) {

      e_Velocity animIncomingVelocity = FloatToEnumVelocity(animations[i]->GetIncomingVelocity());

      if (query.incomingVelocity_Strict == false) {

        if (query.incomingVelocity_NoDribbleToIdle) {
          if (animIncomingVelocity == e_Velocity_Idle && query.incomingVelocity == e_Velocity_Dribble) continue;
        }
        if (animIncomingVelocity == e_Velocity_Idle && query.incomingVelocity == e_Velocity_Walk) continue;
        if (animIncomingVelocity == e_Velocity_Idle && query.incomingVelocity == e_Velocity_Sprint) continue;
        if (animIncomingVelocity == e_Velocity_Dribble && query.incomingVelocity == e_Velocity_Idle) continue;
        if (animIncomingVelocity == e_Velocity_Walk && query.incomingVelocity == e_Velocity_Idle) continue;
        if (animIncomingVelocity == e_Velocity_Sprint && query.incomingVelocity == e_Velocity_Idle) continue;

        if (query.incomingVelocity_NoDribbleToSprint) {
          if (animIncomingVelocity == e_Velocity_Sprint && query.incomingVelocity == e_Velocity_Dribble) continue;
        }

        if (query.incomingVelocity_ForceLinearity) {
          // disallow going from current -> slower/faster -> current; the complete section needs to be linear
          float animIncomingVelocityFloat = RangeVelocity(animations[i]->GetIncomingVelocity());
          float animOutgoingVelocityFloat = RangeVelocity(animations[i]->GetOutgoingVelocity());
          float queryVelocityFloat = EnumToFloatVelocity(query.incomingVelocity);

          // treat dribble and walk the same
          if (FloatToEnumVelocity(animIncomingVelocityFloat) == e_Velocity_Dribble) animIncomingVelocityFloat = walkVelocity;
          if (FloatToEnumVelocity(animOutgoingVelocityFloat) == e_Velocity_Dribble) animOutgoingVelocityFloat = walkVelocity;
          if (FloatToEnumVelocity(queryVelocityFloat) == e_Velocity_Dribble) queryVelocityFloat = walkVelocity;

          if (animIncomingVelocityFloat > std::max(queryVelocityFloat, animOutgoingVelocityFloat)) continue;
          if (animIncomingVelocityFloat < std::min(queryVelocityFloat, animOutgoingVelocityFloat)) continue;
        }

      } else {
        // strict
        if (animIncomingVelocity != query.incomingVelocity) continue;
      }

      // test: disallow idle -> moving and other way around
      //e_Velocity animOutgoingVelocity = FloatToEnumVelocity(animations[i]->GetOutgoingVelocity());
      //if (animIncomingVelocity == e_Velocity_Idle && animOutgoingVelocity != e_Velocity_Idle) continue;
      //if (animOutgoingVelocity == e_Velocity_Idle && animIncomingVelocity != e_Velocity_Idle) continue;
    }

    // test: disable all sprint anims
    // e_Velocity animIncomingVelocity = FloatToEnumVelocity(animations[i]->GetIncomingVelocity());
    // e_Velocity animOutgoingVelocity = FloatToEnumVelocity(animations[i]->GetOutgoingVelocity());
    // if (animIncomingVelocity == e_Velocity_Sprint || animOutgoingVelocity == e_Velocity_Sprint) continue;


    // select by OUTGOING VELOCITY

    if (query.byOutgoingVelocity == true) {
      if (FloatToEnumVelocity(animations[i]->GetOutgoingVelocity()) != query.outgoingVelocity) continue;
    }

    // CULL WRONG ROTATIONAL SIDE

    if (query.bySide == true) {

      Vector3 animIncomingDirection = animations[i]->GetIncomingBodyDirection();

      // find out in what direction the anim rotates
      Vector3 animOutgoingDirection = animations[i]->GetOutgoingDirection().GetRotated2D(animations[i]->GetOutgoingBodyAngle());
      radian animTurnAngle = animOutgoingDirection.GetAngle2D(animIncomingDirection);

      // anim should not pass through opposite (180 deg) of desired look angle
      Vector3 fencedDirection = query.lookAtVecRel.GetRotated2D(pi);

      if (fabs(animTurnAngle) > 0.06f * pi) { // threshold
        e_Side animSide = (animTurnAngle > 0) ? e_Side_Left : e_Side_Right;

        radian animIncomingToFenceAngle = fencedDirection.GetAngle2D(animIncomingDirection);
        radian queryIncomingToFenceAngle = fencedDirection.GetAngle2D(query.incomingBodyDirection);
        radian fenceToOutgoingAngle = animOutgoingDirection.GetAngle2D(fencedDirection);

        e_Side animIncomingToFenceSide = (animIncomingToFenceAngle > 0) ? e_Side_Left : e_Side_Right;
        e_Side queryIncomingToFenceSide = (queryIncomingToFenceAngle > 0) ? e_Side_Left : e_Side_Right;
        e_Side fenceToAnimOutgoingSide = (fenceToOutgoingAngle > 0) ? e_Side_Left : e_Side_Right;

        // passes through fence! n000!
        if (animIncomingToFenceSide  == animSide && fenceToAnimOutgoingSide == animSide && fabs(animIncomingToFenceAngle + fenceToOutgoingAngle) < pi) continue;
        if (queryIncomingToFenceSide == animSide && fenceToAnimOutgoingSide == animSide && fabs(queryIncomingToFenceSide + fenceToOutgoingAngle) < pi) continue;
      }
    }

    // select by RETAIN BALL

    if (query.byPickupBall == true) {
      if ((animations[i]->GetVariable("outgoing_retain_state") == "" && query.pickupBall == true) ||
          (animations[i]->GetVariable("outgoing_retain_state") != "" && query.pickupBall == false)) {
        continue;
      }
    }


    // select LAST DITCH ANIMS

    if (query.allowLastDitchAnims == false) {
      if (animations[i]->GetVariableCache().lastditch()) {
        continue;
      }
    }


    // select by INCOMING BODY ANGLE

    if (query.byIncomingBodyDirection == true && !(query.byIncomingVelocity == true && query.incomingVelocity == e_Velocity_Idle)) {

      radian marginRadians = 0.06f * pi; // anims can deviate a few degrees from the desired (quantized) directions

      if (FloatToEnumVelocity(animations[i]->GetIncomingVelocity()) != e_Velocity_Idle) {

        Vector3 incomingBodyDir = animations[i]->GetIncomingBodyDirection();

/* this is implicitly happening already because of the section after this one anyway, so disable *todo: is it?
          //if (selectAnim) {
            if (query.incomingBodyDirection_Strict == true) { // == non-movement, atm
              // strict
              //if (fabs(incomingBodyDir.GetAngle2D(Vector3(0, -1, 0))) > fabs(query.incomingBodyDirection.GetAngle2D(Vector3(0, -1, 0))) + marginRadians) continue;
              // allow 45
              //if (fabs(incomingBodyDir.GetAngle2D(Vector3(0, -1, 0))) > fabs(query.incomingBodyDirection.GetAngle2D(Vector3(0, -1, 0))) + 0.25f * pi + marginRadians) continue;

              //} else {
              // if (fabs(query.incomingBodyDirection.GetAngle2D(animations[i]->GetIncomingBodyDirection())) > marginRadians) continue;
              //}
            }
          //}
*/
        // disallow larger than x radians diff
        //if (fabs(query.incomingBodyDirection.GetAngle2D(animations[i]->GetIncomingBodyDirection())) > marginRadians) continue;
        // disallow larger incoming than current
        if (fabs(FixAngle(animations[i]->GetIncomingBodyDirection().GetAngle2D())) > fabs(FixAngle(query.incomingBodyDirection.GetAngle2D())) + marginRadians) continue;


        // absolute outgoing body dir is body dir + outgoing dir
        Vector3 outgoingBodyDir = Vector3(0, -1, 0).GetRotated2D(animations[i]->GetOutgoingBodyAngle() + animations[i]->GetOutgoingAngle());

        // disallow > ~135 degrees (not really needed, i guess)
        //if (fabs(animations[i]->GetIncomingBodyDirection().GetAngle2D(query.incomingBodyDirection)) > 0.75f * pi + marginRadians) continue;
        //if (fabs(animations[i]->GetIncomingBodyDirection().GetAngle2D(query.incomingBodyDirection)) > 0.5f * pi + marginRadians) continue;

        // disallow > ~90 degrees larger incoming than current
        //if (fabs(FixAngle(animations[i]->GetIncomingBodyDirection().GetAngle2D())) > fabs(FixAngle(query.incomingBodyDirection.GetAngle2D())) + 0.5f * pi + marginRadians) continue;
        // disallow > ~90 degrees different incoming than current
        //if (fabs(fabs(FixAngle(animations[i]->GetIncomingBodyDirection().GetAngle2D())) - fabs(FixAngle(query.incomingBodyDirection.GetAngle2D()))) > 0.5f * pi + marginRadians) continue;
        //if (fabs(animations[i]->GetIncomingBodyDirection().GetAngle2D(Vector3(0, -1, 0)) - query.incomingBodyDirection.GetAngle2D(Vector3(0, -1, 0))) > 0.5f * pi + marginRadians) continue;
        // this version is not just moar beautiful, but also allows for -135 to 135 deg and vice versa
        if (query.incomingBodyDirection_Strict == true) {
          if (fabs(incomingBodyDir.GetAngle2D(query.incomingBodyDirection)) > marginRadians) continue;
        } else {
          if (fabs(incomingBodyDir.GetAngle2D(query.incomingBodyDirection)) > 0.5f * pi + marginRadians) continue;
        }

        // disallow > ~135 degrees between query incoming and abs outgoing body dir (kills 180 deg anims!)
        //if (fabs(outgoingBodyDir.GetAngle2D(query.incomingBodyDirection)) > 0.75f * pi + marginRadians) continue;

        if (query.incomingBodyDirection_ForceLinearity) {
          // if anim incoming body dir == between (including) query incoming and anim outgoing dir, then this anim is legal (or rather: won't look idiotic)
          // how do we check this?
          // 1. if we look at the smallest angles between (anim incoming -> query incoming) and (anim incoming -> anim outgoing), one has to be positive, the other negative.
          // 2. the (absolute) angles added up have to be < pi radians. else, we could be on the 'other side' of the 'virtual half circle' and still have the former condition met

          radian shortestAngle1 = incomingBodyDir.GetAngle2D(outgoingBodyDir);
          radian shortestAngle2 = incomingBodyDir.GetAngle2D(query.incomingBodyDirection);
          if ((shortestAngle1 >  marginRadians && shortestAngle2 >  marginRadians) ||
              (shortestAngle1 < -marginRadians && shortestAngle2 < -marginRadians)) {
            continue;
          }
          if (fabs(shortestAngle1) + fabs(shortestAngle2) > pi + marginRadians) continue;

        }

      } else if (FloatToEnumVelocity(animations[i]->GetIncomingVelocity()) == e_Velocity_Idle) {

        // allow only same angle (which is moving anims with 0 outgoing body angle. since @ idle, that will become their only angle)
        //if (fabs(animations[i]->GetIncomingBodyDirection().GetAngle2D(query.incomingBodyDirection)) > marginRadians) continue;
        if (query.incomingBodyDirection_Strict == true) {
          if (fabs(Vector3(0, -1, 0).GetAngle2D(query.incomingBodyDirection)) > marginRadians) continue;
        } else {
          if (fabs(Vector3(0, -1, 0).GetAngle2D(query.incomingBodyDirection)) > 0.25f * pi + marginRadians) continue;
        }

      }
      // no backwards body angles (test) if (fabs(animations[i]->GetIncomingBodyAngle()) > 0.5 * pi || fabs(animations[i]->GetOutgoingBodyAngle()) > 0.5 * pi) continue;
    }



    // select by INCOMING BALL DIRECTION

    if (query.byIncomingBallDirection == true) {
      Vector3 animBallDirection = GetVectorFromString(animations[i]->GetVariable("incomingballdirection"));
      if (animBallDirection.GetLength() < 0.1f) {
        Log(e_FatalError, "AnimCollection", "Crudeselection", "Anim " + animations[i]->GetName() + " missing incoming ball direction");
      }
      if (animBallDirection.GetLength() != 0.0f && query.incomingBallDirection.GetLength() != 0.0f) {

        // decimate height diff
        animBallDirection.coords[2] *= 0.4f;
        animBallDirection.Normalize();
        Vector3 adaptedIncomingBallDirection = query.incomingBallDirection;
        adaptedIncomingBallDirection.coords[2] *= 0.4f;
        adaptedIncomingBallDirection.Normalize();

        //float ballDirectionSimilarity = adaptedIncomingBallDirection.GetDotProduct(animBallDirection);// * 0.5 + 0.5;
        radian ballDirectionAngle = fabs(adaptedIncomingBallDirection.GetAngle2D(animBallDirection));
        //printf("%s\n", animations[i]->GetName().c_str());
        //query.incomingBallDirection.Print();
        //animBallDirection.Print();
        //printf("%f\n", ballDirectionDiff);
        radian maxDeviation = fabs(atof(animations[i]->GetVariable("incomingballdirection_maxdeviation").c_str()) * pi);
        if (maxDeviation == 0.0f) {
          maxDeviation = maxIncomingBallDirectionDeviation;//0.45f;
          if (animType == e_DefString_Deflect) maxDeviation = 0.4f * pi;
        }
        if (ballDirectionAngle > maxDeviation) continue;
      }
    }


    // select by OUTGOING BALL DIRECTION

    if (query.byOutgoingBallDirection == true) {
      Vector3 animBallDirection = GetVectorFromString(animations[i]->GetVariable("balldirection"));
      animBallDirection.Normalize(Vector3(0));
      //printf("%s\n", animations[i]->GetName().c_str());
      //float ballDirectionSimilarity = query.outgoingBallDirection.Get2D().GetNormalized(animBallDirection).GetDotProduct(animBallDirection);
      radian ballDirectionAngle = fabs(query.outgoingBallDirection.Get2D().GetNormalized(animBallDirection).GetAngle2D(animBallDirection));
      //query.outgoingBallDirection.Print();
      //animBallDirection.Print();
      //printf("%f\n", ballDirectionDiff);
      radian maxDeviation = fabs(atof(animations[i]->GetVariable("outgoingballdirection_maxdeviation").c_str()) * pi);
      if (maxDeviation == 0.0) {
        maxDeviation = maxOutgoingBallDirectionDeviation;
      }
      if (ballDirectionAngle > maxDeviation) continue;
    }


    // select by PROPERTIES

    if (query.properties.incoming_special_state().compare(animations[i]->GetVariableCache().incoming_special_state()) != 0) continue;
    // hax: allow switching of hands (except for deflect anims) (in future, maybe make special case for 'both hands at the same time')
    if ((query.functionType == e_FunctionType_Deflect || ((query.properties.incoming_retain_state().compare("") != 0) != (animations[i]->GetVariableCache().incoming_retain_state().compare("") != 0))) &&
        query.properties.incoming_retain_state().compare(animations[i]->GetVariableCache().incoming_retain_state()) != 0) continue;
    if (query.properties.specialvar1() != animations[i]->GetVariableCache().specialvar1()) continue;
    if (query.properties.specialvar2() != animations[i]->GetVariableCache().specialvar2()) continue;


    // select by TRIP TYPE

    if (query.byTripType == true) {
      if (int(round(atof(animations[i]->GetVariable("triptype").c_str()))) != query.tripType) continue;
    }


    // select by FORCED FOOT


    if (query.heedForcedFoot == true) {

      std::string forcedFoot = animations[i]->GetVariable("forcedfoot");
      int which = 0;
      if (forcedFoot.compare("strong") == 0) which = 1;
      else if (forcedFoot.compare("weak") == 0) which = 2;
      if (which != 0) {

        std::string touchFoot = animations[i]->GetVariable("touchfoot");
        e_Foot animFoot = e_Foot_Right;
        if (touchFoot.compare("left") == 0) animFoot = e_Foot_Left;

          // for mirrored anims that, therefore, don't start with right foot
        if (animations[i]->GetCurrentFoot() == e_Foot_Left) {
          if (animFoot == e_Foot_Left) animFoot = e_Foot_Right; else animFoot = e_Foot_Left;
        }

        if (which == 1 && query.strongFoot != animFoot) continue;
        if (which == 2 && query.strongFoot == animFoot) continue;
        //if (!selectAnim) printf("deleting %s\n", animations[i]->GetName().c_str());
      }
    }
    dataSet.push_back(i);
  }
}

int AnimCollection::GetQuadrantID(Animation *animation, const Vector3 &movement, radian angle) const {
    // assign the animation it's rightful quadrant

  Vector3 adaptedMovement = movement.GetNormalized(0) * RangeVelocity(movement.GetLength());
  int quadrantID = 0;
  float shortestDistance = 100000.0f;
  for (unsigned int i = 0; i < quadrants.size(); i++) {
    float distance = adaptedMovement.GetDistance(quadrants[i].position);
    if (distance < shortestDistance) {
      shortestDistance = distance;
      quadrantID = quadrants[i].id;
    }
  }

  return quadrantID;
}

// adds touches around main touch
int AddExtraTouches(Animation* animation, boost::intrusive_ptr<Node> playerNode, const std::list < boost::intrusive_ptr<Object> > &bodyParts, const NodeMap &nodeMap) {
  Vector3 animBallPos;
  int animTouchFrame = -1;
  bool isTouch = boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->GetFirstTouch(animBallPos, animTouchFrame);
  //printf("touchframe: %i\n", animTouchFrame);
  if (isTouch) {
    //printf("[touchframe: %i(%i); nodeMap size: %i] ", animTouchFrame, animation->GetFrameCount(), nodeMap.size());

    // find out what body part the balltouchpos is closest to
    animation->Apply(nodeMap, animTouchFrame, 0, false);

    boost::intrusive_ptr<Object> closestBodyPart = (*bodyParts.begin());
    float closestDistance = 100;
    Vector3 toBallVector = Vector3(0);
    std::list < boost::intrusive_ptr<Object> > ::const_iterator iter = bodyParts.begin();

    while (iter != bodyParts.end()) {
      float distance = (animBallPos - (*iter)->GetDerivedPosition()).GetLength();
      if (distance < closestDistance) {
        closestDistance = distance;
        closestBodyPart = *iter;
        toBallVector = animBallPos - (*iter)->GetDerivedPosition();
      }
      iter++;
    }
    //printf("closest: %s\n", closestBodyPart->GetName().c_str());
    animation->SetVariable("touch_bodypart", closestBodyPart->GetName());

    return animTouchFrame; // XDEBUG disable this

    int heightCheat = 0.0f;
    if (animBallPos.coords[2] > 0.8f) heightCheat = 2.0f;

    int range_pre = 2 + heightCheat;
    int range_post = 2 + heightCheat;

    int frameOffset = 0;

    boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->DeleteKeyFrame(animTouchFrame);

/*
    float bodypartBias = 0.9f;
    std::string animType = animation->GetVariable("type");
    if (animType.compare("ballcontrol") != 0) {
    //if (animType.compare("shortpass") == 0 || animType.compare("highpass") == 0 || animType.compare("shot") == 0) {
      bodypartBias = 0.95f;
    }
*/
    float bodypartBias = 1.0f;

    for (int i = animTouchFrame - range_pre; i < animTouchFrame + range_post + 1; i += 1) {
      if (i >= 0 && i < animation->GetFrameCount() - frameOffset - 1) {

        // set animation to this frame
        animation->Apply(nodeMap, i, 0, false);

        // find new ball position, based on the closest body part's position in this frame
        Vector3 position = closestBodyPart->GetDerivedPosition() + toBallVector;
        position.coords[2] = position.coords[2] * 0.4f + animBallPos.coords[2] * 0.6f; // take anim's height more seriously

        // correction for animation smoothing
        Vector3 origBodyPartPos = closestBodyPart->GetDerivedPosition() * bodypartBias + nodeMap[player]->GetDerivedPosition() * (1.0 - bodypartBias);

        Vector3 futureBodyPartPos = origBodyPartPos;
        // set anim to expected frame
        animation->Apply(nodeMap, i + frameOffset, 0, false);
        futureBodyPartPos = closestBodyPart->GetDerivedPosition() * bodypartBias + nodeMap[player]->GetDerivedPosition() * (1.0 - bodypartBias);

        //origBodyPos.Print();
        Vector3 diff2D = (futureBodyPartPos - origBodyPartPos).Get2D();

        Vector3 resultPosition = position + diff2D;

        // scale ballposition: this is to correct for the change in animation 'player dud' scale and actual scale (on average - since players may have different heights, of course)
        resultPosition.coords[0] = nodeMap[player]->GetDerivedPosition().coords[0] * 0.12f + resultPosition.coords[0] * 0.88f; // for some reason, anim ball pos is way too far from body (todo: investigate)
        resultPosition.coords[1] = nodeMap[player]->GetDerivedPosition().coords[1] * 0.12f + resultPosition.coords[1] * 0.88f;

        /*
        // experiment: ball more on the sides (y) to get more incoming anti-outgoingdir-movement before touch (aesthetics effect?)
        Vector3 outVec = Vector3(0, -1, 0).GetRotated2D(animation->GetOutgoingAngle() + animation->GetOutgoingBodyAngle());
        float dot = 1.0f - fabs(Vector3(0, -1, 0).GetDotProduct(outVec));
        resultPosition += outVec * dot * 0.30f;
        */

        Quaternion orientation;
        boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->SetKeyFrame(i + frameOffset, orientation, resultPosition, 0);
      }
    }
    return animTouchFrame + frameOffset;
  }

  return animTouchFrame; // default
}

float CalculateAnimDifficulty(Animation *animation, float &absoluteDifficulty) {
  Vector3 animBallPos;
  int animTouchFrame = 0;
  bool isTouch = boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->GetFirstTouch(animBallPos, animTouchFrame);

  float bodyDirDifficulty = clamp(fabs(animation->GetIncomingBodyDirection().GetAngle2D(animation->GetOutgoingBodyDirection()) / pi), 0.0, 1.0);

  float directionDifficulty = clamp(fabs(Vector3(0, -1, 0).GetAngle2D(animation->GetOutgoingDirection()) / pi), 0.0, 1.0);

  float veloChangeDifficulty = clamp(fabs(animation->GetIncomingVelocity() - animation->GetOutgoingVelocity()) / sprintVelocity, 0.0, 1.0);
  float accelDifficulty = clamp((animation->GetOutgoingVelocity() - animation->GetIncomingVelocity()) / sprintVelocity, 0.0, 1.0);
  float veloDifficulty = veloChangeDifficulty * 0.5 + accelDifficulty * 0.5; // decelerating is easier than accelerating

  float averageVelocity = clamp((animation->GetIncomingVelocity() + animation->GetOutgoingVelocity()) / (sprintVelocity * 2.0), 0.0, 1.0);
  float movementDifficulty = clamp((animation->GetIncomingMovement() -
                                    animation->GetOutgoingMovement())
                                           .GetLength() /
                                       sprintVelocity,
                                   0.0, 1.0) *
                             std::pow(averageVelocity, 2.0f);

  float bodyDirDifficultyWeight = 0.5f;
  float directionDifficultyWeight = 1.0f;
  float veloDifficultyWeight = 1.0f;
  float movementDifficultyWeight = 4.0f;

  float result = bodyDirDifficulty * bodyDirDifficultyWeight +
                 directionDifficulty * directionDifficultyWeight +
                 veloDifficulty * veloDifficultyWeight +
                 movementDifficulty * movementDifficultyWeight;

  result /= bodyDirDifficultyWeight + directionDifficultyWeight + veloDifficultyWeight + movementDifficultyWeight;

  float expectedFrameCount = 20 + result * 80;

  float absoluteDifficultyFactor = result; // towards 0.0 == easy .. towards 1.0 == hard
  absoluteDifficulty = clamp(absoluteDifficultyFactor, 0.0, 1.0);

  if (isTouch) {
    expectedFrameCount *= 1.1f;
    expectedFrameCount += 4;
  }

  absoluteDifficulty *= 0.88f;
  if (isTouch) absoluteDifficulty += 0.12f;

  expectedFrameCount = clamp(expectedFrameCount * 1.0f, 1, animation->GetEffectiveFrameCount() + 16);
  return expectedFrameCount;
}

void AnimCollection::_PrepareAnim(Animation *animation, boost::intrusive_ptr<Node> playerNode, const std::list < boost::intrusive_ptr<Object> > &bodyParts, const NodeMap &nodeMap, bool convertAngledDribbleToWalk) {

  //animation->Hax();

  Vector3 animBallPos;
  int animTouchFrame = 0;
  bool isTouch = boost::static_pointer_cast<FootballAnimationExtension>(animation->GetExtension("football"))->GetFirstTouch(animBallPos, animTouchFrame);
  if (isTouch && (animation->GetAnimType() == e_DefString_Movement || animation->GetAnimType() == e_DefString_Trip || animation->GetAnimType() == e_DefString_Special)) printf("invalid ball touch for animtype: %s\n", animation->GetName().c_str());
  if (!isTouch && (animation->GetAnimType() != e_DefString_Movement && animation->GetAnimType() != e_DefString_Trip && animation->GetAnimType() != e_DefString_Special)) printf("invalid ball touch for animtype: %s\n", animation->GetName().c_str());

  float absDiff = 0.0f;
  float expectedFrameCount = CalculateAnimDifficulty(animation, absDiff);
  animation->SetVariable("animdifficultyfactor", real_to_str(absDiff));
  //printf("diff: %f\n", absDiff);

  //Slowdown(animation, 1.0f, expectedFrameCount, true);
  //SmoothPositions(animation, convertAngledDribbleToWalk);

  int touchFrame = AddExtraTouches(animation, playerNode, bodyParts, nodeMap);
  animation->SetVariable("touchframe", int_to_str(touchFrame));


  // assign the animation its rightful quadrant

  Vector3 movement = animation->GetOutgoingMovement();
  radian angle = animation->GetOutgoingAngle();
  int quadrantID = GetQuadrantID(animation, movement, angle);

  animation->SetVariable("quadrant_id", int_to_str(quadrantID));

  animations.push_back(animation);
}

inline bool AnimCollection::_CheckFunctionType(e_DefString functionType, e_FunctionType queryFunctionType) const {
  switch (queryFunctionType) {

    case e_FunctionType_Movement:
      return functionType == e_DefString_Movement;

    case e_FunctionType_BallControl:
      return functionType == e_DefString_BallControl;

    case e_FunctionType_Trap:
      return functionType == e_DefString_Trap;

    case e_FunctionType_ShortPass:
      return functionType == e_DefString_ShortPass;

    case e_FunctionType_LongPass:
      return functionType == e_DefString_LongPass;

    case e_FunctionType_HighPass:
      return functionType == e_DefString_HighPass;

    case e_FunctionType_Shot:
      return functionType == e_DefString_Shot;

    case e_FunctionType_Deflect:
      return functionType == e_DefString_Deflect;

    case e_FunctionType_Catch:
      return functionType == e_DefString_Catch;

    case e_FunctionType_Interfere:
      return functionType == e_DefString_Interfere;

    case e_FunctionType_Trip:
      return functionType == e_DefString_Trip;

    case e_FunctionType_Sliding:
      return functionType == e_DefString_Sliding;

    case e_FunctionType_Special:
      return functionType == e_DefString_Special;

    default:
      return false;
  }
}
