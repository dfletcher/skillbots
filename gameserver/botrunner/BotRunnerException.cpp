/**
 *  @file BotRunnerException.cpp
 */

#include <BotRunnerException.hpp>

BotRunnerException::BotRunnerException(const std::string &m) : msg(m) {}

BotRunnerException::BotRunnerException(const char *m) : msg(m) {}

BotRunnerException::~BotRunnerException() throw() {}

const char* BotRunnerException::what() const throw() {
  return msg.c_str();
}
