/**
 *  @file BotRunnerException.cpp
 */

#include <BotRunnerException.hpp>

BotRunnerException::BotRunnerException(std::string &m) {
  this->msg = m;
}

BotRunnerException::BotRunnerException(std::stringstream &m) {
  this->msg = m.str();
}

BotRunnerException::BotRunnerException(const char *m) {
  this->msg = m;
}

BotRunnerException::~BotRunnerException() throw() {
  ;
}

const char* BotRunnerException::what() const throw() {
  return msg.c_str();
}
