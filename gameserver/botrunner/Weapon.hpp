/**
 *  @file Weapon.hpp
 */

#ifndef BOTRUNNER_WEAPON
#define BOTRUNNER_WEAPON

#include <map>

class Weapon {
  public:
    Weapon(void);
    ~Weapon(void);
    int id;
    double power, aim;
};

typedef std::map<int, Weapon> WeaponMap;
typedef WeaponMap::const_iterator WeaponMapIterator;

#endif /* [BOTRUNNER_WEAPON] */
