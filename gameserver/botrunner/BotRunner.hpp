/**
 *  @file BotLanguage.hpp
 */

#ifndef BOTRUNNER_BOTRUNNER
#define BOTRUNNER_BOTRUNNER

#include <Utility.hpp>
#include <BotLanguage.hpp>
#include <BotRunnerException.hpp>

class BotRunner {

  public:
    BotRunner(void);
    int run(BotLanguage &language, int argc, char* argv[]);
    ~BotRunner(void);
};

#endif /* [BOTRUNNER_BOTRUNNER] */
