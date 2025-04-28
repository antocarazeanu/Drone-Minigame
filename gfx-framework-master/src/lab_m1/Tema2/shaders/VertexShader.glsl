#version 330

// Input attributes
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;

// Uniforms for transformations
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

// Noise parameters
uniform float frequency; // Adjust for detail of terrain
uniform float amplitude; // Adjust for height scaling

// Outputs to the fragment shader
out vec3 fragPosition;
out vec3 fragNormal;

float generateNoise(vec2 pos) {
    return fract(sin(dot(pos, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    // Generate noise for height displacement
    vec3 position = v_position;
    float noiseValue = generateNoise(position.xz * frequency); // Apply noise based on XZ plane
    position.y += noiseValue * amplitude; // Modify the Y-coordinate based on noise

    // Transform vertex position and normal
    fragPosition = vec3(Model * vec4(position, 1.0));
    fragNormal = mat3(transpose(inverse(Model))) * v_normal; // Normal transformation

    // Final position for the vertex
    gl_Position = Projection * View * vec4(fragPosition, 1.0);
}
