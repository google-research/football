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

#include "goalie_default.hpp"

#include "../../../../../base/geometry/triangle.hpp"

#include "../../../../../main.hpp"

GoalieDefaultStrategy::GoalieDefaultStrategy(ElizaController *controller) : Strategy(controller) {
  name = "goalie default";

  ballBoundForGoal_ycoord = 0.0f;
}

GoalieDefaultStrategy::~GoalieDefaultStrategy() {
}

void GoalieDefaultStrategy::RequestInput(const MentalImage *mentalImage, Vector3 &direction, float &velocity) {

  // base position
  float lineDistance = 10.0f; // default distance keeper stays in front of goal line
  Vector3 ballPos = mentalImage->GetBallPrediction(600 + CastPlayer()->GetTimeNeededToGetToBall_ms() * 0.2f).Get2D();
  Vector3 targetPos = Vector3((pitchHalfW - lineDistance) * team->GetSide(), 0, 0);
  Vector3 goalPos = Vector3(pitchHalfW * team->GetSide(), 0, 0);

  float maxVelocity = sprintVelocity;

  if (ballPos.coords[0] * team->GetSide() > 0) { // optimization

    CalculateIfBallIsBoundForGoal(mentalImage);

    if (!IsBallBoundForGoal()) {

      // tactical position, make goal as small as possible


      maxVelocity = sprintVelocity;//walkVelocity;

      // first, make line from ballPos to one post, then one to the other post, then calculate the line in between.
      // this line is the line we want our goalie to be on: it splits the goal in to equal halves (in the 'ball view projection', that is)
      Vector3 toPost1 = Vector3(pitchHalfW * team->GetSide(),  3.7f, 0) - ballPos;
      Vector3 toPost2 = Vector3(pitchHalfW * team->GetSide(), -3.7f, 0) - ballPos;
      radian angle = toPost2.GetAngle2D(toPost1);
      Vector3 middle = toPost1.GetRotated2D(angle * 0.5f).GetNormalized(Vector3(team->GetSide(), 0, 0));
      Line ballToGoal;
      ballToGoal.SetVertex(0, ballPos);
      ballToGoal.SetVertex(1, ballPos + middle);

      // this line is now arbitrary length - make it so long that v2 is on the backline
      // (or rather, near the backline - keeping ON the backline is dangerous; some anims may only touch the ball when it's already behind the line, which is pretty useless :p)
      Line backLine;
      backLine.SetVertex(0, Vector3((pitchHalfW - 0.7f) * team->GetSide(), -pitchHalfH, 0));
      backLine.SetVertex(1, Vector3((pitchHalfW - 0.7f) * team->GetSide(),  pitchHalfH, 0));
      Vector3 intersect = ballToGoal.GetIntersectionPoint(backLine).Get2D();
      intersect.coords[1] = clamp(intersect.coords[1], -3.7f, 3.7f);
      ballToGoal.SetVertex(1, intersect);

      float awayFromGoalOffset_m = 0.7f; // meters away from goal line (over the ballToGoal line, not straight forward)
      float awayFromGoalBias = 0.3f; // factor between goal and ball
      awayFromGoalBias *= NormalizedClamp(controller->GetFadingTeamPossessionAmount(), 1.0f, 1.5f);


      // when opponent comes rushing in and team mates are too far away to help, come out to 'reduce goal size'

      if (controller->GetFadingTeamPossessionAmount() < 1.0f) {

        Player *opp = controller->GetOppTeam()->GetDesignatedTeamPossessionPlayer();
        Vector3 oppPos = opp->GetPosition() + opp->GetMovement() * 0.32f;

        // if opp isn't in ball control, don't use ball pos but opp pos
        if (opp->HasPossession() == false) {
          ballToGoal.SetVertex(0, oppPos * 0.6f + ballPos * 0.4f);
        } else {
          ballToGoal.SetVertex(0, oppPos * 0.4f + ballPos * 0.6f);
        }

        // first, calculate how close the opponent on the ball is to the goal/shooting treshold
        float shootThreshold = 20.0f; // average/base value; this distance is dynamic
        float oppToGoalDistance = (goalPos - oppPos).GetLength();
        float oppToThresholdDistance = clamp(oppToGoalDistance - shootThreshold * NormalizedClamp(oppToGoalDistance, 0.0f, shootThreshold * 2.0f), 0.0f, pitchHalfW); // variable threshold distance
        Vector3 shootingPoint = oppPos + (goalPos - oppPos).GetNormalized(0) * oppToThresholdDistance;
        //SetGreenDebugPilon(shootingPoint);

        // now calculate the distance between this shooting point and our closest mate
        Player *mate = AI_GetClosestPlayer(team, shootingPoint, false, CastPlayer());
        float mateToThresholdDistance = 99999;
        if (mate) {
          Vector3 matePos = mate->GetPosition() + mate->GetMovement() * 0.24f;
          mateToThresholdDistance = (shootingPoint - matePos).GetLength();
        }

        if (mateToThresholdDistance > oppToThresholdDistance + 1.0f) { // come out, brave keeper!

          awayFromGoalBias = 1.0f;

          // the amount of 'come out bias' is related to how dangerous the opponent's closest mate is if they are to receive the ball.
          // basically, the same as the above code, but with the secondary opponent and mate
          Player *oppHelper = AI_GetClosestPlayer(controller->GetOppTeam(), goalPos, false, opp);
          if (oppHelper) {

            Vector3 oppHelperPosition = oppHelper->GetPosition() + oppHelper->GetMovement() * 0.32f;

            // first, calculate how close the opponent helper is to the goal/shooting treshold
            float helperShootThreshold = 24.0f; // average/base value; this distance is dynamic
            float oppHelperToGoalDistance = (goalPos - oppHelperPosition).GetLength();
            float oppHelperToThresholdDistance = clamp(oppHelperToGoalDistance - helperShootThreshold * NormalizedClamp(oppHelperToGoalDistance, 0.0f, helperShootThreshold * 2.0f), 0.0f, pitchHalfW); // variable threshold distance
            Vector3 helperShootingPoint = oppHelperPosition + (goalPos - oppHelperPosition).GetNormalized(0) * oppHelperToThresholdDistance;
            //SetYellowDebugPilon(helperShootingPoint);

            // now calculate the distance between this shooting point and our closest mate
            Player *mateHelper = AI_GetClosestPlayer(team, helperShootingPoint, false, CastPlayer());
            float mateHelperToThresholdDistance = 99999;
            if (mateHelper) mateHelperToThresholdDistance = (helperShootingPoint - (mateHelper->GetPosition() + mateHelper->GetMovement() * 0.24f)).GetLength();

            float secondaryDistanceDiff = 0.0f;
            // if this var is bigger, LESS likely to come out because of secondary danger
            if (mateHelperToThresholdDistance > oppHelperToThresholdDistance) secondaryDistanceDiff = NormalizedClamp(mateHelperToThresholdDistance - oppHelperToThresholdDistance, 0.0f, 2.0f);

            // also take into account the ratio between the primary opp to goal and the helper opp to goal distance
            // if this var is bigger, LESS likely to come out because of secondary danger
            float helperVSPrimaryDistanceRatio = 1.0f - NormalizedClamp(oppHelperToThresholdDistance / (oppToThresholdDistance + 0.0001f), 1.0f, 1.5f);
            helperVSPrimaryDistanceRatio *= 0.7f; // always allow some coming out despite opp mate danger

            awayFromGoalBias = clamp(1.0f - (secondaryDistanceDiff * helperVSPrimaryDistanceRatio), 0.0f, 1.0f);
          }

        }

      } // end keeper come out code

      bool applyRushOut = team->GetController()->GetEndApplyKeeperRush_ms() > match->GetActualTime_ms();
      if (applyRushOut) awayFromGoalBias = 1.0f;

      float distance = std::max(ballToGoal.GetLength() - 0.5f, 0.0f); // keep distance from target, we don't want to overshoot
      awayFromGoalOffset_m = clamp(distance * awayFromGoalBias, awayFromGoalOffset_m, pitchHalfW); // when ball is farther away, goalie stays farther away from goal (to make runs when necessary)

      // offset from goal line
      targetPos = ballToGoal.GetVertex(1) + (ballToGoal.GetVertex(0) - ballToGoal.GetVertex(1)).GetNormalized(0) * awayFromGoalOffset_m;
      //SetGreenDebugPilon(targetPos);

/* disabled: just rush to ball even if we probably can't make it there. this stuff doesn't work good enough (yet?)
      // time we assume it will take for the ball to arrive at some point (in other words: how fast the attacker gets the ball there)
      float time1_sec = (targetPos - ballToGoal.GetVertex(0)).GetLength() / sprintVelocity;
      float time2_sec = (ballToGoal.GetVertex(1) - ballToGoal.GetVertex(0)).GetLength() / sprintVelocity;

      //SetYellowDebugPilon(targetPos);
      targetPos = CalculateBestAchievableTarget(CastPlayer(), targetPos, time1_sec, ballToGoal.GetVertex(1), time2_sec);
      SetRedDebugPilon(targetPos + Vector3(0, 0, 0.1f));
*/

      // when going back to goal: go slower to allow for proper body direction
      float u = 0.0f;
      float distanceToBallToGoalLine = ballToGoal.GetDistanceToPoint(player->GetPosition(), u);
      if ((targetPos - goalPos).GetLength() < (player->GetPosition() - goalPos).GetLength() &&
          distanceToBallToGoalLine < 1.0f && u > 0.0f) maxVelocity = walkVelocity;

      targetPos.coords[0] = clamp(targetPos.coords[0], -pitchHalfW + 0.2f, pitchHalfW - 0.2f); // not very useful to stand behind backline

    } else {

      // intercept ball


      maxVelocity = sprintVelocity;

      Line ballToGoal;
      ballToGoal.SetVertex(0, mentalImage->GetBallPrediction(10).Get2D());
      float minGoalLineDist = 0.4f;
      Vector3 ballOverGoalLinePos = Vector3(pitchHalfW * team->GetSide(), ballBoundForGoal_ycoord, 0);
      ballOverGoalLinePos += (ballToGoal.GetVertex(0) - ballOverGoalLinePos).GetNormalized(0) * minGoalLineDist;
      ballToGoal.SetVertex(1, ballOverGoalLinePos);
      float u = 0.0f;
      float distance = ballToGoal.GetDistanceToPoint(player->GetPosition() + player->GetMovement() * 0.05f, u);

      float u_at_1sec = 0.0f;
      float distance_at_1sec = ballToGoal.GetDistanceToPoint(mentalImage->GetBallPrediction(1010).Get2D(), u_at_1sec);

      bool should_gk_run_towards_the_goal = false;
      if (u_at_1sec > 1e-4) {
        float time_to_reach_gk = u / u_at_1sec;
        Vector3 ball_position_at_gk = mentalImage->GetBallPrediction(10 + 1000 * time_to_reach_gk);
        if (ball_position_at_gk.coords[2] > 2.5) {
          should_gk_run_towards_the_goal = true;
        }
      }

      u = clamp(u, 0.0f, 1.0f);

      if (should_gk_run_towards_the_goal) {
        targetPos = ballOverGoalLinePos;
      } else {
        targetPos = ballToGoal.GetVertex(0) + (ballToGoal.GetVertex(1) - ballToGoal.GetVertex(0)) * u;
        targetPos.coords[2] = 0.0;
        targetPos.coords[0] = clamp(targetPos.coords[0], -pitchHalfW + 0.2f, pitchHalfW - 0.2f); // not very useful to stand behind line
      }
    }
  }

  direction = targetPos - player->GetPosition();
  velocity = clamp(direction.GetLength() * distanceToVelocityMultiplier, idleVelocity, maxVelocity);
  direction.Normalize(player->GetDirectionVec());
}

