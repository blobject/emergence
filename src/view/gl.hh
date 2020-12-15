//===-- gl.hh - OpenGL wrapping ---------------------------------*- C++ -*-===//
///
/// \file
/// Declarations and definitions of helper classes that wrap around OpenGL
/// constructs which in turn retain graphical data of the particle system.
/// It is used by the Canvas class.
///
//===----------------------------------------------------------------------===//

#pragma once

#include "../util/log.hh"
#include "../util/util.hh"
#include <GL/glew.h> // include glew.h before gl.h
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>



// VertexBuffer: Wrapper around OpenGL vertex buffers.

class VertexBuffer
{
  public:
    /// constructor: glGenBuffers wrapping.
    /// \param data  vector of data bound to the vertex buffer.
    /// \param size  total size of the data vector bound to the vertex buffer.
    inline VertexBuffer(const void* data, GLuint size)
        : data_(data), size_(size)
    {
        DOGL(glGenBuffers(1, &this->id_));
    }

    /// destructor: glDeleteBuffers wrapping.
    inline ~VertexBuffer()
    {
        DOGL(glDeleteBuffers(1, &this->id_));
    }

    /// bind(): glBindBuffer wrapping.
    inline void
    bind() const
    {
        DOGL(glBindBuffer(GL_ARRAY_BUFFER, this->id_));
    }

    /// unbind(): glBindBuffer wrapping.
    inline void
    unbind() const
    {
        DOGL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    /// buffer(): glBufferData wrapping.
    inline void
    buffer() const
    {
        DOGL(glBufferData(GL_ARRAY_BUFFER, this->size_, this->data_,
                          GL_STATIC_DRAW));
    }

    /// update(): Change data and size of the vertex buffer, and rebind and
    ///           rebuffer it.
    /// \param data  vector of data bound to the vertex buffer.
    /// \param size  total size of the data vector bound to the vertex buffer.
    void update(const void* data, GLuint size);

  private:
    GLuint      id_;   // handle on the vertex buffer
    const void* data_; // vector of data bound to the vertex buffer
    GLuint      size_; // total size of the data vector.
};


// IndexBuffer (unused): Wrapper around OpenGL index buffers.

class IndexBuffer
{
  public:
    /// constructor: GL_ELEMENT_ARRAY_BUFFER initialisation wrapping.
    /// \param data  vector of indices for the index buffer.
    /// \param count  number of indices for the index buffer.
    IndexBuffer(const GLuint* data, GLuint count);

    /// destructor: glDeleteBuffers wrapping.
    inline ~IndexBuffer()
    {
        DOGL(glDeleteBuffers(1, &this->id_));
    }

    /// bind(): glBindBuffer wrapping.
    inline void
    bind() const
    {
        DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id_));
    }

    /// unbind(): glBindBuffer wrapping.
    inline void
    unbind() const
    {
        DOGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    /// get_count(): Get the number of indices for the index buffer.
    /// \returns  number of indices for the index buffer.
    inline GLuint
    get_count() const
    {
        return this->count_;
    }

  private:
    GLuint id_;    // handle on the index buffer
    GLuint count_; // number of indices for the index buffer
};


// Attribs: Helper for matching OpenGL vertex arrays to buffers.

struct Attribs
{
    GLuint        type;   // type of datum
    GLuint        count;  // number of data per vertex attribute
    GLboolean     norm;   // whether to normalise fixed-point data
    GLsizei       stride; // offset between vertex attributes
    const GLvoid* offset; // offset of the first vertex attribute
};


// VertexBufferAttribs: Helper for OpenGL vertex arrays.

class VertexBufferAttribs
{
  public:
    /// gen(): Construct Attribs.
    /// \param count  number of data per vertex attribute
    /// \param size  number of data per stride
    /// \param offset  offset of the first vertex attribute
    /// \returns  an Attrib
    template<typename T> inline static Attribs
    gen(GLuint count, GLuint size, uintptr_t offset) {}

    // float version of gen().
    template<> inline Attribs
    gen<GLfloat>(GLuint count, GLuint size, uintptr_t offset)
    {
        return {GL_FLOAT, count, GL_FALSE,
                static_cast<GLsizei>(size * sizeof(float)),
                (const GLvoid*)offset};
    }

    // unsigned int version of gen().
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
    /// constructor: glGenVertexArrays wrapping.
    inline VertexArray()
    {
        DOGL(glGenVertexArrays(1, &this->id_));
    }

