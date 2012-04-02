/**
 *  @file Obstacle.hpp
 */

#ifndef BOTRUNNER_OBSTACLE
#define BOTRUNNER_OBSTACLE

#include <map>

class Obstacle {
  public:
    Obstacle(void);
    ~Obstacle(void);
    int id, x, y, radius;
    bool inrange;
};

typedef std::map<int, Obstacle> ObstacleMap;
typedef ObstacleMap::const_iterator ObstacleMapIterator;

#endif /* [BOTRUNNER_OBSTACLE] */
