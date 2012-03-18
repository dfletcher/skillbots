#include <streambuf>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <v8.h>

using namespace v8;

std::string init_cmd("init");
std::string stateChange_cmd("stateChange");
std::string aim_cmd("aim");
std::string move_cmd("move");
std::string quit_cmd("quit");
std::string time_cmd("time");
std::string obstacle_cmd("obstacle");
std::string obstacle_inrange_cmd("obstacle-in-range");
std::string weapon_cmd("weapon");
std::string enemy_cmd("enemy");
std::string enemy_weapon_cmd("enemy-weapon");
std::string bot_cmd("bot");

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

class Arena {
  public:
    int w, h, t;
    Arena() {
      w = 0;
      h = 0;
      t = 0;
    }
};

class Obstacle {
  public:
    int x, y, radius;
    bool inrange;
    Obstacle() {
      x = 0;
      y = 0;
      radius = 0;
      inrange = false;
    }
};

class Weapon {
  public:
    double power, aim;
    Weapon() {
      power = 0.0;
      aim = 0.0;
    }
};

class Enemy {
  public:
    int x, y;
    bool inrange;
    double energy, condition, speed;
    std::map<int, Weapon> weapons;
    Enemy() {
      x = 0;
      y = 0;
      energy = 0.0;
      condition = 0.0;
      speed = 0.0;
      inrange = false;
    }
};

int readfile(const char *path, std::stringstream &buf) {
  int status = 0;
  std::ifstream i;
  i.open(path);
  if (i.is_open()) {
    buf << i.rdbuf();
    status = 1;
  }
  i.close();
  return status;
}

int str2int(std::string s) {
  int n;
  std::istringstream(s) >> n;
  return n;
}

double str2double(std::string s) {
  double n;
  std::istringstream(s) >> n;
  return n;
}

int handle_init(Persistent<Context> &context, const char *path, Persistent<Object> *bot, Persistent<Value> *stateChange, Persistent<Value> *aim, Persistent<Value> *move) {

  // Read source file.
  std::stringstream botcode;
  if (!readfile(path, botcode)) {
    std::cout << "FATAL: could not read source file: " << path << std::endl;
    return 0;
  }
  Handle<String> source = String::New(botcode.str().c_str());

  // Compile.
  Handle<Script> script;
  try {
    script = Script::Compile(source, String::New(path));
  }
  catch(...) {
    std::cout << "FATAL: exception compiling source file: " << path << std::endl;
    return 0;
  }
  if (!*script) {
    std::cout << "FATAL: failed to compile source file: " << path << std::endl;
    return 0;
  }

  // Evaluate.
  try {
    script->Run();
  }
  catch(...) {
    std::cout << "FATAL: exception evaluating source file: " << path << std::endl;
    return 0;
  }

  // The bot code should have left a global Bot object, test it.
  Handle<Value> _bot = context->Global()->Get(String::New("Bot"));
  if (!_bot->IsObject()) {
    std::cout << "FATAL: global Bot object not defined in: " << path << std::endl;
    return 0;
  }
  *bot = Persistent<Object>::New(_bot->ToObject());

  // The bot object must have stateChange() function.
  Handle<Value> func = (*bot)->Get(String::New("stateChange"));
  if (!func->IsFunction()) {
    std::cout << "FATAL: Bot.stateChange() not defined in: " << path << std::endl;
    return 0;
  }
  *stateChange = Persistent<Value>::New(func);

  // The bot object must have aim() function.
  func = (*bot)->Get(String::New("aim"));
  if (!func->IsFunction()) {
    std::cout << "FATAL: Bot.aim() not defined in: " << path << std::endl;
    return 0;
  }
  *aim = Persistent<Value>::New(func);

  // The bot object must have move() function.
  func = (*bot)->Get(String::New("move"));
  if (!func->IsFunction()) {
    std::cout << "FATAL: Bot.move() not defined in: " << path << std::endl;
    return 0;
  }
  *move = Persistent<Value>::New(func);

  // All good.
  return 1;
}

int handle_stateChange(Handle<Object> &bot, Handle<Value> *args, Persistent<Value> &stateChange) {
  Handle<Value> result = Function::Cast(*stateChange)->Call(bot, 4, args);
  if (!result->IsString()) {
    std::cout << "FATAL: broken stateChange() implementation, it must return a string." << std::endl;
    return 0;
  }
  String::AsciiValue ascii(result);
  if ((state_stop.compare(*ascii) != 0) && (state_move.compare(*ascii) != 0) && (state_attack.compare(*ascii) != 0) && (state_attack_move.compare(*ascii) != 0) && (state_defend.compare(*ascii) != 0) && (state_defend_move.compare(*ascii) != 0)) {
    std::cout << "FATAL: broken stateChange() implementation, returned unknown state, should be one of: (stop,move,attack,attack+move,defend,defend+move) but received: " << *ascii << std::endl;
    return 0;
  }
  std::cout << *ascii << std::endl;
  return 1;
}

