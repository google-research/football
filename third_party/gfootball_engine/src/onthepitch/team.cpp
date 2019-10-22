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
// this work is public domain. the code is undocumented, scruffy, untested, and
// should generally not be used for anything important. i do not offer support,
// so don't ask. to be used for inspiration :)

#include "team.hpp"

#include "../gamedefines.hpp"
#include "../main.hpp"
#include "../utils.hpp"
#include "AIsupport/AIfunctions.hpp"
#include "match.hpp"

Team::Team(int id, Match *match, TeamData *teamData, float aiDifficulty,
           int side)
    : id(id), match(match), teamData(teamData), aiDifficulty(aiDifficulty),
      side(side) {
  assert(id == 0 || id == 1);
  // assert(teamData->GetPlayerNum() >= playerNum); // does team have enough
  // players?

  teamNode =
      boost::intrusive_ptr<Node>(new Node("team node #" + int_to_str(id)));
  teamNode->SetLocalMode(e_LocalMode_Absolute);
  match->GetDynamicNode()->AddNode(teamNode);

  teamController = new TeamAIController(this);

  timeNeededToGetToBall_ms = 100;
  hasPossession = false;

  teamPossessionAmount = 1.0;
  fadingTeamPossessionAmount = 1.0;
}

Team::~Team() {}

void Team::Mirror() {
  side *= -1;
  for (auto& p : players) {
    p->Mirror();
  }
}

void Team::Exit() {
  Hide2D();

  for (unsigned int i = 0; i < humanGamers.size(); i++) {
    delete humanGamers[i];
  }
  for (unsigned int i = 0; i < players.size(); i++) {
    delete players[i];
  }

  delete teamController;

  playerNode->Exit();
  playerNode.reset();

  match->GetDynamicNode()->DeleteNode(teamNode);
}

void Team::InitPlayers(boost::intrusive_ptr<Node> fullbodyNode,
                       std::map<Vector3, Vector3> &colorCoords) {
  // first, load 1 instance of a player

  ObjectLoader loader;
  playerNode = loader.LoadObject("media/objects/players/player.object");
  playerNode->SetName("player");
  playerNode->SetLocalMode(e_LocalMode_Absolute);

  // load all players in the team, even the players who sit on the bench. aww.
  for (int i = 0; i < (signed int)teamData->GetPlayerNum(); i++) {
    PlayerData *playerData = teamData->GetPlayerData(i);
    Player *player = new Player(this, playerData);
    players.push_back(player);

    if (i < playerNum) {
      // activate playerCount players (the starting eleven, usually)
      std::string kitFilename;
      // printf("%i player id\n", player->GetID());
      auto formation = GetFormationEntry(player);
      if (formation.role != e_PlayerRole_GK) {
        kitFilename = GetTeamData()->GetKitUrl() + "_kit_0" +
                      int_to_str(GetMenuTask()->GetTeamKitNum(GetID())) +
                      ".png";
      } else {
        kitFilename = "media/objects/players/textures/goalie_kit.png";
      }
      kit = GetContext().surface_manager.Fetch(kitFilename);
      player->Activate(
          playerNode, fullbodyNode,
          colorCoords, kit, match->GetAnimCollection(), formation.lazy);
    }
  }

  designatedTeamPossessionPlayer = players.at(0);
}

signed int Team::GetSide() {
  return side;
}

FormationEntry Team::GetFormationEntry(void* player) {
  for (int i = 0; i < (signed int)players.size(); i++) {
    if (players[i] == player) {
      return teamData->GetFormationEntry(i);
    }
  }

  assert(1 == 2);
  FormationEntry fail;
  return fail;
}

void Team::SetFormationEntry(Player* player, FormationEntry entry) {
  for (int i = 0; i < (signed int)players.size(); i++) {
    if (players[i] == player) {
      teamData->SetFormationEntry(i, entry);
    }
  }
}

void Team::GetActivePlayers(std::vector<Player *> &activePlayers) {
  for (auto player : players) {
    if (player->IsActive()) activePlayers.push_back(player);
  }
}

int Team::GetActivePlayersCount() const {
  int count = 0;
  for (auto player : players) {
    if (player->IsActive()) count++;
  }
  return count;
}

void Team::AddHumanGamer(IHIDevice *hid, e_PlayerColor color) {
  HumanGamer *humanGamer = new HumanGamer(this, hid, color);

  humanGamers.push_back(humanGamer);

  Player *player =
      AI_GetClosestPlayer(this, match->GetBall()->Predict(0).Get2D(), true);
  if (player) {
    humanGamer->SetSelectedPlayer(player);
  }
  UpdateDesignatedTeamPossessionPlayer();
  switchPriority.push_back(humanGamers.size() - 1);
}

