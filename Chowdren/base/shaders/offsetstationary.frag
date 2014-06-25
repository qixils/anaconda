#version 120

varying vec2 texture_coordinate0;
varying vec2 texture_coordinate1;
uniform sampler2D texture;
uniform sampler2D background_texture;
uniform vec2 texture_size;
uniform float xoff, yoff, width, height, alpha;

#define HARD(B, L) (L<0.5?(2.0*B*L):(1.0-2.0*(1.0-L)*(1.0-B)))

float fmod(float x, float y) {
    return x - y * trunc(x/y);
}

/**
 * TODO: Edge bleeding; also present in HLSL equivalent, we will look into this later
 */

void main()
{
    vec2 In0 = texture_coordinate0;
    vec2 In1 = texture_coordinate1;
    vec2 shiftcoor = vec2(fmod((In0.x + xoff*texture_size.x),1),fmod((In0.y + yoff*texture_size.y),1));
    vec4 shift = texture2D(texture, shiftcoor) * gl_Color;
    vec2 off = vec2(width,height) * texture_size;
    off.x *= 2*(shift.r-0.5);
    off.y *= -2*(shift.g-0.5); //-1 hack, wut
    vec4 o = texture2D(background_texture, In1+off);
    o += texture2D(background_texture, In1+off+vec2(texture_size.x*0.5,0));
    o += texture2D(background_texture, In1+off+vec2(texture_size.x*-0.5,0));
    o += texture2D(background_texture, In1+off+vec2(0,texture_size.y*0.5));
    o += texture2D(background_texture, In1+off+vec2(0,texture_size.y*-0.5));
    o /= 5;

    gl_FragColor = o;
}