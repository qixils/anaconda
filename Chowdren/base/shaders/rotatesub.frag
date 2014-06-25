#version 120

varying vec2 texture_coordinate;
uniform sampler2D texture;
uniform float fA;
uniform float fX;
uniform float fY;
uniform float fSx;
uniform float fSy;

void main()
{    
    vec2 coord = texture_coordinate;
    coord.x -= fSx;
    coord.y -= fSy;
    float x = fX;
    float y = fY;
    float a = fA;
    x += 0.5f;
    y += 0.5f;
    a *= 0.0174532925f;
    float Ray = sqrt(pow(coord.x-x,2)+pow(coord.y-y,2));
    float Angle;
    if(coord.y-y>0)
    {
        Angle = acos((coord.x-x)/Ray);
    }
    else
    {
        Angle = 0-acos((coord.x-x)/Ray);
    }
        
    coord.x = x + cos(Angle+a)*Ray;
    coord.y = y + sin(Angle+a)*Ray;

    vec4 col;
    if(coord.x >= 0 && coord.x <= 1 && coord.y >= 0 && coord.y <= 1)
    col = texture2D(texture, coord);
    else col = vec4(0);

    gl_FragColor = col * gl_Color;
}