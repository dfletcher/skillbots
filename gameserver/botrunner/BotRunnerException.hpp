/**
 *  @file BotRunnerException.hpp
 */

#ifndef BOTRUNNER_EXCEPTION
#define BOTRUNNER_EXCEPTION

#include <string>
#include <exception>

class BotRunnerException : public std::exception {
  public:
    BotRunnerException(const std::string&);
    BotRunnerException(const char*);
    ~BotRunnerException() throw();
    const char* what(void) const throw();
    std::string msg;
};

#endif /* [BOTRUNNER_EXCEPTION] */
