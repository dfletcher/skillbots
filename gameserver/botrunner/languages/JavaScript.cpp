
#include <v8.h>
#include <BotRunner.hpp>

using namespace v8;

class JavaScriptException : public BotRunnerException {
  public:
    JavaScriptException(const char *func, Handle<Message> message) {
      line = message->GetLineNumber();
      column = message->GetStartColumn();
      std::stringstream smsg;
      String::AsciiValue sourceline(message->GetSourceLine());
      String::AsciiValue jsmsg(message->Get());
      smsg << "Exception occurred in " << func << " at line " << line;
      smsg << ", column " << column << ". " << *jsmsg << '.' << std::endl;
      Handle<StackTrace> st = message->GetStackTrace();
      if (*st) {
        for (uint32_t i=0; i < st->GetFrameCount(); i++) {
          Handle<StackFrame> sf = st->GetFrame(i);
          String::AsciiValue jsfunc(sf->GetFunctionName());
          smsg << " at " << *jsfunc << ", line " << sf->GetLineNumber();
          smsg << ", column " << sf->GetColumn() << std::endl;
        }
      }
      msg = smsg.str();
    }
    ~JavaScriptException() throw() { ; }
};

class JavaScript : public BotLanguage {

  public:

    JavaScript(Persistent<Context> c) {
      context = c;
    }

    void init(const char *path) {

      TryCatch exception;
      HandleScope handle_scope;

      // Read source file.
      std::stringstream botcode;
      if (!Utility::readfile(path, botcode)) {
        std::stringstream msg;
        msg << "FATAL: could not read source file: " << path << std::endl;
        throw BotRunnerException(msg);
      }
      Handle<String> source = String::New(botcode.str().c_str());

      // Compile.
      Handle<Script> script;
      try {
        script = Script::Compile(source, String::New(path));
      }
      catch(...) {
        std::stringstream msg;
        msg << "FATAL: exception compiling source file: " << path << std::endl;
        throw BotRunnerException(msg);
      }
      if (exception.HasCaught()) {
        throw JavaScriptException("init", exception.Message());
      }
      if (!*script) {
        std::stringstream msg;
        msg << "FATAL: failed to compile source file: " << path << std::endl;
        throw BotRunnerException(msg);
      }

      // Evaluate.
      try {
        script->Run();
      }
      catch(...) {
        std::stringstream msg;
        msg << "FATAL: exception evaluating source file: " << path << std::endl;
        throw BotRunnerException(msg);
      }

      // The bot code should have left a global Bot object, test it.
      Handle<Value> _bot = context->Global()->Get(String::New("Bot"));
      if (!_bot->IsObject()) {
        std::stringstream msg;
        msg << "FATAL: global Bot object not defined in: " << path << std::endl;
        throw BotRunnerException(msg);
      }

      // Create the JS arena object.
      arenaObject = Persistent<Object>::New(Object::New());
      Local<Object> bot = _bot->ToObject();
      arenaObject->Set(String::New("bot"), bot);
      arenaObject->Set(String::New("enemies"), Array::New());
      arenaObject->Set(String::New("obstacles"), Array::New());

      // The bot object must have stateChange() function.
      Handle<Value> func = (*bot)->Get(String::New("stateChange"));
      if (!func->IsFunction()) {
        std::stringstream msg;
        msg << "FATAL: Bot.stateChange() not defined in: " << path << std::endl;
        throw BotRunnerException(msg);
      }
      stateChangeImpl = Persistent<Value>::New(func);

      // The bot object must have aim() function.
      func = (*bot)->Get(String::New("aim"));
      if (!func->IsFunction()) {
        std::stringstream msg;
        msg << "FATAL: Bot.aim() not defined in: " << path << std::endl;
        throw BotRunnerException(msg);
      }
      aimImpl = Persistent<Value>::New(func);

      // The bot object must have move() function.
      func = (*bot)->Get(String::New("move"));
      if (!func->IsFunction()) {
        std::stringstream msg;
        msg << "FATAL: Bot.move() not defined in: " << path << std::endl;
        throw BotRunnerException(msg);
      }
      moveImpl = Persistent<Value>::New(func);

      // Optional collisionWithObstacle() function.
      func = (*bot)->Get(String::New("collisionWithObstacle"));
      if (func->IsFunction()) {
        collisionObstacleImpl = Persistent<Value>::New(func);
      }
      else {
        collisionObstacleImpl = Persistent<Value>::New(Undefined());
      }

      // Optional collisionWithBot() function.
      func = (*bot)->Get(String::New("collisionWithBot"));
      if (func->IsFunction()) {
        collisionBotImpl = Persistent<Value>::New(func);
      }
      else {
        collisionBotImpl = Persistent<Value>::New(Undefined());
      }

      // Optional shotFiredHitObstacle() function.
      func = (*bot)->Get(String::New("shotFiredHitObstacle"));
      if (func->IsFunction()) {
        shotFiredObstacleImpl = Persistent<Value>::New(func);
      }
      else {
        shotFiredObstacleImpl = Persistent<Value>::New(Undefined());
      }

      // Optional shotFiredHitBot() function.
      func = (*bot)->Get(String::New("shotFiredHitBot"));
      if (func->IsFunction()) {
        shotFiredBotImpl = Persistent<Value>::New(func);
      }
      else {
        shotFiredBotImpl = Persistent<Value>::New(Undefined());
      }
    }

