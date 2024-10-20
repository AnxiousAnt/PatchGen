//setup for 2 texture
varying vec2 texcoord0;
varying vec2 texcoord1;
varying vec2 texdim0;
uniform vec2 zoom;
uniform vec2 offset;
uniform float theta; 
uniform vec2 anchor;
uniform int boundmode; 
uniform sampler2DRect tex0;
uniform sampler2DRect tex1;
const float pi=3.1415926;

void main()
{
	// where is the point?
	vec2 sizea = texdim0;
	vec2 point = texcoord0;
	
	//transormation matrices
	mat2 sca = mat2 (1./zoom.x,0.,0.,1./zoom.y);//scaling matrix (zoom)
	mat2 rot = mat2 (cos(theta),sin(theta),-sin(theta),cos(theta));//rotation matrix

	//perform transform
	vec2 no = ((((point-anchor*sizea)*rot)*sca)+anchor*sizea)+offset;

	//create boundmodes
	vec2 no2 = mod(no,sizea);//wrap	
	
	vec2 no4 = mix(mod(no,sizea),sizea-mod(no,sizea),floor(mod(no,sizea*2.)/sizea));//folded coords
	
	// sampler coord
	vec2 tc = no*float(boundmode==0) + no*float(boundmode==1) + no2*float(boundmode==2) + no*float(boundmode==3) + no4*float(boundmode==4);
	
	
	//sample textures
	vec4 smp0 = texture2DRect(tex0,tc);
	vec4 smp1 = texture2DRect(tex1,texcoord0);
	
	vec2 outbound = sign(floor(no/sizea));//check for point>size
	float boundchk = float(sign(float(outbound.x!=0.)+float(outbound.y!=0.)));
	float checkm0 = float(boundmode==0)*boundchk;
	float checkm1 = float(boundmode==1)*float(boundchk==0.);
	vec4 ifb0 = mix(smp0,smp1,checkm0);//ignore
	vec4 final = ifb0*float(boundmode != 1) + ifb0*float(checkm1==1.);//clear
	
	
	// output texture
	gl_FragColor = final;
}