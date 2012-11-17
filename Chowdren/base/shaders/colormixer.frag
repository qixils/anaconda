varying vec2 texture_coordinate;
uniform vec4 r, g, b;
uniform sampler2D texture;

void main()
{
    vec4 i = texture2D(texture, texture_coordinate);
    gl_FragColor.a = i.a * gl_Color.a;
    gl_FragColor.rgb = r.rgb*i.r + g.rgb*i.g + b.rgb*i.b;
}