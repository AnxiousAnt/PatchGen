uniform sampler2D curtex;

void main (void)
{
 vec2 xy=gl_TexCoord[0].st;

 vec4 frontcolor = texture2D(curtex, xy);
 
 gl_FragColor = frontcolor;
 
}
