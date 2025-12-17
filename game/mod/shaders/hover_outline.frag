const float offset = 2.0 / 128.0;
varying vec2 v_texCoords;
uniform sampler2D u_texture;


void main() {
    vec4 col = texture2D(u_texture, v_texCoords);
    if (col.a > 0.5) {
        gl_FragColor = col;
    } else {
        float a = texture2D(u_texture, vec2(v_texCoords.x + offset, v_texCoords.y)).a +
                  texture2D(u_texture, vec2(v_texCoords.x, v_texCoords.y - offset)).a +
                  texture2D(u_texture, vec2(v_texCoords.x - offset, v_texCoords.y)).a +
                  texture2D(u_texture, vec2(v_texCoords.x, v_texCoords.y + offset)).a;

        if (col.a < 1.0 && a > 0.0) {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        } else {
            gl_FragColor = col;
        }
    }
}

