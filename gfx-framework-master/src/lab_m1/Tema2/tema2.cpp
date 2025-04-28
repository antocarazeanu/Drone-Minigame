#include "lab_m1/tema2/tema2.h"

#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>  // For rand() function
#include <ctime>    // For seeding the random number generator

using namespace std;
using namespace m1;


Tema2::Tema2() :
    dronePosition(glm::vec3(0, 0, 0)),
    droneRotation(0),
    droneSpeed(10.0f),
    droneDirection(glm::vec3(1, 0, 0)),
    armExtension(nullptr),
    droneBody(nullptr),
    terrain(nullptr),
    cameraSpeed(5.0f),
    isFPSCamera(true),
    cameraDistance(5.0f),
    terrainHeight(0.0f),
    nx(20),
    nz(20),
    width(250),
    height(250),
    houseSize(5.0f),
    trunkRadius(0.2f),
    trunkHeight(4.0f),
    leavesRadius(1.0f),
    leavesHeight(3.0f),
    arrow(nullptr),
    marker(nullptr),
    houseBody(nullptr),
    houseRoof(nullptr),
    parcel(nullptr),
	currentTarget(0),
	miniModelMatrix(1),
	score(0),
    lives(3),
	dropZone(nullptr),
    hasParcel(false),
	endgamePrinted(false)
{
}


Tema2::~Tema2()
{
    delete camera;
    delete armExtension;
    delete droneBody;
    delete terrain;
	delete arrow;
	delete marker;
	delete houseBody;
	delete houseRoof;
	delete parcel;
	delete dropZone;
    for (auto& mesh : meshes) {
        delete mesh.second;
    }
}

// ======================================= Initialization =======================================
void Tema2::Init()
{
    // Initialize the camera
    camera = new implemented::Camera();

    // Set camera position to be at the drone's position
    camera->Set(dronePosition, dronePosition + droneDirection, glm::vec3(0, 1, 0));
	projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);
  

    // Create a shader program for drawing face polygon with the color of the normal
    {
        Shader* shader = new Shader("NoiseShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Init the terrain mesh
    terrain = GenerateTerrain(nx, nz, width, height);
    meshes["terrain"] = terrain;

	// drone position
    float initialHeight = GetTerrainHeight(dronePosition.x, dronePosition.z);
    dronePosition.y = initialHeight + 10.0f;  // Spawn 10 units above the terrain

    // Create drone body mesh
    droneBody = CreateDroneBody("droneBody", glm::vec3(0.5f, 0.5f, 0.5f));  // Grey color
    meshes[droneBody->GetMeshID()] = droneBody;

    // Create arm extensions
    armExtension = CreateArmExtension("armExtension", glm::vec3(0.5f, 0.5f, 0.5f));  // Grey color
    meshes[armExtension->GetMeshID()] = armExtension;

    // Create rotor mesh
    Mesh* rotor = CreateRotor("rotor", glm::vec3(0.0f, 0.0f, 0.0f));  // Black color
    meshes[rotor->GetMeshID()] = rotor;

    // Create house body (cube) mesh
    Mesh* cubeBuilding = CreateCubeMesh("cubeBuilding", glm::vec3(houseSize, houseSize, houseSize), glm::vec3(0.4f, 0.4f, 0.4f));
    meshes[cubeBuilding->GetMeshID()] = cubeBuilding;

    // Create house roof (pyramid) mesh
    Mesh* pyramidBuilding = CreatePyramidMesh("pyramidBuilding", glm::vec3(houseSize, houseSize, houseSize), houseSize, glm::vec3(0.5f, 0.12f, 0.5f));
    meshes[pyramidBuilding->GetMeshID()] = pyramidBuilding;

    // Generate 2 buildings in different locations
    GenerateRandomHouses(7);

    // Create tree trunk (cylinder) mesh
    treeTrunk = generateTrunk(trunkRadius, trunkHeight, 20, "treeTrunk");

    // Create tree leaves (cone) mesh
    treeLeaves = generateLeaves(leavesRadius, leavesHeight, 20, "treeLeaves");

    // Add the meshes to the mesh manager (or a similar system)
    meshes["treeTrunk"] = treeTrunk;
    meshes["treeLeaves"] = treeLeaves;
    // Generate random trees
    GenerateRandomTrees(25);

    // Create parcel mesh
    Mesh* parcelMesh = CreateCubeMesh("parcelMesh", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.6f, 0.4f, 0.2f));
    meshes[parcelMesh->GetMeshID()] = parcelMesh;

    // Generate parcels for houses
    GenerateParcelsForHouses(0.0f, houseSize * 2);

    // Generate circle mesh for destination of package
    Mesh* dropZone = GenerateCircleMesh("dropzone", glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 36);
    meshes["dropzone"] = dropZone;

    // Define the minimap viewport
    glm::ivec2 resolution = window->GetResolution();

    float miniMapWidth = resolution.x / 5.0f;
    float miniMapHeight = resolution.y / 5.0f;
    miniViewportArea = ViewportArea(
        resolution.x - miniViewportArea.width - 20, // 20-pixel margin from the right
        resolution.y - miniViewportArea.height - 20, // 20-pixel margin from the top
        resolution.x / 5.f,                         // Minimap width
        resolution.y / 5.f                          // Minimap height
    );


    // Marker for the house
    marker = CreateLocationMarkerMesh("marker", glm::vec3(3.5f, 3.5f, 3.5f), glm::vec3(1, 1, 1));
    meshes["marker"] = marker;

    // Arrow that point the direction
    arrow = CreateArrowMesh("arrow", 1.0f, 0.3f, glm::vec3(1, 0, 0));
	meshes["arrow"] = arrow;

    // Create the single cloud mesh
    cloudMesh = CreateCloudMesh(glm::vec3(1.0f, 1.0f, 1.0f), "cloud", 1.0f);
	meshes["cloud"] = cloudMesh;

    // Generate clouds with random positions
    for (int i = 0; i < 20; i++) {
        Cloud cloud;
        cloud.position = glm::vec3(
            RandomFloat(-70.0f, 70.0f),
            RandomFloat(40.0f, 60.0f), // (height)
            RandomFloat(-75.0f, 75.0f)
        );
        cloud.cloud = "cloud"; // Use the same mesh name for all clouds
        cloudMeshes.push_back(cloud);
    }

    // Generate life bar mesh
    lifeBarMesh = GenerateLifeBarMesh("lifeBar", glm::vec3(1.0f, 1.0f, 1.0f));
    meshes[lifeBarMesh->GetMeshID()] = lifeBarMesh;
    // Generate heart mesh
    Mesh* heartMesh = GenerateHeartMesh("heart", glm::vec3(1.0f, 0.0f, 0.0f)); // Red heart
    meshes[heartMesh->GetMeshID()] = heartMesh;


    // Game start
    currentTarget = static_cast<int>(rand() % houses.size());
    houses[currentTarget].isTarget = true;
    parcels[currentTarget].isActive = true;
	cout << "Lives remaining: " << lives << endl;
	cout << "Score: " << score << endl;

    // Generate the sun
    std::vector<VertexFormat> sunVertices;
    std::vector<unsigned int> sunIndices;
    GenerateSphere(sunVertices, sunIndices, glm::vec3(0, 0, 0), 1.0f, glm::vec3(1, 1, 0)); // Yellow color

    Mesh* sunMesh = new Mesh("sun");
    sunMesh->InitFromData(sunVertices, sunIndices);
    meshes["sun"] = sunMesh;
}

float Tema2::RandomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

