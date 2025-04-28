#version 330

layout(location = 0) in vec3 v_position; // Vertex position
layout(location = 1) in vec2 v_texCoord; // Texture coordinate

out vec2 TexCoord;

uniform mat4 projection; // 2D orthographic projection matrix

void main() {
    gl_Position = projection * vec4(v_position, 1.0);
    TexCoord = v_texCoord;
}
