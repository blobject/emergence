#pragma once

#include "../util/log.hh"
#include "../util/util.hh"
#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>



// VertexBuffer: Wrapper around OpenGL vertex buffers.

class VertexBuffer
{
  public:
    VertexBuffer(const void* data, GLuint size)
        : data_(data), size_(size)
    {
        DOGL(glGenBuffers(1, &this->id_));
    }
    ~VertexBuffer()
    {
        DOGL(glDeleteBuffers(1, &this->id_));
    }
    inline void
    bind() const
    {
        DOGL(glBindBuffer(GL_ARRAY_BUFFER, this->id_));
    }
    inline void
    unbind() const
    {
        DOGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }
    inline void
    buffer() const
    {
        DOGL(glBufferData(GL_ARRAY_BUFFER, this->size_, this->data_,
                          GL_STATIC_DRAW));
    }
    void update(const void* data, GLuint size);

  private:
    GLuint      id_;
    const void* data_;
    GLuint      size_;
};


// IndexBuffer: Wrapper around OpenGL index buffers.

class IndexBuffer
{
  public:
    IndexBuffer(const GLuint* data, GLuint count);
    ~IndexBuffer();
    inline void
    bind() const
    {
        DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id_));
    }
    inline void
    unbind() const
    {
        DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }
    inline GLuint
    get_count() const
    {
        return this->count_;
    }

  private:
    GLuint id_;
    GLuint count_;
};


// Attribs: Helper for matching OpenGL vertex arrays to buffers.

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
    template<typename T> inline static Attribs
    gen(GLuint count, GLuint size, uintptr_t offset) {}
    template<> inline Attribs
    gen<GLfloat>(GLuint count, GLuint size, uintptr_t offset)
    {
        return {GL_FLOAT, count, GL_FALSE,
                static_cast<GLsizei>(size * sizeof(float)),
                (const GLvoid*)offset};
    }
    template<> inline Attribs
    gen<GLuint>(GLuint count, GLuint size, uintptr_t offset)
    {
        return {GL_UNSIGNED_INT, count, GL_FALSE,
                static_cast<GLsizei>(size * sizeof(unsigned int)),
                (const GLvoid*)offset};
    }
};


// VertexArray: Wrapper around OpenGL vertex arrays.

class VertexArray
{
  public:
    VertexArray() { DOGL(glGenVertexArrays(1, &this->id_)); }
    ~VertexArray() { DOGL(glDeleteVertexArrays(1, &this->id_)); }
    void add_buffer(GLuint id, const VertexBuffer& vb, const Attribs& attribs);
    inline void
    bind() const
    {
        DOGL(glBindVertexArray(this->id_));
    }
    inline void
    unbind() const
    {
        DOGL(glBindVertexArray(0));
    }
    inline GLuint
    get_id() const
    {
        return this->id_;
    }

  private:
    GLuint id_;
};


// Shader: Wrapper around OpenGL shaders and uniforms.

class Shader
{
  public:
    Shader(Log& log) : id_(0) { this->id_ = Shader::create_shader(log); }
    ~Shader() { DOGL(glDeleteProgram(this->id_)); }
    inline void
    bind() const
    {
        DOGL(glUseProgram(this->id_));
    }
    inline void
    unbind() const
    {
        DOGL(glUseProgram(0));
    }
    static GLuint create_shader(Log& log);
    static GLuint compile_shader(GLuint type, const std::string& source,
                                 Log& log);
    int           get_uniform_location(const std::string& name);
    inline void
    set_uniform_1f(const std::string& name, float v)
    {
        DOGL(glUniform1f(Shader::get_uniform_location(name), v));
    }
    inline void
    set_uniform_4f(const std::string& name,
                   float v0, float v1, float v2, float v3)
    {
        DOGL(glUniform4f(Shader::get_uniform_location(name), v0, v1, v2, v3));
    }
    inline void
    set_uniform_mat4f(const std::string& name, const glm::mat4& mat)
    {
        DOGL(glUniformMatrix4fv(Shader::get_uniform_location(name), 1, GL_FALSE,
                                &mat[0][0]));
    }

  private:
    GLuint                              id_;
    std::unordered_map<std::string,int> uniform_location_cache_;
};


// FrameBuffer: Wrapper around OpenGL frame buffers.

class FrameBuffer
{
  public:
    FrameBuffer() { DOGL(glGenFramebuffers(1, &this->id_)); }
    ~FrameBuffer() { DOGL(glDeleteFramebuffers(1, &this->id_)); }
    inline void
    bind() const
    {
        DOGL(glBindFramebuffer(GL_FRAMEBUFFER, this->id_));
    }
    inline void
    unbind() const
    {
        DOGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

  private:
    GLuint id_;
};