int handle_move(Handle<Object> &bot, Handle<Value> *args, Persistent<Value> &move) {
  Handle<Value> result = Function::Cast(*move)->Call(bot, 4, args);
  if (!result->IsArray()) {
    std::cout << "FATAL: broken move() implementation, did not return an array." << std::endl;
    return 0;
  }
  Array *a = Array::Cast(*result);
  if (a->Length() != 2) {
    std::cout << "FATAL: broken move() implementation, returned array must contain exactly two items." << std::endl;
    return 0;
  }
  Handle<Value> a0 = a->Get(0);
  if (!a0->IsString()) {
    std::cout << "FATAL: broken move() implementation, first element of returned array must be a string." << std::endl;
    return 0;
  }
  String::AsciiValue a0ascii(a0);
  if ((dir_n.compare(*a0ascii) != 0) && (dir_ne.compare(*a0ascii) != 0) && (dir_e.compare(*a0ascii) != 0) && (dir_se.compare(*a0ascii) != 0) && (dir_s.compare(*a0ascii) != 0) && (dir_sw.compare(*a0ascii) != 0) && (dir_w.compare(*a0ascii) != 0) && (dir_nw.compare(*a0ascii) != 0)) {
    std::cout << "FATAL: broken move() implementation, first element of returned array returned unknown direction should be one of: (n,ne,e,se,s,sw,w,nw) but received: " << *a0ascii << std::endl;
    return 0;
  }
  Handle<Value> a1 = a->Get(1);
  if (!a1->IsNumber()) {
    std::cout << "FATAL: broken move() implementation, second element of returned array must be numeric." << std::endl;
    return 0;
  }
  String::AsciiValue a1ascii(a1);
  std::cout << *a0ascii << " " << *a1ascii << std::endl;
  return 1;
}

int handle_aim(Handle<Object> &bot, Handle<Value> *args, Persistent<Value> &aim) {
  Handle<Value> result = Function::Cast(*aim)->Call(bot, 4, args);
  if (!result->IsNumber()) {
    std::cout << "FATAL: broken aim() implementation, it must return a number." << std::endl;
    return 0;
  }
  String::AsciiValue ascii(result);
  std::cout << *ascii << std::endl;
  return 1;
}

void reset_args(Handle<Value> *args) {
  args[0] = Undefined();
  args[1] = Undefined();
  args[2] = Undefined();
  args[3] = Undefined();
}

void prepare_args(Handle<Value> *args, Arena &arena, std::map<int, Enemy> &enemies, std::map<int, Obstacle> &obstacles, int weaponid=0) {

  // sig: arena, enemies, obstacles, weapon

  // arena
  Handle<Object> argarena = Object::New();
  argarena->Set(String::New("width"), Integer::New(arena.w));
  argarena->Set(String::New("height"), Integer::New(arena.h));
  argarena->Set(String::New("time"), Integer::New(arena.t));
  args[0] = argarena;

  // enemies
  Handle<Array> argenemies = Array::New();
  for (std::map<int, Enemy>::iterator ienemies = enemies.begin(); ienemies != enemies.end(); ienemies++) {
    if (ienemies->second.inrange) {
      Handle<Object> enemy = Object::New();
      enemy->Set(String::New("id"), Integer::New(ienemies->first));
      enemy->Set(String::New("x"), Integer::New(ienemies->second.x));
      enemy->Set(String::New("y"), Integer::New(ienemies->second.y));
      enemy->Set(String::New("energy"), Number::New(ienemies->second.energy));
      enemy->Set(String::New("condition"), Number::New(ienemies->second.condition));
      enemy->Set(String::New("speed"), Number::New(ienemies->second.speed));
      Handle<Array> argweapons = Array::New();
      for (std::map<int, Weapon>::iterator iweapons = ienemies->second.weapons.begin(); iweapons != ienemies->second.weapons.end(); iweapons++) {
        Handle<Object> weapon = Object::New();
        weapon->Set(String::New("power"), Number::New(iweapons->second.power));
        weapon->Set(String::New("aim"), Number::New(iweapons->second.aim));
        argweapons->Set(argweapons->Length(), weapon);
      }
      enemy->Set(String::New("weapons"), argweapons);
      argenemies->Set(argenemies->Length(), enemy);
    }
  }
  args[1] = argenemies;

  // obstacles
  Handle<Array> argobstacles = Array::New();
  for (std::map<int, Obstacle>::iterator iobstacles = obstacles.begin(); iobstacles != obstacles.end(); iobstacles++) {
    if (iobstacles->second.inrange) {
      Handle<Object> obstacle = Object::New();
      obstacle->Set(String::New("id"), Integer::New(iobstacles->first));
      obstacle->Set(String::New("x"), Integer::New(iobstacles->second.x));
      obstacle->Set(String::New("y"), Integer::New(iobstacles->second.y));
      obstacle->Set(String::New("radius"), Number::New(iobstacles->second.radius));
      argobstacles->Set(argobstacles->Length(), obstacle);
    }
  }
  args[2] = argobstacles;

  // weapon
  args[3] = Integer::New(weaponid);
}

