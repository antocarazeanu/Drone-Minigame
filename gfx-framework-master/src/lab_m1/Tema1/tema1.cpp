#include "lab_m1/tema1/tema1.h"

#include <vector>
#include <iostream>
#include <cmath>

#include "lab_m1/lab3/transform2D.h"
#include "lab_m1/lab3/object2D.h"

using namespace std;
using namespace m1;

Tema1::Tema1() :
    terrainWidth(1000),
    terrainStep(5.0f),
    translateX(0),
    translateY(0),
    scaleX(1), scaleY(1),
    angularStep(0),
    speed(50),
    angularSpeed(1),
    myTankBase(nullptr),
    myTankBarrel(nullptr),
    myTankLifeBar(nullptr),
    enemyTankBase(nullptr),
    enemyTankBarrel(nullptr),
    enemyTankLifeBar(nullptr),
    projectile(nullptr),
    explosionMesh (nullptr),
	enemyexplosionMesh(nullptr),
    gravity(300.0f),
    projectileSpeed(400.0f),
    myTankX(50.0f),
    enemyTankX(950.0f),
    myTankPosition(glm::vec3(50.0f, 0, 0)),
    enemyTankPosition(glm::vec3(950.0f, 0, 0)),
    myTankAngle(0.0f),
    enemyTankAngle(0.0f),
    myTankTurretAngle(0.0f),
    enemyTankTurretAngle(0.0f),
    myTankLife(100.0f),
    enemyTankLife(100.0f),
    maxLife(100.0f),
    collisionThreshold(5.0f),
	terrainDeformed(false),
    mydamage(10.0f),
    enemydamage(10.0f)
{
}

Tema1::~Tema1()
{
}

void Tema1::Init() {
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Render the sky gradient
    CreateSkyGradient();

	// Create the terrain mesh
    terrainMesh = CreateTerrainMesh();
    
    // Create the sun mesh
    sunMesh = CreateSunMesh();

    // Create the tank meshes
    glm::vec3 myTankColor(0.8f, 0.7f, 0.5f); // Light brown
    glm::vec3 enemyTankColor(1.0f, 0.0f, 0.0f); // Red

    // Create meshes for myTank (base and barrel)
    myTankBase = CreateTankBaseMesh(myTankColor);
    myTankBarrel = CreateTankBarrelMesh();

    // Create meshes for enemyTank (base and barrel)
    enemyTankBase = CreateTankBaseMesh(enemyTankColor);
    enemyTankBarrel = CreateTankBarrelMesh();

    // Create life bars
    myTankLifeBar = CreateLifeBarBorderMesh();
    myTankLifeBarInner = CreateLifeBarInnerMesh(myTankLife);
    enemyTankLifeBar = CreateLifeBarBorderMesh();
    enemyTankLifeBarInner = CreateLifeBarInnerMesh(enemyTankLife);

    // Create the projectile mesh
    projectile = CreateProjectileMesh();

	// Create the hitpoint circle for my tank
	myhitpointsCircle = CreateCircleMesh(myTankX, myTankY);
	enemyhitpointsCircle = CreateCircleMesh(enemyTankX, enemyTankY);

	// Create the trajectory mesh
	trajectoryMesh = CreateTrajectoryMesh();

    cloudMesh = CreateCloudMesh();  // Create the cloud mesh

    // Define the number of clouds and randomize positions
    int numClouds = 10;
    for (int i = 0; i < numClouds; ++i) {
        float xPos = i * 150.0f + (rand() % 100);  // Random horizontal spacing
        float yPosBase = 450.0f + (i % 3) * 100.0f; // Three different base vertical positions
        float yPos = yPosBase + (rand() % 50);     // Random vertical position within a range
        cloudPositions.push_back(glm::vec2(xPos, yPos));
    }

    // Create the flag mesh
    flagMesh1 = CreateFlagMesh();
	flagMesh2 = CreateFlagMesh();

    // Spawn two flags on the map with random positions
    for (int i = 0; i < 2; ++i) {
        float xPos = rand() % terrainWidth;
        int index = static_cast<int>(xPos / terrainStep);
        float terrainHeight = heightMap[index];
        float yPos = terrainHeight;
        flagPositions.push_back(glm::vec2(xPos, yPos));
    }
}

// ========================================Decoration========================================

Mesh* Tema1::CreateCloudMesh() {
    std::vector<VertexFormat> cloudVertices;
    std::vector<unsigned int> cloudIndices;

    // Define circle positions and colors for the cloud shape
    std::vector<glm::vec3> positions = {
        glm::vec3(-20, 0, 0),  // Left circle
        glm::vec3(0, 0, 0),    // Center circle
        glm::vec3(20, 0, 0),   // Right circle
        glm::vec3(-10, 15, 0), // Top-left circle
        glm::vec3(10, 15, 0)   // Top-right circle
    };
    glm::vec3 cloudColor(0.9f, 0.9f, 0.9f);  // Light gray color for the cloud

    // Generate vertices and indices for each circle
    int indexOffset = 0;
    for (const auto& pos : positions) {
        int circleSegments = 20;
        float radius = 15.0f;

        for (int i = 0; i < circleSegments; ++i) {
            float angle = i * 2.0f * glm::pi<float>() / circleSegments;
            glm::vec3 vertexPos = pos + glm::vec3(radius * cos(angle), radius * sin(angle), 0);
            cloudVertices.emplace_back(vertexPos, cloudColor);

            if (i > 0) {
                cloudIndices.push_back(indexOffset);
                cloudIndices.push_back(indexOffset + i);
                cloudIndices.push_back(indexOffset + i + 1);
            }
        }
        // Close the circle by connecting the last vertex back to the start
        cloudIndices.push_back(indexOffset);
        cloudIndices.push_back(indexOffset + circleSegments - 1);
        cloudIndices.push_back(indexOffset + 1);

        indexOffset += circleSegments;
    }

    Mesh* cloudMesh = new Mesh("cloud");
    cloudMesh->InitFromData(cloudVertices, cloudIndices);
    return cloudMesh;
}

