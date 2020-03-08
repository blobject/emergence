#pragma once

#include <unordered_map>
#include <glm/glm.hpp>


// Shader: Wrapper around OpenGL shaders.

class Shader
{
 private:
  const std::string                   path_;
  unsigned int                        id_;
  std::unordered_map<std::string,int> uniform_location_cache_;

 public:
  Shader(const std::string &path);
  ~Shader();

  void                Bind() const;
  void                Unbind() const;
  static unsigned int CompileShader(unsigned int type,
                                    const std::string &source);
  static unsigned int CreateShader(const std::string &vshader,
                                   const std::string &fshader,
                                   const std::string &gshader);
  int                 GetUniformLocation(const std::string &name);
  void                SetUniform1f(const std::string &name, float v);
  void                SetUniform4f(const std::string &name,
                                   float v0, float v1, float v2, float v3);
  void                SetUniformMat4f(const std::string &name,
                                      const glm::mat4 &mat);
};

