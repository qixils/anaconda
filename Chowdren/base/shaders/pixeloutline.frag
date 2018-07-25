#version 120

varying vec2 texture_coordinate;
uniform vec2 texture_size;
uniform sampler2D texture;

uniform vec4 color;

// const vec2 dirs[8] = vec2[8](
//     vec2(1.0, 0.0),
//     vec2(1.0, 1.0),
//     vec2(0.0, 1.0),
//     vec2(-1.0, 1.0),
//     vec2(-1.0, 0.0),
//     vec2(-1.0, -1.0),
//     vec2(0.0, -1.0),
//     vec2(1.0, -1.0)
// );

#define SET_SOLID(dir) \
            col.a = texture2D(texture, texture_coordinate + \
                              dir * texture_size).a; \
            if (col.a > 0.0) \
                return col;

vec4 getter(vec4 src)
{
    if (src.a != 0.0)
        return src;
    vec4 col;
    col.rgb = color.rgb;
    SET_SOLID(vec2(1.0, 0.0))
    SET_SOLID(vec2(1.0, 1.0))
    SET_SOLID(vec2(0.0, 1.0))
    SET_SOLID(vec2(-1.0, 1.0))
    SET_SOLID(vec2(-1.0, 0.0))
    SET_SOLID(vec2(-1.0, -1.0))
    SET_SOLID(vec2(0.0, -1.0))
    SET_SOLID(vec2(1.0, -1.0))
    return src;
}

void main()
{
    vec4 src = texture2D(texture, texture_coordinate) * gl_Color;
    src = getter(src);
    gl_FragColor = src;
}
