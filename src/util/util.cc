#include "common.hh"
#include "util.hh"
#include <regex>


bool
Util::debug_gl(const std::string& func, const std::string& path, int line)
{
    std::string error;
    while (GLenum e = glGetError()) {
        switch (e) {
        case GL_INVALID_ENUM:      error = "invalid enum";      break;
        case GL_INVALID_VALUE:     error = "invalid value";     break;
        case GL_INVALID_OPERATION: error = "invalid operation"; break;
        case GL_STACK_OVERFLOW:    error = "stack overflow";    break;
        case GL_STACK_UNDERFLOW:   error = "stack underflow";   break;
        case GL_OUT_OF_MEMORY:     error = "out of memory";     break;
        default:                   error = "unknown";           break;
        }
        std::cerr << "Error(gl): " << error << " at "
                  << Util::relative(path) << ':'
                  << std::to_string(line) << "\n  " << func << "\n";
        return false;
    }
    return true;
}


std::string
Util::relative(const std::string& path)
{
    return std::regex_replace(path, std::regex("^.*[\\/]src[\\/]"), "");
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

