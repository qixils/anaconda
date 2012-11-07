#version 120

varying vec2 texture_coordinate0;
varying vec2 texture_coordinate1;
uniform vec2 texture_size;
uniform sampler2D tex;
uniform sampler2D background_texture;
uniform int vertical;
uniform float radius;

#define iterations 28

void main()
{
        vec4 col = texture2D(tex, texture_coordinate);
        col *= vec4(0.299,0.587,0.114,1.0);
        col.rgb = vec3(col.r+col.g+col.b);
        gl_FragColor = col * gl_Color;
}

float4 ps_main(float2 In : TEXCOORD0) : COLOR0 {    
    float4 b = 0, b2 = tex2D(bg, In);
    float4 o = tex2D(img, In), temp;
    float2 coeff = (vertical == 0) ? float2(fPixelWidth*radius, 0) : float2(0, fPixelHeight*radius);
    for(int i = 0; i < iterations; i++) {
        temp = tex2D(bg, clamp(In+coeff*2*(i/(float)(iterations-1) - 0.5), 0, 0.9999));
        b += max(temp, b2);
    }
    b /= iterations;
    o = (o == 1.0) ? 1.0 : saturate(b/(1.0-o));
    o.a = 1.0;  
    return o;
}