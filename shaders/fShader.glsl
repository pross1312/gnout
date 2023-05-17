#version 430

uniform float time;

in vec2 UV;
in vec4 fore_ground;
in vec4 back_ground;
in vec2 position;
out vec4 color;
// 
// 
uniform sampler2D cache_font_texture;

float map01(float x) {
    return (x + 1.0f) / 2.0f;
}

void main() {
    vec4 tex_color = texture(cache_font_texture, UV);
    vec4 t = vec4(map01(sin(time + UV.x * 100 + position.x)), 
                  map01(cos(time + UV.y + 100 + position.y)),
                  map01(sin(time + UV.y*100 + position.y)) * map01(cos(time + UV.x*100) + position.x),
                  1.0f);
    color = (1 - tex_color.w) * back_ground + tex_color.w * fore_ground * t;

}
