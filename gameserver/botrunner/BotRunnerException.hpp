/**
 *  @file BotRunnerException.hpp
 */

#ifndef BOTRUNNER_EXCEPTION
#define BOTRUNNER_EXCEPTION

#include <string>
#include <sstream>
#include <exception>

class BotRunnerException : public std::exception {
  public:
    BotRunnerException(void);
    BotRunnerException(std::string &msg);
    BotRunnerException(std::stringstream &msg);
    BotRunnerException(const char *msg);
    ~BotRunnerException() throw();
    const char* what(void) const throw();
  protected:
    std::string msg;
};

#endif /* [BOTRUNNER_EXCEPTION] */
