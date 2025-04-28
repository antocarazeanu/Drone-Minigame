#version 330

in vec3 fragPosition;
in vec3 fragNormal;

// Output color
out vec4 fragColor;

void main() {
    float height = fragPosition.y;

    // Blend colors based on height (low = dirt, high = grass, very high = snow)
    vec3 lowColor = vec3(0.3, 0.2, 0.1);  // Dirt color
    vec3 midColor = vec3(0.2, 0.6, 0.2);  // Grass color
    vec3 highColor = vec3(1.0, 1.0, 1.0); // Snow color

    vec3 terrainColor = mix(lowColor, midColor, smoothstep(0.0, 5.0, height));
    terrainColor = mix(terrainColor, highColor, smoothstep(5.0, 10.0, height));

    fragColor = vec4(terrainColor, 1.0);
}
