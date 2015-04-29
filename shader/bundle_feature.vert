#version 120

attribute vec4 in_Position;
attribute vec4 in_Color;

varying vec4 vs_Color;

uniform mat4 u_ModelViewMatrix;
uniform mat3 u_CalibrationMatrix;

void main()
{
  vec4 pos;

  pos = u_ModelViewMatrix * in_Position;
  pos.xyz = u_CalibrationMatrix * pos.xyz;
  pos /= pos.z;

  //gl_Position = in_Position;
  gl_Position = vec4(pos.xy, 1.0, 1.0);
  vs_Color = in_Color;
}