Mesh* Tema1::CreateSunMesh() {
    glm::ivec2 resolution = window->GetResolution();
    float sunRadius = 50.0f;
    glm::vec3 innerColor(1.0f, 0.5f, 0.0f); // Darker orange for the center of the sun
    glm::vec3 outerColor(1.0f, 0.8f, 0.0f); // Lighter yellow-orange for the outer part

    std::vector<VertexFormat> sunVertices;
    std::vector<unsigned int> sunIndices;

    // Number of segments for the sun circle and rays
    int circleSegments = 30;
    float angleStep = 2.0f * M_PI / circleSegments;

    // Add center point for the inner gradient circle
    glm::vec3 centerPosition(resolution.x - sunRadius, resolution.y - sunRadius, -0.5f);
    sunVertices.emplace_back(centerPosition, innerColor);
    unsigned int centerIndex = 0;

    // Create vertices for the inner gradient circle
    for (int i = 0; i < circleSegments; ++i) {
        float angle = i * angleStep;

        // Interpolating color for gradient effect
        glm::vec3 gradientColor = glm::mix(innerColor, outerColor, static_cast<float>(i) / circleSegments);

        // Inner circle vertex with gradient color
        float xInner = centerPosition.x + sunRadius * 0.8f * cos(angle); // Slightly smaller radius for the inner circle
        float yInner = centerPosition.y + sunRadius * 0.8f * sin(angle);
        sunVertices.emplace_back(glm::vec3(xInner, yInner, -0.5f), gradientColor);
        sunIndices.push_back(centerIndex); // Center point index
        sunIndices.push_back(i + 1); // Current inner vertex
        sunIndices.push_back((i + 1) % circleSegments + 1); // Next inner vertex
    }

    // Create vertices for the spiky rays
    for (int i = 0; i < circleSegments; ++i) {
        float angle = i * angleStep;

        // Inner vertex (for the base of each ray)
        float xInnerRayBase = centerPosition.x + sunRadius * cos(angle);
        float yInnerRayBase = centerPosition.y + sunRadius * sin(angle);
        sunVertices.emplace_back(glm::vec3(xInnerRayBase, yInnerRayBase, -0.5f), outerColor);
        sunIndices.push_back(i * 2 + circleSegments + 1); // Base of the ray

        // Outer vertex (tip of each ray)
        float rayLength = sunRadius * 1.4f;
        float xOuterRayTip = centerPosition.x + rayLength * cos(angle);
        float yOuterRayTip = centerPosition.y + rayLength * sin(angle);
        sunVertices.emplace_back(glm::vec3(xOuterRayTip, yOuterRayTip, -0.5f), outerColor);
        sunIndices.push_back(i * 2 + circleSegments + 2); // Tip of the ray
    }

    // Close the loop for the rays by connecting the last pair to the first
    sunIndices.push_back(circleSegments + 1);
    sunIndices.push_back(circleSegments + 2);

    // Initialize and return the sun mesh
    Mesh* sunMesh = new Mesh("gradientSun");
    sunMesh->InitFromData(sunVertices, sunIndices);

    return sunMesh;
}

// Create the sky gradient quad
void Tema1::CreateSkyGradient()
{
    glm::ivec2 resolution = window->GetResolution();

    std::vector<VertexFormat> skyVertices;
    std::vector<unsigned int> skyIndices;

    skyVertices.emplace_back(glm::vec3(0, 0, -1.0f), glm::vec3(0.4f, 0.8f, 1.0f)); // Light blue
    skyVertices.emplace_back(glm::vec3(resolution.x, 0, -1.0f), glm::vec3(0.4f, 0.8f, 1.0f)); // Light blue
    skyVertices.emplace_back(glm::vec3(resolution.x, resolution.y, -1.0f), glm::vec3(0.1f, 0.3f, 0.6f)); // Darker blue
    skyVertices.emplace_back(glm::vec3(0, resolution.y, -1.0f), glm::vec3(0.1f, 0.3f, 0.6f)); // Darker blue

    skyIndices.push_back(0); skyIndices.push_back(1); skyIndices.push_back(2);
    skyIndices.push_back(2); skyIndices.push_back(3); skyIndices.push_back(0);

    skyMesh = new Mesh("sky");
    skyMesh->InitFromData(skyVertices, skyIndices);
}

// ========================================Terrain========================================

// Create terrain mesh
Mesh* Tema1::CreateTerrainMesh()
{
    // Generate the height map with complex terrain
    heightMap.resize(terrainWidth);
    for (int i = 0; i < terrainWidth; ++i) {
        float t = i * terrainStep;
        heightMap[i] = 125 + (130 * sin(0.002f * t) + 90 * sin(0.0003f * t) + 80 * sin(0.015f * t));
    }

    // Create vertices and indices for the terrain mesh
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    for (int i = 0; i < terrainWidth; ++i) {
        float x = i * terrainStep;
        float y = heightMap[i];
        vertices.emplace_back(glm::vec3(x, y, 0), glm::vec3(0, 1, 0)); // Top vertex (terrain height), z = 0.1f
        vertices.emplace_back(glm::vec3(x, 0, 0), glm::vec3(0, 0.8f, 0)); // Bottom vertex (ground level), z = 0.1f
    }

    // Create triangle strip indices for terrain
    for (int i = 0; i < terrainWidth - 1; ++i) {
        indices.push_back(2 * i);         // Top vertex of current point
        indices.push_back(2 * i + 1);     // Bottom vertex of current point
        indices.push_back(2 * (i + 1));   // Top vertex of next point
        indices.push_back(2 * (i + 1));   // Top vertex of next point
        indices.push_back(2 * i + 1);     // Bottom vertex of current point
        indices.push_back(2 * (i + 1) + 1); // Bottom vertex of next point
    }

    terrainMesh = new Mesh("terrain");
    terrainMesh->InitFromData(vertices, indices);

    return terrainMesh;
}

void Tema1::DeformTerrain(glm::vec2 impactPoint, float radius, float depth) {
    int centerIndex = static_cast<int>(impactPoint.x / terrainStep);
    int radiusInSteps = static_cast<int>(radius / terrainStep);

    for (int i = std::max(0, centerIndex - radiusInSteps); i <= std::min(terrainWidth - 1, centerIndex + radiusInSteps); ++i) {
        // Calculate the distance from the impact point
        float distance = std::abs(impactPoint.x - (i * terrainStep));

        if (distance < radius) {
            // Use the equation of a circle to calculate the lowering factor
            float loweringFactor = std::sqrt(radius * radius - distance * distance) / radius * depth;
            heightMap[i] = std::min(heightMap[i], impactPoint.y - loweringFactor); // Lower the height to form a perfect semicircle
        }
    }

    UpdateTerrainMesh(); // Update the mesh with the new heightMap values
}

