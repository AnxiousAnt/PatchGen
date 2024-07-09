uniform float K1,K3,D1;
uniform sampler2D texture;

const float dx = 1./1024.; 
const float dy = 1./1024.;

void main (void)
{
	float x,y;
	vec4 C;
	vec2 texture1 = gl_TexCoord[0].st ;

	x=texture1.x;
	y=texture1.y;

	C = texture2D(texture, vec2(x, y));
	vec4 color_cote  = texture2D(texture, vec2(x-dx, y-dy));
	color_cote += texture2D(texture, vec2(x+dx, y-dy));
	color_cote += texture2D(texture, vec2(x-dx, y+dy));
	color_cote += texture2D(texture, vec2(x+dx, y+dy));
	color_cote += texture2D(texture, vec2(x, y-dy));
	color_cote += texture2D(texture, vec2(x-dx, y));
	color_cote += texture2D(texture, vec2(x+dx, y));
	color_cote += texture2D(texture, vec2(x, y+dy));


	float K = K1 * color_cote.r + K3 * 0.5 - (8. * K1 + K3) * C.r;
	float D = D1 * (C.r - C.g);

	C.b = C.g;
	C.g = C.r;
	C.r = (2.* C.g) - C.b + K - D ;

	gl_FragColor = vec4(C.rgb,1.);
}