    void stateChange(const Arena &arena, std::stringstream &rval) {
      TryCatch exception;
      HandleScope handle_scope;
      Handle<Object> bot = prepareArena(arena);
      Handle<Value> args[1] = { arenaObject };
      Handle<Value> result = Function::Cast(*stateChangeImpl)->Call(bot, 1, args);
      if (exception.HasCaught()) {
        throw JavaScriptException("stateChange", exception.Message());
      }
      if (!result->IsString()) {
        throw BotRunnerException("FATAL: broken stateChange() implementation, it must return a string.");
      }
      String::AsciiValue ascii(result);
      rval << *ascii;
    }

    void move(const Arena &arena, std::stringstream &rvaldir, double &rvalspeed) {
      TryCatch exception;
      HandleScope handle_scope;
      Handle<Object> bot = prepareArena(arena);
      Handle<Value> args[1] = { arenaObject };
      Handle<Value> result = Function::Cast(*moveImpl)->Call(bot, 1, args);
      if (exception.HasCaught()) {
        throw JavaScriptException("move", exception.Message());
      }
      if (!result->IsArray()) {
        throw BotRunnerException("FATAL: broken move() implementation, did not return an array.");
      }
      Array *a = Array::Cast(*result);
      if (a->Length() != 2) {
        throw BotRunnerException("FATAL: broken move() implementation, returned array must contain exactly two items.");
      }
      Handle<Value> a0 = a->Get(0);
      if (!a0->IsString()) {
        throw BotRunnerException("FATAL: broken move() implementation, first element of returned array must be a string.");
      }
      Handle<Value> a1 = a->Get(1);
      if (!a1->IsNumber()) {
        throw BotRunnerException("FATAL: broken move() implementation, second element of returned array must be numeric.");
      }
      String::AsciiValue a0ascii(a0);
      rvaldir << *a0ascii;
      rvalspeed = a1->ToNumber()->Value();
      String::AsciiValue a1ascii(a1);
    }

    void aim(const Arena &arena, const Weapon &weapon, double &rval) {
      TryCatch exception;
      HandleScope handle_scope;
      Handle<Object> bot = prepareArena(arena);
      Handle<Object> jsweapon = Local<Object>::New(Object::New());
      prepareWeapon(weapon, jsweapon);
      Handle<Value> args[2] = { arenaObject, jsweapon };
      Handle<Value> result = Function::Cast(*aimImpl)->Call(bot, 2, args);
      if (exception.HasCaught()) {
        throw JavaScriptException("aim", exception.Message());
      }
      if (!result->IsNumber()) {
        throw BotRunnerException("FATAL: broken aim() implementation, it must return a number.");
      }
      rval = result->ToNumber()->Value();
    }

