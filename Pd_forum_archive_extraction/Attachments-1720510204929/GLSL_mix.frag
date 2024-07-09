//jack/RYBN 2010
#extension GL_EXT_gpu_shader4 : enable
#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect Ttex1;
uniform sampler2DRect Ttex2;
uniform sampler2DRect Ttex3;
uniform sampler2DRect tex0;
uniform float style, target;
uniform float mix_factor, mix_color;
uniform float R, G, B, A;
uniform float FlipV1, FlipV2, FlipH1, FlipH2;


varying vec2 texcoord0;
ivec2 size1 = textureSize2DRect(Ttex1, 0);
ivec2 size2 = textureSize2DRect(Ttex2, 0);
ivec2 size3 = textureSize2DRect(Ttex3, 0);
ivec2 size0 = textureSize2DRect(tex0, 0);
vec2 texcoord1,texcoord2;
void main (void)
{
	
	float sizeF1X = float(size1.x)/float(size0.x);
	float sizeF1Y = float(size1.y)/float(size0.y);
	float sizeF2X = float(size2.x)/float(size0.x);
	float sizeF2Y = float(size2.y)/float(size0.y);
	float sizeF3X = float(size3.x)/float(size0.x);
	float sizeF3Y = float(size3.y)/float(size0.y);
	
	if (FlipV1 == 0.) {
		if (FlipH1 == 0.) {
			texcoord1 = vec2(texcoord0.s*sizeF1X, texcoord0.t*sizeF1Y);
		 } else if (FlipH1 == 1.) {
			texcoord1 =  vec2(float(size1.x) - texcoord0.s*sizeF1X, texcoord0.t*sizeF1Y);
		}
	} else if (FlipV1 == 1.) {
		if (FlipH1 == 0.) {
			texcoord1 =  vec2(texcoord0.s*sizeF1X,float(size1.y) - texcoord0.t*sizeF1Y);
		} else if (FlipH1 == 1.) {
			texcoord1 =  vec2(float(size1.x) - texcoord0.s*sizeF1X,float(size1.y) - texcoord0.t*sizeF1Y);
		}
	}
	if (FlipV2 == 0.) {
		if (FlipH2 == 0.) {
			texcoord2 =  vec2(texcoord0.s*sizeF2X,texcoord0.t*sizeF2Y);
		} else if (FlipH2 == 1.) {
			texcoord2 =  vec2(float(size2.x) - texcoord0.s*sizeF2X,texcoord0.t*sizeF2Y);
		}
	} else if (FlipV2 == 1.) {
		if (FlipH2 == 0.) {
			texcoord2 =  vec2(texcoord0.s*sizeF2X,float(size2.y) - texcoord0.t*sizeF2Y);
		} else if (FlipH2 == 1.) {
			texcoord2 =  vec2(float(size2.x) - texcoord0.s*sizeF2X,float(size2.y) - texcoord0.t*sizeF2Y);
		}
	}
	vec4 color1 = texture2DRect(Ttex1, texcoord1);
	vec4 color2 = texture2DRect(Ttex2, texcoord2);
	vec4 color3 = vec4(R, G, B, A);
	if (style == 0.) {
		if (target == 0.) {
			gl_FragColor = (color1 + color2);
		} else if (target == 1.) {
			gl_FragColor = (color1 + color3);
		}
	} else if (style == 1.) {
		if (target == 0.) {
			gl_FragColor = (color1 - color2);
		} else if (target == 1.) {
			gl_FragColor = (color1 - color3);
		}
	} else if (style == 2.) {
		if (target == 0.) {
			gl_FragColor = abs(color1 - color2);
		} else if (target == 1.) {
			gl_FragColor = abs(color1 - color3);
		}
	} else if (style == 3.) {
		if (target == 0.) {
			gl_FragColor = (color1 * color2);
		} else if (target == 1.) {
			gl_FragColor = (color1 * color3);
		}
	} else if (style == 4.) {
		gl_FragColor = mix(color1,color2,mix_factor);
	} else if (style == 5.) {
		gl_FragColor = mix(mix(color1,color2,mix_factor),color3,mix_color);
	}


}

