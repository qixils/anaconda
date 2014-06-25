#version 120

varying vec2 texture_coordinate;
uniform sampler2D texture;
uniform bool iR, iG, iB, iA, iInvert;
uniform float fSeed, fStrength;
#define M_PI 3.1415926535897932384626433832795

float mccool_rand(vec2 ij)
{
  const vec4 a = vec4(pow(M_PI,4.0),exp(4.0),pow(10.0, M_PI*0.5),sqrt(1997.0));
  vec4 result = vec4(ij,ij);

  for(int i = 0; i < 3; i++) {
      result.x = fract(dot(result, a));
      result.y = fract(dot(result, a));
      result.z = fract(dot(result, a));
      result.w = fract(dot(result, a));
  }
  return result.x;
}

void main()
{
    vec4 col = texture2D(texture, texture_coordinate);
    float rand = mccool_rand(texture_coordinate+fSeed)*fStrength;
    rand = iInvert ? 1-rand : rand;
    if (iR) col.r *= rand;
    if (iG) col.g *= rand;
    if (iB) col.b *= rand;
    if (iA) col.a *= rand;
    gl_FragColor = col * gl_Color;
}