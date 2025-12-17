uniform sampler2D u_texture;


void main() {
    vec4 tex = texture2D(u_texture, gl_TexCoord[0].xy);

    float val = tex.r * 0.299 + tex.g * 0.587 + tex.b * 0.114;

    gl_FragColor = gl_Color * vec4(val, val, val, 1.0);
}