    /// destructor: glDeleteVertexArrays wrapping.
    inline ~VertexArray()
    {
        DOGL(glDeleteVertexArrays(1, &this->id_));
    }

    /// add_buffer(): glEnableVertexAttribArray and glVertexAttribPointer
    ///               wrapping after binding and buffering vertex buffers.
    /// \param id  vertex buffer handle
    /// \param vb  vertex buffer that ought to be attached
    /// \param attribs Attribs object
    void add_buffer(GLuint id, const VertexBuffer& vb, const Attribs& attribs);

    /// bind(): glBindVertexArray wrapping.
    inline void
    bind() const
    {
        DOGL(glBindVertexArray(this->id_));
    }

    /// unbind(): glBindVertexArray wrapping.
    inline void
    unbind() const
    {
        DOGL(glBindVertexArray(0));
    }

    /// get_id(): Get the vertex array handle.
    /// \returns  vertex array handle
    inline GLuint
    get_id() const
    {
        return this->id_;
    }

  private:
    GLuint id_; // handle on the vertex array
};


// Shader: Wrapper around OpenGL shaders and uniforms.

class Shader
{
  public:
    /// constructor: Execute create_shader().
    /// \param log  Log object
    inline Shader(Log& log)
        : id_(0)
    {
        this->id_ = Shader::create_shader(log);
    }

    /// destructor: glDeleteProgram wrapping.
    inline ~Shader()
    {
        DOGL(glDeleteProgram(this->id_));
    }

    /// bind(): glUseProgram wrapping.
    inline void
    bind() const
    {
        DOGL(glUseProgram(this->id_));
    }

    /// unbind(): glUseProgram wrapping.
    inline void
    unbind() const
    {
        DOGL(glUseProgram(0));
    }

    /// create_shader(): Load the embedded shader code.
    /// \param log  Log object
    /// \returns  shader program handle
    static GLuint create_shader(Log& log);


    /// create_shader(): Compile the loaded shader code.
    /// \param type  type of shader
    /// \param source  shader source code
    /// \param log  Log object
    /// \returns  shader handle
    static GLuint compile_shader(GLuint type, const std::string& source,
                                 Log& log);

    /// get_uniform_location(): Get the uniform handle, with caching.
    /// \param name  name of the uniform
    /// \returns  uniform location
    int get_uniform_location(const std::string& name);

    /// set_uniform_1f(): Set a single float as a uniform.
    /// \param name  name of the uniform
    /// \param v  the uniform datum
    inline void
    set_uniform_1f(const std::string& name, float v)
    {
        DOGL(glUniform1f(Shader::get_uniform_location(name), v));
    }

    /// set_uniform_4f(): Set a float vec4 as a uniform.
    /// \param name  name of the uniform
    /// \param v0  the first uniform datum
    /// \param v1  the second uniform datum
    /// \param v2  the third uniform datum
    /// \param v3  the fourth uniform datum
    inline void
    set_uniform_4f(const std::string& name,
                   float v0, float v1, float v2, float v3)
    {
        DOGL(glUniform4f(Shader::get_uniform_location(name), v0, v1, v2, v3));
    }

    /// set_uniform_mat4f(): Set a float mat4 as a uniform.
    /// \param name  name of the uniform
    /// \param mat  the uniform data
    inline void
    set_uniform_mat4f(const std::string& name, const glm::mat4& mat)
    {
        DOGL(glUniformMatrix4fv(Shader::get_uniform_location(name), 1, GL_FALSE,
                                &mat[0][0]));
    }

  private:
    GLuint id_; // handle on the shader
    // cache of uniform locations
    std::unordered_map<std::string,int> uniform_location_cache_;
};


// FrameBuffer (unused): Wrapper around OpenGL frame buffers.

class FrameBuffer
{
  public:
    /// constructor: glGenFramebuffers wrapping.
    inline FrameBuffer()
    {
        DOGL(glGenFramebuffers(1, &this->id_));
    }

    /// destructor: glDeleteFramebuffers wrapping.
    inline ~FrameBuffer()
    {
        DOGL(glDeleteFramebuffers(1, &this->id_));
    }

    /// bind(): glBindFramebuffer wrapping.
    inline void
    bind() const
    {
        DOGL(glBindFramebuffer(GL_FRAMEBUFFER, this->id_));
    }

    /// unbind(): glBindFramebuffer wrapping.
    inline void
    unbind() const
    {
        DOGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

  private:
    GLuint id_; // handle on the frame buffer.
};

