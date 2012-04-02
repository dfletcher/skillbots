/**
 *  @file Bot.hpp
 */

#ifndef BOTRUNNER_BOT
#define BOTRUNNER_BOT

#include <map>
#include <string>
#include <Weapon.hpp>

class Bot {
  public:
    Bot(void);
    ~Bot(void);
    int id, x, y;
    bool inrange;
    double energy, condition, speed;
    std::string state;
    WeaponMap weapons;
};

typedef std::map<int, Bot> BotMap;
typedef BotMap::const_iterator BotMapIterator;

#endif /* [BOTRUNNER_BOT] */
