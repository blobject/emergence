#pragma once

#include <glm/glm.hpp>
#include <unordered_map>


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

