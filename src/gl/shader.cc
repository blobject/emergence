#include <iostream>
#include <GL/glew.h>

#include "shader.hh"
#include "../util/util.hh"


Shader::Shader(const std::string &path)
  : path_(path), id_(0)
{
  LoadShaderOut source = Util::LoadShader(path);
  this->id_ = Shader::CreateShader(source.Vertex,
                                   source.Geometry,
                                   source.Fragment);
}


Shader::~Shader()
{
  DOGL(glDeleteProgram(this->id_));
}


void
Shader::Bind() const
{
  DOGL(glUseProgram(this->id_));
}


void
Shader::Unbind() const
{
  DOGL(glUseProgram(0));
}


unsigned int
Shader::CreateShader(const std::string &vertex,
                     const std::string &geometry,
                     const std::string &fragment)
{
  DOGL(unsigned int program = glCreateProgram());
  unsigned int v;
  unsigned int g;
  unsigned int f;
  if (! vertex.empty())
  {
    v = Shader::CompileShader(GL_VERTEX_SHADER, vertex);
    DOGL(glAttachShader(program, v));
  }
  if (! geometry.empty())
  {
    g = Shader::CompileShader(GL_GEOMETRY_SHADER, geometry);
    DOGL(glAttachShader(program, g));
  }
  if (! fragment.empty())
  {
    f = Shader::CompileShader(GL_FRAGMENT_SHADER, fragment);
    DOGL(glAttachShader(program, f));
  }
  DOGL(glLinkProgram(program));
  DOGL(glValidateProgram(program));
  if (! vertex.empty()) DOGL(glDeleteShader(v));
  if (! geometry.empty()) DOGL(glDeleteShader(g));
  if (! fragment.empty()) DOGL(glDeleteShader(f));
  return program;
}


unsigned int
Shader::CompileShader(unsigned int type, const std::string &source)
{
  DOGL(unsigned int id = glCreateShader(type));
  const char* src = source.c_str();
  DOGL(glShaderSource(id, 1, &src, nullptr));
  DOGL(glCompileShader(id));
  int success;
  DOGL(glGetShaderiv(id, GL_COMPILE_STATUS, &success));
  if (GL_FALSE == success)
  {
    int length;
    DOGL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
    char* message = (char*) alloca(length * sizeof(char));
    DOGL(glGetShaderInfoLog(id, length, &length, message));
    std::cerr << "Error: failed to compile "
              << (type == GL_VERTEX_SHADER ? "vertex" :
                  type == GL_GEOMETRY_SHADER ? "geometry" :
                  "fragment")
              << '\n';
    std::cerr << message << std::endl;
    DOGL(glDeleteShader(id));
    return 0;
  }
  return id;
}


int
Shader::GetUniformLocation(const std::string &name)
{
  if (this->uniform_location_cache_.find(name) !=
      this->uniform_location_cache_.end())
  {
    return this->uniform_location_cache_[name];
  }
  DOGL(int location = glGetUniformLocation(this->id_, name.c_str()));
  this->uniform_location_cache_[name] = location;
  return location;
}


void
Shader::SetUniform4f(const std::string &name,
                     float v0, float v1, float v2, float v3)
{
  DOGL(glUniform4f(Shader::GetUniformLocation(name), v0, v1, v2, v3));
}

