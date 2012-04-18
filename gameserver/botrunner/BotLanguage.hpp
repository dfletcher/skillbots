/**
 *  @file BotLanguage.hpp
 *  BotLanguage interface.
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

/**
 *  This class is an interface that supports a bridge between bots in game
 *  and code written in a specific computer language. Implementations are
 *  expected to implement all virtual functions in this class.
 *
 *  The init() function will be called first and the implementation should
 *  load or compile the user code and prepare to execute game commands.
 *
 *  The stateChange(), move(), and aim() functions are the primary bot API and
 *  implementations should wrap the native C++ arguments and invoke the
 *  user implementation of the function. Values returned by these functions
 *  will be validated by the BotRunner calling these functions, so
 *  implementations are free to return results without checking for errors
 *  in output.
 *
 *  The collisionWithObstacle(), collisionWithBot(), shotFiredHitObstacle() and
 *  shotFiredHitBot() functions are callbacks. If the function is implemented
 *  in the target language, the arguments should be wrapped and the user
 *  callback invoked. Users are not required to implement any of these
 *  functions. The functions all have a void return type, any returned values
 *  should be ignored or deleted as appropriate.
 *
 */
class BotLanguage {

  public:

    /**
     *  Language modules are required to implement this function. The path is a
     *  path to bot source code written in the target language. Implementations
     *  must compile or evaluate this file and make any necessary preparations
     *  that need to happen early, before other BotLanguage API is called from
     *  the main arena loop.
     *
     *  In addition to loading code and doing language runtime init, this
     *  function should also check that stateChange(), move(), and aim() are
     *  existing functions implemented within the loaded code. If these are not
     *  found, or found to be malformed somehow, init() should throw a
     *  BotRunnerException.
     *
     *  The function may also check for the availability of
     *  collisionWithObstacle(), collisionWithBot(), shotFiredHitObstacle() and
     *  shotFiredHitBot() implementations in the loaded code. However, these
     *  must not be required for the bot to function.
     *
     *  @param path Path to the source code file.
     *
     *  @todo: Better support for compiled languages. API for associating
     *  source code path with compiled output path.
     */
    virtual void init(const char *path)=0;

    /**
     *  Call the stateChange() API function in the target language.
     *  @param arena Arena where the game is taking place.
     *  @param rval Output parameter. Implementations should << concat the
     *  string value returned by stateChange implementation.
     */
    virtual void stateChange(const Arena &arena, std::stringstream &rval)=0;

    /**
     *  Call the move() API function in the target language. All parameters
     *  should be wrapped and passed to user move().
     *  @param arena Arena where the game is taking place.
     *  @param rvaldir Output parameter. Implementations should << concat the
     *  direction value returned by move implementation.
     *  @param rvalspeed Output parameter. Implementations should set this to the
     *  speed value returned by move implementation.
     */
    virtual void move(const Arena&, std::stringstream &rvaldir, double &rvalspeed)=0;

    /**
     *  Call the aim() API function in the target language. All parameters
     *  should be wrapped and passed to user aim().
     *  @param arena Arena where the game is taking place.
     *  @param weapon Weapon that will fire.
     *  @param rval Output parameter. Implementations should set this to the
     *  angle value returned by aim implementation.
     */
    virtual void aim(const Arena &arena, const Weapon &weapon, double &rval)=0;

    /**
     *  Call the collisionWithObstacle() API function in the target language,
     *  if it exists. All parameters should be wrapped and passed to user
     *  collisionWithObstacle().
     *  @param arena Arena where the game is taking place.
     *  @param self True if bot is the user's bot.
     *  @param obstacle Obstacle involved in the collision.
     *  @param other Other Bot involved in collision.
     *  @param damage Damage the bot received.
     */
    virtual void collisionWithObstacle(const Arena &arena, bool self, const Bot &bot, const Obstacle &obstacle, double damage)=0;

    /**
     *  Call the collisionWithBot() API function in the target language,
     *  if it exists. All parameters should be wrapped and passed to user
     *  collisionWithBot().
     *  @param arena Arena where the game is taking place.
     *  @param self True if bot is the user's bot.
     *  @param bot Bot involved in the collision.
     *  @param other Other Bot involved in collision.
     *  @param damage Damage the bot received.
     */
    virtual void collisionWithBot(const Arena &arena, bool self, const Bot &bot, const Bot &other, double damage)=0;

    /**
     *  Call the shotFiredHitObstacle() API function in the target language,
     *  if it exists. All parameters should be wrapped and passed to user
     *  shotFiredHitObstacle().
     *  @param arena Arena where the game is taking place.
     *  @param self True if bot is the user's bot.
     *  @param bot Bot doing the firing. bot.id might equal zero, which means
     *  that the bot doing the firing was not in sensor range of this bot.
     *  Implementations should pass NULL or similar sentinel parameter in this
     *  case.
     *  @param obstacle Obstacle hit by the shot.
     *  @param angle Angle of the shot.
     */
    virtual void shotFiredHitObstacle(const Arena &arena, bool self, const Bot &bot, const Obstacle &obstacle, double angle)=0;

    /**
     *  Call the shotFiredHitBot() API function in the target language,
     *  if it exists. All parameters should be wrapped and passed to user
     *  shotFiredHitBot().
     *  @param arena Arena where the game is taking place.
     *  @param self True if bot is the user's bot.
     *  @param bot Bot doing the firing. bot.id might equal zero, which means
     *  that the bot doing the firing was not in sensor range of this bot.
     *  Implementations should pass NULL or similar sentinel parameter in this
     *  case.
     *  @param other Bot being fired upon.
     *  @param obstacle Obstacle hit by the shot.
     *  @param angle Angle of the shot.
     *  @param damage Damage the bot received.
     */
    virtual void shotFiredHitBot(const Arena&, bool self, const Bot &bot, const Bot &other, double angle, double damage)=0;

    /**
     *  Implementations can use this for cleanup, if needed.
     */
    virtual ~BotLanguage(void);
};

#endif /* [BOTRUNNER_BOTLANGUAGE] */
