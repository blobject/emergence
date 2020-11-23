#pragma once

#include <GL/gl.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>


// VertexBuffer: Wrapper around OpenGL vertex buffers.

class VertexBuffer
{
 private:
  GLuint      id_;
  const void* data_;
  GLuint      size_;

 public:
  VertexBuffer(const void* data, GLuint size);
  ~VertexBuffer();

  void Bind() const;
  void Buffer() const;
  void Update(const void* data, GLuint size);
  void Unbind() const;
};


// IndexBuffer: Wrapper around OpenGL index buffers.

class IndexBuffer
{
 private:
  GLuint id_;
  GLuint count_;

 public:
  IndexBuffer(const GLuint* data, GLuint count);
  ~IndexBuffer();

  inline GLuint get_count() const { return this->count_; }

  void          Bind() const;
  void          Unbind() const;
};


// Attribs: Helper for OpenGL vertex arrays.

struct Attribs
{
  GLuint        type;
  GLuint        count;
  GLboolean     norm;
  GLsizei       stride;
  const GLvoid* offset;
};


// VertexBufferAttribs: Helper for OpenGL vertex arrays.

class VertexBufferAttribs
{
 public:
  template<typename T> static Attribs Gen(GLuint count,
                                          GLuint size,
                                          uintptr_t offset);
  template<> Attribs Gen<GLfloat>(GLuint count, GLuint size, uintptr_t offset);
  template<> Attribs Gen<GLuint>(GLuint count, GLuint size, uintptr_t offset);
};


// VertexArray: Wrapper around OpenGL vertex arrays.

class VertexArray
{
 private:
  GLuint id_;

 public:
  VertexArray();
  ~VertexArray();

  inline GLuint get_id() const { return this->id_; }

  void AddBuffer(GLuint id,
                 const VertexBuffer &vb,
                 const Attribs &attribs);
  void Bind() const;
  void Unbind() const;
};


// Shader: Wrapper around OpenGL shaders.

class Shader
{
 private:
  GLuint                              id_;
  std::unordered_map<std::string,int> uniform_location_cache_;

 public:
  Shader();
  ~Shader();

  void          Bind() const;
  void          Unbind() const;
  static GLuint CompileShader(GLuint type, const std::string &source);
  static GLuint CreateShader();
  int           GetUniformLocation(const std::string &name);
  void          SetUniform1f(const std::string &name, float v);
  void          SetUniform4f(const std::string &name,
                             float v0, float v1, float v2, float v3);
  void          SetUniformMat4f(const std::string &name, const glm::mat4 &mat);
};


// FrameBuffer: Wrapper around OpenGL frame buffers.

class FrameBuffer
{
 private:
  GLuint id_;

 public:
  FrameBuffer();
  ~FrameBuffer();

  void Bind() const;
  void Unbind() const;
};

