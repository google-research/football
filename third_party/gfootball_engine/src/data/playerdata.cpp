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

#include "playerdata.hpp"

#include <cmath>

#include "../base/utils.hpp"

#include "../main.hpp"

PlayerStat PlayerStatFromString(const std::string& name) {
  if (name == "physical_balance") return physical_balance;
  if (name == "physical_reaction") return physical_reaction;
  if (name == "physical_acceleration") return physical_acceleration;
  if (name == "physical_velocity") return physical_velocity;
  if (name == "physical_stamina") return physical_stamina;
  if (name == "physical_agility") return physical_agility;
  if (name == "physical_shotpower") return physical_shotpower;
  if (name == "technical_standingtackle") return technical_standingtackle;
  if (name == "technical_slidingtackle") return technical_slidingtackle;
  if (name == "technical_ballcontrol") return technical_ballcontrol;
  if (name == "technical_dribble") return technical_dribble;
  if (name == "technical_shortpass") return technical_shortpass;
  if (name == "technical_highpass") return technical_highpass;
  if (name == "technical_header") return technical_header;
  if (name == "technical_shot") return technical_shot;
  if (name == "technical_volley") return technical_volley;
  if (name == "mental_calmness") return mental_calmness;
  if (name == "mental_workrate") return mental_workrate;
  if (name == "mental_resilience") return mental_resilience;
  if (name == "mental_defensivepositioning") return mental_defensivepositioning;
  if (name == "mental_offensivepositioning") return mental_offensivepositioning;
  if (name == "mental_vision") return mental_vision;
  if (name == "player_stat_max") return player_stat_max;
  Log(e_FatalError, "PlayerStatFromString", "", name);
  return player_stat_max;
}

