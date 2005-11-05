/*
 * $Id$
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Foe commands data.
 *
 * @version $Revision$
 */
#ifndef FOECOMMAND_H_
#define FOECOMMAND_H_

#include "bulletml/bulletmlparser.h"
#include "bulletml/bulletmlparser-tinyxml.h"
#include "bulletml/bulletmlrunner.h"
#include "foe.h"

class FoeCommand : public BulletMLRunner {
 public:
  FoeCommand(BulletMLParser* parser, struct foe* f);
  FoeCommand(BulletMLState* state, struct foe* f);

  virtual ~FoeCommand();

  virtual float getBulletDirection();
  virtual float getAimDirection();
  virtual float getBulletSpeed();
  virtual float getDefaultSpeed();
  virtual float getRank();
  virtual void createSimpleBullet(float direction, float speed);
  virtual void createBullet(BulletMLState* state, float direction, float speed);
  virtual int getTurn();
  virtual void doVanish();
  
  virtual void doChangeDirection(float d);
  virtual void doChangeSpeed(float s);
  virtual void FoeCommand::doAccelX(float ax);
  virtual void FoeCommand::doAccelY(float ay);
  virtual float FoeCommand::getBulletSpeedX();
  virtual float FoeCommand::getBulletSpeedY();
  
 private:
  struct foe *foe;
};
#endif


