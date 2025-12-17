uniform sampler2D u_texture;
uniform vec2 u_resolution;


void main() {
    vec2 factor = u_resolution * 0.5;
    vec2 uv = floor(gl_TexCoord[0].xy * factor + 0.5) / factor;
    // vec2 uv = gl_TexCoord[0].xy;
    gl_FragColor = gl_Color * texture2D(u_texture, uv);
}

