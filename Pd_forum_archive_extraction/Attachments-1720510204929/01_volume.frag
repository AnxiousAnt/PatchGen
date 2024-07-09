uniform sampler2D texture;
uniform float volume;


void main()
{
	vec4 B = texture2D(texture, gl_TexCoord[0].st);
	gl_FragColor = B * volume;
}