void GoalieDefaultStrategy::CalculateIfBallIsBoundForGoal(const MentalImage *mentalImage) {

  ballBoundForGoal = false;
  bool intersect = false;
  ballBoundForGoal_ycoord = 0;

  int side = team->GetSide();

  float panic = 1.02f + (1.0f - (CastPlayer()->GetStat(mental_defensivepositioning) * 0.6f + CastPlayer()->GetStat(mental_vision) * 0.4f)) * 0.5f;
  if (mentalImage->GetBallPrediction(4000).coords[0] * side > pitchHalfW && (player->GetPosition() - mentalImage->GetBallPrediction(250)).GetLength() < 32.0f) { // only if ball is close enough (cpu optimization)

/* 3d version
    Line line;
    line.SetVertex(0, mentalImage->GetBallPrediction(40));
    line.SetVertex(1, mentalImage->GetBallPrediction(4000));
    //line.SetVertex(1, mentalImage->GetBallPrediction(0) + match->GetBall()->GetMovement() * 800);
    Triangle goal1;
    goal1.SetVertex(0, Vector3((pitchHalfW - 0.0) * side,  3.7 * panic, 0));
    goal1.SetVertex(1, Vector3((pitchHalfW - 0.0) * side, -3.7 * panic, 0));
    goal1.SetVertex(2, Vector3((pitchHalfW - 0.0) * side,  3.7 * panic, 2.5 * panic));
    goal1.SetNormals(Vector3(-side, 0, 0));
    Triangle goal2;
    goal2.SetVertex(0, Vector3((pitchHalfW - 0.0) * side, -3.7 * panic, 0));
    goal2.SetVertex(1, Vector3((pitchHalfW - 0.0) * side, -3.7 * panic, 2.5 * panic));
    goal2.SetVertex(2, Vector3((pitchHalfW - 0.0) * side,  3.7 * panic, 2.5 * panic));
    goal2.SetNormals(Vector3(-side, 0, 0));

    //match->SetDebugPilon(Vector3(55 * side, 3.66, 2.44));
    //match->SetDebugPilon2(line.GetVertex(1));

    Vector3 intersectVec;
    intersect = goal1.IntersectsLine(line, intersectVec);
    if (!intersect) {
      intersect = goal2.IntersectsLine(line, intersectVec);
    }
*/

    // 2d version

    Line ballToGoal;
    ballToGoal.SetVertex(0, mentalImage->GetBallPrediction(0).Get2D());
    ballToGoal.SetVertex(1, mentalImage->GetBallPrediction(800).Get2D());
    Line goalLine;
    goalLine.SetVertex(0, Vector3(pitchHalfW * side, -pitchHalfH, 0));
    goalLine.SetVertex(1, Vector3(pitchHalfW * side,  pitchHalfH, 0));

    Vector3 intersectPoint = ballToGoal.GetIntersectionPoint(goalLine).Get2D();
    if (fabs(intersectPoint.coords[1]) > 3.7 * panic) intersect = false; else intersect = true;

    if (intersect) {
      //SetGreenDebugPilon(intersectPoint);
      ballBoundForGoal_ycoord = intersectPoint.coords[1];
      ballBoundForGoal = true;
    } else {
      //SetGreenDebugPilon(Vector3(0, 0, -10));
    }
  }

}
