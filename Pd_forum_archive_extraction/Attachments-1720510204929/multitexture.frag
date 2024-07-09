uniform sampler2DRect MyTex;
uniform sampler2DRect MyTex1;

varying vec2 texcoord1;
varying vec2 texcoord2;

uniform float test;


void main (void)
{

 float try = test * test;

 vec4 color = texture2DRect(MyTex, texcoord1);
 vec4 color2 = texture2DRect(MyTex1, texcoord1);
 vec4 temp = color * color2;
 gl_FragColor = temp;
}

