/**
 *  @file Utils.hpp
 */

#ifndef BOTRUNNER_UTILS
#define BOTRUNNER_UTILS

#include <string>
#include <sstream>

class Utility {
  public:
    static int str2int(std::string s);
    static double str2double(std::string s);
    static int readfile(const char *path, std::stringstream &buf);
};

#endif /* [BOTRUNNER_UTILS] */
