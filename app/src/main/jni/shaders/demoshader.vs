#version 320 es
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout (location = 2) uniform mat4 u_Model;
layout (location = 3) uniform mat4 u_View;
layout (location = 4) uniform mat4 u_Projection;
out vec4 vColor;
void main() {
    vec4 hom_Pos = vec4(a_Position, 1.0);
    gl_Position = u_Projection * u_View * u_Model * hom_Pos;
    vColor = a_Color;
}
