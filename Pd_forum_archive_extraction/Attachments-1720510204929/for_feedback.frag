// Jack/RYBN 2013
#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect MyTex1, MyTex2;

void main() {
	vec2 tex_coord = gl_FragCoord.st;
	vec4 color = texture2DRect(MyTex1, tex_coord);
	vec4 color0 = texture2DRect(MyTex2, tex_coord);
	gl_FragColor = color+color0;
	gl_FragColor.a = 0.9;
 }