// ============================================================== Terrain Generation ==============================================================
Mesh* Tema2::GenerateTerrain(int m, int n, float width, float height) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float stepX = width / (m - 1);
    float stepZ = height / (n - 1);

    // Generate vertices
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            float x = i * stepX - width / 2.0f;
            float z = j * stepZ - height / 2.0f;
            vertices.push_back(VertexFormat(glm::vec3(x, 0, z), glm::vec3(0, 0.5f, 0), glm::vec3(1, 1, 1)));
        }
    }

    // Generate indices for triangle strips
    for (int i = 0; i < m - 1; i++) {
        for (int j = 0; j < n - 1; j++) {
            indices.push_back(i * n + j);
            indices.push_back((i + 1) * n + j);
            indices.push_back(i * n + j + 1);

            indices.push_back(i * n + j + 1);
            indices.push_back((i + 1) * n + j);
            indices.push_back((i + 1) * n + j + 1);
        }
    }

    Mesh* terrain = new Mesh("terrain");
    if (terrain == nullptr) {
        std::cerr << "Failed to create terrain mesh" << std::endl;
        return nullptr;
    }
    terrain->InitFromData(vertices, indices);
    return terrain;
}

// C++ function that mimics the noise generation in the shader
float Tema2::generateNoise(glm::vec2 pos) {
    return glm::fract(glm::sin(glm::dot(pos, glm::vec2(12.9898f, 78.233f))) * 43758.5453f);
}

