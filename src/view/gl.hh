#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>


// VertexBuffer: Wrapper around OpenGL vertex buffers.

class VertexBuffer
{
 private:
  unsigned int id_;
  const void*  data_;
  unsigned int size_;

 public:
  VertexBuffer(const void* data, unsigned int size);
  ~VertexBuffer();

  void Bind() const;
  void Buffer() const;
  void Update(const void* data);
  void Unbind() const;
};


// IndexBuffer: Wrapper around OpenGL index buffers.

class IndexBuffer
{
 private:
  unsigned int id_;
  unsigned int count_;

 public:
  IndexBuffer(const unsigned int* data, unsigned int count);
  ~IndexBuffer();

  inline unsigned int get_count() const { return this->count_; }

  void                Bind() const;
  void                Unbind() const;
};


// LayoutItem: Helper for OpenGL vertex arrays.

struct LayoutItem
{
  unsigned int  type;
  unsigned int  count;
  unsigned char normalised;
  unsigned int  stride;

  static unsigned int TypeSize(unsigned int type);
};


// VertexBufferLayout: Helper for OpenGL vertex arrays.

class VertexBufferLayout
{
 public:
  template<typename T> static LayoutItem Make(unsigned int count);
  template<> LayoutItem Make<float>(unsigned int count);
  template<> LayoutItem Make<unsigned int>(unsigned int count);
  template<> LayoutItem Make<unsigned char>(unsigned int count);
};


// VertexArray: Wrapper around OpenGL vertex arrays.

class VertexArray
{
 private:
  unsigned int id_;

 public:
  VertexArray();
  ~VertexArray();

  inline unsigned int get_id() const { return this->id_; }

  void AddBuffer(unsigned int id, const VertexBuffer &vb,
                 const LayoutItem &layout);
  void Bind() const;
  void Unbind() const;
};


// UniformBuffer: Wrapper around OpenGL uniform buffers.

class UniformBuffer
{
 private:
  unsigned int id_;

 public:
  UniformBuffer();
  ~UniformBuffer();

  void                Bind() const;
  void                Unbind() const;
};


// Shader: Wrapper around OpenGL shaders.

class Shader
{
 private:
  unsigned int                        id_;
  std::unordered_map<std::string,int> uniform_location_cache_;

 public:
  Shader();
  ~Shader();

  void                Bind() const;
  void                Unbind() const;
  static unsigned int CompileShader(unsigned int type,
                                    const std::string &source);
  static unsigned int CreateShader();
  int                 GetUniformLocation(const std::string &name);
  void                SetUniform1f(const std::string &name, float v);
  void                SetUniform4f(const std::string &name,
                                   float v0, float v1, float v2, float v3);
  void                SetUniformMat4f(const std::string &name,
                                      const glm::mat4 &mat);
};

