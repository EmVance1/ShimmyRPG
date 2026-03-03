uniform sampler2D u_texture;


float nColors = 8.0;

vec3 lerp(vec3 colorone, vec3 colortwo, float value) {
    return (colorone + value*(colortwo-colorone));
}

vec3 RGBToHSV( vec3 RGB ){
    vec4 k = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    vec4 p = RGB.g < RGB.b ? vec4(RGB.b, RGB.g, k.w, k.z) : vec4(RGB.gb, k.xy);
    vec4 q = RGB.r < p.x   ? vec4(p.x, p.y, p.w, RGB.r) : vec4(RGB.r, p.yzx);
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 HSVToRGB( vec3 HSV ){
    vec4 k = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(HSV.xxx + k.xyz) * 6.0 - k.www);
    return HSV.z * lerp(k.xxx, clamp(p - k.xxx, 0.0, 1.0), HSV.y);
}


void main() {
    vec4 tex = texture2D(u_texture, gl_TexCoord[0].xy);
    vec3 tc = tex.rgb;
    vec2 coord = vec2(0.,0.);

    float cutColor = 1./nColors;

    tc = RGBToHSV(tc);
    vec2 target_c = cutColor * floor(tc.gb/cutColor);
    tc = HSVToRGB(vec3(tc.r,target_c));

    gl_FragColor = gl_Color * vec4(tc, tex.a);
}

