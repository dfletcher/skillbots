/**
 *  @file BotLanguage.hpp
 *
 *
 */

#ifndef BOTRUNNER_BOTLANGUAGE
#define BOTRUNNER_BOTLANGUAGE

#include <string>
#include <sstream>
#include "Bot.hpp"
#include "Arena.hpp"
#include "Obstacle.hpp"
#include "Weapon.hpp"

class BotImplementation {

  public:

    virtual void init(const Arena&, const std::string &path)=0;

    virtual void stateChange(const Arena&, std::stringstream &rval)=0;

    virtual void move(const Arena&, std::stringstream &rvaldir, double &rvalspeed)=0;

    virtual void aim(const Arena&, const Weapon&, double &rval)=0;
};

#endif /* [BOTRUNNER_BOTLANGUAGE] */

