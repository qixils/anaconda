#version 120

varying vec2 texture_coordinate;
uniform sampler2D texture;
uniform vec2 texture_size;
uniform float x, y;
uniform bool limit;

void main()
{
    vec4 color = 0.0;
    vec2 pos = vec2(x,y);
    if(limit)
        pos -= floor(pos);
    pos = texture_coordinate-pos*texture_size;
    if(pos.x>=0&&pos.x<=1&&pos.y>=0&&pos.y<=1)
        color = texture2D(texture,pos);
    gl_FragColor = color * gl_Color;
}