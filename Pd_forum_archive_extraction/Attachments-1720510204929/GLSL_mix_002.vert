//jack/RYBN 2010
varying vec2 texcoord0;

uniform float K1,K2,K3,K4;
void main()
{
	texcoord0 = (gl_TextureMatrix[0]*gl_MultiTexCoord0* vec4(K1,K2,0.,0.)+vec4(K3,K4,0.,0.) ).st;
	gl_Position = ftransform();

}
