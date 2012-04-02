/**
 *  @file Arena.hpp
 */

#ifndef BOTRUNNER_ARENA
#define BOTRUNNER_ARENA

#include <Bot.hpp>
#include <Obstacle.hpp>

class Arena {
  public:
    Arena(void);
    ~Arena(void);
    Bot bot;
    BotMap enemies;
    ObstacleMap obstacles;
    int id, w, h, t, d;
};

#endif /* [BOTRUNNER_ARENA] */
