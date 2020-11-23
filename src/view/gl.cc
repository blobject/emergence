#include <GL/glew.h>

#include "gl.hh"
#include "../util/util.hh"


// VertexBuffer

VertexBuffer::VertexBuffer(const void* data, GLuint size)
  : data_(data), size_(size)
{
  DOGL(glGenBuffers(1, &this->id_));
}

VertexBuffer::~VertexBuffer()
{
  DOGL(glDeleteBuffers(1, &this->id_));
}

void
VertexBuffer::Bind() const
{
  DOGL(glBindBuffer(GL_ARRAY_BUFFER, this->id_));
}

void
VertexBuffer::Buffer() const
{
  DOGL(glBufferData(GL_ARRAY_BUFFER, this->size_, this->data_, GL_STATIC_DRAW));
}

void
VertexBuffer::Update(const void* data, GLuint size)
{
  this->data_ = data;
  this->size_ = size;
  this->Bind();
  this->Buffer();
}

void
VertexBuffer::Unbind() const
{
  DOGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}


// IndexBuffer

IndexBuffer::IndexBuffer(const GLuint* data, GLuint count)
  : count_(count)
{
  DOGL(glGenBuffers(1, &this->id_));
  DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id_));
  DOGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data,
                    GL_STATIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
  DOGL(glDeleteBuffers(1, &this->id_));
}

void
IndexBuffer::Bind() const
{
  DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id_));
}

void
IndexBuffer::Unbind() const
{
  DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}


// VertexBufferAttribs

template<typename T> Attribs
VertexBufferAttribs::Gen(GLuint count, GLuint size, uintptr_t offset)
{
  //static_assert(false);
}

template<> Attribs
VertexBufferAttribs::Gen<GLfloat>(GLuint count, GLuint size, uintptr_t offset)
{
  return { GL_FLOAT, count, GL_FALSE,
           static_cast<GLsizei>(size * sizeof(float)),
           (const GLvoid*)offset };
}

template<> Attribs
VertexBufferAttribs::Gen<GLuint>(GLuint count, GLuint size, uintptr_t offset)
{
  return { GL_UNSIGNED_INT, count, GL_FALSE,
           static_cast<GLsizei>(size * sizeof(unsigned int)),
           (const GLvoid*)offset };
}


// VertexArray

VertexArray::VertexArray()
{
  DOGL(glGenVertexArrays(1, &this->id_));
}

VertexArray::~VertexArray()
{
  DOGL(glDeleteVertexArrays(1, &this->id_));
}

void
VertexArray::AddBuffer(GLuint id,
                       const VertexBuffer &vb,
                       const Attribs &attribs)
{
  this->Bind();
  vb.Bind();
  vb.Buffer();
  DOGL(glEnableVertexAttribArray(id));
  DOGL(glVertexAttribPointer(id, attribs.count, attribs.type, attribs.norm,
                             attribs.stride, attribs.offset));
}

void
VertexArray::Bind() const
{
  DOGL(glBindVertexArray(this->id_));
}

void
VertexArray::Unbind() const
{
  DOGL(glBindVertexArray(0));
}


// Shader

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

GLuint
Shader::CreateShader()
{
  std::string vertex =
    "#version 330 core\n"
    "uniform mat4 mvp;\n"
    "layout (location = 0) in vec3 xyz;\n"
    "layout (location = 1) in vec4 rgba;\n"
    "layout (location = 2) in vec2 quad;\n"
    "out vec4 frag_rgba;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = mvp * vec4(xyz + vec3(quad, 0.0f), 1.0f);\n"
    "  frag_rgba = rgba;\n"
    "}\n";

  std::string geometry = "";

  std::string fragment =
    "#version 330 core\n"
    "in vec4 frag_rgba;\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    /**
    "  if (pow(frag_xyz.x, 2) + pow(frag_xyz.y, 2) <= frag_rad)\n"
    "  {\n"
    "    color = vec4(0.0f);\n"
    "  }\n"
    "  else\n"
    "  {\n"
    "    color = frag_color;\n"
    "  }\n"
    //*/
    "  color = frag_rgba;\n"
    "}\n";

  DOGL(GLuint program = glCreateProgram());
  GLuint v;
  GLuint g;
  GLuint f;
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

GLuint
Shader::CompileShader(GLuint type, const std::string &source)
{
  DOGL(GLuint id = glCreateShader(type));
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
  /**
  if (-1 == location)
  {
    Util::Warn("uniform '" + name + "' not found");
  }
  //*/
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


// FrameBuffer

FrameBuffer::FrameBuffer()
{
  DOGL(glGenFramebuffers(1, &this->id_));
}

FrameBuffer::~FrameBuffer()
{
  DOGL(glDeleteFramebuffers(1, &this->id_));
}

void
FrameBuffer::Bind() const
{
  DOGL(glBindFramebuffer(GL_FRAMEBUFFER, this->id_));
}

void
FrameBuffer::Unbind() const
{
  DOGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