float Tema2::GetTerrainHeight(float x, float z) {
    float stepX = width / (nx - 1);
    float stepZ = height / (nz - 1);

    // Convert world-space (x, z) to grid indices
    int i = static_cast<int>((x + width / 2.0f) / stepX);
    int j = static_cast<int>((z + height / 2.0f) / stepZ);

    // Clamp indices to terrain bounds
    i = glm::clamp(i, 0,nx - 2);
    j = glm::clamp(j, 0, nz - 2);

    // Generate height values based on noise for each grid vertex
    float noiseValue00 = generateNoise(glm::vec2(i, j)); // Noise at (i, j)
    float noiseValue10 = generateNoise(glm::vec2(i + 1, j)); // Noise at (i+1, j)
    float noiseValue01 = generateNoise(glm::vec2(i, j + 1)); // Noise at (i, j+1)
    float noiseValue11 = generateNoise(glm::vec2(i + 1, j + 1)); // Noise at (i+1, j+1)

    // Bilinear interpolation for smooth height
    float localX = fmod((x + width / 2.0f), stepX) / stepX;
    float localZ = fmod((z + height / 2.0f), stepZ) / stepZ;

    float heightValue = (1 - localX) * (1 - localZ) * noiseValue00 +
                        localX * (1 - localZ) * noiseValue10 +
                        (1 - localX) * localZ * noiseValue01 +
                        localX * localZ * noiseValue11;

    return heightValue * 7.5f;  // Scale by amplitude for proper height
}
// ============================================================== Drone Rendering ==============================================================
Mesh* Tema2::CreateDroneBody(const std::string& name, const glm::vec3& color) {
    // create 2 grey longer boxes that intersect in the middle of each other
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Define the size of the boxes
    float length = 2.0f;
    float width = 0.2f;
    float height = 0.2f;

    // Define vertices for the first box (along the X-axis)
    vertices.push_back(VertexFormat(glm::vec3(-length / 2, -height / 2, -width / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(length / 2, -height / 2, -width / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(length / 2, height / 2, -width / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(-length / 2, height / 2, -width / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(-length / 2, -height / 2, width / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(length / 2, -height / 2, width / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(length / 2, height / 2, width / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(-length / 2, height / 2, width / 2), color));

    // Define vertices for the second box (along the Z-axis)
    vertices.push_back(VertexFormat(glm::vec3(-width / 2, -height / 2, -length / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(width / 2, -height / 2, -length / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(width / 2, height / 2, -length / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(-width / 2, height / 2, -length / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(-width / 2, -height / 2, length / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(width / 2, -height / 2, length / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(width / 2, height / 2, length / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(-width / 2, height / 2, length / 2), color));

    // Define indices for the first box
    std::vector<unsigned int> boxIndices = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 1, 5, 5, 4, 0,
        2, 3, 7, 7, 6, 2,
        0, 3, 7, 7, 4, 0,
        1, 2, 6, 6, 5, 1
    };

    // Add indices for the first box
    indices.insert(indices.end(), boxIndices.begin(), boxIndices.end());

    // Offset for the second box indices
    unsigned int offset = 8;

    // Add indices for the second box
    for (unsigned int i : boxIndices) {
        indices.push_back(i + offset);
    }

    // Create the mesh
    Mesh* droneBody = new Mesh(name);
    droneBody->InitFromData(vertices, indices);
    return droneBody;
}

Mesh* Tema2::CreateArmExtension(const std::string& name, const glm::vec3& color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Define the size of the arm extension box
    float armLength = 0.2f;
    float armWidth = 0.2f;
    float armHeight = 0.4f;

    // Define vertices for the arm extension box (along the Y-axis)
    vertices.push_back(VertexFormat(glm::vec3(-armWidth / 2, -armHeight / 2, -armLength / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(armWidth / 2, -armHeight / 2, -armLength / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(armWidth / 2, armHeight / 2, -armLength / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(-armWidth / 2, armHeight / 2, -armLength / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(-armWidth / 2, -armHeight / 2, armLength / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(armWidth / 2, -armHeight / 2, armLength / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(armWidth / 2, armHeight / 2, armLength / 2), color));
    vertices.push_back(VertexFormat(glm::vec3(-armWidth / 2, armHeight / 2, armLength / 2), color));

    // Define indices for the arm extension box
    indices = {
        0, 1, 2, 2, 3, 0, // Front face
        4, 5, 6, 6, 7, 4, // Back face
        0, 1, 5, 5, 4, 0, // Bottom face
        2, 3, 7, 7, 6, 2, // Top face
        0, 3, 7, 7, 4, 0, // Left face
        1, 2, 6, 6, 5, 1  // Right face
    };

    Mesh* armExtension = new Mesh(name);
    armExtension->InitFromData(vertices, indices);
    return armExtension;
}

Mesh* Tema2::CreateRotor(const std::string& name, const glm::vec3& color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float length = 0.2f;  // Length of each arm
    float width = 0.05f;  // Width of each arm

    // Define vertices for a cross (two rectangles forming a "+" shape)
    vertices.push_back(VertexFormat(glm::vec3(-length, 0, -width), color));
    vertices.push_back(VertexFormat(glm::vec3(-length, 0, width), color));
    vertices.push_back(VertexFormat(glm::vec3(length, 0, width), color));
    vertices.push_back(VertexFormat(glm::vec3(length, 0, -width), color));

    vertices.push_back(VertexFormat(glm::vec3(-width, 0, -length), color));
    vertices.push_back(VertexFormat(glm::vec3(-width, 0, length), color));
    vertices.push_back(VertexFormat(glm::vec3(width, 0, length), color));
    vertices.push_back(VertexFormat(glm::vec3(width, 0, -length), color));

    // Define indices for two rectangles
    indices = {
        0, 1, 2, 0, 2, 3,  // Horizontal rectangle
        4, 5, 6, 4, 6, 7   // Vertical rectangle
    };

    // Create the cross mesh
    Mesh* cross = new Mesh(name);
    cross->InitFromData(vertices, indices);
    return cross;
}

// ============================================================== Tree Obstacles ==============================================================
Mesh* Tema2::generateTrunk(float radius, float height, unsigned int segments, const std::string& name) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Create the vertices for the base and top circles
    for (unsigned int i = 0; i < segments; i++) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Bottom circle vertices
        vertices.push_back(VertexFormat(glm::vec3(x, 0.0f, z), glm::vec3(0.5f, 0.25f, 0.0f)));  // Brown trunk

        // Top circle vertices
        vertices.push_back(VertexFormat(glm::vec3(x, height, z), glm::vec3(0.5f, 0.25f, 0.0f)));  // Brown trunk
    }

    // Create indices for the trunk sides
    for (unsigned int i = 0; i < segments; i++) {
        unsigned int next = (i + 1) % segments;

        // Side triangles (two per segment)
        indices.push_back(i);              // Bottom circle current
        indices.push_back(next);           // Bottom circle next
        indices.push_back(segments + i);   // Top circle current

        indices.push_back(next);           // Bottom circle next
        indices.push_back(segments + next);// Top circle next
        indices.push_back(segments + i);   // Top circle current
    }

    // Create the trunk mesh
    Mesh* trunk = new Mesh(name);
    trunk->InitFromData(vertices, indices);
    return trunk;
}


Mesh* Tema2::generateLeaves(float radius, float height, unsigned int segments, const std::string& name) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Add the top point of the cone
    vertices.push_back(VertexFormat(glm::vec3(0.0f, height, 0.0f), glm::vec3(0.6f, 0.6f, 0.0f)));  // Green leaves

    // Create the base circle
    for (unsigned int i = 0; i < segments; i++) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        vertices.push_back(VertexFormat(glm::vec3(x, 0.0f, z), glm::vec3(0.6f, 0.6f, 0.0f)));  // Green leaves
    }

    // Create indices for the sides
    for (unsigned int i = 0; i < segments; i++) {
        unsigned int next = (i + 1) % segments;

        // Connect top point with each base vertex
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(next + 1);
    }

    // Create the leaves mesh
    Mesh* leaves = new Mesh(name);
    leaves->InitFromData(vertices, indices);
    return leaves;
}

void Tema2::GenerateRandomTrees(unsigned int numberOfTrees) {
    srand(static_cast<unsigned int>(time(0)));  // Seed the random number generator with the current time

    // Clear any existing trees
    trees.clear();

    // Define the minimum distance from the drone's spawn position
    const float minDistanceFromDrone = 20.0f;

    for (unsigned int i = 0; i < numberOfTrees; ++i) {
        bool validPosition = false;
        glm::vec3 treePosition;

        while (!validPosition) {
            // Randomly generate X and Z positions within the map bounds (200 x 200)
            float randomX = static_cast<float>(rand() % 200) - 100.0f;  // Random X in range [-100, 100]
            float randomZ = static_cast<float>(rand() % 200) - 100.0f;  // Random Z in range [-100, 100]

            // Calculate the terrain height at this position
            float terrainHeight1 = GetTerrainHeight(randomX, randomZ);

            // Store the tree's position (X, Y, Z) where Y is the calculated terrain height
            treePosition = glm::vec3(randomX, terrainHeight1 - 5.0f, randomZ);

            // Check if the position is within the perimeter of any house or too close to the drone's spawn position
            validPosition = true;
            for (const auto& house : houses) {
                float houseMinX = house.position.x - houseSize;
                float houseMaxX = house.position.x + houseSize;
                float houseMinZ = house.position.z - houseSize;
                float houseMaxZ = house.position.z + houseSize;

                if (randomX >= houseMinX && randomX <= houseMaxX && randomZ >= houseMinZ && randomZ <= houseMaxZ) {
                    validPosition = false;
                    break;
                }
            }

            // Check if the tree is too close to the drone's spawn position
            if (glm::distance(glm::vec2(randomX, randomZ), glm::vec2(dronePosition.x, dronePosition.z)) < minDistanceFromDrone) {
                validPosition = false;
            }
        }

        // Create a tree by storing its position and a reference to the trunk and leaves meshes
        trees.push_back({ treePosition, "treeTrunk", "treeLeaves" });
    }
}

bool Tema2::IsCollidingWithTrees(const glm::vec3& position) {
    float droneRadius = 1.75f;  // Approximate radius of the drone
    float droneHeight = 2.0f;   // Approximate height of the drone

    for (const auto& tree : trees) {
        // Trunk parameters
        glm::vec3 trunkBase = tree.position;
        glm::vec3 trunkTop = trunkBase + glm::vec3(0, trunkHeight * 4.0f, 0);

        // --- Trunk Collision ---
        float distanceXZ = glm::length(glm::vec2(position.x - trunkBase.x, position.z - trunkBase.z));
        bool isInsideTrunkCylinder = distanceXZ <= trunkRadius + droneRadius &&
            position.y + droneHeight / 2 >= trunkBase.y &&
            position.y - droneHeight / 2 <= trunkTop.y;

        if (isInsideTrunkCylinder) {
            return true;  // Collision with the trunk
        }

        // --- Leaves Collision ---
        float leavesRadius = 8.75f;  // Scaled radius of the first cone
        float leavesHeight = 5.0f;  // Scaled height of the first cone

        // First cone (top of the trunk)
        if (distanceXZ <= leavesRadius) {
            float maxHeightAtXZ = trunkTop.y + leavesHeight * (1 - distanceXZ / leavesRadius);

            bool isInsideFirstCone = position.y + droneHeight / 2 >= trunkTop.y &&
                position.y - droneHeight / 2 <= maxHeightAtXZ;

            if (isInsideFirstCone) {
                return true;  // Collision with the first cone of leaves
            }
        }

        // Second cone (middle of the first cone)
        glm::vec3 secondConeBase = trunkTop + glm::vec3(0, leavesHeight / 2.0f, 0);
        if (distanceXZ <= leavesRadius) {
            float maxHeightAtXZ = secondConeBase.y + leavesHeight * (1 - distanceXZ / leavesRadius);

            bool isInsideSecondCone = position.y + droneHeight / 2 >= secondConeBase.y &&
                position.y - droneHeight / 2 <= maxHeightAtXZ;

            if (isInsideSecondCone) {
                return true;  // Collision with the second cone of leaves
            }
        }
    }

    return false;  // No collision
}


// ============================================================== House Obstacles ==============================================================
Mesh* Tema2::CreateCubeMesh(const std::string& name, glm::vec3 size, glm::vec3 color) {
    std::vector<VertexFormat> vertices = {
        // Front face
        VertexFormat(glm::vec3(-size.x, -size.y,  size.z), color),
        VertexFormat(glm::vec3(size.x, -size.y,  size.z), color),
        VertexFormat(glm::vec3(size.x,  size.y,  size.z), color),
        VertexFormat(glm::vec3(-size.x,  size.y,  size.z), color),

        // Back face
        VertexFormat(glm::vec3(-size.x, -size.y, -size.z), color),
        VertexFormat(glm::vec3(size.x, -size.y, -size.z), color),
        VertexFormat(glm::vec3(size.x,  size.y, -size.z), color),
        VertexFormat(glm::vec3(-size.x,  size.y, -size.z), color),

        // Left face
        VertexFormat(glm::vec3(-size.x, -size.y, -size.z), color),
        VertexFormat(glm::vec3(-size.x, -size.y,  size.z), color),
        VertexFormat(glm::vec3(-size.x,  size.y,  size.z), color),
        VertexFormat(glm::vec3(-size.x,  size.y, -size.z), color),

        // Right face
        VertexFormat(glm::vec3(size.x, -size.y, -size.z), color),
        VertexFormat(glm::vec3(size.x, -size.y,  size.z), color),
        VertexFormat(glm::vec3(size.x,  size.y,  size.z), color),
        VertexFormat(glm::vec3(size.x,  size.y, -size.z), color),

        // Top face
        VertexFormat(glm::vec3(-size.x,  size.y, -size.z), color),
        VertexFormat(glm::vec3(size.x,  size.y, -size.z), color),
        VertexFormat(glm::vec3(size.x,  size.y,  size.z), color),
        VertexFormat(glm::vec3(-size.x,  size.y,  size.z), color),

        // Bottom face
        VertexFormat(glm::vec3(-size.x, -size.y, -size.z), color),
        VertexFormat(glm::vec3(size.x, -size.y, -size.z), color),
        VertexFormat(glm::vec3(size.x, -size.y,  size.z), color),
        VertexFormat(glm::vec3(-size.x, -size.y,  size.z), color),
    };

    std::vector<unsigned int> indices = {
        // Front face
        0, 1, 2, 2, 3, 0,
        // Back face
        4, 5, 6, 6, 7, 4,
        // Left face
        8, 9, 10, 10, 11, 8,
        // Right face
        12, 13, 14, 14, 15, 12,
        // Top face
        16, 17, 18, 18, 19, 16,
        // Bottom face
        20, 21, 22, 22, 23, 20,
    };

    Mesh* cube = new Mesh(name);
    cube->InitFromData(vertices, indices);
    return cube;
}

Mesh* Tema2::CreatePyramidMesh(const std::string& name, glm::vec3 baseSize, float height, glm::vec3 color) {
    std::vector<VertexFormat> vertices = {
        // Base vertices
        VertexFormat(glm::vec3(-baseSize.x, 0, -baseSize.z), color),
        VertexFormat(glm::vec3(baseSize.x, 0, -baseSize.z), color),
        VertexFormat(glm::vec3(baseSize.x, 0,  baseSize.z), color),
        VertexFormat(glm::vec3(-baseSize.x, 0,  baseSize.z), color),

        // Apex vertex
        VertexFormat(glm::vec3(0, height, 0), color),
    };

    std::vector<unsigned int> indices = {
        // Base face
        0, 1, 2, 2, 3, 0,
        // Side faces
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        3, 0, 4,
    };

    Mesh* pyramid = new Mesh(name);
    pyramid->InitFromData(vertices, indices);
    return pyramid;
}

void Tema2::GenerateRandomHouses(unsigned int numberofHouses) {
    srand(static_cast<unsigned int>(time(0)));  // Seed the random number generator with the current time

    // Clear any existing houses
    houses.clear();

    const float minSpacing = 20.0f;  // Minimum distance between houses

    for (unsigned int i = 0; i < numberofHouses; ++i) {
        glm::vec3 housePosition;
        bool isTooClose;

        do {
            // Randomly generate X and Z positions within the map bounds (200 x 200)
            float randomX = static_cast<float>(rand() % 200) - 100.0f;  // Random X in range [-100, 100]
            float randomZ = static_cast<float>(rand() % 200) - 100.0f;  // Random Z in range [-100, 100]

            // Calculate the terrain height at this position
            float terrainHeight1 = GetTerrainHeight(randomX, randomZ);

            // Store the house's position (X, Y, Z) where Y is the calculated terrain height
            housePosition = glm::vec3(randomX, terrainHeight1, randomZ);

            // Check distance to ensure houses are spaced apart
            isTooClose = false;
            for (const auto& house : houses) {
                float distance = glm::distance(housePosition, house.position);
                if (distance < minSpacing) {
                    isTooClose = true;  // Too close to an existing house
                    break;
                }
            }
        } while (isTooClose);

        // Create a house by storing its position and a reference to the roof and house meshes
        houses.push_back({ housePosition, "pyramidBuilding", "cubeBuilding" });
    }
}

bool Tema2::IsCollidingWithBuildings(const glm::vec3& position) {
    float droneRadius = 1.0f;
    float droneHeight = 1.0f;

    for (const auto& house : houses) {
        // Base collision (cube)
        float houseMinX = house.position.x - houseSize - droneRadius;
        float houseMaxX = house.position.x + houseSize + droneRadius;
        float houseMinZ = house.position.z - houseSize - droneRadius;
        float houseMaxZ = house.position.z + houseSize + droneRadius;

        // House base and initial height bounds
        float houseBaseHeight = house.position.y;
        float houseRoofBaseHeight = house.position.y + houseSize;

        // Roof parameters
        float roofHeight = houseSize; // Pyramid height
        float roofBaseSize = houseSize; // Half-width of roof's base

        // Check if the drone is within the base X-Z bounds
        if (position.x >= houseMinX && position.x <= houseMaxX &&
            position.z >= houseMinZ && position.z <= houseMaxZ) {

            // Base collision: Ensure the drone isn't inside the house's cube base
            if (position.y >= houseBaseHeight && position.y <= houseRoofBaseHeight) {
                return true; // Drone collides with the cube part
            }

            // Roof collision: Check if the drone is within the pyramid's volume
            float relativeX = fabs(position.x - house.position.x);
            float relativeZ = fabs(position.z - house.position.z);

            if (relativeX <= roofBaseSize && relativeZ <= roofBaseSize) {
                // Calculate the maximum allowable height at this X-Z position
                float maxRoofHeight = houseRoofBaseHeight + roofHeight *
                    (1 - (relativeX + relativeZ) / (2 * roofBaseSize));

                // If the drone is below the roof's calculated height, it collides
                if (position.y <= maxRoofHeight) {
                    return true; // Drone collides with the roof
                }
            }
        }
    }

    return false; // No collision
}

// ============================================================== Parcel Delivery ==============================================================
Mesh* Tema2::CreateParcelMesh(const std::string& name, const glm::vec3& dimensions, const glm::vec3& color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Define the 8 vertices of the box
    glm::vec3 halfDim = dimensions * 0.5f;
    vertices.push_back(VertexFormat(glm::vec3(-halfDim.x, -halfDim.y, halfDim.z), color));
    vertices.push_back(VertexFormat(glm::vec3(halfDim.x, -halfDim.y, halfDim.z), color));
    vertices.push_back(VertexFormat(glm::vec3(-halfDim.x, halfDim.y, halfDim.z), color));
    vertices.push_back(VertexFormat(glm::vec3(halfDim.x, halfDim.y, halfDim.z), color));
    vertices.push_back(VertexFormat(glm::vec3(-halfDim.x, -halfDim.y, -halfDim.z), color));
    vertices.push_back(VertexFormat(glm::vec3(halfDim.x, -halfDim.y, -halfDim.z), color));
    vertices.push_back(VertexFormat(glm::vec3(-halfDim.x, halfDim.y, -halfDim.z), color));
    vertices.push_back(VertexFormat(glm::vec3(halfDim.x, halfDim.y, -halfDim.z), color));

    // Define the 12 triangles (two per face)
    indices = {
        0, 1, 2, 1, 3, 2,  // Front face
        4, 5, 6, 5, 7, 6,  // Back face
        0, 1, 4, 1, 5, 4,  // Bottom face
        2, 3, 6, 3, 7, 6,  // Top face
        0, 2, 4, 2, 6, 4,  // Left face
        1, 3, 5, 3, 7, 5   // Right face
    };

    Mesh* box = new Mesh(name);
    box->InitFromData(vertices, indices);
    return box;
}

void Tema2::GenerateParcelsForHouses(float offsetX, float offsetZ) {
    parcels.clear();

    int i = 0;
    for (const auto& house : houses) {
        // Randomize parcel dimensions
        float randomSize = 1.0f;

        glm::vec3 parcelDimensions = glm::vec3(randomSize, randomSize, randomSize);

        // Calculate the parcel's position relative to the house
        glm::vec3 parcelPosition = house.position + glm::vec3(offsetX, 0, offsetZ);

        // Adjust the parcel's y-coordinate based on the terrain height at the parcel's position
        float terrainHeightAtParcel = GetTerrainHeight(parcelPosition.x, parcelPosition.z);
        parcelPosition.y = terrainHeightAtParcel;

        parcels.push_back({ i, parcelPosition, "parcelMesh", parcelDimensions, false });
        i++;
    }
}

bool Tema2::IsCollidingWithParcel(const glm::vec3& dronePosition, const glm::vec3& parcelPosition, const glm::vec3& parcelDimensions) {
    glm::vec3 halfDim = parcelDimensions * 0.5f;

    // Check if the drone is horizontally above the parcel
    bool isHorizontallyAbove = (dronePosition.x >= parcelPosition.x - halfDim.x && dronePosition.x <= parcelPosition.x + halfDim.x) &&
        (dronePosition.z >= parcelPosition.z - halfDim.z && dronePosition.z <= parcelPosition.z + halfDim.z);

    // Check if the drone is vertically above the parcel (drone is at least at the parcel's top height)
    bool isVerticallyAbove = dronePosition.y >= (parcelPosition.y + halfDim.y);

    if (isHorizontallyAbove && isVerticallyAbove) {
        return true; // Drone is above the parcel
    }

    return false;
}

Mesh* Tema2::GenerateCircleMesh(const std::string& name, glm::vec3 color, float radius, int segments) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Add the center of the circle
    vertices.emplace_back(glm::vec3(0, 0, 0), color);

    // Generate points on the circumference
    float angleStep = 2 * M_PI / segments;
    for (int i = 0; i <= segments; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.emplace_back(glm::vec3(x, 0, z), color);

        // Add the triangle for each segment
        if (i > 0) {
            indices.push_back(0);           // Center
            indices.push_back(i);           // Current point
            indices.push_back(i - 1);       // Previous point
        }
    }

    // Add the last triangle to close the circle
    indices.push_back(0);                   // Center
    indices.push_back(1);                   // First point on the circumference
    indices.push_back(segments);            // Last point on the circumference

    // Create the mesh
    Mesh* circleMesh = new Mesh(name);
    circleMesh->InitFromData(vertices, indices);
    return circleMesh;
}

bool Tema2::IsAboveDropzone(const glm::vec3& dronePosition, const glm::vec3& dropzonePosition, const glm::vec3& dropzoneDimensions) {
    glm::vec3 halfDim = dropzoneDimensions * 0.5f;

    // Check if the drone is horizontally above the dropzone
    bool isHorizontallyAbove = (dronePosition.x >= dropzonePosition.x - halfDim.x && dronePosition.x <= dropzonePosition.x + halfDim.x) &&
        (dronePosition.z >= dropzonePosition.z - halfDim.z && dronePosition.z <= dropzonePosition.z + halfDim.z);

    // Check if the drone is vertically within the dropzone tolerance
    bool isVerticallyAbove = fabs(dronePosition.y - dropzonePosition.y) <= 5.0f; // Allow some vertical tolerance

    return isHorizontallyAbove && isVerticallyAbove;
}


// =========================================================== Marker and Arrow ==============================================================
Mesh* Tema2::CreateLocationMarkerMesh(const std::string& name, const glm::vec3& dimensions, const glm::vec3& color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Dimensions
    glm::vec3 halfDim = dimensions * 0.5f;

    // Make the main body longer by extending the height of the rectangular section
    float bodyHeightFactor = 1.5f; // Adjust this factor to make the body longer
    float bodyHeight = dimensions.y * bodyHeightFactor;

    // Define the rectangle (main body of the marker)
    vertices.push_back(VertexFormat(glm::vec3(-halfDim.x, 0.0f, halfDim.z), color));  // 0: bottom-front-left
    vertices.push_back(VertexFormat(glm::vec3(halfDim.x, 0.0f, halfDim.z), color));   // 1: bottom-front-right
    vertices.push_back(VertexFormat(glm::vec3(-halfDim.x, bodyHeight, halfDim.z), color)); // 2: top-front-left
    vertices.push_back(VertexFormat(glm::vec3(halfDim.x, bodyHeight, halfDim.z), color));  // 3: top-front-right
    vertices.push_back(VertexFormat(glm::vec3(-halfDim.x, 0.0f, -halfDim.z), color)); // 4: bottom-back-left
    vertices.push_back(VertexFormat(glm::vec3(halfDim.x, 0.0f, -halfDim.z), color));  // 5: bottom-back-right
    vertices.push_back(VertexFormat(glm::vec3(-halfDim.x, bodyHeight, -halfDim.z), color)); // 6: top-back-left
    vertices.push_back(VertexFormat(glm::vec3(halfDim.x, bodyHeight, -halfDim.z), color));  // 7: top-back-right

    // Define the downward-pointing triangle's tip
    vertices.push_back(VertexFormat(glm::vec3(0.0f, -dimensions.y, 0.0f), color)); // 8: bottom triangle tip

    // Indices for the rectangle
    indices = {
        0, 1, 2, 1, 3, 2,  // Front face
        4, 5, 6, 5, 7, 6,  // Back face
        0, 1, 4, 1, 5, 4,  // Bottom face
        2, 3, 6, 3, 7, 6,  // Top face
        0, 2, 4, 2, 6, 4,  // Left face
        1, 3, 5, 3, 7, 5   // Right face
    };

    // Indices for the triangle part
    indices.insert(indices.end(), {
        0, 4, 8,  // Bottom-front-left to tip
        1, 5, 8,  // Bottom-front-right to tip
        4, 5, 8   // Bottom-back to tip
        });

    // Create and return the mesh
    Mesh* markerMesh = new Mesh(name);
    markerMesh->InitFromData(vertices, indices);
    return markerMesh;
}

Mesh* Tema2::CreateArrowMesh(const std::string& name, float length, float radius, const glm::vec3& color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Arrow shaft (cylinder)
    unsigned int shaftSlices = 20; // Number of slices for a smoother cylinder
    float shaftLength = length * 0.8f; // Shaft takes up 80% of the arrow length
    for (unsigned int i = 0; i < shaftSlices; ++i) {
        float angle = 2 * M_PI * i / shaftSlices;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Bottom circle (start of shaft)
        vertices.push_back(VertexFormat(glm::vec3(x, -shaftLength / 2.0f, z), color));

        // Top circle (end of shaft)
        vertices.push_back(VertexFormat(glm::vec3(x, shaftLength / 2.0f, z), color));
    }

    // Indices for the shaft (cylinder)
    for (unsigned int i = 0; i < shaftSlices; ++i) {
        unsigned int next = (i + 1) % shaftSlices;

        // Side triangles
        indices.push_back(i * 2);
        indices.push_back(i * 2 + 1);
        indices.push_back(next * 2);

        indices.push_back(i * 2 + 1);
        indices.push_back(next * 2 + 1);
        indices.push_back(next * 2);
    }

    // Arrowhead (cone)
    float coneLength = length * 0.2f; // Cone takes up 20% of the arrow length
    float coneBaseHeight = shaftLength / 2.0f; // Base of the cone starts where the shaft ends
    float coneBaseRadius = radius * 1.5f; // Smaller cone base relative to the shaft
    unsigned int coneStartIndex = vertices.size(); // Starting index for cone vertices

    // Add cone base vertices
    for (unsigned int i = 0; i < shaftSlices; ++i) {
        float angle = 2 * M_PI * i / shaftSlices;
        float x = coneBaseRadius * cos(angle);
        float z = coneBaseRadius * sin(angle);
        float y = coneBaseHeight; // Height of the base of the cone

        vertices.push_back(VertexFormat(glm::vec3(x, y, z), color));
    }

    // Tip of the cone
    glm::vec3 tipPosition(0, coneBaseHeight + coneLength + 0.5f, 0); // At the top of the cone
    vertices.push_back(VertexFormat(tipPosition, color));
    unsigned int coneTipIndex = vertices.size() - 1;

    // Indices for the cone
    for (unsigned int i = 0; i < shaftSlices; ++i) {
        unsigned int next = (i + 1) % shaftSlices;

        // Connect the base of the cone to the tip
        indices.push_back(coneStartIndex + i);
        indices.push_back(coneTipIndex);
        indices.push_back(coneStartIndex + next);
    }

    // Create the mesh
    Mesh* arrow = new Mesh(name);
    arrow->InitFromData(vertices, indices);
    return arrow;
}

// ============================================================= Clouds ============================================================
Mesh* Tema2::CreateCloudMesh(const glm::vec3& uniformColor, const std::string& name, float baseSize) {
    Mesh* cloud = new Mesh(name);

    // Define sphere positions for the cloud
    glm::vec3 positions[] = {
        glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.5f, 0.0f),
        glm::vec3(0.0f, -0.5f, 0.0f)
    };

    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    // Generate spheres with uniform white color
    for (const auto& pos : positions) {
        float randomScale = baseSize + RandomFloat(-0.2f, 0.2f); // Slightly vary size
        GenerateSphere(vertices, indices, pos, randomScale, glm::vec3(1.0f, 1.0f, 1.0f)); // White color
    }

    // Initialize the mesh with the generated data
    cloud->InitFromData(vertices, indices);
    meshes[name] = cloud;
    return cloud;
}

void Tema2::GenerateSphere(std::vector<VertexFormat>& vertices, std::vector<unsigned int>& indices,
    const glm::vec3& center, float radius, const glm::vec3& uniformColor) {
    const int stacks = 10; // Number of horizontal segments
    const int slices = 10; // Number of vertical segments
    size_t startIndex = vertices.size();

    for (int i = 0; i <= stacks; ++i) {
        float phi = glm::pi<float>() * float(i) / stacks; // Angle from top to bottom
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * glm::pi<float>() * float(j) / slices; // Angle around the circle

            // Calculate the position of the vertex
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            glm::vec3 position = center + glm::vec3(x, y, z);
            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z)); // Normal points outwards

            // Use the uniform color provided
			vertices.push_back(VertexFormat(position, uniformColor, normal));
        }
    }

    // Generate indices for the sphere
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = startIndex + i * (slices + 1) + j;
            int second = first + slices + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void Tema2::UpdateClouds(float deltaTimeSeconds) {
    for (auto& cloud : cloudMeshes) {
        // Move clouds along the x-axis
        cloud.position.x += RandomFloat(0.5f, 2.0f) * deltaTimeSeconds * 5.0f;

        // Reset position if the cloud goes off the map
        if (cloud.position.x > 50.0f) {
            cloud.position.x = -50.0f;
            cloud.position.z = RandomFloat(-75.0f, 75.0f); // Randomize z
            cloud.position.y = RandomFloat(40.0f, 60.0f);  // Randomize height
        }
    }
}

void Tema2::RenderClouds() {
    for (const auto& cloud : cloudMeshes) {
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), cloud.position);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(10.0f, 5.0f, 10.0f));
        RenderMesh(meshes[cloud.cloud], shaders["VertexColor"], modelMatrix);
    }
}

// ============================================================== Life ==============================================================
Mesh* Tema2::GenerateLifeBarMesh(const std::string& name, glm::vec3 color) {
    std::vector<VertexFormat> vertices = {
        VertexFormat(glm::vec3(-0.5f, 0.0f, 0.0f), color),
        VertexFormat(glm::vec3(0.5f, 0.0f, 0.0f), color),
        VertexFormat(glm::vec3(0.5f, 0.1f, 0.0f), color),
        VertexFormat(glm::vec3(-0.5f, 0.1f, 0.0f), color),
    };

    std::vector<unsigned int> indices = {
        0, 1, 2,
        2, 3, 0
    };

    Mesh* LifeBarMesh = new Mesh(name);
    LifeBarMesh->InitFromData(vertices, indices);
    return LifeBarMesh;
}

Mesh* Tema2::GenerateHeartMesh(const std::string& name, glm::vec3 color) {
    std::vector<VertexFormat> vertices = {
        VertexFormat(glm::vec3(-0.3f,  0.4f, 0.0f), color), // V0 (top left)
        VertexFormat(glm::vec3(-0.5f,  0.0f, 0.0f), color), // V1 (mid-left)
        VertexFormat(glm::vec3(0.0f, -0.6f, 0.0f), color), // V2 (bottom tip)
        VertexFormat(glm::vec3(0.5f,  0.0f, 0.0f), color), // V3 (mid-right)
        VertexFormat(glm::vec3(0.3f,  0.4f, 0.0f), color), // V4 (top right)
        VertexFormat(glm::vec3(0.0f,  0.2f, 0.0f), color)  // V5 (center point for rounded top)
    };

    std::vector<unsigned int> indices = {
        0, 1, 5, // Left rounded top triangle
        5, 3, 4, // Right rounded top triangle
        1, 3, 2,  // Bottom triangle forming the tip
        1, 5, 3
    };

    Mesh* HeartMesh = new Mesh(name);
    HeartMesh->InitFromData(vertices, indices);
    return HeartMesh;
}

// ============================================================== Workflow ==============================================================
void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);

    // Pass noise parameters to the shader
    shaders["NoiseShader"]->Use();
    glUniform1f(glGetUniformLocation(shaders["NoiseShader"]->GetProgramID(), "frequency"), 0.05f);
    glUniform1f(glGetUniformLocation(shaders["NoiseShader"]->GetProgramID(), "amplitude"), 2.5f);
}

void Tema2::Update(float deltaTimeSeconds) {
    // Render the main scene
	RenderScene(deltaTimeSeconds);

    // Now, render the minimap
    glm::ivec2 resolution = window->GetResolution();
    miniViewportArea = ViewportArea(
        resolution.x - miniViewportArea.width - 20, // 20-pixel margin from the right
        resolution.y - miniViewportArea.height - 20, // 20-pixel margin from the top
        resolution.x / 5.f,                         // Minimap width
        resolution.y / 5.f                          // Minimap height
    );

    // Set the orthographic projection for the minimap
    projectionMatrixOrtho = glm::ortho(
        -width / 2.0f, width / 2.0f,  // Left and Right
        -height / 2.0f, height / 2.0f, // Bottom and Top
        0.1f, 100.0f  // Near and Far planes
    );

    // Position the minimap camera directly above the center of the map
    glm::vec3 minimapCameraPosition = glm::vec3(0.0f, 40.0f, 0.0f);
    glm::vec3 minimapTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 minimapUp = glm::vec3(0.0f, 0.0f, -1.0f);

    // Create the minimap view matrix (top-down view)
    minimapViewMatrix = glm::lookAt(minimapCameraPosition, minimapTarget, minimapUp);
    // Set the minimap viewport
    glViewport(miniViewportArea.x, miniViewportArea.y, miniViewportArea.width, miniViewportArea.height);
    
	// Render the scene with the custom view from the minimap
    RenderSceneWithCustomView(deltaTimeSeconds, minimapViewMatrix, projectionMatrixOrtho);
}

void Tema2::RenderSceneWithCustomView(float deltaTimeSeconds, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    // render random trees
    for (const auto& tree : trees) {
        // Render the trunk
        glm::mat4 trunkModelMatrix = glm::mat4(1.0f);
        trunkModelMatrix = glm::translate(trunkModelMatrix, tree.position);
        trunkModelMatrix = glm::scale(trunkModelMatrix, glm::vec3(5.0f, 4.0f, 5.0f));
        RenderMeshOrtho(meshes[tree.trunkMesh], shaders["VertexColor"], trunkModelMatrix);

        // Render multiple layers of leaves
        float trunkHeight = 15.0f;    // Height of the trunk
        int numLeafLayers = 3;       // Number of leaf layers
        float layerHeight = 1.5f;    // Height offset for each layer
        float trunkThickness = 5.0f; // Thickness of the trunk (same scaling for leaves)

        // First cone starting from the top of the cylinder
        glm::mat4 firstConeModelMatrix = glm::mat4(1.0f);
        firstConeModelMatrix = glm::translate(firstConeModelMatrix, tree.position);
        firstConeModelMatrix = glm::translate(firstConeModelMatrix, glm::vec3(0.0f, trunkHeight, 0.0f));
        firstConeModelMatrix = glm::scale(firstConeModelMatrix, glm::vec3(trunkThickness * 1.75f, trunkThickness, trunkThickness));

        RenderMeshOrtho(meshes[tree.leavesMesh], shaders["VertexColor"], firstConeModelMatrix);

        // Second cone starting from the middle of the first one
        glm::mat4 secondConeModelMatrix = glm::mat4(1.0f);
        secondConeModelMatrix = glm::translate(secondConeModelMatrix, tree.position);
        secondConeModelMatrix = glm::translate(secondConeModelMatrix, glm::vec3(0.0f, trunkHeight + trunkThickness, 0.0f));
        secondConeModelMatrix = glm::scale(secondConeModelMatrix, glm::vec3(trunkThickness * 1.75f, trunkThickness, trunkThickness));

        RenderMeshOrtho(meshes[tree.leavesMesh], shaders["VertexColor"], secondConeModelMatrix);
    }

    // Update drone position based on input
    OnInputUpdate(deltaTimeSeconds, 0);

    // render random houses
    for (const auto& house : houses) {
        // Render house body
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), house.position);
        RenderMeshOrtho(meshes[house.houseMesh], shaders["VertexColor"], modelMatrix);

        // Render house roof
        modelMatrix = glm::translate(glm::mat4(1.0f), house.position + glm::vec3(0, 5.0f, 0));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
        RenderMeshOrtho(meshes[house.roofMesh], shaders["VertexColor"], modelMatrix);

        if (house.isTarget && lives) {
            // Render the checkpoint marker above the target house
            modelMatrix = glm::translate(glm::mat4(1.0f), house.position + glm::vec3(0, 15.0f, 0));
            RenderMeshOrtho(meshes["marker"], shaders["VertexColor"], modelMatrix);
        }
    }

    if (lives > 0) {
        // Render drone body
        static float rotorAngle = 0.0f;
        rotorAngle += deltaTimeSeconds * 5.0f;

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, dronePosition);
        modelMatrix = glm::rotate(modelMatrix, droneRotation + 0.8f, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(10, 10, 10));
        RenderMeshOrtho(meshes["droneBody"], shaders["VertexColor"], modelMatrix);

        // Render arm extensions and rotors
        glm::vec3 rotorPositions[] = {
            glm::vec3(1.0f, 0.2f, 0.0f),  // Right
            glm::vec3(-1.0f, 0.2f, 0.0f), // Left
            glm::vec3(0.0f, 0.2f, 1.0f),  // Front
            glm::vec3(0.0f, 0.2f, -1.0f)  // Back
        };

        for (const auto& pos : rotorPositions) {
            glm::mat4 armMatrix = glm::translate(modelMatrix, pos);
            RenderMeshOrtho(meshes["armExtension"], shaders["VertexColor"], armMatrix);

            glm::mat4 rotorMatrix = glm::translate(modelMatrix, pos + glm::vec3(0, 0.21f, 0));
            rotorMatrix = glm::rotate(rotorMatrix, rotorAngle, glm::vec3(0, 1, 0));
            RenderMeshOrtho(meshes["rotor"], shaders["VertexColor"], rotorMatrix);
        }
    }
    else {
        // mark the death location
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), dronePosition);
        RenderMeshOrtho(meshes["marker"], shaders["VertexColor"], modelMatrix);
    }

    // Render terrain
    if (terrain) {
        RenderMeshOrtho(meshes["terrain"], shaders["VertexColor"], glm::mat4(1));
    }
}

void Tema2::RenderScene(float deltaTimeSeconds) {
    UpdateClouds(deltaTimeSeconds);
    RenderClouds();

    // Render the sun
    glm::mat4 sunModelMatrix = glm::mat4(1.0f);
    sunModelMatrix = glm::translate(sunModelMatrix, glm::vec3(0, 60, 0));
	sunModelMatrix = glm::scale(sunModelMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
    RenderMesh(meshes["sun"], shaders["VertexColor"], sunModelMatrix);

    // terrain height
    terrainHeight = GetTerrainHeight(dronePosition.x, dronePosition.z);
    if (dronePosition.y < terrainHeight + 0.5f) {
        dronePosition.y = terrainHeight + 0.5f;
    }
    if (dronePosition.y < terrainHeight + 2.5f && hasParcel) {
        dronePosition.y = terrainHeight + 2.5f;
    }

    // Calculate the potential new position
    glm::vec3 potentialPosition = dronePosition;

    if (droneSpeed != 0) {
        potentialPosition += droneDirection * droneSpeed * deltaTimeSeconds;
    }

    // Check for collisions with houses
    if (!IsCollidingWithBuildings(potentialPosition) && !IsCollidingWithTrees(potentialPosition)) {
        dronePosition = potentialPosition; // Update the position only if there's no collision
    }

    // Update camera
    if (isFPSCamera) {
        glm::vec3 cameraOffset = glm::vec3(0, 0.5f, 0);
        glm::vec3 cameraTarget = dronePosition + droneDirection;
        camera->Set(dronePosition + cameraOffset, cameraTarget + cameraOffset, glm::vec3(0, 1, 0));
    }
    else {
        // Adjust for drone's current facing direction
        glm::vec3 behindDrone = -droneDirection; // Vector pointing behind the drone
        behindDrone = glm::normalize(behindDrone) * cameraDistance; // Scale based on distance

        float cameraY = dronePosition.y + 2.0f; // Set desired height
        glm::vec3 cameraPosition = dronePosition + glm::vec3(behindDrone.x, cameraY - dronePosition.y, behindDrone.z);

        // Set the camera position behind the drone
        camera->Set(cameraPosition, dronePosition, glm::vec3(0, 1, 0));
    }

    // render random trees
    for (const auto& tree : trees) {
        // Render the trunk
        glm::mat4 trunkModelMatrix = glm::mat4(1.0f);
        trunkModelMatrix = glm::translate(trunkModelMatrix, tree.position); // Position the trunk
        trunkModelMatrix = glm::scale(trunkModelMatrix, glm::vec3(5.0f, 4.0f, 5.0f)); // Make the trunk 5x thicker (X and Z)
        RenderMesh(meshes[tree.trunkMesh], shaders["VertexColor"], trunkModelMatrix);

        // Render multiple layers of leaves
        float trunkHeight = 15.0f;    // Height of the trunk
        int numLeafLayers = 3;       // Number of leaf layers
        float layerHeight = 1.5f;    // Height offset for each layer
        float trunkThickness = 5.0f; // Thickness of the trunk (same scaling for leaves)

        // First cone starting from the top of the cylinder
        glm::mat4 firstConeModelMatrix = glm::mat4(1.0f);
        firstConeModelMatrix = glm::translate(firstConeModelMatrix, tree.position);
        firstConeModelMatrix = glm::translate(firstConeModelMatrix, glm::vec3(0.0f, trunkHeight, 0.0f)); // Position leaves above the trunk
        firstConeModelMatrix = glm::scale(firstConeModelMatrix, glm::vec3(trunkThickness * 1.75f, trunkThickness, trunkThickness)); // Adjust scaling for the first cone

        RenderMesh(meshes[tree.leavesMesh], shaders["VertexColor"], firstConeModelMatrix);

        // Second cone starting from the middle of the first one
        glm::mat4 secondConeModelMatrix = glm::mat4(1.0f);
        secondConeModelMatrix = glm::translate(secondConeModelMatrix, tree.position);
        secondConeModelMatrix = glm::translate(secondConeModelMatrix, glm::vec3(0.0f, trunkHeight + trunkThickness, 0.0f)); // Position leaves above the middle of the first cone
        secondConeModelMatrix = glm::scale(secondConeModelMatrix, glm::vec3(trunkThickness * 1.75f, trunkThickness, trunkThickness)); // Adjust scaling for the second cone

        RenderMesh(meshes[tree.leavesMesh], shaders["VertexColor"], secondConeModelMatrix);
    }

    // Render houses and markers
    for (const auto& house : houses) {
        // Render house body
        glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), house.position);
        RenderMesh(meshes[house.houseMesh], shaders["VertexColor"], modelMatrix);

        // Render house roof
        modelMatrix = glm::translate(glm::mat4(1.0f), house.position + glm::vec3(0, 5.0f, 0));
        RenderMesh(meshes[house.roofMesh], shaders["VertexColor"], modelMatrix);

        if (house.isTarget && lives) {
            // Render the checkpoint marker above the target house
            modelMatrix = glm::translate(glm::mat4(1.0f), house.position + glm::vec3(0, 15.0f, 0));
            RenderMesh(meshes["marker"], shaders["VertexColor"], modelMatrix);

            // Render the dropzones next to the houses
            modelMatrix = glm::translate(glm::mat4(1.0f), house.position + glm::vec3(0, -house.position.y + GetTerrainHeight(house.position.x, house.position.z - 2 * houseSize), -2 * houseSize));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(2.5f, 2.5f, 2.5f));
            RenderMesh(meshes["dropzone"], shaders["VertexColor"], modelMatrix);

            
           // Direction from drone to the target house
           glm::vec3 targetDirection = house.position - dronePosition;
           targetDirection.y = 0.0f;

           targetDirection = glm::normalize(targetDirection);

           float angle = atan2(targetDirection.z, targetDirection.x) - glm::radians(90.0f);

           glm::mat4 arrowMatrix = glm::mat4(1.0f);
           glm::vec3 arrowPosition = dronePosition + droneDirection * 2.0f;

           // Place the arrow in front of the drone
           arrowMatrix = glm::translate(arrowMatrix, arrowPosition);
           arrowMatrix = glm::rotate(arrowMatrix, -angle, glm::vec3(0, 1, 0));
           arrowMatrix = glm::rotate(arrowMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));

           // Render the arrow mesh
           RenderMesh(meshes["arrow"], shaders["VertexColor"], arrowMatrix);
        }
    }

    if (lives > 0) {
        if (IsAboveDropzone(dronePosition, houses[currentTarget].position + glm::vec3(0, 0, -2 * houseSize), glm::vec3(2.5f, 2.5f, 2.5f)) && hasParcel) {
            // Drop the parcel
            hasParcel = false;
            score++;
            cout << "Score: " << score << endl << endl;
        }

        if ((IsCollidingWithBuildings(potentialPosition - glm::vec3(0.5f, 0, 0.5f)) || IsCollidingWithTrees(potentialPosition - glm::vec3(0.5f, 0, 0.5f))) && hasParcel) {
            hasParcel = false;
            lives--;
            cout << "Parcel lost!" << endl;
            cout << "Lives remaining: " << lives << endl << endl;
        }

        if (hasParcel) {
            glm::vec3 parcelattached = dronePosition - glm::vec3(0, 1.5f, 0);
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), parcelattached);
            RenderMesh(meshes["parcelMesh"], shaders["VertexColor"], modelMatrix);
        }

    for (const auto& parcel : parcels) {
        if (parcel.houseNo == currentTarget && parcel.isActive) {
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), parcel.position);
            modelMatrix = glm::scale(modelMatrix, parcel.dimensions); // Scale parcel mesh to its dimensions
            RenderMesh(meshes[parcel.parcelMesh], shaders["VertexColor"], modelMatrix);
        }
        if (IsCollidingWithParcel(dronePosition, parcel.position, parcel.dimensions) && parcel.isActive && !hasParcel) {
			hasParcel = true;
            houses[currentTarget].isTarget = false;
			parcels[currentTarget].isActive = false;
            float newTarget = rand() % houses.size();
            if (newTarget != currentTarget) {
                currentTarget = newTarget;
                houses[currentTarget].isTarget = true;
				parcels[currentTarget].isActive = true;
            }
            else {
                while (newTarget == currentTarget) {
                    newTarget = rand() % houses.size();
                }
                currentTarget = newTarget;
                houses[currentTarget].isTarget = true;
                parcels[currentTarget].isActive = true;
            }
        }
    }

        glm::vec3 lifeBarOffset = glm::vec3(0.0f, 2.0f, 0.0f);
        glm::vec3 lifeBarPosition = dronePosition + lifeBarOffset;

        for (int i = 0; i < lives; i++) {
            glm::mat4 heartModelMatrix = glm::mat4(1.0f);

            // Translate to the drone's position, then offset the hearts horizontally
            glm::vec3 heartOffset = glm::vec3(0.0f, i * 0.6f - (lives - 1) * 0.3f, 0);
            heartModelMatrix = glm::translate(heartModelMatrix, lifeBarPosition + heartOffset);
            heartModelMatrix = glm::rotate(heartModelMatrix, droneRotation + glm::radians(90.0f), glm::vec3(0, 1, 0));
            heartModelMatrix = glm::scale(heartModelMatrix, glm::vec3(0.3f, 0.3f, 1.0f));

            // Render the heart mesh
            RenderMesh(meshes["heart"], shaders["VertexColor"], heartModelMatrix);
        }


        // Update drone position based on input
        OnInputUpdate(deltaTimeSeconds, 0);

        // Render drone body
        static float rotorAngle = 0.0f;
        rotorAngle += deltaTimeSeconds * 5.0f;

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, dronePosition);
        modelMatrix = glm::rotate(modelMatrix, droneRotation + 0.8f, glm::vec3(0, 1, 0));
        RenderMesh(meshes["droneBody"], shaders["VertexColor"], modelMatrix);

        // Render arm extensions and rotors
        glm::vec3 rotorPositions[] = {
            glm::vec3(1.0f, 0.2f, 0.0f),  // Right
            glm::vec3(-1.0f, 0.2f, 0.0f), // Left
            glm::vec3(0.0f, 0.2f, 1.0f),  // Front
            glm::vec3(0.0f, 0.2f, -1.0f)  // Back
        };

        for (const auto& pos : rotorPositions) {
            glm::mat4 armMatrix = glm::translate(modelMatrix, pos);
            RenderMesh(meshes["armExtension"], shaders["VertexColor"], armMatrix);

            glm::mat4 rotorMatrix = glm::translate(modelMatrix, pos + glm::vec3(0, 0.21f, 0));
            rotorMatrix = glm::rotate(rotorMatrix, rotorAngle, glm::vec3(0, 1, 0));
            RenderMesh(meshes["rotor"], shaders["VertexColor"], rotorMatrix);
        }
	}
	else if (!endgamePrinted) {
		cout << "Game over!" << endl;
		cout << "Final score: " << score << endl;
		endgamePrinted = true;
	}

    // Render terrain
    if (terrain) {
        RenderMesh(meshes["terrain"], shaders["NoiseShader"], glm::mat4(1));
    }
}

