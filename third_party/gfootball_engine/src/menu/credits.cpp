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

#include "credits.hpp"
#include <cmath>

#include "../main.hpp"

#include "../base/utils.hpp"

CreditsPage::CreditsPage(Gui2WindowManager *windowManager, const Gui2PageData &pageData) : Gui2Page(windowManager, pageData) {

  windowManager->BlackoutBackground(true);

  this->SetFocus();

  bg = new Gui2Image(windowManager, "image_credits_bg", 0, 0, 100, 100);
  bg->LoadImage("media/menu/credits/bg.png");
  this->AddView(bg);
  bg->Show();

  float textHeight = 60 / float(numtexts - 1);

  for (int i = 0; i < numtexts; i++) {
    text[i] = new Gui2Caption(windowManager, "credittext" + int_to_str(i), 0, 0, 60, textHeight, "");//"test" + int_to_str(i));
    this->AddView(text[i]);
    text[i]->Show();
  }

  for (int i = 0; i < numballs; i++) {
    balls[i] = new Gui2Image(windowManager, "ball" + int_to_str(i), ballPos[i].coords[0], ballPos[i].coords[1], 4, 5);
    this->AddView(balls[i]);
    balls[i]->LoadImage("media/menu/credits/ball.png");
    ballPos[i] = Vector3(random(60, 90), random(-60, -5), 0);
    ballMov[i] = Vector3(random(-3, 7), random(-40, -10), 0);
    balls[i]->SetPosition(ballPos[i].coords[0], ballPos[i].coords[1]);
    balls[i]->Show();
  }

  scrollOffset = 0;
  creditOffset = 0;
  previousStartIndex = 0;

  InitCreditsContents();

  this->Show();
}

CreditsPage::~CreditsPage() {

  windowManager->BlackoutBackground(false);

}

