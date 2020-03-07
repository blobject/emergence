#include <regex>

#include "util.hh"


// Relative: Get the file path relative to the project directory.

std::string
Util::Relative(const std::string &path)
{
  std::regex re("^.*[\\/](src[\\/])");
  return std::regex_replace(path, re, "$1");
}


// Trim: Remove whitespace at the ends of a string.

std::string
Util::Trim(std::string s)
{
  s.erase(std::find_if_not(s.rbegin(), s.rend(),
                           [](int c) { return std::isspace(c); }).base(),
          s.end());
  s.erase(s.begin(), std::find_if_not(s.begin(), s.end(),
                                      [](int c) { return std::isspace(c); }));
  return s;
}

