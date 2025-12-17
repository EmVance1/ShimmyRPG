uniform sampler2D u_texture;
uniform vec2 u_resolution;
uniform int u_dist;


void main() {
    vec2 distance = u_dist * 1.0 / vec2(u_resolution);
    distance.x = -distance.x;

    vec4 pixLeft = texture2D(u_texture, gl_TexCoord[0].xy - distance);
    vec4 pixel = texture2D(u_texture, gl_TexCoord[0].xy);
    vec4 pixRight = texture2D(u_texture, gl_TexCoord[0].xy + distance);

    gl_FragColor = gl_Color * vec4(pixLeft.r, pixel.g, pixRight.b, pixel.a);
}