PlayerData::PlayerData(int playerDatabaseID) : databaseID(playerDatabaseID) {

  std::string profileString;
  float baseStat = 0.0f;
  int age = 15;

  skinColor = int(std::round(boostrandom(1, 4)));
  hairStyle = "short01";
  hairColor = "darkblonde";
  height = 1.8f;

  switch (playerDatabaseID) {
    case 398:
      firstName = "Marc-Andr�";
      lastName = "ten Stegosaur";
      baseStat = 0.661913;
      profileString =
          "<physical_balance>0.650000</"
          "physical_balance><physical_reaction>0.850000</"
          "physical_reaction><physical_acceleration>0.550000</"
          "physical_acceleration><physical_velocity>0.450000</"
          "physical_velocity><physical_stamina>0.550000</"
          "physical_stamina><physical_agility>0.650000</"
          "physical_agility><physical_shotpower>0.650000</"
          "physical_shotpower><technical_standingtackle>0.250000</"
          "technical_standingtackle><technical_slidingtackle>0.250000</"
          "technical_slidingtackle><technical_ballcontrol>0.350000</"
          "technical_ballcontrol><technical_dribble>0.250000</"
          "technical_dribble><technical_shortpass>0.650000</"
          "technical_shortpass><technical_highpass>0.750000</"
          "technical_highpass><technical_header>0.250000</"
          "technical_header><technical_shot>0.350000</"
          "technical_shot><technical_volley>0.250000</"
          "technical_volley><mental_calmness>0.550000</"
          "mental_calmness><mental_workrate>0.550000</"
          "mental_workrate><mental_resilience>0.550000</"
          "mental_resilience><mental_defensivepositioning>0.950000</"
          "mental_defensivepositioning><mental_offensivepositioning>0.150000</"
          "mental_offensivepositioning><mental_vision>0.550000</mental_vision>";
      age = 22;
      skinColor = 1;
      hairStyle = "short02";
      hairColor = "blonde";
      height = 1.87;
      break;
    case 11:
      firstName = "Jordi";
      lastName = "Alblabla";
      baseStat = 0.619111;
      profileString =
          "<physical_balance>0.569697</"
          "physical_balance><physical_reaction>0.536364</"
          "physical_reaction><physical_acceleration>0.536364</"
          "physical_acceleration><physical_velocity>0.536364</"
          "physical_velocity><physical_stamina>0.469697</"
          "physical_stamina><physical_agility>0.536364</"
          "physical_agility><physical_shotpower>0.569697</"
          "physical_shotpower><technical_standingtackle>0.536364</"
          "technical_standingtackle><technical_slidingtackle>0.536364</"
          "technical_slidingtackle><technical_ballcontrol>0.503030</"
          "technical_ballcontrol><technical_dribble>0.469697</"
          "technical_dribble><technical_shortpass>0.569697</"
          "technical_shortpass><technical_highpass>0.536364</"
          "technical_highpass><technical_header>0.469697</"
          "technical_header><technical_shot>0.436364</"
          "technical_shot><technical_volley>0.303030</"
          "technical_volley><mental_calmness>0.469697</"
          "mental_calmness><mental_workrate>0.469697</"
          "mental_workrate><mental_resilience>0.469697</"
          "mental_resilience><mental_defensivepositioning>0.536364</"
          "mental_defensivepositioning><mental_offensivepositioning>0.436364</"
          "mental_offensivepositioning><mental_vision>0.503030</mental_vision>";
      age = 25;
      skinColor = 1;
      hairStyle = "short02";
      hairColor = "black";
      height = 1.7;
      break;
    case 254:
      firstName = "Javier";
      lastName = "Masqueranus";
      baseStat = 0.64269;
      profileString =
          "<physical_balance>0.713636</"
          "physical_balance><physical_reaction>0.613636</"
          "physical_reaction><physical_acceleration>0.463636</"
          "physical_acceleration><physical_velocity>0.463636</"
          "physical_velocity><physical_stamina>0.463636</"
          "physical_stamina><physical_agility>0.463636</"
          "physical_agility><physical_shotpower>0.563636</"
          "physical_shotpower><technical_standingtackle>0.713636</"
          "technical_standingtackle><technical_slidingtackle>0.663636</"
          "technical_slidingtackle><technical_ballcontrol>0.463636</"
          "technical_ballcontrol><technical_dribble>0.313636</"
          "technical_dribble><technical_shortpass>0.563636</"
          "technical_shortpass><technical_highpass>0.563636</"
          "technical_highpass><technical_header>0.513636</"
          "technical_header><technical_shot>0.313636</"
          "technical_shot><technical_volley>0.213636</"
          "technical_volley><mental_calmness>0.463636</"
          "mental_calmness><mental_workrate>0.463636</"
          "mental_workrate><mental_resilience>0.463636</"
          "mental_resilience><mental_defensivepositioning>0.663636</"
          "mental_defensivepositioning><mental_offensivepositioning>0.313636</"
          "mental_offensivepositioning><mental_vision>0.563636</mental_vision>";
      age = 30;
      skinColor = 2;
      hairStyle = "bald";
      hairColor = "black";
      height = 1.74;
      break;
    case 320:
      firstName = "Gerard";
      lastName = "Pitoresqué";
      baseStat = 0.625396;
      profileString =
          "<physical_balance>0.736364</"
          "physical_balance><physical_reaction>0.686364</"
          "physical_reaction><physical_acceleration>0.486364</"
          "physical_acceleration><physical_velocity>0.486364</"
          "physical_velocity><physical_stamina>0.486364</"
          "physical_stamina><physical_agility>0.436364</"
          "physical_agility><physical_shotpower>0.636364</"
          "physical_shotpower><technical_standingtackle>0.786364</"
          "technical_standingtackle><technical_slidingtackle>0.786364</"
          "technical_slidingtackle><technical_ballcontrol>0.386364</"
          "technical_ballcontrol><technical_dribble>0.236364</"
          "technical_dribble><technical_shortpass>0.436364</"
          "technical_shortpass><technical_highpass>0.436364</"
          "technical_highpass><technical_header>0.586364</"
          "technical_header><technical_shot>0.236364</"
          "technical_shot><technical_volley>0.186364</"
          "technical_volley><mental_calmness>0.486364</"
          "mental_calmness><mental_workrate>0.486364</"
          "mental_workrate><mental_resilience>0.486364</"
          "mental_resilience><mental_defensivepositioning>0.786364</"
          "mental_defensivepositioning><mental_offensivepositioning>0.236364</"
          "mental_offensivepositioning><mental_vision>0.486364</mental_vision>";
      age = 27;
      skinColor = 1;
      hairStyle = "medium01";
      hairColor = "black";
      height = 1.93;
      break;
    case 103:
      firstName = "";
      lastName = "Danny Ballfs";
      baseStat = 0.60786;
      profileString =
          "<physical_balance>0.562121</"
          "physical_balance><physical_reaction>0.528788</"
          "physical_reaction><physical_acceleration>0.528788</"
          "physical_acceleration><physical_velocity>0.528788</"
          "physical_velocity><physical_stamina>0.462121</"
          "physical_stamina><physical_agility>0.495455</"
          "physical_agility><physical_shotpower>0.528788</"
          "physical_shotpower><technical_standingtackle>0.595455</"
          "technical_standingtackle><technical_slidingtackle>0.562121</"
          "technical_slidingtackle><technical_ballcontrol>0.528788</"
          "technical_ballcontrol><technical_dribble>0.428788</"
          "technical_dribble><technical_shortpass>0.628788</"
          "technical_shortpass><technical_highpass>0.595455</"
          "technical_highpass><technical_header>0.462121</"
          "technical_header><technical_shot>0.362121</"
          "technical_shot><technical_volley>0.262121</"
          "technical_volley><mental_calmness>0.462121</"
          "mental_calmness><mental_workrate>0.462121</"
          "mental_workrate><mental_resilience>0.462121</"
          "mental_resilience><mental_defensivepositioning>0.595455</"
          "mental_defensivepositioning><mental_offensivepositioning>0.362121</"
          "mental_offensivepositioning><mental_vision>0.595455</mental_vision>";
      age = 31;
      skinColor = 2;
      hairStyle = "short01";
      hairColor = "black";
      height = 1.72;
      break;
    case 188:
      firstName = "Andr�s";
      lastName = "Ingestia";
      baseStat = 0.68319;
      profileString =
          "<physical_balance>0.386364</"
          "physical_balance><physical_reaction>0.486364</"
          "physical_reaction><physical_acceleration>0.586364</"
          "physical_acceleration><physical_velocity>0.586364</"
          "physical_velocity><physical_stamina>0.486364</"
          "physical_stamina><physical_agility>0.686364</"
          "physical_agility><physical_shotpower>0.686364</"
          "physical_shotpower><technical_standingtackle>0.186364</"
          "technical_standingtackle><technical_slidingtackle>0.186364</"
          "technical_slidingtackle><technical_ballcontrol>0.586364</"
          "technical_ballcontrol><technical_dribble>0.686364</"
          "technical_dribble><technical_shortpass>0.586364</"
          "technical_shortpass><technical_highpass>0.486364</"
          "technical_highpass><technical_header>0.386364</"
          "technical_header><technical_shot>0.686364</"
          "technical_shot><technical_volley>0.486364</"
          "technical_volley><mental_calmness>0.486364</"
          "mental_calmness><mental_workrate>0.486364</"
          "mental_workrate><mental_resilience>0.486364</"
          "mental_resilience><mental_defensivepositioning>0.286364</"
          "mental_defensivepositioning><mental_offensivepositioning>0.686364</"
          "mental_offensivepositioning><mental_vision>0.386364</mental_vision>";
      age = 30;
      skinColor = 1;
      hairStyle = "short01";
      hairColor = "black";
      height = 1.71;
      break;
    case 74:
      firstName = "Sergio";
      lastName = "Buckets";
      baseStat = 0.645507;
      profileString =
          "<physical_balance>0.663636</"
          "physical_balance><physical_reaction>0.563636</"
          "physical_reaction><physical_acceleration>0.463636</"
          "physical_acceleration><physical_velocity>0.463636</"
          "physical_velocity><physical_stamina>0.463636</"
          "physical_stamina><physical_agility>0.463636</"
          "physical_agility><physical_shotpower>0.563636</"
          "physical_shotpower><technical_standingtackle>0.663636</"
          "technical_standingtackle><technical_slidingtackle>0.563636</"
          "technical_slidingtackle><technical_ballcontrol>0.463636</"
          "technical_ballcontrol><technical_dribble>0.363636</"
          "technical_dribble><technical_shortpass>0.663636</"
          "technical_shortpass><technical_highpass>0.663636</"
          "technical_highpass><technical_header>0.463636</"
          "technical_header><technical_shot>0.363636</"
          "technical_shot><technical_volley>0.263636</"
          "technical_volley><mental_calmness>0.463636</"
          "mental_calmness><mental_workrate>0.463636</"
          "mental_workrate><mental_resilience>0.463636</"
          "mental_resilience><mental_defensivepositioning>0.563636</"
          "mental_defensivepositioning><mental_offensivepositioning>0.363636</"
          "mental_offensivepositioning><mental_vision>0.563636</mental_vision>";
      age = 26;
      skinColor = 1;
      hairStyle = "short02";
      hairColor = "black";
      height = 1.89;
      break;
    case 332:
      firstName = "Ivan";
      lastName = "Rattizić";
      baseStat = 0.63531;
      profileString =
          "<physical_balance>0.525000</"
          "physical_balance><physical_reaction>0.525000</"
          "physical_reaction><physical_acceleration>0.525000</"
          "physical_acceleration><physical_velocity>0.525000</"
          "physical_velocity><physical_stamina>0.475000</"
          "physical_stamina><physical_agility>0.575000</"
          "physical_agility><physical_shotpower>0.625000</"
          "physical_shotpower><technical_standingtackle>0.425000</"
          "technical_standingtackle><technical_slidingtackle>0.375000</"
          "technical_slidingtackle><technical_ballcontrol>0.525000</"
          "technical_ballcontrol><technical_dribble>0.525000</"
          "technical_dribble><technical_shortpass>0.625000</"
          "technical_shortpass><technical_highpass>0.575000</"
          "technical_highpass><technical_header>0.425000</"
          "technical_header><technical_shot>0.525000</"
          "technical_shot><technical_volley>0.375000</"
          "technical_volley><mental_calmness>0.475000</"
          "mental_calmness><mental_workrate>0.475000</"
          "mental_workrate><mental_resilience>0.475000</"
          "mental_resilience><mental_defensivepositioning>0.425000</"
          "mental_defensivepositioning><mental_offensivepositioning>0.525000</"
          "mental_offensivepositioning><mental_vision>0.475000</mental_vision>";
      age = 26;
      skinColor = 1;
      hairStyle = "long01";
      hairColor = "blonde";
      height = 1.84;
      break;
    case 290:
      firstName = "";
      lastName = "Niemeyer";
      baseStat = 0.716847;
      profileString =
          "<physical_balance>0.381818</"
          "physical_balance><physical_reaction>0.481818</"
          "physical_reaction><physical_acceleration>0.631818</"
          "physical_acceleration><physical_velocity>0.631818</"
          "physical_velocity><physical_stamina>0.481818</"
          "physical_stamina><physical_agility>0.681818</"
          "physical_agility><physical_shotpower>0.681818</"
          "physical_shotpower><technical_standingtackle>0.181818</"
          "technical_standingtackle><technical_slidingtackle>0.131818</"
          "technical_slidingtackle><technical_ballcontrol>0.531818</"
          "technical_ballcontrol><technical_dribble>0.731818</"
          "technical_dribble><technical_shortpass>0.581818</"
          "technical_shortpass><technical_highpass>0.531818</"
          "technical_highpass><technical_header>0.381818</"
          "technical_header><technical_shot>0.681818</"
          "technical_shot><technical_volley>0.431818</"
          "technical_volley><mental_calmness>0.481818</"
          "mental_calmness><mental_workrate>0.481818</"
          "mental_workrate><mental_resilience>0.481818</"
          "mental_resilience><mental_defensivepositioning>0.256818</"
          "mental_defensivepositioning><mental_offensivepositioning>0.706818</"
          "mental_offensivepositioning><mental_vision>0.431818</mental_vision>";
      age = 22;
      skinColor = 2;
      hairStyle = "medium01";
      hairColor = "black";
      height = 1.74;
      break;
    case 391:
      firstName = "Luis";
      lastName = "Sáreusz";
      baseStat = 0.693097;
      profileString =
          "<physical_balance>0.381818</"
          "physical_balance><physical_reaction>0.481818</"
          "physical_reaction><physical_acceleration>0.631818</"
          "physical_acceleration><physical_velocity>0.631818</"
          "physical_velocity><physical_stamina>0.481818</"
          "physical_stamina><physical_agility>0.681818</"
          "physical_agility><physical_shotpower>0.681818</"
          "physical_shotpower><technical_standingtackle>0.181818</"
          "technical_standingtackle><technical_slidingtackle>0.131818</"
          "technical_slidingtackle><technical_ballcontrol>0.531818</"
          "technical_ballcontrol><technical_dribble>0.731818</"
          "technical_dribble><technical_shortpass>0.581818</"
          "technical_shortpass><technical_highpass>0.531818</"
          "technical_highpass><technical_header>0.381818</"
          "technical_header><technical_shot>0.681818</"
          "technical_shot><technical_volley>0.431818</"
          "technical_volley><mental_calmness>0.481818</"
          "mental_calmness><mental_workrate>0.481818</"
          "mental_workrate><mental_resilience>0.481818</"
          "mental_resilience><mental_defensivepositioning>0.256818</"
          "mental_defensivepositioning><mental_offensivepositioning>0.706818</"
          "mental_offensivepositioning><mental_vision>0.431818</mental_vision>";
      age = 27;
      skinColor = 2;
      hairStyle = "medium01";
      hairColor = "black";
      height = 1.81;
      break;
    case 264:
      firstName = "Lionel";
      lastName = "Messy";
      baseStat = 0.718346;
      profileString =
          "<physical_balance>0.381818</"
          "physical_balance><physical_reaction>0.481818</"
          "physical_reaction><physical_acceleration>0.631818</"
          "physical_acceleration><physical_velocity>0.631818</"
          "physical_velocity><physical_stamina>0.481818</"
          "physical_stamina><physical_agility>0.681818</"
          "physical_agility><physical_shotpower>0.681818</"
          "physical_shotpower><technical_standingtackle>0.181818</"
          "technical_standingtackle><technical_slidingtackle>0.131818</"
          "technical_slidingtackle><technical_ballcontrol>0.531818</"
          "technical_ballcontrol><technical_dribble>0.731818</"
          "technical_dribble><technical_shortpass>0.581818</"
          "technical_shortpass><technical_highpass>0.531818</"
          "technical_highpass><technical_header>0.381818</"
          "technical_header><technical_shot>0.681818</"
          "technical_shot><technical_volley>0.431818</"
          "technical_volley><mental_calmness>0.481818</"
          "mental_calmness><mental_workrate>0.481818</"
          "mental_workrate><mental_resilience>0.481818</"
          "mental_resilience><mental_defensivepositioning>0.256818</"
          "mental_defensivepositioning><mental_offensivepositioning>0.706818</"
          "mental_offensivepositioning><mental_vision>0.431818</mental_vision>";
      age = 27;
      skinColor = 1;
      hairStyle = "short02";
      hairColor = "black";
      height = 1.69;
      break;
  }

  // get average stat for current age

  XMLLoader loader;
  XMLTree tree = loader.Load(profileString);

  //printf("player: %s, %s (age %i)\n", lastName.c_str(), firstName.c_str(), age);
  map_XMLTree::const_iterator iter = tree.children.begin();
  while (iter != tree.children.end()) {
    float profileStat = atof((*iter).second.value.c_str()); // profile value

    float value = CalculateStat(baseStat, profileStat, age, e_DevelopmentCurveType_Normal);
    //printf("base: %f; profile: %f; result: %f\n", baseStat, profileStat, value);

    stats.Set(PlayerStatFromString((*iter).first), value);
    iter++;
  }
  UpdateValues();
}

