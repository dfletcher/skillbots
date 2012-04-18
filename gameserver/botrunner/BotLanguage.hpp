/**
 *  @file BotLanguage.hpp
 *
 *
 */

#ifndef BOTRUNNER_BOTLANGUAGE
#define BOTRUNNER_BOTLANGUAGE

#include <string>
#include <sstream>
#include <Bot.hpp>
#include <Arena.hpp>
#include <Weapon.hpp>
#include <Obstacle.hpp>

class BotLanguage {

  public:

    virtual void init(const char *path)=0;

    virtual void stateChange(const Arena&, std::stringstream &rval)=0;

    virtual void move(const Arena&, std::stringstream &rvaldir, double &rvalspeed)=0;

    virtual void aim(const Arena&, const Weapon&, double &rval)=0;

    virtual void collisionWithObstacle(const Arena&, bool self, const Bot&, const Obstacle&, double angle, double damage)=0;

    virtual void collisionWithBot(const Arena&, bool self, const Bot&, const Bot&, double angle, double damage)=0;

    virtual void shotFiredHitObstacle(const Arena&, bool self, const Bot&, const Obstacle&, double angle, double damage)=0;

    virtual void shotFiredHitBot(const Arena&, bool self, const Bot&, const Bot&, double angle, double damage)=0;

    virtual ~BotLanguage(void);
};

#endif /* [BOTRUNNER_BOTLANGUAGE] */
