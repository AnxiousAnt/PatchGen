varying vec2 texcoord1;
varying vec2 texcoord2;

void main()
{
	
    texcoord1 = gl_MultiTexCoord0.st;
    texcoord2 = gl_MultiTexCoord1.st;
   // gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	gl_Position = ftransform();

}
