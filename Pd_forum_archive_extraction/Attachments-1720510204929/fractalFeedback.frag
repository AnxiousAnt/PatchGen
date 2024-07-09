
uniform float time, alpha_back;
uniform vec2 mouse;
uniform vec2 resolution;

uniform sampler2D backbuffer;

#define PI2 6.2831853070


// definition for a scale, rotate, and color transform
// color transform is multiply (darken) only
struct Transform {
	vec4 color;
	vec2 center;
	float scale;
	float rotation;
};

vec2 applyTransform(const Transform t, vec2 v) {
	v -= t.center;
	float c = cos(t.rotation);
	float s = sin(t.rotation);
	return (vec2(
		(v.x*c - v.y*s),
		(v.y*c + v.x*s)) * t.scale + t.center);
}

#define SCALE 5.0

// model will be scaled such that the rectangle (-SCALE/2, -SCALE/2), (SCALE/2, SCALE/2)
// will be centered and fit the window with square aspect ratio.
float scale = SCALE / min(resolution.y,resolution.x);

vec2 mapModelToTexture(const vec2 v) {
	return (v/(resolution*scale) + 0.5);
}

vec2 mapTextureToModel(const vec2 v) {
	return (v - 0.5 * resolution) * scale;
}

// apply transform, return backbuffer sample
vec4 getBackbufferTransform(const Transform t, vec2 v) {
	return t.color * 
		texture2D(backbuffer, 
		   mapModelToTexture(applyTransform(t, v)));
}

vec4 whiten(const vec4 color, float f) {
	return (1.-f)*color + vec4(f);
}

void main( void ) {
	// transform fragment coords to model coords
	vec2 v = mapTextureToModel(gl_FragCoord.xy);

	// draw orbs
	
	const float innerRadius = 0.01;
	float outerRadius = .24 + .03 * (sin(17.*time) + sin(11.*time));
	
	vec2 p1 = (vec2(sin(time), cos(time)));
	vec2 p2 = (vec2(sin(time*1.1), cos(time*1.1)));
	vec2 p3 = (vec2(sin(time*1.3), cos(time*1.3)));

	float d1 = length(v -p1);
	float d2 = length(v -p2);
	float d3 = length(v -p3);

	const float aa = 0.95;
	const float bb = 0.65;
	const float cc = 0.05;
	const vec4 color1 = vec4(aa, bb, cc, 2.0);
	const vec4 color2 = vec4(cc, bb, aa, 2.0);
	const vec4 color3 = vec4(aa, cc, bb, 2.0);

	vec4 drawing = vec4(0.,0.,0.,0.);
	drawing += smoothstep(outerRadius, innerRadius, d1) * color1;
	drawing += smoothstep(outerRadius, innerRadius, d2) * color2;
	drawing += smoothstep(outerRadius, innerRadius, d3) * color3;
	
	// draw ring/disc
	float rr = length(v);
	drawing += vec4(1.)
		* smoothstep(1.1, 1., rr)
		* smoothstep(0.9, 0.5+0.5*sin(0.1*time), rr);

	// apply feedback transforms to backbuffer
	
	// define transform structs
	float r = time*.27;
	Transform t1 = Transform(
		whiten(vec4(aa, bb, cc, 1.0), 0.7), 
		vec2(-.7, 0.),
		1.5, 
		-r*3.);
	Transform t2 = Transform(
		whiten(vec4(cc, bb, aa, 1.0), 0.7),
		vec2(0.7, 0.),
		1.5,
		r*2.);

	// composite feedback with drawing
	vec4 fc = max(	getBackbufferTransform(t1, v),
			getBackbufferTransform(t2, v));
	gl_FragColor = (1.-drawing[3])*fc -vec4(.01) + drawing[3]*drawing;
	gl_FragColor.a = alpha_back;
}