void CreditsPage::InitCreditsContents() {

  AddCredit("PROPERLY DECENT presents");
  AddHeader("GAMEPLAY FOOTBALL");
  AddWhitespace();
  AddWhitespace();


  AddHeader("blunt3d game engine");

  AddSubHeader("programming");
  AddCredit("bastiaan schuiling");

  AddSubHeader("multithreaded 'tasksequence' concept");
  AddCredit("jurian broertjes");
  AddCredit("bastiaan schuiling");

  AddSubHeader("GUI");
  AddCredit("bastiaan schuiling");

  AddSubHeader("loosely based on the Intel paper:");
  AddCredit("'designing the framework of a parallel game engine'");

  AddSubHeader("some maths code inspired by:");
  AddCredit("OGRE graphics rendering engine");

  AddSubHeader("some GUI architecture inspired by:");
  AddCredit("QT cross-platform application and UI framework");

  AddSubHeader("programs/libraries/code used:");
  AddCredit("GNU C++ compiler + mingw32");
  AddCredit("Boost C++ libraries");
  AddCredit("OpenGL - open graphics library");
  AddCredit("OpenAL - open audio library");
  AddCredit("SDL - simple directmedia layer");
  AddCredit("GLEE - OpenGL Easy Extension library");
  AddCredit("sqlite3 - database");
  AddCredit("FastApprox by Paul Mineiro");
  AddCredit("Dev-C++ IDE");
  AddCredit("Sublime Text - best editor ever");

  AddSubHeader("helpful technical papers");
  AddCredit("AMD/ATI, Intel, NVidia");

  AddSubHeader("credit for small pieces of code i used");
  AddCredit("Anthony Williams, sabbac, reedbeta, cm_rollo");
  AddCredit("gathering.tweakers.net");
  AddCredit("www.3dkingdoms.com");
  AddCredit("www.gamedev.net");
  AddCredit("www.euclideanspace.com");
  AddCredit("stackoverflow.com");

  AddSubHeader("very helpful folks @ gathering.tweakers.net");
  AddCredit(".oisyn <-- special mention! thanks for all the help!");
  AddCredit("Zoijar, Soultaker, H!GHGuY,");
  AddCredit("dusty, engelbertus, pedorus,");
  AddCredit("PrisonerOfPain, en iedereen die ik vergeet!");

  AddSubHeader("early morning debugging");
  AddCredit("joris zwart");


  AddHeader("gameplay football");

  AddSubHeader("programming");
  AddCredit("bastiaan schuiling");

  AddSubHeader("graphics, modelling");
  AddCredit("bastiaan schuiling");

  AddSubHeader("audio");
  AddCredit("bastiaan schuiling");
  AddCredit("harm-jan wiechers");

  AddSubHeader("player animations & animation editor coding");
  AddCredit("bastiaan schuiling");

  AddSubHeader("font used");
  AddCredit("'Alegreya Sans SC'");
  AddCredit("by juan pablo del peral (juan@huertatipografica.com.ar)");

  AddSubHeader("libraries/code used:");
  AddCredit("libhungarian by cyrill stachniss");
  AddCredit("perlin noise by ken perlin");

  AddSubHeader("multi-agent positional forcefield concept");
  AddCredit("harmi praagman & alexander woldhek");

  AddSubHeader("miscellaneous architectural support");
  AddCredit("jurian broertjes");

  AddSubHeader("workspace/community for indie game development");
  AddCredit("indietopia groningen");


  AddHeader("now for the fun part!");

  AddSubHeader("mental support");
  AddCredit("jurian broertjes");
  AddCredit("harmi, alexander & stan praagman");
  AddCredit("jorrit grave");
  AddCredit("wouter grevink");
  AddCredit("tessa kusters");
  AddCredit("joey frankhuijzen");
  AddCredit("johan & nia schuiling");
  AddCredit("margit & michel vedder");

  AddSubHeader("want to say hi to:");
  AddCredit("everyone at indietopia!");
  AddCredit("special mention for job talle, because coders rule");
  AddCredit("stephan & amaaaaa, worm & anouk @ irc <3,");
  AddCredit("femke schouten, jan bart leeuw, lennart van luijk,");
  AddCredit("pieter eisenga, albert jan nijburg (SNORT SNORT!!!)");
  AddCredit("ernie getz (mijn geheime muze)");
  AddCredit("het zaterdag/dinsdag voetbal team! hup hup");
  AddCredit("het donderdag futsal team! hup hup");

  AddWhitespace();
  AddCredit("..en zoveel meer mensen die belangrijk zijn (geweest)");
  AddCredit("in mijn leven.. maar jullie passen er niet op! anders");
  AddCredit("blijf ik bezig :p sorry! maar bedankt :)");

  AddSubHeader("life- and project coaching");
  AddCredit("VNN: margriete de jong, for keeping me somewhat sane!");
  AddCredit("3daagse: adriaan, janneke, en de rest!");
  AddCredit("en de VNN groep: eric, menno, jelle, en de rest!");
  AddCredit("chris guikema (life coach), fred vredeveld (sozawe groningen)");

  AddSubHeader("the gameplay football community!!!");
  AddCredit("broxopios, tureckirumun, nlp, balder..");
  AddCredit("everyone basically! i wuvz u <3");
  AddCredit("and sorry i'm such a slowpoke coder!");

  AddSubHeader("lead financial scamming");
  AddCredit("centrale brasschaatse bank");

  AddSubHeader("shouts out to ##club-ubuntu on freenode");
  AddCredit("alexbobp, ldp, ljl, anastasius,");
  AddCredit("lleberg, darkmatter, henux, spec,");
  AddCredit("nocode, avasz, emma, lizzie,");
  AddCredit("gabs, eri, em, kn100, quup,");
  AddCredit("netdaemon, m00se, m0nk, mc44,");
  AddCredit("syrinx_, drderek, and everyone else!");

  AddSubHeader("shouts out to #ranchorelaxo and #properlydecent!");

  AddSubHeader("loud shouts out to ranchorelaxoradio!");

  AddSubHeader("best boy & dolly grip");
  AddCredit("dr. smokey & dj josti");

  AddSubHeader("first international supporter awards goes to");
  AddCredit("victor pardinho from brazil");

  AddSubHeader("1st one to donate!");
  AddCredit("folkert van heusden");

  AddSubHeader("respect for creating a better world!");
  AddCredit("pirate parties international (PPI)");
  AddCredit("electronic frontier foundation (EFF)");
  AddCredit("free software foundation (FSF)");
  AddCredit("bits of freedom (BOF)");
  AddCredit("creative commons (CC)");
  AddCredit("wikipedia");

  AddSubHeader("to the best football club in the world...");
  AddCredit("HUP FC GRONINGEN!");

  AddWhitespace();
  AddWhitespace();
  AddSubHeader("i dedicate this game to koen konings, my father. RIP");
  AddCredit("~ papa, ik lijk steeds meer op jou! ~");
  AddWhitespace();
  AddWhitespace();

  AddSubHeader("and finally, to everyone: stay beautiful!");

  AddHeader("THE END");

  for (int i = 0; i < numtexts; i++) AddWhitespace();
  AddSubHeader("no really, there's nothing more to say!");

  for (int i = 0; i < 12; i++) AddWhitespace();
  AddSubHeader("go away! nothing to see here!");

  for (int i = 0; i < 12; i++) AddWhitespace();
  AddHeader("stop rocking the boat!");

  for (int i = 0; i < 16; i++) AddWhitespace();
  AddSubHeader("i'm going to call the police, you stalker!");

  for (int i = 0; i < 12; i++) AddWhitespace();
  AddSubHeader("okay, i'm just going to rewind to the beginning now! hah!");

  for (int i = 0; i < 5; i++) AddWhitespace();
  AddCredit("PROPERLY DECENT presents");
  AddHeader("GAMEPLAY FOOTBALL");

  for (int i = 0; i < 12; i++) AddWhitespace();
  AddSubHeader("hah you fell for it, didn't you?");

  for (int i = 0; i < 12; i++) AddWhitespace();
  AddCredit("okay, now i'm really going to rewind to the beginning. but you");
  AddCredit("will never be sure until you watched all the credits again! muhaha!");

  for (int i = 0; i < 6; i++) AddWhitespace();
  AddHeader("<3 BYE! <3");

  for (int i = 0; i < numtexts; i++) AddWhitespace();
}