void Team::UpdateDesignatedTeamPossessionPlayer() {
  designatedTeamPossessionPlayer =
      AI_GetClosestPlayer(this, match->GetBall()->Predict(0).Get2D(), false);
}

void Team::DeleteHumanGamers() {
  for (unsigned int i = 0; i < humanGamers.size(); i++) {
    delete humanGamers[i];
  }
  humanGamers.clear();
  switchPriority.clear();
}

e_PlayerColor Team::GetPlayerColor(PlayerBase* player) {
  for (unsigned int h = 0; h < humanGamers.size(); h++) {
    if (humanGamers.at(h)->GetSelectedPlayer() == player)
      return humanGamers.at(h)->GetPlayerColor();
  }
  return e_PlayerColor_Default;
}

bool Team::IsHumanControlled(PlayerBase* player) {
  for (unsigned int h = 0; h < humanGamers.size(); h++) {
    if (humanGamers.at(h)->GetSelectedPlayer() == player) return true;
  }
  return false;
}

int Team::HumanControlledToBallDistance() {
  int timeToBall = 10000;
  for (auto human : humanGamers) {
    if (human->GetSelectedPlayer()) {
      timeToBall =
          std::min(timeToBall,
                   human->GetSelectedPlayer()->GetTimeNeededToGetToBall_ms());
    }
  }
  return timeToBall;
}

bool Team::HasPossession() const { return hasPossession; }

bool Team::HasUniquePossession() const {
  return HasPossession() && !match->GetTeam(1 - id)->HasPossession();
}

int Team::GetTimeNeededToGetToBall_ms() const {
  return timeNeededToGetToBall_ms;
}

Player *Team::GetBestPossessionPlayer() {
  int bestTime_ms = 10000000;
  Player *bestPlayer = 0;
  for (auto p : players) {
    if (p->IsActive()) {
      int time_ms = p->GetTimeNeededToGetToBall_ms();
      if (time_ms < bestTime_ms) {
        bestTime_ms = time_ms;
        bestPlayer = p;
      }
    }
  }

  assert(bestPlayer);

  return bestPlayer;
}

float Team::GetTeamPossessionAmount() const { return teamPossessionAmount; }

float Team::GetFadingTeamPossessionAmount() const {
  return fadingTeamPossessionAmount;
}

void Team::SetFadingTeamPossessionAmount(float value) {
  fadingTeamPossessionAmount = clamp(value, 0.5, 1.5);
}

void Team::SetLastTouchPlayer(Player *player, e_TouchType touchType) {
  lastTouchPlayer = player;
  player->SetLastTouchTime_ms(match->GetActualTime_ms());
  player->SetLastTouchType(touchType);
  match->SetLastTouchTeamID(GetID(), touchType);
}

void Team::ResetSituation(const Vector3 &focusPos) {
  timeNeededToGetToBall_ms = 100;
  hasPossession = false;

  teamPossessionAmount = 1.0f;
  fadingTeamPossessionAmount = 1.0f;
  lastTouchPlayer = 0;

  designatedTeamPossessionPlayer = players.at(0);

  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i]->IsActive()) {
      players[i]->ResetSituation(focusPos);
    }
  }

  GetController()->Reset();
}

void Team::HumanGamersSelectAnyone() {
  // make sure all human gamers have a player selected

  if (match->IsInPlay()) {
    for (unsigned int i = 0; i < humanGamers.size(); i++) {
      if (!humanGamers[i]->GetSelectedPlayer()) {
        Player *player = AI_GetClosestPlayer(
            this, match->GetBall()->Predict(0).Get2D(), true);
        if (player) {
          humanGamers[i]->SetSelectedPlayer(player);
        }
      }
    }
  }
}

void Team::SelectPlayer(Player *player) {
  if (!IsHumanControlled(player) && humanGamers.size() != 0) {  // already selected
    humanGamers.at(*switchPriority.begin())->SetSelectedPlayer(player);
    switchPriority.push_back(*switchPriority.begin());
    switchPriority.pop_front();
  }
  designatedTeamPossessionPlayer = player;
}

void Team::DeselectPlayer(Player *player) {
  for (unsigned int i = 0; i < humanGamers.size(); i++) {
    Player* selectedPlayer = humanGamers[i]->GetSelectedPlayer();
    if (selectedPlayer == player) {
      Player *somePlayer =
          AI_GetClosestPlayer(this, player->GetPosition(), true, player);
      if (somePlayer) {
        humanGamers[i]->SetSelectedPlayer(somePlayer);
      } else {
        humanGamers[i]->SetSelectedPlayer(0);
      }
    }
  }
}

