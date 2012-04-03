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
    BotRunnerException(int line=0, int column=0);
    BotRunnerException(std::string &msg, int line=0, int column=0);
    BotRunnerException(std::stringstream &msg, int line=0, int column=0);
    BotRunnerException(const char *msg, int line=0, int column=0);
    ~BotRunnerException() throw();
    const char* what(void) const throw();
    std::string msg;
    int line, column;
};

#endif /* [BOTRUNNER_EXCEPTION] */