void CreditsPage::AddHeader(const std::string &blah) {
  AddWhitespace();
  AddWhitespace();
  AddWhitespace();
  CreditsContents credit;
  credit.text = blah;
  credit.color.Set(255, 255, 255);
  credits.push_back(credit);
  AddWhitespace();
}

void CreditsPage::AddSubHeader(const std::string &blah) {
  AddWhitespace();
  CreditsContents credit;
  credit.text = blah;
  credit.color.Set(255, 255, 200);
  credits.push_back(credit);
}

void CreditsPage::AddCredit(const std::string &blah) {
  CreditsContents credit;
  credit.text = blah;
  credit.color.Set(200, 200, 255);
  credits.push_back(credit);
}

void CreditsPage::AddWhitespace() {
  CreditsContents credit;
  credit.text = "";
  credit.color.Set(0, 0, 0);
  credits.push_back(credit);
}

void CreditsPage::Process() {

  // SCROLLTEXT

  float fullHeight = 100.0 / float(numtexts - 1);
  unsigned int startIndex = int(scrollOffset / fullHeight) % numtexts;

  float yOffset = std::fmod(scrollOffset, fullHeight);
  for (int i = 0; i < numtexts; i++) {

    int index = (startIndex + i) % numtexts;

    text[index]->SetPosition(14 + sin(atoi(text[index]->GetName().c_str()) * 0.24 - scrollOffset * 0.05) * 5
                                + cos(atoi(text[index]->GetName().c_str()) * 1.0  + scrollOffset * 0.10) * 1, i * fullHeight - yOffset);
    text[index]->Show();
  }

  scrollOffset += 0.11;//windowManager->GetTimeStep_ms() / 60.0;

  // time to add a new text
  if (startIndex != previousStartIndex) {

    int index = (startIndex + numtexts - 1) % numtexts;

    text[index]->SetCaption(credits.at(creditOffset).text);
    text[index]->SetColor(credits.at(creditOffset).color);
    text[index]->SetName(int_to_str(creditOffset));
    text[index]->Redraw();

    creditOffset++;
    if (creditOffset == credits.size()) creditOffset = 0;

    previousStartIndex = startIndex;
  }


  // BALLS

  for (int i = 0; i < numballs; i++) {
    ballMov[i].coords[1] += 0.25; // gravity
    if (ballPos[i].coords[0] > 100 - 4 && ballMov[i].coords[0] > 0) ballMov[i].coords[0] = -ballMov[i].coords[0] * 0.7;
    if (ballPos[i].coords[0] <       0 && ballMov[i].coords[0] < 0) ballMov[i].coords[0] = -ballMov[i].coords[0] * 0.7;
    if (ballPos[i].coords[1] > 100 - 5 && ballMov[i].coords[1] > 20) ballMov[i].coords[1] = -ballMov[i].coords[1] * 0.6;
    if (ballPos[i].coords[1] > 100) {
      ballPos[i] = Vector3(random(60, 90), random(-60, -5), 0);
      ballMov[i] = Vector3(random(-3, 7), random(-40, -10), 0);
    }
    ballPos[i] += ballMov[i] * 0.05;
    balls[i]->SetPosition(ballPos[i].coords[0], ballPos[i].coords[1]);
  }

}

void CreditsPage::ProcessJoystickEvent(JoystickEvent *event) {
  Gui2Page::ProcessJoystickEvent(event);
}
