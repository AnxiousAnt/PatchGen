// porcodio

#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect curtex;

void main (void)
{
 vec2 xy=gl_TexCoord[0].st;

 vec4 frontcolor = texture2DRect(curtex, xy);
 
 gl_FragColor = frontcolor;
 
}