void Tema1::UpdateTerrainMesh() {
    if (terrainMesh) {
        std::vector<VertexFormat> vertices;
        std::vector<unsigned int> indices;

        for (int i = 0; i < terrainWidth; ++i) {
            float x = i * terrainStep;
            float y = heightMap[i];
            vertices.emplace_back(glm::vec3(x, y, 0), glm::vec3(0, 1, 0));
            vertices.emplace_back(glm::vec3(x, 0, 0), glm::vec3(0, 0.8f, 0));
        }

        for (int i = 0; i < terrainWidth - 1; ++i) {
            indices.push_back(2 * i);
            indices.push_back(2 * i + 1);
            indices.push_back(2 * (i + 1));
            indices.push_back(2 * (i + 1));
            indices.push_back(2 * i + 1);
            indices.push_back(2 * (i + 1) + 1);
        }

        terrainMesh->InitFromData(vertices, indices);
    }
    else {
        std::cerr << "Error: terrainMesh is not initialized." << std::endl;
    }
}

void Tema1::SimulateLandslide(float deltaTimeSeconds) {
    float thresholdHeightDiff = 10.0f; // Threshold for height difference to trigger landslide
    float epsilonBase = 10.0f; // Base epsilon for landslide transfer
    float epsilonFactor = 0.05f; // Factor to adjust transfer based on height difference
    float decayFactor = 0.001f; // Small decay factor to reduce height at each step

    float epsilon = epsilonBase * deltaTimeSeconds; // Adjust transfer amount based on deltaTime

    // Iterate through the height map to perform height transfer
    for (int i = 0; i < heightMap.size() - 1; ++i) {
        float heightDiff = heightMap[i] - heightMap[i + 1];

        // Check if height difference exceeds the threshold
        if (std::abs(heightDiff) > thresholdHeightDiff) {
            // Calculate an adjusted epsilon for the current height difference
            float adjustedEpsilon = epsilon + epsilonFactor * (std::abs(heightDiff) - thresholdHeightDiff);
            float transferAmount = std::min(std::abs(heightDiff) - thresholdHeightDiff, adjustedEpsilon);

            if (heightDiff > 0) {
                // Transfer height from the current point to the next point
                heightMap[i] -= transferAmount;
                heightMap[i + 1] += transferAmount;

                // Apply the decay factor to reduce the height of the current point
                heightMap[i] -= decayFactor;
            }
            else {
                // Transfer height from the next point to the current point
                heightMap[i] += transferAmount;
                heightMap[i + 1] -= transferAmount;

                // Apply the decay factor to reduce the height of the next point
                heightMap[i + 1] -= decayFactor;
            }
        }
    }
}

// ========================================Tank========================================
Mesh* Tema1::CreateTankBaseMesh(glm::vec3 tankColor) {
    std::vector<VertexFormat> tankVertices;
    std::vector<unsigned int> tankIndices;

    // Dimensions and positioning
    float baseWidthTop = 40.0f;
    float baseWidthBottom = 60.0f;
    float baseHeight = 10.0f;
    float smallerBaseWidthTop = 30.0f;
    float smallerBaseWidthBottom = 50.0f;
    float layerOffset = -10.0f;
    float circleRadius = 20.0f;
    int circleSegments = 20;

    // Darker color for lower trapezoid
    glm::vec3 darkerColor = glm::max(tankColor - glm::vec3(0.2f), glm::vec3(0.0f));

    // Upper trapezoidal base
    tankVertices.emplace_back(glm::vec3(-baseWidthBottom / 2, 0, 0), tankColor);
    tankVertices.emplace_back(glm::vec3(-baseWidthTop / 2, baseHeight, 0), tankColor);
    tankVertices.emplace_back(glm::vec3(baseWidthTop / 2, baseHeight, 0), tankColor);
    tankVertices.emplace_back(glm::vec3(baseWidthBottom / 2, 0, 0), tankColor);

    tankIndices.push_back(0); tankIndices.push_back(1); tankIndices.push_back(2);
    tankIndices.push_back(0); tankIndices.push_back(2); tankIndices.push_back(3);

    // Lower trapezoidal base
    tankVertices.emplace_back(glm::vec3(-smallerBaseWidthBottom / 2, layerOffset + baseHeight, 0), darkerColor);
    tankVertices.emplace_back(glm::vec3(-smallerBaseWidthTop / 2, layerOffset, 0), darkerColor);
    tankVertices.emplace_back(glm::vec3(smallerBaseWidthTop / 2, layerOffset, 0), darkerColor);
    tankVertices.emplace_back(glm::vec3(smallerBaseWidthBottom / 2, layerOffset + baseHeight, 0), darkerColor);

    tankIndices.push_back(4); tankIndices.push_back(5); tankIndices.push_back(6);
    tankIndices.push_back(4); tankIndices.push_back(6); tankIndices.push_back(7);

    // Turret vertices (semicircular shape above the base, using the tankColor)
    float turretRadius = 12.5f;
    float turretY = baseHeight; // Position turret above trapezoidal bases
    int turretStartIdx = tankVertices.size();
    for (int i = 0; i <= 180; i += 10) {
        float angle = glm::radians((float)i);
        float x = turretRadius * cos(angle);
        float y = turretRadius * sin(angle) + turretY;
        tankVertices.emplace_back(glm::vec3(x, y, 0), tankColor); // Using tankColor
    }

    // Indices for the turret (triangle fan)
    for (int i = 0; i < 18; i++) {
        tankIndices.push_back(turretStartIdx);
        tankIndices.push_back(turretStartIdx + i);
        tankIndices.push_back(turretStartIdx + i + 1);
    }

    Mesh* tankBaseMesh = new Mesh("tankBase");
    tankBaseMesh->InitFromData(tankVertices, tankIndices);
    return tankBaseMesh;
}

// Hitbox circle mesh for the tank
Mesh* Tema1::CreateCircleMesh(float radius, int segments = 30) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;
    
    glm::vec4 transparentColor(1.0f, 1.0f, 1.0f, 0.0f);
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), transparentColor));  // Center of the circle

    for (int i = 0; i < segments; i++) {  // Changed from i <= segments to i < segments
        float angle = i * glm::two_pi<float>() / segments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, 0), transparentColor));

        // Add indices for triangle fan
        if (i > 0) {
            indices.push_back(0);         // Center
            indices.push_back(i);         // Current vertex
            indices.push_back(i + 1);     // Next vertex
        }
    }

    // Close the circle by connecting the last vertex back to the first vertex
    indices.push_back(0);
    indices.push_back(segments);
    indices.push_back(1);

    Mesh* circleMesh = new Mesh("circleMesh");
    circleMesh->InitFromData(vertices, indices);
    return circleMesh;
}

