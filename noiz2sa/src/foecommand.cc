/*
 * $Id: foecommand.cc,v 1.2 2003/08/10 04:09:46 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Handle bullet commands.
 *
 * @version $Revision: 1.2 $
 */
#include "bulletml/bulletmlparser.h"
#include "bulletml/bulletmlparser-tinyxml.h"
#include "bulletml/bulletmlrunner.h"
#include "foe.h"

extern "C" {
#include "noiz2sa.h"
#include "degutil.h"
#include "ship.h"
}

#define COMMAND_SCREEN_SPD_RATE 800
#define COMMAND_SCREEN_VEL_RATE 800

FoeCommand::FoeCommand(BulletMLParser *parser, Foe *f)
  : BulletMLRunner(parser) {
  foe = f;
}

FoeCommand::FoeCommand(BulletMLState *state, Foe *f)
  : BulletMLRunner(state) {
  foe = f;
}

FoeCommand::~FoeCommand() {}

float FoeCommand::getBulletDirection() {
  return (float)foe->d*360/DIV;
}

float FoeCommand::getAimDirection() {
  return ((float)getPlayerDeg(foe->pos.x, foe->pos.y)*360/DIV);
}

float FoeCommand::getBulletSpeed() {
  return ((float)foe->spd)/COMMAND_SCREEN_SPD_RATE;
}

float FoeCommand::getDefaultSpeed() {
  return 1;
}

float FoeCommand::getRank() {
  return foe->rank;
}

void FoeCommand::createSimpleBullet(float direction, float speed) {
  int d = (int)(direction*DIV/360); d &= (DIV-1);
  addFoeNormalBullet(&(foe->pos), foe->rank, 
		     d, (int)(speed*COMMAND_SCREEN_SPD_RATE), foe->color+1);
}

void FoeCommand::createBullet(BulletMLState* state, float direction, float speed) {
  int d = (int)(direction*DIV/360); d &= (DIV-1);
  addFoeActiveBullet(&(foe->pos), foe->rank, 
		     d, (int)(speed*COMMAND_SCREEN_SPD_RATE), foe->color+1, state);
}

int FoeCommand::getTurn() {
  return tick;
}

void FoeCommand::doVanish() {
  removeFoe(foe);
}

void FoeCommand::doChangeDirection(float d) {
  foe->d = (int)(d*DIV/360);
}

void FoeCommand::doChangeSpeed(float s) {
  foe->spd = (int)(s*COMMAND_SCREEN_SPD_RATE);
}

void FoeCommand::doAccelX(float ax) {
  foe->vel.x = (int)(ax*COMMAND_SCREEN_VEL_RATE);
}

void FoeCommand::doAccelY(float ay) {
  foe->vel.y = (int)(ay*COMMAND_SCREEN_VEL_RATE);
}

float FoeCommand::getBulletSpeedX() {
  return ((float)foe->vel.x/COMMAND_SCREEN_VEL_RATE);
}

float FoeCommand::getBulletSpeedY() {
  return ((float)foe->vel.y/COMMAND_SCREEN_VEL_RATE);
}
