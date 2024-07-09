uniform sampler2D tex0;

varying vec4 vertColor;

void main() {
  vec4 image = texture2D(tex0, gl_TexCoord[0].st);
  gl_FragColor = image * vertColor;
}