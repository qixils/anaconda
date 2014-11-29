#version 120

varying vec2 texture_coordinate1;
uniform sampler2D texture;
uniform sampler2D background_texture;
uniform float brightness, saturation;

void main()
{
    vec4 color = texture2D(background_texture, texture_coordinate1);
    float f = (color.r+color.g+color.b)/3;
    color.rgb = brightness+f*(1.0f-saturation)+color.rgb * saturation;
    gl_FragColor = color;
}