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

#include "gamedefines.hpp"

#include <cmath>

#include "base/log.hpp"
#include "base/utils.hpp"
#include "file.h"
#include "main.hpp"

struct index3 {
  int index[3];
};

void GetVertexColors(std::map<Vector3, Vector3> &colorCoords) {
  DO_VALIDATION;
  std::vector<Vector3> vertices;
  std::vector<Vector3> colors;
  std::vector<index3> faces;
  std::vector<index3> colorFaces;

  std::string data = GetFile(GetGameConfig().updatePath("media/objects/players/models/fullbody.ase"));
  int pos = 0;
  int last_pos = 0;
  while (pos < data.length()) {
    DO_VALIDATION;
    while (pos < data.length() && data[pos] != '\n') pos++;
    std::string line_str = data.substr(last_pos, pos - last_pos);
    pos++;
    last_pos = pos;

    std::vector<std::string> tokens;
    tokenize(line_str, tokens, " \t");

    if (tokens.size() > 0) {
      DO_VALIDATION;
      if (tokens.at(0).compare("*MESH_NORMALS") == 0) {
        DO_VALIDATION;  // end of useful block

        // complete previous object

        for (unsigned int i = 0; i < colorFaces.size(); i++) {
          DO_VALIDATION;
          for (unsigned int v = 0; v < 3; v++) {
            DO_VALIDATION;
            Vector3 coord = vertices.at(faces[i].index[v]);
            Vector3 color = colors.at(colorFaces[i].index[v]);
            if (colorCoords.find(coord) == colorCoords.end()) {
              DO_VALIDATION;
              colorCoords.insert(std::pair<Vector3, Vector3>(coord, color));
            } else {
              // assert(colorCoords.find(coord)->second == color);
            }
          }
        }

        // start recording a new object

        vertices.clear();
        colors.clear();
        faces.clear();
        colorFaces.clear();
      }

      if (tokens.at(0).compare("*MESH_VERTEX") == 0) {
        DO_VALIDATION;
        assert(tokens.size() > 4);
        Vector3 bla(atof(tokens.at(2).c_str()), atof(tokens.at(3).c_str()),
                    atof(tokens.at(4).c_str()));
        vertices.push_back(bla);
      }

      if (tokens.at(0).compare("*MESH_FACE") == 0) {
        DO_VALIDATION;
        assert(tokens.size() > 7);
        index3 bla;
        bla.index[0] = atoi(tokens.at(3).c_str());
        bla.index[1] = atoi(tokens.at(5).c_str());
        bla.index[2] = atoi(tokens.at(7).c_str());
        faces.push_back(bla);
      }

      if (tokens.at(0).compare("*MESH_VERTCOL") == 0) {
        DO_VALIDATION;
        assert(tokens.size() > 4);
        Vector3 bla(atof(tokens.at(2).c_str()), atof(tokens.at(3).c_str()),
                    atof(tokens.at(4).c_str()));
        bla *= 255;
        bla.coords[0] = std::round(bla.coords[0]);
        bla.coords[1] = std::round(bla.coords[1]);
        bla.coords[2] = std::round(bla.coords[2]);
        colors.push_back(bla);
      }

      if (tokens.at(0).compare("*MESH_CFACE") == 0) {
        DO_VALIDATION;
        assert(tokens.size() > 4);
        index3 bla;
        bla.index[0] = atoi(tokens.at(2).c_str());
        bla.index[1] = atoi(tokens.at(3).c_str());
        bla.index[2] = atoi(tokens.at(4).c_str());
        colorFaces.push_back(bla);
      }
    }
  }
}

e_PlayerRole GetRoleFromString(const std::string &roleString) {
  DO_VALIDATION;
  if (roleString.compare("GK") == 0) return e_PlayerRole_GK;
  if (roleString.compare("CB") == 0) return e_PlayerRole_CB;
  if (roleString.compare("LB") == 0) return e_PlayerRole_LB;
  if (roleString.compare("RB") == 0) return e_PlayerRole_RB;
  if (roleString.compare("DM") == 0) return e_PlayerRole_DM;
  if (roleString.compare("CM") == 0) return e_PlayerRole_CM;
  if (roleString.compare("LM") == 0) return e_PlayerRole_LM;
  if (roleString.compare("RM") == 0) return e_PlayerRole_RM;
  if (roleString.compare("AM") == 0) return e_PlayerRole_AM;
  if (roleString.compare("CF") == 0) return e_PlayerRole_CF;
  return e_PlayerRole_CM;  // default
}