    void collisionWithObstacle(const Arena &arena, bool self, const Bot &bot, const Obstacle &target, int x, int y, double damage) {
      if (collisionObstacleImpl->IsUndefined()) {
        return;
      }
      TryCatch exception;
      HandleScope handle_scope;
      Handle<Object> collidebot, selfbot, obstacle;
      prepareObstacle(target, obstacle);
      prepareBot(bot, collidebot);
      prepareBot(arena.bot, selfbot);
      Handle<Value> args[7] = {
        arenaObject, Boolean::New(self), collidebot, obstacle,
        Integer::New(x), Integer::New(y), Number::New(damage)
      };
      Function::Cast(*collisionObstacleImpl)->Call(selfbot, 7, args);
      exception.Reset();
    }

    void collisionWithBot(const Arena &arena, bool self, const Bot &bot, const Bot &target, int x, int y, double damage) {
      if (collisionBotImpl->IsUndefined()) {
        return;
      }
      TryCatch exception;
      HandleScope handle_scope;
      Handle<Object> collidebot, selfbot, targetbot;
      prepareBot(target, targetbot);
      prepareBot(bot, collidebot);
      prepareBot(arena.bot, selfbot);
      Handle<Value> args[7] = {
        arenaObject, Boolean::New(self), collidebot, targetbot,
        Integer::New(x), Integer::New(y), Number::New(damage)
      };
      Function::Cast(*collisionBotImpl)->Call(selfbot, 7, args);
      exception.Reset();
    }

    void shotFiredHitObstacle(const Arena &arena, bool self, const Bot &bot, const Obstacle &target, int x, int y, double angle) {
      if (shotFiredObstacleImpl->IsUndefined()) {
        return;
      }
      TryCatch exception;
      HandleScope handle_scope;
      Handle<Value> _firingbot;
      Handle<Object> firingbot, selfbot, obstacle;
      prepareObstacle(target, obstacle);
      if (bot.id == 0) {
        _firingbot = Null();
      }
      else {
        prepareBot(bot, firingbot);
        _firingbot = firingbot;
      }
      prepareBot(arena.bot, selfbot);
      Handle<Value> args[7] = {
        arenaObject, Boolean::New(self), _firingbot, obstacle,
        Integer::New(x), Integer::New(y), Number::New(angle)
      };
      Function::Cast(*shotFiredObstacleImpl)->Call(selfbot, 7, args);
      exception.Reset();
    }

    void shotFiredHitBot(const Arena &arena, bool self, const Bot &bot, const Bot &target, double angle, double damage) {
      if (shotFiredBotImpl->IsUndefined()) {
        return;
      }
      TryCatch exception;
      HandleScope handle_scope;
      Handle<Value> _firingbot;
      Handle<Object> firingbot, selfbot, targetbot;
      prepareBot(target, targetbot);
      if (bot.id == 0) {
        _firingbot = Null();
      }
      else {
        prepareBot(bot, firingbot);
        _firingbot = firingbot;
      }
      prepareBot(arena.bot, selfbot);
      Handle<Value> args[6] = {
        arenaObject, Boolean::New(self), _firingbot,
        targetbot, Number::New(angle), Number::New(damage)
      };
      Function::Cast(*shotFiredBotImpl)->Call(selfbot, 6, args);
      exception.Reset();
    }

    ~JavaScript(void) {
      stateChangeImpl.Dispose();
      aimImpl.Dispose();
      moveImpl.Dispose();
      arenaObject.Dispose();
      context.Dispose();
    }

  private:

    Persistent<Context> context;
    Persistent<Object> arenaObject;
    Persistent<Value> stateChangeImpl, aimImpl, moveImpl;
    Persistent<Value> collisionObstacleImpl, collisionBotImpl;
    Persistent<Value> shotFiredObstacleImpl, shotFiredBotImpl;

    void prepareWeapon(const Weapon &cppweapon, Handle<Object> jsweapon) {
      jsweapon->Set(String::New("id"), Integer::New(cppweapon.id));
      jsweapon->Set(String::New("power"), Number::New(cppweapon.power));
      jsweapon->Set(String::New("aim"), Number::New(cppweapon.aim));
    }

