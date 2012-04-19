/**
 *  @file BotRunner.cpp
 */

#include <streambuf>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <BotRunner.hpp>
#include <Utility.hpp>

std::string cmd_init("init");
std::string cmd_state_change("state-change");
std::string cmd_aim("aim");
std::string cmd_move("move");
std::string cmd_collision_obstacle("collision-with-obstacle");
std::string cmd_collision_bot("collision-with-bot");
std::string cmd_shot_fired_obstacle("shot-fired-obstacle");
std::string cmd_shot_fired_bot("shot-fired-bot");
std::string cmd_time("time");
std::string cmd_obstacle("obstacle");
std::string cmd_obstacle_inrange("obstacle-in-range");
std::string cmd_weapon("weapon");
std::string cmd_enemy("enemy");
std::string cmd_enemy_weapon("enemy-weapon");
std::string cmd_bot("bot");
std::string cmd_quit("quit");

std::string dir_n("n");
std::string dir_ne("ne");
std::string dir_e("e");
std::string dir_se("se");
std::string dir_s("s");
std::string dir_sw("sw");
std::string dir_w("w");
std::string dir_nw("nw");

std::string state_stop("stop");
std::string state_move("move");
std::string state_attack("attack");
std::string state_attack_move("attack+move");
std::string state_defend("defend");
std::string state_defend_move("defend+move");

BotRunner::BotRunner(void) {
  ;
}

