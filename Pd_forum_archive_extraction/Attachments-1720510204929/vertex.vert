#define PI 3.1415926535897932384626433832795

uniform float deformation;

varying vec4 vertColor;

void main() {
  vertColor = gl_Color;
  vec4 pos_vertex = gl_Vertex;
  float sign = sign(pos_vertex.y);
  float add_y = cos(pos_vertex.x * PI * 0.5) * deformation;
  pos_vertex.y += add_y * sign;
  gl_Position = gl_ModelViewProjectionMatrix * pos_vertex;
}