PlayerData::PlayerData() {
  // officials, for example, use this constructor
  skinColor = int(std::round(boostrandom(1, 4)));
  hairStyle = "short01";
  hairColor = "darkblonde";
  height = 1.8f;

  stats.Set(physical_balance, 0.6);
  stats.Set(physical_reaction, 0.6);
  stats.Set(physical_acceleration, 0.6);
  stats.Set(PlayerStat::physical_velocity, 0.6);
  stats.Set(physical_stamina, 0.6);
  stats.Set(physical_agility, 0.6);
  stats.Set(physical_shotpower, 0.6);
  stats.Set(technical_standingtackle, 0.6);
  stats.Set(technical_slidingtackle, 0.6);
  stats.Set(technical_ballcontrol, 0.6);
  stats.Set(technical_dribble, 0.6);
  stats.Set(technical_shortpass, 0.6);
  stats.Set(technical_highpass, 0.6);
  stats.Set(technical_header, 0.6);
  stats.Set(technical_shot, 0.6);
  stats.Set(technical_volley, 0.6);
  stats.Set(mental_calmness, 0.6);
  stats.Set(mental_workrate, 0.6);
  stats.Set(mental_resilience, 0.6);
  stats.Set(mental_defensivepositioning, 0.6);
  stats.Set(mental_offensivepositioning, 0.6);
  stats.Set(mental_vision, 0.6);
  UpdateValues();
}

void PlayerData::UpdateValues() {
  physical_velocity = GetStat(PlayerStat::physical_velocity);
}


PlayerData::~PlayerData() {
}
