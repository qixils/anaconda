#version 120

varying vec2 texture_coordinate0;
varying vec2 texture_coordinate1;
uniform vec2 texture_size;
uniform sampler2D texture;
uniform sampler2D background_texture;
uniform float x, y, alpha;

#define iterations 12

//Thanks to
//http://www.klopfenstein.net/lorenz.aspx/gamecomponents-the-bloom-post-processing-filter
const vec2 offsets[iterations] = vec2[iterations](
   vec2(-0.326212, -0.405805),
   vec2(-0.840144, -0.073580),
   vec2(-0.695914,  0.457137),
   vec2(-0.203345,  0.620716),
   vec2( 0.962340, -0.194983),
   vec2( 0.473434, -0.480026),
   vec2( 0.519456,  0.767022),
   vec2( 0.185461, -0.893124),
   vec2( 0.507431,  0.064425),
   vec2( 0.896420,  0.412458),
   vec2(-0.321940, -0.932615),
   vec2(-0.791559, -0.597705)
);

void main()
{
    vec4 back = texture2D(background_texture, texture_coordinate1);
    vec4 fore = texture2D(texture, texture_coordinate0);
    vec2 rad = vec2(x, y);
    for (int i=0; i < iterations; i++)
        back += texture2D(background_texture, max(vec2(0.0), min(vec2(1.0), texture_coordinate1 + rad*texture_size*offsets[i])));
    back /= (iterations + 1);
    back += (texture2D(background_texture, texture_coordinate1)-back) * (1.0 - fore.a);
    back.rgb += (fore.rgb-back.rgb) * fore.a * alpha;
    back.a += fore.a;
    gl_FragColor = back;
}