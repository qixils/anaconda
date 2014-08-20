#version 120

varying vec2 texture_coordinate0;
varying vec2 texture_coordinate1;
uniform vec2 texture_size;
uniform sampler2D texture;
uniform sampler2D background_texture;
uniform sampler2D pattern;
uniform float x, y, alpha, width, height;

void main()
{
    vec2 In = texture_coordinate0;
    vec4 o = texture2D(texture, In);
    vec4 B = texture2D(background_texture, texture_coordinate1);

    In /= texture_size;
    In += vec2(x, y);
    In /= vec2(width, height);
    vec4 p = texture2D(pattern, mod(In, 1)) * alpha;

    gl_FragColor = p/B;
}