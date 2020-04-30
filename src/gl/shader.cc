#include <GL/glew.h>

#include "shader.hh"
#include "../util/util.hh"


Shader::Shader()
  : id_(0)
{
  this->id_ = Shader::CreateShader();
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
Shader::CreateShader()
{
  std::string vertex =
    "#version 330 core\n"
    "layout (location = 0) in vec2 shape;\n"
    "layout (location = 1) in vec2 trans;\n"
    "uniform mat4 mvp;\n"
    "void main()\n"
    "{\n"
    "  vec4 position = vec4(shape.x + trans.x, shape.y + trans.y, 0.0, 1.0);\n"
    "  gl_Position = mvp * position;\n"
    "}\n";
  std::string geometry = "";
  std::string fragment =
    "#version 330 core\n"
    "layout (location = 0) out vec4 color;\n"
    "void main()\n"
    "{\n"
    "  color = vec4(1.0, 1.0, 1.0, 1.0f);\n"
    "}\n";

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
  if (! vertex.empty()) { DOGL(glDeleteShader(v)); }
  if (! geometry.empty()) { DOGL(glDeleteShader(g)); }
  if (! fragment.empty()) { DOGL(glDeleteShader(f)); }
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
    char* message = static_cast<char*>(alloca(length * sizeof(char)));
    DOGL(glGetShaderInfoLog(id, length, &length, message));
    Util::Err("failed to compile "
              + std::string(type == GL_VERTEX_SHADER ? "vertex" :
                            type == GL_GEOMETRY_SHADER ? "geometry" :
                            "fragment")
              + '\n' + message);
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
  if (-1 == location)
  {
    Util::Warn("uniform '" + name + "' not found");
  }
  this->uniform_location_cache_[name] = location;
  return location;
}

void
Shader::SetUniform1f(const std::string &name, float v)
{
  DOGL(glUniform1f(Shader::GetUniformLocation(name), v));
}

void
Shader::SetUniform4f(const std::string &name,
                     float v0, float v1, float v2, float v3)
{
  DOGL(glUniform4f(Shader::GetUniformLocation(name), v0, v1, v2, v3));
}

void
Shader::SetUniformMat4f(const std::string &name, const glm::mat4 &mat)
{
  DOGL(glUniformMatrix4fv(Shader::GetUniformLocation(name), 1, GL_FALSE,
                          &mat[0][0]));
}

