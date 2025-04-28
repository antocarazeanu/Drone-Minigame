#version 330

in vec2 TexCoord;

out vec4 fragColor;

uniform sampler2D textTexture; // Text atlas texture
uniform vec3 textColor;        // Color of the text

void main() {
    vec4 sampled = texture(textTexture, TexCoord);
    fragColor = vec4(textColor, 1.0) * sampled;
}
