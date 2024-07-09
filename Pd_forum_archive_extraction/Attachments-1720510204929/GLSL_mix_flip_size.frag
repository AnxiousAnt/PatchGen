//jack/RYBN 2013
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect Ttex1;
uniform sampler2DRect Ttex2;
uniform sampler2DRect tex0;
uniform float style;
uniform float mix_factor;
uniform vec2 flip1, flip2;
uniform vec2 dimen, resize1, resize2, move1, move2;

ivec2 size1 = textureSize2DRect(Ttex1, 0);
ivec2 size2 = textureSize2DRect(Ttex2, 0);

void main (void)
{
	vec2 sizeF1 = vec2(size1)*(1.0/resize1);
	vec2 sizeF2 = vec2(size2)*(1.0/resize2);
	vec4 color1 = texture2DRect(Ttex1, abs(flip1-vec2(gl_FragCoord)/dimen)*sizeF1-move1/resize1);
	vec4 color2 = texture2DRect(Ttex2, abs(flip2-vec2(gl_FragCoord)/dimen)*sizeF2-move2/resize2);

	if (style == 0.) {
		gl_FragColor = (color1 + color2);
	} else if (style == 1.) {
		gl_FragColor = (color1 - color2);
	} else if (style == 2.) {
		gl_FragColor = abs(color1 - color2);
	} else if (style == 3.) {
		gl_FragColor = (color1 * color2);
	} else if (style == 4.) {
		gl_FragColor = mix(color1,color2,mix_factor);
	}

}

