uniform sampler2D curtex;
uniform sampler2D bgndtex;
uniform int blendmode;

uniform float cropleft, cropright, croptop, cropbottom;

float computeGrey(vec4 srccolor);

void main (void)
{
 vec2 xy=gl_TexCoord[0].st;
 float x=gl_TexCoord[0].s;
 float y=gl_TexCoord[0].t;

 vec4 frontcolor = texture2D(curtex, xy);
 
 float bx=x*cropright+(1.-x)*cropleft;
 float by=y*croptop+(1.-y)*cropbottom;

 vec4 bgndcolor = texture2D(bgndtex,vec2(bx,by));

 if (blendmode==0) //normal
  gl_FragColor = frontcolor;
 else if (blendmode==1) //add
  gl_FragColor = frontcolor + bgndcolor;
 else if (blendmode==2) //multiply
  gl_FragColor = frontcolor * bgndcolor;
 else if (blendmode==3) { //difference
  float frontgrey=computeGrey(frontcolor);
  float bggrey=computeGrey(bgndcolor);
  float resultgrey=abs(frontgrey-bggrey);
  gl_FragColor=vec4(resultgrey, resultgrey, resultgrey, 1.);
 }
 else { //subtract
  float frontgrey=computeGrey(frontcolor);
  float bggrey=computeGrey(bgndcolor);
  float resultgrey=max(0.,min(1.,bggrey-frontgrey));
  gl_FragColor=vec4(resultgrey,resultgrey,resultgrey,1.);
 }

  gl_FragColor.a=1.;

}

float computeGrey(vec4 srccolor) {
  return 0.3*srccolor.r+0.59*srccolor.g+0.11*srccolor.b;
}