void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}

void Tema2::RenderMeshOrtho(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
    if (!mesh || !shader || !shader->program)
        return;

    // Render an object using the specified shader and the specified position
    shader->Use();
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(minimapViewMatrix));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrixOrtho));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}

void Tema2::FrameEnd()
{
}

void Tema2::OnInputUpdate(float deltaTime, int mods) {
    const float DRONE_MOVEMENT_SPEED = 10.0f;

    // Initialize the movement vector
    glm::vec3 movement(0.0f);

    // Forward and backward movement
    if (window->KeyHold(GLFW_KEY_W)) {
        movement += droneDirection * DRONE_MOVEMENT_SPEED * deltaTime;
    }
    else if (window->KeyHold(GLFW_KEY_S)) {
        movement -= droneDirection * DRONE_MOVEMENT_SPEED * deltaTime;
    }
    else {
		droneSpeed = 0;
    }

    // Strafe left and right movement
    glm::vec3 strafeDirection = glm::cross(droneDirection, glm::vec3(0, 1, 0)); // Perpendicular to drone's forward
    if (window->KeyHold(GLFW_KEY_D)) {
        movement += strafeDirection * DRONE_MOVEMENT_SPEED * deltaTime;
    }
    if (window->KeyHold(GLFW_KEY_A)) {
        movement -= strafeDirection * DRONE_MOVEMENT_SPEED * deltaTime;
    }

    // Calculate the potential new position
    glm::vec3 potentialPosition = dronePosition + movement;

    // Check for collisions before applying movement
    if (!IsCollidingWithBuildings(potentialPosition) && !IsCollidingWithTrees(potentialPosition)) {
        dronePosition = potentialPosition; // Update the position only if there's no collision
    }

    // Rotate the drone
    if (window->KeyHold(GLFW_KEY_LEFT)) {
        droneRotation += deltaTime; // Rotate left
    }
    if (window->KeyHold(GLFW_KEY_RIGHT)) {
        droneRotation -= deltaTime; // Rotate right
    }

    // Move the drone up/down
    if (window->KeyHold(GLFW_KEY_E)) {
        dronePosition.y += 4.0f * deltaTime; // Move up
    }
    if (window->KeyHold(GLFW_KEY_Q) && !IsCollidingWithBuildings(dronePosition) && !IsCollidingWithTrees(dronePosition)) {
        dronePosition.y -= 4.0f * deltaTime; // Move down
    }

    // Update the drone's direction based on rotation
    droneDirection = glm::vec3(-cos(droneRotation), 0, sin(droneRotation));
}

void Tema2::OnKeyPress(int key, int mods)
{
	if (key == GLFW_KEY_SPACE) {
		isFPSCamera = !isFPSCamera;
	}
}


void Tema2::OnKeyRelease(int key, int mods)
{
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}


void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Tema2::OnWindowResize(int width, int height)
{
}
