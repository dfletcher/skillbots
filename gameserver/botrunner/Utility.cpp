/**
 *  @file Utility.cpp
 */

#include <Utility.hpp>
#include <fstream>

int Utility::str2int(std::string s) {
  int n;
  std::istringstream(s) >> n;
  return n;
}

double Utility::str2double(std::string s) {
  double n;
  std::istringstream(s) >> n;
  return n;
}

int Utility::readfile(const char *path, std::stringstream &buf) {
  int status = 0;
  std::ifstream i;
  i.open(path);
  if (i.is_open()) {
    buf << i.rdbuf();
    status = 1;
  }
  i.close();
  return status;
}