// Create the tank barrel mesh
Mesh* Tema1::CreateTankBarrelMesh() {
    std::vector<VertexFormat> barrelVertices;
    std::vector<unsigned int> barrelIndices;

    // New dimensions and positioning (thinner and shorter barrel)
    float barrelLength = 30.0f;  // Shorter barrel
    float barrelWidth = 5.0f;    // Thinner barrel

    // Dark color for the barrel (dark grey or black)
    glm::vec3 darkColor(0.1f, 0.1f, 0.1f);  // Dark grey (you can adjust to black with (0.0f, 0.0f, 0.0f))

    // Barrel vertices
    barrelVertices.emplace_back(glm::vec3(-barrelWidth / 2, 0, 0), darkColor);
    barrelVertices.emplace_back(glm::vec3(barrelWidth / 2, 0, 0), darkColor);
    barrelVertices.emplace_back(glm::vec3(barrelWidth / 2, barrelLength, 0), darkColor);
    barrelVertices.emplace_back(glm::vec3(-barrelWidth / 2, barrelLength, 0), darkColor);

    barrelIndices.push_back(0); barrelIndices.push_back(1); barrelIndices.push_back(2);
    barrelIndices.push_back(0); barrelIndices.push_back(2); barrelIndices.push_back(3);

    Mesh* barrelMesh = new Mesh("tankBarrel");
    barrelMesh->InitFromData(barrelVertices, barrelIndices);
    return barrelMesh;
}


// ========================================Life Bar========================================
Mesh* Tema1::CreateLifeBarBorderMesh() {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Dimensions of the border
    float barWidth = 60.0f;
    float barHeight = 10.0f;

    // White border vertices
    vertices.emplace_back(glm::vec3(-barWidth / 2, -barHeight / 2, 0), glm::vec3(1, 1, 1));  // Bottom-left
    vertices.emplace_back(glm::vec3(barWidth / 2, -barHeight / 2, 0), glm::vec3(1, 1, 1));   // Bottom-right
    vertices.emplace_back(glm::vec3(barWidth / 2, barHeight / 2, 0), glm::vec3(1, 1, 1));    // Top-right
    vertices.emplace_back(glm::vec3(-barWidth / 2, barHeight / 2, 0), glm::vec3(1, 1, 1));   // Top-left

    // Indices for the border (line loop)
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);  // Close the loop

    Mesh* lifeBarBorderMesh = new Mesh("lifeBarBorder");
    lifeBarBorderMesh->InitFromData(vertices, indices);
    lifeBarBorderMesh->SetDrawMode(GL_LINE_LOOP);  // Set draw mode to line loop

    return lifeBarBorderMesh;
}

Mesh* Tema1::CreateLifeBarInnerMesh(float lifePercentage) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Full life bar width (scaled based on life percentage)
    float barWidth = 60.0f * (lifePercentage / 100.0f);
    float barHeight = 9.8f;  // Slightly smaller than the outer border

    // Red inner life bar vertices
    vertices.emplace_back(glm::vec3(-barWidth / 2, -barHeight / 2, 0), glm::vec3(1, 1, 1));   // Bottom-left
    vertices.emplace_back(glm::vec3(barWidth / 2, -barHeight / 2, 0), glm::vec3(1, 1, 1));    // Bottom-right
    vertices.emplace_back(glm::vec3(barWidth / 2, barHeight / 2, 0), glm::vec3(1, 1, 1));     // Top-right
    vertices.emplace_back(glm::vec3(-barWidth / 2, barHeight / 2, 0), glm::vec3(1, 1, 1));    // Top-left

    // Indices for the inner life bar (two triangles)
    indices.push_back(0); indices.push_back(1); indices.push_back(2);
    indices.push_back(2); indices.push_back(3); indices.push_back(0);

    Mesh* lifeBarInnerMesh = new Mesh("lifeBarInner");
    lifeBarInnerMesh->InitFromData(vertices, indices);

    return lifeBarInnerMesh;
}

void Tema1::ReduceLife(bool isMyTank, float damage) {
    if (isMyTank) {
        myTankLife -= damage;
        if (myTankLife < 0) myTankLife = 0;
    }
    else {
        enemyTankLife -= damage;
        if (enemyTankLife < 0) enemyTankLife = 0;
    }
}

void Tema1::UpdateLifeBarInnerMesh(Mesh* lifeBarMesh, float lifePercentage) {
    if (!lifeBarMesh) return;

    // Scale life bar width based on life percentage
    float barWidth = 60.0f * (lifePercentage / 100.0f);
    float barHeight = 9.8f;

    // Update vertices directly
    lifeBarMesh->vertices[0] = VertexFormat(glm::vec3(-barWidth / 2, -barHeight / 2, 0), glm::vec3(1, 1, 1)); // Bottom-left
    lifeBarMesh->vertices[1] = VertexFormat(glm::vec3(barWidth / 2, -barHeight / 2, 0), glm::vec3(1, 1, 1));  // Bottom-right
    lifeBarMesh->vertices[2] = VertexFormat(glm::vec3(barWidth / 2, barHeight / 2, 0), glm::vec3(1, 1, 1));   // Top-right
    lifeBarMesh->vertices[3] = VertexFormat(glm::vec3(-barWidth / 2, barHeight / 2, 0), glm::vec3(1, 1, 1));  // Top-left

    // Re-upload updated vertex data to the GPU
    lifeBarMesh->InitFromData(lifeBarMesh->vertices, lifeBarMesh->indices);
}

// ========================================Projectile========================================
Mesh* Tema1::CreateProjectileMesh() {
    std::vector<VertexFormat> projectileVertices;
    std::vector<unsigned int> projectileIndices;

    // Parameters for the circle (projectile)
    const int numSegments = 20;  // Number of segments for the circle
    const float radius = 5.0f;   // Radius of the circle (projectile)
    glm::vec3 darkColor(0.1f, 0.1f, 0.1f);  // Dark color (dark grey or black)

    // Center vertex (used for triangle fan)
    projectileVertices.emplace_back(glm::vec3(0, 0, 0), darkColor);

    // Create vertices for the circle
    for (int i = 0; i < numSegments; ++i) {
        float angle = (2 * glm::pi<float>() * i) / numSegments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        projectileVertices.emplace_back(glm::vec3(x, y, 0), darkColor);
    }

    // Create indices for the circle (using triangles)
    for (int i = 1; i < numSegments; ++i) {
        projectileIndices.push_back(0); // Center vertex
        projectileIndices.push_back(i); // Current vertex
        projectileIndices.push_back(i + 1); // Next vertex
    }
    // Close the circle (last segment)
    projectileIndices.push_back(0);
    projectileIndices.push_back(numSegments); // Last vertex
    projectileIndices.push_back(1); // First vertex

    // Create the mesh
    Mesh* projectileMesh = new Mesh("projectile");
    projectileMesh->InitFromData(projectileVertices, projectileIndices);
    return projectileMesh;
}

