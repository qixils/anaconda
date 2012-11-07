varying vec2 texture_coordinate0;
varying float alpha;

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    texture_coordinate0 = vec2(gl_MultiTexCoord0);
    gl_FrontColor = gl_Color;
}