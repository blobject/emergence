# vertex
//#version 330 core
//layout (location = 0) in vec4 position;
//void main() {
//  gl_Position = position;
//}

#version 330 core
layout (location = 0) in vec2 shape;
layout (location = 1) in vec2 trans;
uniform mat4 mvp;
void main()
{
  vec4 position = vec4(shape.x + trans.x, shape.y + trans.y, 0.0, 1.0);
  gl_Position = mvp * position;
}

# geometry

# fragment
//#version 330 core
//layout (location = 0) out vec4 color;
//uniform vec4 uColor;
//void main() {
//  color = uColor;
//}

#version 330 core
layout (location = 0) out vec4 color;
void main()
{
  color = vec4(1.0, 1.0, 1.0, 1.0f);
}

