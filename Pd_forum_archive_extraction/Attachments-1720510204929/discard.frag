//Jack/RYBN 2009
#extension GL_ARB_texture_rectangle : enable
uniform sampler2DRect MyTex;
uniform vec3 colorToDiscard;
uniform float seuil;
float rouge, vert, bleu;

void main (void)
{
	vec4 color = texture2DRect(MyTex, gl_TexCoord[0].st);

	if (abs(color.r - colorToDiscard.r) >= seuil) {
		rouge = 1.0;
	} else {
		if ((color.r - colorToDiscard.r) <= 0.0) {
			rouge = (color.r - colorToDiscard.r)/seuil;
		} else {
			rouge = (colorToDiscard.r - color.r)/seuil;
		}
	}

	if (abs(color.g - colorToDiscard.g) >= seuil) {
		vert = 1.0;
	} else {
		if ((color.g - colorToDiscard.g) <= 0.0) {
			vert = (color.g - colorToDiscard.g)/seuil;
		} else {
			vert = (colorToDiscard.g - color.g)/seuil;
		}
	}

	if (abs(color.b - colorToDiscard.b) >= seuil) {
		bleu = 1.0;
	} else {
		if ((color.b - colorToDiscard.b) <= 0.0) {
			bleu = (color.b - colorToDiscard.b)/seuil;
		} else {
			bleu = (colorToDiscard.b - color.b)/seuil;
		}
	}

	color.a = (rouge+vert+bleu) / 3.0;
	gl_FragColor = color;
}
