#version 430

in vec2 UV;
in vec4 fore_ground;
out vec4 color;


uniform sampler2D renderingTexture;


void main() {
    color = texture(renderingTexture, UV) * fore_ground * vec4(UV, UV);
}
