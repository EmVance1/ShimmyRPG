#ifdef GL_ES
   precision highp float;
#endif

uniform sampler2D u_texture;
uniform vec2 u_curvature;
 

vec2 curveRemapUV(vec2 uv) {
    uv = uv * 2.0 - 1.0;
    vec2 offset = abs(uv.yx) / u_curvature.xy;
    uv = uv + uv * offset * offset;
    uv = uv * 0.5 + 0.5;

    return uv;
}

void main() {
    vec2 remappedUV = curveRemapUV(gl_TexCoord[0].xy);
    vec4 baseColor = texture2D(u_texture, remappedUV);

    if (remappedUV.x < 0.0 || remappedUV.y < 0.0 || remappedUV.x > 1.0 || remappedUV.y > 1.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        gl_FragColor = baseColor;
    }
}

