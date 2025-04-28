#pragma once
#ifndef TEMA1_H
#define TEMA1_H

#include <vector>
#include <cmath>
#include "utils/gl_utils.h"

class Terrain {
public:
    Terrain(int width, int height, float amplitude, float frequency);
    void generateTerrain();
    void drawTerrain();

private:
    int width;
    int height;
    float amplitude;
    float frequency;
    std::vector<float> heightMap;
};

#endif

