/**
 *  @file BotLanguage.hpp
 *
 *
 */

#ifndef BOTRUNNER_BOTLANGUAGE
#define BOTRUNNER_BOTLANGUAGE

#include <string>
#include <sstream>
#include <Arena.hpp>
#include <Weapon.hpp>

class BotLanguage {

  public:

    virtual void init(const char *path)=0;

    virtual void stateChange(const Arena&, std::stringstream &rval)=0;

    virtual void move(const Arena&, std::stringstream &rvaldir, double &rvalspeed)=0;

    virtual void aim(const Arena&, const Weapon&, double &rval)=0;

    virtual ~BotLanguage(void);
};

#endif /* [BOTRUNNER_BOTLANGUAGE] */
