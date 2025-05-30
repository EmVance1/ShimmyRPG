uniform sampler2D u_texture;


void main() {
    vec2 factor = vec2(1920, 1200.0) * 0.5;
    vec2 uv = floor(gl_TexCoord[0].xy * factor + 0.5) / factor;
    // vec2 uv = gl_TexCoord[0].xy;
    gl_FragColor = gl_Color * texture2D(u_texture, uv);
}