void Team::RelaxFatigue(float howMuch) {
  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i]->IsActive()) {
      players[i]->RelaxFatigue(howMuch);
    }
  }
}

void Team::Process() {
  if (!match->GetPause()) {
    teamPossessionAmount = (float)(match->GetTeam(abs(GetID() - 1))
                                       ->GetTimeNeededToGetToBall_ms() +
                                   1500) /
                           (float)(GetTimeNeededToGetToBall_ms() + 1500);
    float tmpFadingTeamPossessionAmount =
        fadingTeamPossessionAmount * 0.995f +
        clamp(teamPossessionAmount, 0.5f, 1.5f) * 0.005f;
    fadingTeamPossessionAmount +=
        clamp(tmpFadingTeamPossessionAmount - fadingTeamPossessionAmount,
              -0.005f, 0.005f);  // maximum change per 10ms

    if (!match->IsInPlay() || match->IsInSetPiece() ||
        match->GetBallRetainer() != 0) {
      if (match->GetBallRetainer() != 0) {
        fadingTeamPossessionAmount = teamPossessionAmount =
            (match->GetBallRetainer()->GetTeam() == this) ? 1.5f : 0.5f;
      } else {
        fadingTeamPossessionAmount = teamPossessionAmount =
            (match->GetBestPossessionTeam() == this) ? 1.5f : 0.5f;
      }
    }

    HumanGamersSelectAnyone();

    if (match->IsInPlay() && !match->IsInSetPiece()) {
      teamController->Process();

      int team_offset = id == match->SecondTeam() ? 200 : 0;
      if ((match->GetActualTime_ms() + team_offset) % 400 == 0) {
        teamController->CalculateDynamicRoles();
      }

      if ((match->GetActualTime_ms() + team_offset + 100) % 400 == 0) {
        teamController->CalculateManMarking();
      }
    }

    for (unsigned int i = 0; i < players.size(); i++) {
      if (players[i]->IsActive()) {
        players[i]->Process();
      }
    }

    if (match->IsInPlay()) {
      for (unsigned int i = 0; i < humanGamers.size(); i++) {
        // switch button
        Player *selectedPlayer = humanGamers[i]->GetSelectedPlayer();
        if (humanGamers[i]->GetHIDevice()->GetButton(e_ButtonFunction_Switch) &&
            !humanGamers[i]->GetHIDevice()->GetPreviousButtonState(
                e_ButtonFunction_Switch) &&
            // don't switch if we are both best AND designated possession
            // player. unless opponent team has ball.
            (!(selectedPlayer == GetBestPossessionPlayer() &&
               selectedPlayer == designatedTeamPossessionPlayer) ||
             GetTeamPossessionAmount() < 1.0f) &&
            !selectedPlayer->HasUniquePossession()) {
          Player *targetPlayer = 0;

          if (!IsHumanControlled(designatedTeamPossessionPlayer) &&
              match->GetBestPossessionTeam() == this) {
            targetPlayer = designatedTeamPossessionPlayer;
          } else if (!IsHumanControlled(GetBestPossessionPlayer()) &&
                     match->GetBestPossessionTeam() == this) {
            targetPlayer = GetBestPossessionPlayer();
          } else {
            targetPlayer = AI_GetBestSwitchTargetPlayer(
                match, this, humanGamers[i]->GetHIDevice()->GetDirection());
            if (targetPlayer)
              if (IsHumanControlled(targetPlayer)) targetPlayer = 0;
          }
          if (targetPlayer == GetGoalie())
            targetPlayer =
                0;  // can not be goalie in current version, at least not during
                    // play, unless being directly passed to by teammate

          if (targetPlayer)
            humanGamers[i]->SetSelectedPlayer(targetPlayer);
        }
      }

    } else {
      // make sure all human gamers don't have a player selected

      for (unsigned int i = 0; i < humanGamers.size(); i++) {
        if (humanGamers[i]->GetSelectedPlayer()) {
          humanGamers[i]->SetSelectedPlayer(0);
        }
      }
    }

    int designatedPlayerTime_ms =
        designatedTeamPossessionPlayer->GetTimeNeededToGetToBall_ms();
    Player *bestPlayer = GetBestPossessionPlayer();
    int oppTime_ms =
        match->GetTeam(abs(GetID() - 1))->GetTimeNeededToGetToBall_ms();
    if (designatedTeamPossessionPlayer != bestPlayer) {
      // switch only if other player is somewhat better, to overcome
      // possession-chaos
      int bestPlayerTime_ms = bestPlayer->GetTimeNeededToGetToBall_ms();
      float timeRating = (float)(bestPlayerTime_ms + 500) /
                         (float)(designatedPlayerTime_ms + 500);

      if (bestPlayer->HasPossession()) timeRating *= 0.5f;
      if (designatedTeamPossessionPlayer->HasPossession()) timeRating /= 0.5f;

      if (IsHumanControlled(bestPlayer)) timeRating *= 0.8f;
      if (IsHumanControlled(designatedTeamPossessionPlayer))
        timeRating /= 0.8f;

      // current player can get to the ball before the closest opponent: less
      // need to switch
      // if (GetID() == 0) printf("opptime: %i, designated time: %i\n",
      // oppTime_ms, designatedPlayerTime_ms);
      if (IsHumanControlled(bestPlayer) == false &&
          designatedPlayerTime_ms < oppTime_ms - 100) {
        timeRating += 0.2f;
        timeRating *= 1.2f;
      }

      if (timeRating < 0.8f) {
        designatedTeamPossessionPlayer = bestPlayer;
      }
    }
  }
}

