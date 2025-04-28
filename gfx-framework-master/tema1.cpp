// Tema1.cpp
#include "tema1.h"

// Constructorul clasei Terrain
Terrain::Terrain(int width, int height, float amplitude, float frequency)
    : width(width), height(height), amplitude(amplitude), frequency(frequency) {
    generateTerrain();
}

// Generarea terenului folosind funcții sinusoidale
void Terrain::generateTerrain() {
    heightMap.resize(width);
    for (int x = 0; x < width; ++x) {
        float heightValue = amplitude * sin(frequency * x) +
            (amplitude / 2) * sin(2 * frequency * x) +
            (amplitude / 4) * sin(4 * frequency * x);
        heightMap[x] = heightValue;
    }
}

// Desenarea terenului folosind OpenGL
void Terrain::drawTerrain() {
    glBegin(GL_LINE_STRIP);
    for (int x = 0; x < width; ++x) {
        float y = heightMap[x];
        glVertex2f(x, y);
    }
    glEnd();
}
