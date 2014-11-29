#version 120

varying vec2 texture_coordinate;
uniform sampler2D texture;
uniform vec2 texture_size;

uniform float x_scale;
uniform float y_scale;
uniform vec4 color_1;
uniform float alpha_1;
uniform vec4 color_2;
uniform float alpha_2;
uniform float coeff;
uniform float offset;
uniform float fade;

void main()
{
    vec4 color;
    float orig_width = (1/texture_size[0]) / x_scale;
    float orig_height = (1/texture_size[1]) / y_scale;
    float chunk_width = orig_width / 3.0;
    float chunk_height = orig_height / 3.0;
    float width = (1/texture_size[0]);
    float height = (1/texture_size[1]);

    vec2 pos;
    pos.x = texture_coordinate.x * width;
    pos.y = texture_coordinate.y * height;

    vec2 orig_pos;
    orig_pos.x = texture_coordinate.x * x_scale;
    orig_pos.y = texture_coordinate.y * y_scale;

    float dx = (orig_width - chunk_width + pos.x - (width-chunk_width));
    float dy = (orig_height - chunk_height + pos.y - (height-chunk_height));
    vec2 slice_pos = vec2((texture_coordinate.x+1)/3, (texture_coordinate.y+1)/3);

    // top
    if (pos.y < chunk_height) {
        slice_pos.y = orig_pos.y;
    }

    // bottom
    if (pos.y > height-chunk_height) {
        slice_pos.y = dy/height * y_scale;
    }

    // left
    if (pos.x< chunk_width) {
        slice_pos.x = orig_pos.x;
    }

    // right
    if (pos.x> width-chunk_width) {
        slice_pos.x = dx/width * x_scale;
    }

    color = texture2D(texture, slice_pos);

    // Output pixel
    vec4 gradient = vec4(0.0);
    float f = (texture_coordinate.x*(1-texture_coordinate.y));
    f = max(0, min(1, f+offset));
    gradient.a = alpha_1 + (alpha_2 - alpha_1) * f;
    gradient.rgb = mix(color_1, color_2, f).rgb;
    color.a *= gradient.a;
    color.rgb = gradient.rgb;
    gl_FragColor = color;
}