    void prepareObstacle(const Obstacle &cppobstacle, Handle<Object> jsobstacle) {
      jsobstacle->Set(String::New("id"), Integer::New(cppobstacle.id));
      jsobstacle->Set(String::New("x"), Integer::New(cppobstacle.x));
      jsobstacle->Set(String::New("y"), Integer::New(cppobstacle.y));
      jsobstacle->Set(String::New("w"), Integer::New(cppobstacle.w));
      jsobstacle->Set(String::New("h"), Integer::New(cppobstacle.h));
    }

    void prepareBot(const Bot &cppbot, Handle<Object> jsbot) {
      jsbot->Set(String::New("id"), Integer::New(cppbot.id));
      jsbot->Set(String::New("x"), Integer::New(cppbot.x));
      jsbot->Set(String::New("y"), Integer::New(cppbot.y));
      jsbot->Set(String::New("inrange"), Boolean::New(cppbot.inrange));
      jsbot->Set(String::New("energy"), Number::New(cppbot.energy));
      jsbot->Set(String::New("condition"), Number::New(cppbot.condition));
      jsbot->Set(String::New("speed"), Number::New(cppbot.speed));
      jsbot->Set(String::New("state"), String::New(cppbot.state.c_str()));
      Handle<Object> weapons;
      Handle<Value> _weapons = jsbot->Get(String::New("weapons"));
      if (_weapons->IsObject()) {
        weapons = _weapons->ToObject();
      }
      else {
        weapons = Local<Object>::New(Array::New());
        jsbot->Set(String::New("weapons"), weapons);
      }
      int idx = 0;
      for (WeaponMapIterator i = cppbot.weapons.begin(); i != cppbot.weapons.end(); i++) {
        Local<Object> weapon = Local<Object>::New(Object::New());
        prepareWeapon(i->second, weapon);
        weapons->Set(idx++, weapon);
      }
    }

    Handle<Object> prepareArena(const Arena &arena) {
      int idx;
      arenaObject->Set(String::New("id"), Integer::New(arena.id));
      arenaObject->Set(String::New("w"), Integer::New(arena.w));
      arenaObject->Set(String::New("h"), Integer::New(arena.h));
      arenaObject->Set(String::New("t"), Integer::New(arena.t));
      arenaObject->Set(String::New("d"), Integer::New(arena.d));
      Handle<Object> bot;
      Handle<Value> _bot = arenaObject->Get(String::New("bot"));
      if (_bot->IsObject()) {
        bot = _bot->ToObject();
      }
      else {
        bot = Local<Object>::New(Object::New());
        arenaObject->Set(String::New("bot"), bot);
      }
      prepareBot(arena.bot, bot);
      Handle<Object> enemies = Local<Object>::New(Array::New());
      arenaObject->Set(String::New("enemies"), enemies);
      idx = 0;
      for (BotMapIterator i = arena.enemies.begin(); i != arena.enemies.end(); i++) {
        if (i->second.inrange) {
          Local<Object> enemy = Local<Object>::New(Object::New());
          prepareBot(i->second, enemy);
          enemies->Set(idx++, enemy);
        }
      }
      Handle<Object> obstacles = Local<Object>::New(Array::New());
      arenaObject->Set(String::New("obstacles"), obstacles);
      idx = 0;
      for (ObstacleMapIterator i = arena.obstacles.begin(); i != arena.obstacles.end(); i++) {
        if (i->second.inrange) {
          Local<Object> obstacle = Local<Object>::New(Object::New());
          prepareObstacle(i->second, obstacle);
          obstacles->Set(idx++, obstacle);
        }
      }
      return bot;
    }
};

int main(int argc, char* argv[]) {
  V8::SetCaptureStackTraceForUncaughtExceptions(true, 25, StackTrace::kDetailed);
  HandleScope handle_scope;
  Persistent<Context> context = Context::New();
  Context::Scope context_scope(context);
  JavaScript language(context);
  BotRunner runner;
  return runner.run(language, argc, argv);
}