// Projectile launch function for each tank
void Tema1::LaunchProjectile(bool isMyTank) {
    Projectile newProjectile;
    float launchAngle;

    if (isMyTank) {
        launchAngle = myTankAngle + myTankTurretAngle + glm::radians(90.0f);
        newProjectile.position = glm::vec3(myTankX, heightMap[myTankX / terrainStep] + 20, 0);
        newProjectile.velocity = glm::vec2(-cos(launchAngle), sin(launchAngle)) * projectileSpeed;
		newProjectile.isFriendly = true;
    }
    else {
        launchAngle = enemyTankAngle + enemyTankTurretAngle + glm::radians(90.0f);
        newProjectile.position = glm::vec3(enemyTankX, heightMap[enemyTankX / terrainStep] + 20, 0);
        newProjectile.velocity = glm::vec2(-cos(launchAngle), sin(launchAngle)) * projectileSpeed;
		newProjectile.isFriendly = false;
    }

    newProjectile.active = true;
    projectiles.push_back(newProjectile);
}

// Update projectile position and handle gravity
void Tema1::UpdateProjectiles(float deltaTime) {
    for (auto& proj : projectiles) {
        if (proj.active) {
            // Update position based on velocity
            proj.position += glm::vec3(proj.velocity * deltaTime, 0);
            proj.velocity.y -= gravity * deltaTime; // Apply gravity to y-velocity

            // Render the projectile at the new position
            glm::mat3 projectileModelMatrix = transform2D::Translate(proj.position.x, proj.position.y);
            RenderMesh2D(projectile, shaders["VertexColor"], projectileModelMatrix);

            // Check collision with terrain
            if (CheckProjectileTerrainCollision(glm::vec2(proj.position.x, proj.position.y))) {
                proj.active = false; // Deactivate projectile on impact
                DeformTerrain(glm::vec2(proj.position.x, proj.position.y), 50.0f, 40.0f); // Deform terrain on impact
                terrainDeformed = true;
                continue; // Skip further checks for this projectile since it's deactivated
            }

            // Check collision with my tank (only if projectile is not friendly)
            if (!proj.isFriendly && CheckCollision(proj.position, myTankX, myTankY, 30.0f) && myTankLife) {
                ReduceLife(true, enemydamage); // Reduce my tank's life by 10
                proj.active = false; // Deactivate projectile on impact
                lastTimeHit = glfwGetTime();
            }

            // Check collision with enemy tank (only if projectile is friendly)
            if (proj.isFriendly && CheckCollision(proj.position, enemyTankX, enemyTankY, 30.0f) && enemyTankLife) {
                ReduceLife(false, mydamage); // Reduce enemy tank's life by 10
                proj.active = false; // Deactivate projectile on impact
				enemylastTimeHit = glfwGetTime();
            }
        }
    }

    // Remove inactive projectiles from vector
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](const Projectile& p) { return !p.active; }), projectiles.end());
}

bool Tema1::CheckCollision(glm::vec3 projectilePos, float tankX, float tankY, float hitRadius) {
    // Calculate distance between the projectile and the tank center
    float distX = projectilePos.x - tankX;
    float distY = projectilePos.y - tankY;
    float distance = sqrt(distX * distX + distY * distY);

    // Check if distance is less than the hit radius
    return distance <= hitRadius;
}

bool Tema1::CheckProjectileTerrainCollision(glm::vec2 projectilePos) {
    int index = static_cast<int>(projectilePos.x / terrainStep);

    if (index < 0 || index >= heightMap.size() - 1) return false;

    // Interpolate terrain height
    float x1 = index * terrainStep;
    float y1 = heightMap[index];
    float x2 = (index + 1) * terrainStep;
    float y2 = heightMap[index + 1];

    float t = (projectilePos.x - x1) / (x2 - x1);
    float terrainHeight = y1 * (1 - t) + y2 * t;

    // Check if projectile is within collisionThreshold of the terrain
    float distanceToTerrain = projectilePos.y - terrainHeight;
    return distanceToTerrain < collisionThreshold;
}

// ========================================Trajectory========================================
Mesh* Tema1::CreateTrajectoryMesh() {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Inițializăm traiectoria cu puncte de bază
    // (Exemplu: putem adăuga un punct inițial și un placeholder pentru următoarele)
    vertices.emplace_back(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0));  // Poziție inițială
    indices.push_back(0);

    // Inițializăm Mesh-ul
    trajectoryMesh = new Mesh("trajectory");
    trajectoryMesh->SetDrawMode(GL_LINE_STRIP);
    trajectoryMesh->InitFromData(vertices, indices);

    return trajectoryMesh;
}
void Tema1::UpdateTrajectoryMesh(float launchAngle, float initialSpeed, float gravity, float TankX, float TankY) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float timeStep = 0.05f;
    glm::vec3 startPosition = glm::vec3(TankX, TankY + 15, 0);
    glm::vec2 velocity = glm::vec2(-cos(launchAngle), sin(launchAngle)) * initialSpeed;

    for (int i = 0; i < 100; ++i) { // Ajustează limita în funcție de lungimea dorită a traiectoriei
        float time = i * timeStep;
        glm::vec3 position = startPosition + glm::vec3(
            velocity.x * time,
            velocity.y * time - 0.5f * gravity * time * time,
            0
        );

        // Adăugăm punctele de traiectorie la lista de vertecși
        vertices.emplace_back(position, glm::vec3(1, 1, 1));  // Roșu pentru traiectorie
        indices.push_back(i);

        // Verificăm dacă traiectoria depășește limitele heightMap
        int heightMapIndex = static_cast<int>(position.x / terrainStep);
        if (heightMapIndex >= 0 && heightMapIndex < heightMap.size()) {
            // Oprim calculul dacă punctul ajunge sub linia terenului
            if (position.y <= heightMap[heightMapIndex]) {
                break;
            }
        }
        else {
            // Ieșim din buclă dacă poziția e în afara limitelor heightMap
            break;
        }
    }

    // Verificăm dacă trajectoryMesh este inițializat
    if (!trajectoryMesh) {
        trajectoryMesh = new Mesh("trajectory");
    }

    // Actualizăm mesh-ul traiectoriei
    trajectoryMesh->InitFromData(vertices, indices);
}

