/**
 *  @file Utils.hpp
 */

#ifndef BOTRUNNER_UTILS
#define BOTRUNNER_UTILS

#include <string>
#include <sstream>

namespace Utility {
  int str2int(std::string s);
  double str2double(std::string s);
  int readfile(const char *path, std::stringstream &buf);
};

#endif /* [BOTRUNNER_UTILS] */
