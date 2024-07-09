varying vec2 vTex;
void main() {
	gl_Position = gl_Vertex;
	vTex = gl_MultiTexCoord0.xy;
}