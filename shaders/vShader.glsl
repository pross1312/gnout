#version 430

layout(location = 0) in vec2 uv;
layout(location = 1) in vec2 uv_size;
layout(location = 2) in vec2 pos;
layout(location = 3) in vec2 char_size;
layout(location = 4) in vec4 fg;
layout(location = 5) in vec4 bg;

out vec2 UV;
out vec2 position;
out vec4 fore_ground;
out vec4 back_ground;

void main() {
    vec2 quad = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1));
    gl_Position = vec4(pos.x + quad.x * char_size.x, pos.y - quad.y * char_size.y, .0f, 1.0f); // flip so that (0, 0) will be top left
    fore_ground = fg;
    back_ground = bg;
    position = gl_Position.xy;
    UV = uv + vec2(quad.x, quad.y) * uv_size;
}
