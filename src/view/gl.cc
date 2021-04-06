#include "gl.hh"


void
VertexBuffer::update(const void* data, GLuint size)
{
  this->data_ = data;
  this->size_ = size;
  this->bind();
  this->buffer();
}


IndexBuffer::IndexBuffer(const GLuint* data, GLuint count)
  : count_(count)
{
  DOGL(glGenBuffers(1, &this->id_));
  DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id_));
  DOGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), data,
                    GL_STATIC_DRAW));
}


void
VertexArray::add_buffer(GLuint id,
                        const VertexBuffer& vb,
                        const Attribs& attribs)
{
  this->bind();
  vb.bind();
  vb.buffer();
  DOGL(glEnableVertexAttribArray(id));
  DOGL(glVertexAttribPointer(id, attribs.count, attribs.type, attribs.norm,
                             attribs.stride, attribs.offset));
}


GLuint
Shader::create_shader(Log& log)
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
    "  color = frag_rgba;\n"
    /**
    "  if (pow(frag_xyz.x, 2) + pow(frag_xyz.y, 2) <= frag_rad) {\n"
    "    color = vec4(0.0f);\n"
    "  } else {\n"
    "    color = frag_color;\n"
    "  }\n"
    //*/
    "}\n";

  DOGL(GLuint program = glCreateProgram());
  GLuint v;
  GLuint g;
  GLuint f;
  if (!vertex.empty()) {
    v = Shader::compile_shader(GL_VERTEX_SHADER, vertex, log);
    DOGL(glAttachShader(program, v));
  }
  if (!geometry.empty()) {
    g = Shader::compile_shader(GL_GEOMETRY_SHADER, geometry, log);
    DOGL(glAttachShader(program, g));
  }
  if (!fragment.empty()) {
    f = Shader::compile_shader(GL_FRAGMENT_SHADER, fragment, log);
    DOGL(glAttachShader(program, f));
  }
  DOGL(glLinkProgram(program));
  DOGL(glValidateProgram(program));
  if (!vertex.empty()) { DOGL(glDeleteShader(v)); }
  if (!geometry.empty()) { DOGL(glDeleteShader(g)); }
  if (!fragment.empty()) { DOGL(glDeleteShader(f)); }
  return program;
}

GLuint
Shader::compile_shader(GLuint type, const std::string& source, Log& log)
{
  DOGL(GLuint id = glCreateShader(type));
  const char* src = source.c_str();
  DOGL(glShaderSource(id, 1, &src, nullptr));
  DOGL(glCompileShader(id));
  int success;
  DOGL(glGetShaderiv(id, GL_COMPILE_STATUS, &success));
  if (GL_FALSE == success) {
    int length;
    DOGL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
    char* message = static_cast<char*>(alloca(length * sizeof(char)));
    DOGL(glGetShaderInfoLog(id, length, &length, message));
    log.add(Attn::Egl,
            "failed to compile "
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
Shader::get_uniform_location(const std::string& name)
{
  if (0 < this->uniform_location_cache_.count(name)) {
    return this->uniform_location_cache_[name];
  }
  DOGL(int location = glGetUniformLocation(this->id_, name.c_str()));
  //if (-1 == location) { Util::Err("uniform '" + name + "' not found"); }
  this->uniform_location_cache_[name] = location;
  return location;
}

