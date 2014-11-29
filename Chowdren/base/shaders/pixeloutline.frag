#version 120

varying vec2 texture_coordinate;
uniform vec2 texture_size;
uniform sampler2D texture;

uniform vec4 color;

const vec2 dirs[8] = vec2[8](
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, 0.0),
    vec2(-1.0, -1.0),
    vec2(0.0, -1.0),
    vec2(1.0, -1.0)
);

void main()
{
    vec4 src = texture2D(texture, texture_coordinate) * gl_Color;

    if (src.a == 0.0) {
        src.rgb = color.rgb;
        float solid = 0.0;
        for (int i=0; i<8 && solid == 0.0; i++) {
            solid = texture2D(texture, texture_coordinate +
                                       dirs[i] * texture_size).a;
            if (solid > 0.0)
                src.a = solid;
        }
    }

    gl_FragColor = src;
}
