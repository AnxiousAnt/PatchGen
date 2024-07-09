uniform float cogs;
uniform float spokes;
uniform float dir;
uniform float sweep;
uniform float teeth;
uniform float hole;
uniform float radiusin;
uniform float radiusout;
uniform float threshin;
uniform float threshout;
uniform float spin;

void main(void) {
  vec2 p = gl_TexCoord[0].xy * 2.0 - 1.0;
  float r = length(p);
  if (1.0 > r && r > hole) {
    float a0 = atan(p.y, p.x);
    float a = a0 + spin;
    float tr = 1.0 - teeth * (1.0 - cos(cogs * a));
    if (tr > r) {
      a *= spokes;
      a += dir * pow(sweep, r);
      float s = cos(a);
      float t = -2.0;
      float h = 10.0;
      float q = cos(8.0 * cogs * r * r);
      if (radiusout > r && r > radiusin) {
        t = mix(threshin, threshout, (r - radiusin) / (radiusout - radiusin));
        q = cos(5.0 * a);
      }
      if (s > t) {
        vec3 u = vec3( cos(a0), sin(a0), 0.5 * q * (1.0 - r * r));
        vec3 v = vec3(-sin(a0), cos(a0), 0.0);
        vec3 n = normalize(cross(u, v));
        vec3 l = normalize(vec3(vec2(960.0, 270.0) - gl_FragCoord.xy, 512.0));
        float d = max(dot(l, n), 0.0);
        vec3 f = 2.0 * d * n - l;
        float c = 0.5 * pow(max(f.z, 0.0), h);
        gl_FragColor = vec4(mix(0.5 * d * gl_Color.rgb, vec3(1.0), clamp(c, 0.0, 1.0)), 1.0);
      } else {
        discard;
      }
    } else {
      discard;
    }
  } else {
    discard;
  }
}