// ========================================Effects========================================
Mesh* Tema1::CreateExplosionMesh()
{
    // Center of the explosion (e.g., where the tank was destroyed)
    glm::vec3 explosionCenter(0, 0, 0);

    // Vertices for a radial explosion effect with colors from yellow to red
    std::vector<VertexFormat> explosionVertices;
    std::vector<unsigned int> explosionIndices;

    // Define explosion radius and number of segments for a circular appearance
    float explosionRadius = 50.0f;
    int segments = 20;

    // Center vertex (yellow color)
    explosionVertices.emplace_back(explosionCenter, glm::vec3(1.0f, 1.0f, 0.0f));

    // Outer vertices (red color)
    for (int i = 0; i <= segments; ++i) {
        float angle = i * 2.0f * glm::pi<float>() / segments;
        glm::vec3 position = explosionCenter + glm::vec3(cos(angle), sin(angle), 0) * explosionRadius;
        explosionVertices.emplace_back(position, glm::vec3(1.0f, 0.0f, 0.0f));

        // Add indices for triangle fan
        if (i > 0) {
            explosionIndices.push_back(0);       // Center of the explosion
            explosionIndices.push_back(i);       // Current segment
            explosionIndices.push_back(i + 1);   // Next segment
        }
    }

    // Initialize the explosion mesh
    explosionMesh = new Mesh("explosion");
    explosionMesh->InitFromData(explosionVertices, explosionIndices);
	return explosionMesh;
}

// ========================================x2 DMG Flag===========================================
Mesh* Tema1::CreateFlagMesh() {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Stick color (brown)
    glm::vec3 stickColor(0.545f, 0.271f, 0.075f); // Brown color

    // Flag color (red)
    glm::vec3 flagColor(1.0f, 0.0f, 0.0f); // Red color

    // Stick (vertical rectangle)
    vertices.emplace_back(glm::vec3(-2, 0, 0), stickColor);
    vertices.emplace_back(glm::vec3(2, 0, 0), stickColor);
    vertices.emplace_back(glm::vec3(2, 100, 0), stickColor);
    vertices.emplace_back(glm::vec3(-2, 100, 0), stickColor);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);

    // Flag (red triangle)
    vertices.emplace_back(glm::vec3(2, 90, 0), flagColor); // Bottom-left corner of the flag
    vertices.emplace_back(glm::vec3(50, 90, 0), flagColor); // Bottom-right corner of the flag
    vertices.emplace_back(glm::vec3(2, 120, 0), flagColor); // Top corner of the flag

    indices.push_back(4);
    indices.push_back(5);
    indices.push_back(6);

    // Optional: Add "x2" on the flag using rendering text or applying a texture
    // This requires additional graphics functionality to overlay text or a texture.

    Mesh* flagMesh = new Mesh("golf_flag");
    flagMesh->InitFromData(vertices, indices);
    return flagMesh;
}


