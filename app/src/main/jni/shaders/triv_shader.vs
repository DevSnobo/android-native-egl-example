#version 330 core
uniform mat4 u_MVP;
uniform vec4 u_Tint;
attribute vec4 a_Position;
attribute vec4 a_Color;
varying vec4 v_Color;
void main()
{
   v_Color = a_Color * u_Tint;
   gl_Position = u_MVP
               * a_Position;
}
