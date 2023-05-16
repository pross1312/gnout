#version 430

layout(location = 0) in vec2 uv;
layout(location = 1) in vec2 uv_size;
layout(location = 2) in vec2 pos;
layout(location = 3) in vec2 char_size;
layout(location = 4) in vec4 fg;

out vec2 UV;
out vec4 fore_ground;

void main() {
    vec2 quad = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1));
    gl_Position = vec4(pos + quad * char_size, .0f, 1.0f);
    fore_ground = fg;
    UV = uv + vec2(quad.x, 1 - quad.y) * uv_size;
}