int main(int argc, char* argv[]) {

  int exitval = 0;
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);
  Arena arena;
  bool need_js_init = true;
  Handle<Value> args[4];
  Persistent<Object> bot;
  Persistent<Value> stateChange, aim, move;
  std::map<int, Obstacle> obstacles;
  std::map<int, Weapon> weapons;
  std::map<int, Enemy> enemies;

  reset_args(args);

  if (argc != 2) {
    std::cout << "FATAL: incorrect program usage: " << argv[0] << " (path to bot file)" << std::endl;
    return 1;
  }

  for(;;) {
    std::string line;
    std::getline(std::cin, line);
    std::stringstream linestream;
    linestream << line;
    std::string word;
    std::vector<std::string> linevec;
    while (getline(linestream, word, ' ')) {
      linevec.push_back(word);
    }
    if (init_cmd.compare(linevec[0]) == 0) {
      if (need_js_init) {
        handle_init(context, argv[1], &bot, &stateChange, &aim, &move);
        need_js_init = false;
      }
      bot->Set(String::New("state"), String::New("move"));
      arena.w = str2int(linevec[1]);
      arena.h = str2int(linevec[2]);
      arena.t = 0;
      std::cout << "ok" << std::endl;
    }
    else if (stateChange_cmd.compare(linevec[0]) == 0) {
      HandleScope local_scope;
      prepare_args(args, arena, enemies, obstacles);
      if (!handle_stateChange(bot, args, stateChange)) {
        exitval = 2;
        break;
      }
      reset_args(args);
    }
    else if (aim_cmd.compare(linevec[0]) == 0) {
      HandleScope local_scope;
      prepare_args(args, arena, enemies, obstacles, str2int(linevec[1]));
      if (!handle_aim(bot, args, aim)) {
        exitval = 3;
        break;
      }
      reset_args(args);
    }
    else if (move_cmd.compare(linevec[0]) == 0) {
      HandleScope local_scope;
      prepare_args(args, arena, enemies, obstacles);
      if (!handle_move(bot, args, move)) {
        exitval = 4;
        break;
      }
      reset_args(args);
    }
    else if (time_cmd.compare(linevec[0]) == 0) {
      arena.t = str2int(linevec[1]);
      std::cout << "ok" << std::endl;
    }
    else if (obstacle_cmd.compare(linevec[0]) == 0) {
      int id = str2int(linevec[1]);
      obstacles[id].x = str2int(linevec[2]);
      obstacles[id].y = str2int(linevec[3]);
      obstacles[id].radius = str2int(linevec[4]);
      std::cout << "ok" << std::endl;
    }
    else if (obstacle_inrange_cmd.compare(linevec[0]) == 0) {
      int id = str2int(linevec[1]);
      obstacles[id].inrange = (str2int(linevec[2]) == 0) ? true : false;
    }
    else if (weapon_cmd.compare(linevec[0]) == 0) {
      int id = str2int(linevec[1]);
      weapons[id].power = str2double(linevec[2]);
      weapons[id].aim = str2double(linevec[3]);
      std::cout << "ok" << std::endl;
    }
    else if (enemy_cmd.compare(linevec[0]) == 0) {
      int id = str2int(linevec[1]);
      enemies[id].x = str2double(linevec[2]);
      enemies[id].y = str2double(linevec[3]);
      enemies[id].energy = str2double(linevec[4]);
      enemies[id].condition = str2double(linevec[5]);
      enemies[id].speed = str2double(linevec[6]);
      enemies[id].inrange = (str2int(linevec[7]) == 0) ? true : false;
      std::cout << "ok" << std::endl;
    }
    else if (enemy_weapon_cmd.compare(linevec[0]) == 0) {
      int eid = str2int(linevec[1]);
      int wid = str2int(linevec[2]);
      enemies[eid].weapons[wid].power = str2double(linevec[3]);
      enemies[eid].weapons[wid].aim = str2double(linevec[4]);
      std::cout << "ok" << std::endl;
    }
    else if (bot_cmd.compare(linevec[0]) == 0) {
      bot->Set(String::New("x"), Integer::New(str2int(linevec[1])));
      bot->Set(String::New("y"), Integer::New(str2int(linevec[2])));
      bot->Set(String::New("energy"), Number::New(str2double(linevec[3])));
      bot->Set(String::New("condition"), Number::New(str2double(linevec[4])));
      bot->Set(String::New("speed"), Number::New(str2double(linevec[5])));
      std::cout << "ok" << std::endl;
    }
    else if (quit_cmd.compare(linevec[0]) == 0) {
      break;
    }
    else {
      std::cerr << "Unknown command: " << linevec[0] << std::endl;
    }
  }

  // Dispose the persistent context.
  stateChange.Dispose();
  aim.Dispose();
  move.Dispose();
  bot.Dispose();
  context.Dispose();

  // Convert the result to an ASCII string and print it.
  return exitval;
}