void Tema1::FrameStart()
{
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::Update(float deltaTimeSeconds)
{
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Render the sky gradient
    if (skyMesh) {
        RenderMesh2D(skyMesh, shaders["VertexColor"], glm::mat3(1));
    }
	if (sunMesh) {
		RenderMesh2D(sunMesh, shaders["VertexColor"], glm::mat3(1));
	}

    // Render the terrain mesh
    if (terrainMesh) {
        RenderMesh2D(terrainMesh, shaders["VertexColor"], glm::mat3(1));
    }
    
    // Define a threshold distance for capturing the flag
    const float captureThreshold = 10.0f;
    
    // Check if the tank captures flag 1
    if (flagMesh1) {
        float mydistanceToFlag1 = glm::distance(glm::vec2(myTankX, myTankY), flagPositions[0]);
        float enemydistanceToFlag1 = glm::distance(glm::vec2(enemyTankX, enemyTankY), flagPositions[0]);
        if (mydistanceToFlag1 < captureThreshold && flag1) {
            flagMesh1 = nullptr; // Remove the flag
            flag1 = false;
            mydamage *= 2;
        }
        if (enemydistanceToFlag1 < captureThreshold && flag1) {
            flagMesh1 = nullptr; // Remove the flag
            flag1 = false;
            enemydamage *= 2;
        }

		// update y position to the height of the terrain
		flagPositions[0].y = heightMap[flagPositions[0].x / terrainStep];
    }

    // Check if the tank captures flag 2
    if (flagMesh2) {
        float mydistanceToFlag2 = glm::distance(glm::vec2(myTankX, myTankY), flagPositions[1]);
        float enemydistanceToFlag2 = glm::distance(glm::vec2(enemyTankX, enemyTankY), flagPositions[1]);
        if (mydistanceToFlag2 < captureThreshold && flag2) {
            flag2 = false;
			mydamage *= 2;
        }
        if (enemydistanceToFlag2 < captureThreshold && flag2) {
            flag2 = false;
            enemydamage *= 2;
        }

		// update y position to the height of the terrain
		flagPositions[1].y = heightMap[flagPositions[1].x / terrainStep];
    }
	if (flagMesh1 && flag1) {
		// Update the flag position based on the wind direction
        glm::mat3 flagModelMatrix1 = transform2D::Translate(flagPositions[0].x, flagPositions[0].y);
		RenderMesh2D(flagMesh1, shaders["VertexColor"], flagModelMatrix1);
	}

	if (flagMesh2 && flag2) {
		// Update the flag position based on the wind direction
		glm::mat3 flagModelMatrix2 = transform2D::Translate(flagPositions[1].x, flagPositions[1].y);
		RenderMesh2D(flagMesh2, shaders["VertexColor"], flagModelMatrix2);
	}

    float cloudSpeed = 20.0f;  // Adjust the speed of the clouds

    for (auto& pos : cloudPositions) {
        pos.x += cloudSpeed * deltaTimeSeconds;

        // Wrap around when the cloud moves off screen
        if (pos.x > window->GetResolution().x) {
            pos.x = -100;  // Reset to the left of the screen
        }

        glm::mat3 cloudModelMatrix = transform2D::Translate(pos.x, pos.y);
        RenderMesh2D(cloudMesh, shaders["VertexColor"], cloudModelMatrix);
    }
    
    // get current time
    auto currentTime = glfwGetTime();
    if (myTankLife > 10 && myTankLife < 100 && (currentTime - lastTimeHit) > 3.0f)
    {
        myTankLife += 0.1f;
		UpdateLifeBarInnerMesh(myTankLifeBarInner, myTankLife);
    }
    if (enemyTankLife > 10 && enemyTankLife < 100 && (currentTime - enemylastTimeHit) > 3.0f)
    {
        enemyTankLife += 0.1f;
        UpdateLifeBarInnerMesh(enemyTankLifeBarInner, enemyTankLife);
    }

	// landslide
    if (terrainDeformed) {
        SimulateLandslide(deltaTimeSeconds);
        UpdateTerrainMesh();
    }

    float turretRotationSpeed = 1.0f; // Speed at which the turret rotates
    const float baseHeight = 0.0f;   // Base height
    const float turretRadius = 13.0f; // Turret radius

    // Adjust and render myTank
    if (myTankBase && myTankBarrel && myTankLife > 0) {
        // Move the tank base with A/D keys
        if (window->KeyHold(GLFW_KEY_A)) {
            int index = myTankX / terrainStep;
            if (index > 0 && index < heightMap.size()) {
                if (heightMap[index] >= heightMap[index - 1]) {
                    myTankX -= (speed + 35) * deltaTimeSeconds;
                }
                else {
                    myTankX -= (speed - 10) * deltaTimeSeconds;
                }
            }
        }
        if (window->KeyHold(GLFW_KEY_D)) {
            if (heightMap[myTankX / terrainStep] >= heightMap[myTankX / terrainStep + 1]) {
                myTankX += (speed + 35) * deltaTimeSeconds;
            }
            else myTankX += (speed - 10) * deltaTimeSeconds;
        }
        myTankX = glm::clamp(myTankX, 0.0f, terrainWidth * terrainStep);

        // Calculate base position and orientation on terrain
        int index = static_cast<int>(myTankX / terrainStep);
        myTankY = heightMap[index] + 10;
        float bx = myTankX + terrainStep;
        int nextIndex = glm::min(index + 1, static_cast<int>(heightMap.size()) - 1);
        float by = heightMap[nextIndex] + 10;
        myTankAngle = atan2(by - myTankY, bx - myTankX);
        myTankAngle = -myTankAngle;

        // Apply base transformation
        glm::mat3 baseModelMatrix = transform2D::Translate(myTankX, myTankY) *
            transform2D::Rotate(myTankAngle);
        RenderMesh2D(myTankBase, shaders["VertexColor"], baseModelMatrix);

        // Adjust turret angle with W/S keys
        if (window->KeyHold(GLFW_KEY_W)) myTankTurretAngle += turretRotationSpeed * deltaTimeSeconds;
        if (window->KeyHold(GLFW_KEY_S)) myTankTurretAngle -= turretRotationSpeed * deltaTimeSeconds;

        // Clamp turret angle to prevent it from rotating inside the tank body
        const float maxTurretAngle = glm::radians(90.0f); // 90 degrees in radians
        myTankTurretAngle = glm::clamp(myTankTurretAngle, -maxTurretAngle, maxTurretAngle);

        // Apply turret transformation
        glm::mat3 turretModelMatrix = baseModelMatrix *
            transform2D::Translate(0, baseHeight + turretRadius) * // Position turret directly above the base
            transform2D::Rotate(myTankTurretAngle); // Rotate around the turret's center

        RenderMesh2D(myTankBarrel, shaders["VertexColor"], turretModelMatrix);

        // Render my tank's life bar (Border first, then Inner)
        glm::mat3 lifeBarModelMatrix = transform2D::Translate(myTankX, myTankY + 50);
        RenderMesh2D(myTankLifeBar, shaders["VertexColor"], lifeBarModelMatrix);

        UpdateLifeBarInnerMesh(myTankLifeBarInner, myTankLife); // Update mesh with current life percentage
        lifeBarModelMatrix = transform2D::Translate(myTankX, myTankY + 50);
        RenderMesh2D(myTankLifeBarInner, shaders["VertexColor"], lifeBarModelMatrix);

        // Actualizează traiectoria în funcție de unghiul și viteza curentă ale tunului
        float launchAngle = myTankTurretAngle + myTankAngle + glm::radians(90.0f); // unghiul de lansare calculat pentru traiectorie

        try {
            UpdateTrajectoryMesh(launchAngle, projectileSpeed, gravity, myTankX, myTankY);
        }
        catch (const std::exception& ex) {
            std::cerr << "Exception occurred in UpdateTrajectoryMesh: " << ex.what() << std::endl;
        }

        // Render hitpoints overlay for my tank
        glm::mat3 hitpointsCircleMatrix = transform2D::Translate(myTankX, myTankY);
        RenderMesh2D(myhitpointsCircle, shaders["VertexColor"], hitpointsCircleMatrix);

        // Render trajectory mesh
        RenderMesh2D(trajectoryMesh, shaders["VertexColor"], glm::mat3(1));
    }

    if (myTankLife <= 0 && !explosionActive && !explosionMesh) {
        // Start the explosion when the tank is destroyed
        explosionActive = true;
        explosionStartTime = glfwGetTime();  // Record the start time
        explosionMesh = CreateExplosionMesh();               // Create the explosion mesh
        myTankLife -= 10;   // decrease this so it will not try to render again

        // deform the terrain from explosion
        glm::vec2 coordonates = { myTankX, myTankY };
        DeformTerrain(coordonates, 50, 50);

        // Start the camera shake effect
        cameraShakeActive = true;
        cameraShakeStartTime = glfwGetTime();
    }

    if (explosionActive && explosionMesh) {
        // Render the explosion at the tank's position for 2 seconds
        float currentTime = glfwGetTime();
        if (currentTime - explosionStartTime < 2.0f) {
            glm::mat3 explosionModelMatrix = transform2D::Translate(myTankX, myTankY);
            RenderMesh2D(explosionMesh, shaders["VertexColor"], explosionModelMatrix);
        }
        terrainDeformed = true;
    }

    if (cameraShakeActive) {
        float currentTime = glfwGetTime();
        if (currentTime - cameraShakeStartTime < shakeDuration) {
            // Apply camera shake
            float shakeX = (rand() % 100 / 100.0f - 0.5f) * shakeMagnitude;
            float shakeY = (rand() % 100 / 100.0f - 0.5f) * shakeMagnitude;
            auto camera = GetSceneCamera();
            camera->SetPosition(glm::vec3(shakeX, shakeY, 50));
            camera->Update();
        }
        else {
            // Stop the camera shake effect
            cameraShakeActive = false;
            auto camera = GetSceneCamera();
            camera->SetPosition(glm::vec3(0, 0, 50));
            camera->Update();
        }
    }

    // Adjust and render enemyTank
    if (enemyTankBase && enemyTankBarrel && enemyTankLife > 0) {
        // Move the enemy tank base
        if (window->KeyHold(GLFW_KEY_LEFT)) {
            int index = enemyTankX / terrainStep;
            if (index > 0 && index < heightMap.size()) {
                if (heightMap[index] >= heightMap[index - 1]) {
                    enemyTankX -= (speed + 35) * deltaTimeSeconds;
                }
                else {
                    enemyTankX -= (speed - 10) * deltaTimeSeconds;
                }
            }
        }
        if (window->KeyHold(GLFW_KEY_RIGHT))
        {
            if (heightMap[enemyTankX / terrainStep] >= heightMap[enemyTankX / terrainStep + 1]) {
                enemyTankX += (speed + 35) * deltaTimeSeconds;
            }
            else enemyTankX += (speed - 10) * deltaTimeSeconds;
        }
        enemyTankX = glm::clamp(enemyTankX, 0.0f, terrainWidth * terrainStep);

        // Calculate base position and orientation on terrain
        int index = static_cast<int>(enemyTankX / terrainStep);
        enemyTankY = heightMap[index] + 10;
        float bx = enemyTankX + terrainStep;
        int nextIndex = glm::min(index + 1, static_cast<int>(heightMap.size()) - 1);
        float by = heightMap[nextIndex] + 10;
        enemyTankAngle = atan2(by - enemyTankY, bx - enemyTankX);
        enemyTankAngle = -enemyTankAngle;

        // Apply base transformation
        glm::mat3 baseModelMatrix = transform2D::Translate(enemyTankX, enemyTankY) *
            transform2D::Rotate(enemyTankAngle);
        RenderMesh2D(enemyTankBase, shaders["VertexColor"], baseModelMatrix);

        // Adjust turret angle
        if (window->KeyHold(GLFW_KEY_UP)) enemyTankTurretAngle += turretRotationSpeed * deltaTimeSeconds;
        if (window->KeyHold(GLFW_KEY_DOWN)) enemyTankTurretAngle -= turretRotationSpeed * deltaTimeSeconds;

        // Clamp turret angle to prevent it from rotating inside the tank body
        const float maxTurretAngle = glm::radians(90.0f); // 90 degrees in radians
        enemyTankTurretAngle = glm::clamp(enemyTankTurretAngle, -maxTurretAngle, maxTurretAngle);

        // Apply turret transformation
        glm::mat3 turretModelMatrix = baseModelMatrix *
            transform2D::Translate(0, baseHeight + turretRadius) * // Position turret directly above the base
            transform2D::Rotate(enemyTankTurretAngle); // Rotate around the turret's center

        RenderMesh2D(enemyTankBarrel, shaders["VertexColor"], turretModelMatrix);

        // Render enemy tank's life bar
        glm::mat3 lifeBarModelMatrix = transform2D::Translate(enemyTankX, enemyTankY + 50);
        RenderMesh2D(enemyTankLifeBar, shaders["VertexColor"], lifeBarModelMatrix);

        UpdateLifeBarInnerMesh(enemyTankLifeBarInner, enemyTankLife); // Update mesh with current life percentage
        lifeBarModelMatrix = transform2D::Translate(enemyTankX, enemyTankY + 50);
        RenderMesh2D(enemyTankLifeBarInner, shaders["VertexColor"], lifeBarModelMatrix);

        // Actualizează traiectoria în funcție de unghiul și viteza curentă ale tunului
        float launchAngle = enemyTankTurretAngle + enemyTankAngle + glm::radians(90.0f); // unghiul de lansare calculat pentru traiectorie

        try {
            UpdateTrajectoryMesh(launchAngle, projectileSpeed, gravity, enemyTankX, enemyTankY);
        }
        catch (const std::exception& ex) {
            std::cerr << "Exception occurred in UpdateTrajectoryMesh: " << ex.what() << std::endl;
        }

        // Render hitpoints overlay for enemy tank
        glm::mat3 enemyHitpointsCircleMatrix = transform2D::Translate(enemyTankX, enemyTankY);
        RenderMesh2D(enemyhitpointsCircle, shaders["VertexColor"], enemyHitpointsCircleMatrix);

        // Render trajectory mesh
        RenderMesh2D(trajectoryMesh, shaders["VertexColor"], glm::mat3(1));
    }

    if (enemyTankLife <= 0 && !enemyexplosionActive && !enemyexplosionMesh) {
        // Start the explosion when the enemy tank is destroyed
        enemyexplosionActive = true;
        enemyexplosionStartTime = glfwGetTime();  // Record the start time for the enemy explosion
        enemyexplosionMesh = CreateExplosionMesh(); // Create the enemy explosion mesh
        enemyTankLife -= 10;   // Reduce life to prevent multiple explosions

        // Deform the terrain from the enemy tank explosion
        glm::vec2 coordinates = { enemyTankX, enemyTankY };
        DeformTerrain(coordinates, 50, 50);

        // Start the camera shake effect
        cameraShakeActive = true;
        cameraShakeStartTime = glfwGetTime();
    }

    if (enemyexplosionActive && enemyexplosionMesh) {
        // Render the enemy tank explosion for 2 seconds
        float currentTime = glfwGetTime();
        if (currentTime - enemyexplosionStartTime < 2.0f) { // Use enemyexplosionStartTime here
            glm::mat3 explosionModelMatrix = transform2D::Translate(enemyTankX, enemyTankY);
            RenderMesh2D(enemyexplosionMesh, shaders["VertexColor"], explosionModelMatrix);
        }
        terrainDeformed = true;
    }

    // Update and render projectiles
    UpdateProjectiles(deltaTimeSeconds);
}

void Tema1::OnKeyPress(int key, int mods) {
    // launch projectiles for my tank
    if (key == GLFW_KEY_SPACE && myTankLife) {
        LaunchProjectile(true);
    }

    // launch projectiles for enemy tank
    if (key == GLFW_KEY_ENTER && enemyTankLife)
    {
        LaunchProjectile(false);
    }
}

void Tema1::FrameEnd()
{
    // Additional end-of-frame rendering if needed
}

void Tema1::OnWindowResize(int width, int height)
{
    // Handle window resize event if necessary
}