int BotRunner::run(BotLanguage &language, int argc, char* argv[]) {

  Arena arena;
  int exitval = 0;

  if (argc != 2) {
    std::cerr << "0 0 FATAL: incorrect program usage: " << argv[0] << " (path to bot file)" << std::endl;
    return 1;
  }

  for(;;) {

    std::string line;
    std::getline(std::cin, line);
    std::stringstream linestream;
    linestream << line;
    std::string word;
    std::vector<std::string> linevec;

    // read next command
    while (getline(linestream, word, ' ')) {
      linevec.push_back(word);
    }

    // init id width height duration
    if (cmd_init.compare(linevec[0]) == 0) {
      arena.id = Utility::str2int(linevec[1]);
      arena.w = Utility::str2int(linevec[2]);
      arena.h = Utility::str2int(linevec[3]);
      arena.d = Utility::str2int(linevec[4]);
      arena.t = -1;
      try {
        language.init(argv[1]);
      }
      catch (BotRunnerException &e) {
        std::cerr << e.line << ' ' << e.column << ' ' << e.msg << '.' << std::endl;
        exitval = 1;
        break;
      }
      catch (std::exception &e) {
        std::cerr << "0 0 " << e.what() << '.' << std::endl;
        exitval = 2;
        break;
      }
      catch (...) {
        std::cerr << "0 0 " << "Unknown exception occurred in language.init()." << std::endl;
        exitval = 3;
        break;
      }
      std::cout << "ok" << std::endl;
    }

    // state-change
    else if (cmd_state_change.compare(linevec[0]) == 0) {
      std::stringstream state;
      try {
        language.stateChange(arena, state);
      }
      catch (BotRunnerException &e) {
        std::cerr << e.line << ' ' << e.column << ' ' << e.msg << '.' << std::endl;
        exitval = 4;
        break;
      }
      catch (std::exception &e) {
        std::cerr << "0 0 " << e.what() << '.' << std::endl;
        exitval = 5;
        break;
      }
      catch (...) {
        std::cerr << "0 0 " << "Unknown exception occurred in language.stateChange()." << std::endl;
        exitval = 6;
        break;
      }
      const char *cstate = state.str().c_str();
      if ((state_stop.compare(cstate) != 0) &&
        (state_move.compare(cstate) != 0) &&
        (state_attack.compare(cstate) != 0) &&
        (state_attack_move.compare(cstate) != 0) &&
        (state_defend.compare(cstate) != 0) &&
        (state_defend_move.compare(cstate) != 0)) {
        std::cerr << "0 0 FATAL: broken stateChange() implementation, returned unknown state, should be one of: (stop,move,attack,attack+move,defend,defend+move) but received: " << state << std::endl;
        exitval = 7;
        break;
      }
      std::cout << cstate << std::endl;
    }

    // aim weapon
    else if (cmd_aim.compare(linevec[0]) == 0) {
      double rval = 0.0;
      try {
        language.aim(arena, arena.bot.weapons[Utility::str2int(linevec[1])], rval);
      }
      catch (BotRunnerException &e) {
        std::cerr << e.line << ' ' << e.column << ' ' << e.msg << '.' << std::endl;
        exitval = 8;
        break;
      }
      catch (std::exception &e) {
        std::cerr << "0 0 " << e.what() << '.' << std::endl;
        exitval = 9;
        break;
      }
      catch (...) {
        std::cerr << "0 0 " << "Unknown exception occurred in language.aim()." << std::endl;
        exitval = 10;
        break;
      }
      std::cout << rval << std::endl;
    }

    // move
    else if (cmd_move.compare(linevec[0]) == 0) {
      double speed = 0.0;
      std::stringstream dir;
      try {
        language.move(arena, dir, speed);
      }
      catch (BotRunnerException &e) {
        std::cerr << e.line << ' ' << e.column << ' ' << e.msg << '.' << std::endl;
        exitval = 11;
        break;
      }
      catch (std::exception &e) {
        std::cerr << "0 0 " << e.what() << '.' << std::endl;
        exitval = 12;
        break;
      }
      catch (...) {
        std::cerr << "0 0 " << "Unknown exception occurred in language.move()." << std::endl;
        exitval = 13;
        break;
      }
      const char *cdir = dir.str().c_str();
      if ((dir_n.compare(cdir) != 0) &&
        (dir_ne.compare(cdir) != 0) &&
        (dir_e.compare(cdir) != 0) &&
        (dir_se.compare(cdir) != 0) &&
        (dir_s.compare(cdir) != 0) &&
        (dir_sw.compare(cdir) != 0) &&
        (dir_w.compare(cdir) != 0) &&
        (dir_nw.compare(cdir) != 0)) {
        std::cerr << "0 0 FATAL: broken move() implementation, first element of returned array returned unknown direction should be one of: (n,ne,e,se,s,sw,w,nw) but received: " << dir << std::endl;
        exitval = 14;
        break;
      }
      std::cout << cdir << ' ' << speed << std::endl;
    }

    else if (cmd_collision_obstacle.compare(linevec[0]) == 0) {
      bool self = Utility::str2int(linevec[1]) != 0;
      int botid = Utility::str2int(linevec[2]);
      int obstacleid = Utility::str2int(linevec[3]);
      int x = Utility::str2int(linevec[4]);
      int y = Utility::str2int(linevec[5]);
      double dmg = Utility::str2double(linevec[6]);
      try {
        language.collisionWithObstacle(
          arena, self, self ? arena.bot : arena.enemies[botid],
          arena.obstacles[obstacleid], x, y, dmg
        );
      }
      catch (BotRunnerException &e) {
        std::cerr << e.line << ' ' << e.column << ' ' << e.msg << '.' << std::endl;
        exitval = 15;
        break;
      }
      catch (std::exception &e) {
        std::cerr << "0 0 " << e.what() << '.' << std::endl;
        exitval = 16;
        break;
      }
      catch (...) {
        std::cerr << "0 0 " << "Unknown exception occurred in language.move()." << std::endl;
        exitval = 17;
        break;
      }
      std::cout << "ok" << std::endl;
    }

    else if (cmd_collision_bot.compare(linevec[0]) == 0) {
      bool self = Utility::str2int(linevec[1]) != 0;
      int botid = Utility::str2int(linevec[2]);
      int targetid = Utility::str2int(linevec[3]);
      double dmg = Utility::str2double(linevec[4]);
      try {
        language.collisionWithBot(
          arena, self, self ? arena.bot : arena.enemies[botid],
          (targetid == arena.bot.id) ? arena.bot : arena.enemies[targetid], dmg
        );
      }
      catch (BotRunnerException &e) {
        std::cerr << e.line << ' ' << e.column << ' ' << e.msg << '.' << std::endl;
        exitval = 18;
        break;
      }
      catch (std::exception &e) {
        std::cerr << "0 0 " << e.what() << '.' << std::endl;
        exitval = 19;
        break;
      }
      catch (...) {
        std::cerr << "0 0 " << "Unknown exception occurred in language.move()." << std::endl;
        exitval = 20;
        break;
      }
      std::cout << "ok" << std::endl;
    }

    else if (cmd_shot_fired_obstacle.compare(linevec[0]) == 0) {
      bool self = Utility::str2int(linevec[1]) != 0;
      int botid = Utility::str2int(linevec[2]);
      int obstacleid = Utility::str2int(linevec[3]);
      int x = Utility::str2int(linevec[4]);
      int y = Utility::str2int(linevec[5]);
      double a = Utility::str2double(linevec[6]);
      try {
        language.shotFiredHitObstacle(
          arena, self, self ? arena.bot : arena.enemies[botid],
          arena.obstacles[obstacleid], x, y, a
        );
      }
      catch (BotRunnerException &e) {
        std::cerr << e.line << ' ' << e.column << ' ' << e.msg << '.' << std::endl;
        exitval = 21;
        break;
      }
      catch (std::exception &e) {
        std::cerr << "0 0 " << e.what() << '.' << std::endl;
        exitval = 22;
        break;
      }
      catch (...) {
        std::cerr << "0 0 " << "Unknown exception occurred in language.move()." << std::endl;
        exitval = 23;
        break;
      }
      std::cout << "ok" << std::endl;
    }

    else if (cmd_shot_fired_bot.compare(linevec[0]) == 0) {
      bool self = Utility::str2int(linevec[1]) != 0;
      int botid = Utility::str2int(linevec[2]);
      int targetid = Utility::str2int(linevec[3]);
      double a = Utility::str2double(linevec[4]);
      double dmg = Utility::str2double(linevec[5]);
      try {
        language.shotFiredHitBot(
          arena, self, self ? arena.bot : arena.enemies[botid],
          (targetid == arena.bot.id) ? arena.bot : arena.enemies[targetid],
          a, dmg
        );
      }
      catch (BotRunnerException &e) {
        std::cerr << e.line << ' ' << e.column << ' ' << e.msg << '.' << std::endl;
        exitval = 24;
        break;
      }
      catch (std::exception &e) {
        std::cerr << "0 0 " << e.what() << '.' << std::endl;
        exitval = 25;
        break;
      }
      catch (...) {
        std::cerr << "0 0 " << "Unknown exception occurred in language.move()." << std::endl;
        exitval = 26;
        break;
      }
      std::cout << "ok" << std::endl;
    }

    // time t
    else if (cmd_time.compare(linevec[0]) == 0) {
      arena.t = Utility::str2int(linevec[1]);
      std::cout << "ok" << std::endl;
    }

    // obstacle id x y radius
    else if (cmd_obstacle.compare(linevec[0]) == 0) {
      int id = Utility::str2int(linevec[1]);
      arena.obstacles[id].id = id;
      arena.obstacles[id].x = Utility::str2int(linevec[2]);
      arena.obstacles[id].y = Utility::str2int(linevec[3]);
      arena.obstacles[id].radius = Utility::str2int(linevec[4]);
      std::cout << "ok" << std::endl;
    }

    // obstacle-in-range id inrange
    else if (cmd_obstacle_inrange.compare(linevec[0]) == 0) {
      int id = Utility::str2int(linevec[1]);
      arena.obstacles[id].inrange = (Utility::str2int(linevec[2]) == 1) ? true : false;
      std::cout << "ok" << std::endl;
    }

    // weapon id power aim
    else if (cmd_weapon.compare(linevec[0]) == 0) {
      int id = Utility::str2int(linevec[1]);
      arena.bot.weapons[id].id = id;
      arena.bot.weapons[id].power = Utility::str2double(linevec[2]);
      arena.bot.weapons[id].aim = Utility::str2double(linevec[3]);
      std::cout << "ok" << std::endl;
    }

    // enemy id x y energy condition speed inrange
    else if (cmd_enemy.compare(linevec[0]) == 0) {
      int id = Utility::str2int(linevec[1]);
      arena.enemies[id].id = id;
      arena.enemies[id].x = Utility::str2int(linevec[2]);
      arena.enemies[id].y = Utility::str2int(linevec[3]);
      arena.enemies[id].energy = Utility::str2int(linevec[4]);
      arena.enemies[id].condition = Utility::str2double(linevec[5]);
      arena.enemies[id].speed = Utility::str2double(linevec[6]);
      arena.enemies[id].inrange = (Utility::str2int(linevec[7]) == 1) ? true : false;
      std::cout << "ok" << std::endl;
    }

    // enemy enemyid weaponid power aim
    else if (cmd_enemy_weapon.compare(linevec[0]) == 0) {
      int eid = Utility::str2int(linevec[1]);
      int wid = Utility::str2int(linevec[2]);
      arena.enemies[eid].weapons[wid].power = Utility::str2double(linevec[3]);
      arena.enemies[eid].weapons[wid].aim = Utility::str2double(linevec[4]);
      std::cout << "ok" << std::endl;
    }

    // bot id x y energy condition speed
    else if (cmd_bot.compare(linevec[0]) == 0) {
      arena.bot.id = Utility::str2int(linevec[1]);
      arena.bot.x = Utility::str2int(linevec[2]);
      arena.bot.y = Utility::str2int(linevec[3]);
      arena.bot.energy = Utility::str2double(linevec[4]);
      arena.bot.condition = Utility::str2double(linevec[5]);
      arena.bot.speed = Utility::str2double(linevec[6]);
      std::cout << "ok" << std::endl;
    }

    // quit
    else if (cmd_quit.compare(linevec[0]) == 0) {
      break;
    }

    // unhandled command
    else {
      std::cerr << "0 0 Unknown command: " << linevec[0] << std::endl;
    }

    std::cout.flush();
    std::cerr.flush();
  }

  // exit
  std::cout.flush();
  std::cerr.flush();
  return exitval;
}

BotRunner::~BotRunner(void) {
  ;
}
