#define PROCESSING_TEXTURE_SHADER

uniform sampler2D texture; // output texture

varying vec2 vTex; // input vector

uniform vec2 LensCenter; // center the distortion function around center of the lens
uniform vec2 ScreenCenter; // texture coordinate for the center of FULL-screen texture
uniform vec2 Scale; // rescale output coordinates back to texture range
uniform vec2 ScaleIn; // rescale input texture coordinates to [-1,1] unit range
uniform vec4 HmdWarpParam; // array of distortion coefficients

vec2 HmdWarp(vec2 texIn)
{
	vec2 theta = (texIn - LensCenter) * ScaleIn;
	float rSq = theta.x * theta.x + theta.y * theta.y;
	vec2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);
	return LensCenter + Scale * theta1;
}

void main()
{
	vec2 tc = HmdWarp(vTex);
	if (any(notEqual(clamp(tc, ScreenCenter-vec2(0.5,0.5), ScreenCenter+vec2(0.5, 0.5)) - tc, vec2(0.0, 0.0))))
		discard;
	else
		gl_FragColor = texture2D(texture, tc);
}