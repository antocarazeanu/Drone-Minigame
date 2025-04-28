#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"
#include "lab_m1/lab5/lab_camera.h"
#include "lab_m1/lab4/transform3D.h"


namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        Tema2();
        ~Tema2();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        // functions
        Mesh* GenerateTerrain(int m, int n, float width, float height);
        Mesh* CreateDroneBody(const std::string& name, const glm::vec3& color);
        Mesh* CreateRotor(const std::string& name, const glm::vec3& color);
        Mesh* CreateArmExtension(const std::string& name, const glm::vec3& color);
        float GetTerrainHeight(float x, float z);
        float generateNoise(glm::vec2 pos);
        void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix);
        void RenderMeshOrtho(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix);
        Mesh* generateTrunk(float radius, float height, unsigned int segments, const std::string& name);
		Mesh* generateLeaves(float radius, float height, unsigned int segments, const std::string& name);
        void GenerateRandomTrees(unsigned int numberOfTrees);
        Mesh* CreateCubeMesh(const std::string& name, glm::vec3 size, glm::vec3 color);
		Mesh* CreatePyramidMesh(const std::string& name, glm::vec3 baseSize, float height, glm::vec3 color);
		void GenerateRandomHouses(unsigned int numberOfHouses);
        bool IsCollidingWithBuildings(const glm::vec3& position);
        bool IsCollidingWithTrees(const glm::vec3& position);
        Mesh* CreateParcelMesh(const std::string& name, const glm::vec3& dimensions, const glm::vec3& color);
        void GenerateParcelsForHouses(float offsetX, float offsetZ);
        void RenderScene(float deltaTimeSeconds);
        void RenderSceneWithCustomView(float deltaTimeSeconds, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        Mesh* CreateLocationMarkerMesh(const std::string& name, const glm::vec3& dimensions, const glm::vec3& color);
        bool IsCollidingWithParcel(const glm::vec3& dronePosition, const glm::vec3& parcelPosition, const glm::vec3& parcelDimensions);
        Mesh* CreateArrowMesh(const std::string& name, float length, float radius, const glm::vec3& color);
        Mesh* GenerateCircleMesh(const std::string& name, glm::vec3 color, float radius, int segments);
        bool IsAboveDropzone(const glm::vec3& dronePosition, const glm::vec3& dropzonePosition, const glm::vec3& dropzoneSize);
        float RandomFloat(float min, float max);

// ======================================================== Variables ========================================================
		// terrain variables
        Mesh *terrain;
		float terrainHeight;
		int width, height; // Dimensions of the terrain
		int nx, nz; // Number of vertices and indexes in the terrain

		// camera variables
        implemented::Camera* camera; // Pointer to the FPS camera
        bool isFPSCamera;     // Flag to toggle between FPS and third-person camera
		float cameraSpeed;    // Camera speed
        glm::mat4 projectionMatrix;
        glm::mat4 projectionMatrixOrtho;
        float cameraDistance;
        glm::mat4 minimapViewMatrix;

		// drone variables
        Mesh* droneBody;
        Mesh* armExtension;
        glm::vec3 dronePosition; // Position of the drone in the world (x, y, z)
        float droneRotation;     // Rotation angle of the rotors
        float droneSpeed;
        glm::vec3 droneDirection;
        bool hasParcel;

		// tree variables
        struct Tree {
            glm::vec3 position;
            std::string trunkMesh;
            std::string leavesMesh;
        };
        std::vector<Tree> trees;  // List to store the generated trees
        Mesh* treeTrunk;
        Mesh* treeLeaves;
		float trunkHeight, trunkRadius, leavesHeight, leavesRadius;

        // building variables
        float houseSize;
        struct House {
            glm::vec3 position;
            std::string roofMesh;
            std::string houseMesh;
			bool isTarget = false;
        };
        std::vector<House> houses;
        Mesh* houseBody;
        Mesh* houseRoof;
        int currentTarget;

		// parcel variables
		Mesh* parcel;
        struct Parcel {
            int houseNo;
            glm::vec3 position;
			std::string parcelMesh;
			glm::vec3 dimensions;
			bool isActive;
        };
		std::vector<Parcel> parcels;
        bool isParcelBeingDelivered = false;
        Mesh* dropZone;

		// minimap variables
        struct ViewportArea
        {
            ViewportArea() : x(0), y(0), width(1), height(1) {}
            ViewportArea(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };
        ViewportArea miniViewportArea;
		glm::mat4 miniModelMatrix;

        // Marker variable
        Mesh* marker;

        // Arrow variable
        Mesh* arrow;

		// score and life variable
        Mesh* GenerateHeartMesh(const std::string& name, glm::vec3 color);
        Mesh* GenerateLifeBarMesh(const std::string& name, glm::vec3 color);
		void UpdateLifeBar();
        Mesh* lifeBarMesh;
        int lives;
        int score;
		bool endgamePrinted;

		// cloud variables
        Mesh* CreateCloudMesh(const glm::vec3& uniformColor, const std::string& name, float baseSize);
        void GenerateSphere(std::vector<VertexFormat>& vertices, std::vector<unsigned int>& indices,
            const glm::vec3& center, float radius, const glm::vec3& uniformColor);
        void UpdateClouds(float deltaTimeSeconds);
        void RenderClouds();
        struct Cloud {
            glm::vec3 position;
            std::string cloud;
        };
        Mesh* cloudMesh;
		std::vector<Cloud> cloudMeshes;

        std::vector<Cloud> clouds;
    };
}   // namespace m1
