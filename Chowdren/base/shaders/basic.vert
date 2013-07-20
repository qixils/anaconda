#version 120

attribute vec4 in_pos;
attribute vec4 in_color;

varying vec4 color;

void main()
{
    gl_Position = in_pos;
    color = in_color;
}