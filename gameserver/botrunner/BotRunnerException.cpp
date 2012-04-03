/**
 *  @file BotRunnerException.cpp
 */

#include <BotRunnerException.hpp>

BotRunnerException::BotRunnerException(int l, int c) {
  msg = "";
  line = l;
  column = c;
}

BotRunnerException::BotRunnerException(std::string &m, int l, int c) {
  msg = m;
  line = l;
  column = c;
}

BotRunnerException::BotRunnerException(std::stringstream &m, int l, int c) {
  msg = m.str();
  line = l;
  column = c;
}

BotRunnerException::BotRunnerException(const char *m, int l, int c) {
  msg = m;
  line = l;
  column = c;
}

BotRunnerException::~BotRunnerException() throw() {
  ;
}

const char* BotRunnerException::what() const throw() {
  return msg.c_str();
}
