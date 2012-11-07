varying vec2 texture_coordinate0;
varying float alpha;
uniform vec2 texture_size;
uniform vec4 r, g, b;
uniform sampler2D texture;

void main()
{
    vec4 i = texture2D(texture, texture_coordinate0);
    gl_FragColor.a = i.a * gl_Color.a;
    gl_FragColor.rgb = r.rgb*i.r + g.rgb*i.g + b.rgb*i.b;
}