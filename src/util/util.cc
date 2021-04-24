#include "common.hh"
#include "util.hh"


std::string
Util::execution_dir()
{
  std::vector<char> buf(1024, 0);
  std::vector<char>::size_type size = buf.size();
  bool path_yes = false;
  bool go = true;

  while (go) {
    int res = readlink("/proc/self/exe", &buf[0], size);
    if (0 > res) {
      go = false;
    } else if (size > static_cast<std::vector<char>::size_type>(res)) {
      path_yes = true;
      go = false;
      size = res;
    } else {
      size *= 2;
      buf.resize(size);
      std::fill(std::begin(buf), std::end(buf), 0);
    }
  }

  if (!path_yes) {
    std::string empty;
    return empty;
  }

  return std::regex_replace(std::string(&buf[0], size),
                            std::regex("/[^/]*$"), "");
}


std::string
Util::working_dir()
{
  char buf[1024];
  getcwd(buf, 1024);
  std::string cwd(buf);
  return cwd;
}


bool
Util::debug_gl(const std::string& func, const std::string& path, int line)
{
  std::string error;
  while (GLenum e = glGetError()) {
    if      (GL_INVALID_ENUM      == e) { error = "invalid enum"; }
    else if (GL_INVALID_VALUE     == e) { error = "invalid value"; }
    else if (GL_INVALID_OPERATION == e) { error = "invalid operation"; }
    else if (GL_STACK_OVERFLOW    == e) { error = "stack overflow"; }
    else if (GL_STACK_UNDERFLOW   == e) { error = "stack underflow"; }
    else if (GL_OUT_OF_MEMORY     == e) { error = "out of memory"; }
    else                                { error = "unknown"; }
    std::cerr << "Error(gl): " << error << " at "
              << std::regex_replace(path, std::regex("^.*[\\/]src[\\/]"), "")
              << ':' << std::to_string(line) << "\n  " << func << "\n";
    return false;
  }
  return true;
}


std::string
Util::trim(std::string s)
{
  s.erase(std::find_if_not(s.rbegin(), s.rend(),
                           [](int c) { return std::isspace(c); }).base(),
          s.end());
  s.erase(s.begin(), std::find_if_not(s.begin(), s.end(),
                                      [](int c) { return std::isspace(c); }));
  return s;
}

