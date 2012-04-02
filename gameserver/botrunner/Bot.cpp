/**
 *  @file Bot.cpp
 */

#include <Bot.hpp>

Bot::Bot(void) : state("move") {
  id = 0;
  x = 0;
  y = 0;
  energy = 0.0;
  condition = 0.0;
  speed = 0.0;
  inrange = false;
}

Bot::~Bot(void) {
  ;
}
