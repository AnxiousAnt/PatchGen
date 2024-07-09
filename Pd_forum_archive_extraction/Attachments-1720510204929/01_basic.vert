uniform sampler2D texture;

void main()
{
    	gl_TexCoord[0] = gl_MultiTexCoord0;
	// perform standard transform on vertex
	gl_Position = ftransform();

}