void Team::PreparePutBuffers() {
  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i]->IsActive()) {
      players[i]->PreparePutBuffers();
    }
  }
}

void Team::FetchPutBuffers() {
  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i]->IsActive()) {
      players[i]->FetchPutBuffers();
    }
  }
}

void Team::Put(bool mirror) {
  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i]->IsActive()) {
      players[i]->Put(mirror);
    }
  }
}

void Team::Put2D(bool mirror) {
  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i]->IsActive()) {
      players[i]->Put2D(mirror);
    }
  }
}

void Team::Hide2D() {
  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i]->IsActive()) {
      players[i]->Hide2D();
    }
  }
}

void Team::UpdatePossessionStats() {
  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i]->IsActive()) {
      players[i]->UpdatePossessionStats();
    }
  }

  // possession?

  hasPossession = false;
  timeNeededToGetToBall_ms = 100000;
  for (int i = 0; i < (signed int)players.size(); i++) {
    if (players[i]->IsActive()) {
      if (players[i]->HasPossession()) hasPossession = true;
      if (players[i]->GetTimeNeededToGetToBall_ms() < timeNeededToGetToBall_ms)
        timeNeededToGetToBall_ms = players[i]->GetTimeNeededToGetToBall_ms();
    }
  }
}

void Team::UpdateSwitch() {
  // lose turn on ball possession

  if (match->IsInPlay() && humanGamers.size() > 1) {
    int myTurn = *switchPriority.begin();
    if (humanGamers.at(myTurn)->GetSelectedPlayer() ==
        match->GetDesignatedPossessionPlayer()) {
      switchPriority.pop_front();
      switchPriority.push_back(myTurn);
    }
  }

  // autoswitch on proximity

  if (match->IsInPlay() && humanGamers.size() > 0) {
    if (!IsHumanControlled(designatedTeamPossessionPlayer) &&
        3 * designatedTeamPossessionPlayer->GetTimeNeededToGetToBall_ms() <
            HumanControlledToBallDistance() &&
        designatedTeamPossessionPlayer->GetFormationEntry().role !=
            e_PlayerRole_GK) {
      SelectPlayer(designatedTeamPossessionPlayer);
    }
  }

  // team player in possession is not human selected

  if (match->IsInPlay() && humanGamers.size() > 0) {
    if (!IsHumanControlled(designatedTeamPossessionPlayer) &&
        (designatedTeamPossessionPlayer->HasUniquePossession() ||
         match->IsInSetPiece())) {
      if (designatedTeamPossessionPlayer != GetGoalie()) {
        SelectPlayer(designatedTeamPossessionPlayer);
      }
    }
  }
}

Player *Team::GetGoalie() {
  for (unsigned int i = 0; i < players.size(); i++) {
    if (players[i]->IsActive()) {
      if (players[i]->GetFormationEntry().role == e_PlayerRole_GK)
        return players[i];
    }
  }

  return 0;
}

void Team::ProcessState(EnvState* state) {
  state->process(hasPossession);
  state->process(timeNeededToGetToBall_ms);
  state->process(designatedTeamPossessionPlayer);
  state->process(teamPossessionAmount);
  state->process(fadingTeamPossessionAmount);
  teamController->ProcessState(state);
  for (auto &g : humanGamers) {
    g->ProcessState(state);
  }
  state->process(switchPriority);
  state->process(lastTouchPlayer);
}
