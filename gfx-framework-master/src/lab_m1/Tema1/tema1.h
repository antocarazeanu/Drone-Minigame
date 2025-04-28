#pragma once

#include "components/simple_scene.h"
#include "lab_m1/lab5/lab_camera.h"

namespace m1 {
    class Tema1 : public gfxc::SimpleScene {
    public:
        Tema1();
        ~Tema1();

        void Init() override;

		// Camera settings
        bool cameraShakeActive = false;
        float cameraShakeStartTime = 0.0f;
        float shakeDuration = 2.0f;
        float shakeMagnitude = 5.0f;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;
        void OnWindowResize(int width, int height) override;
		void OnKeyPress(int key, int mods) override;
		void CreateSkyGradient();
        Mesh* CreateTankBaseMesh(glm::vec3 tankColor);
		Mesh* CreateTankBarrelMesh();
        Mesh* CreateLifeBarBorderMesh();
        Mesh* CreateLifeBarInnerMesh(float life);
        Mesh* CreateProjectileMesh();
        Mesh* CreateTerrainMesh();
		void LaunchProjectile(glm::vec3 position, glm::vec3 direction, float speed);
		glm::vector<glm::vec3> GetTrajectoryPoints(glm::vec3 position, float speed, float gravity, int numPoints);
        void RenderProjectileTrajectory(bool isMyTank);
        void UpdateProjectiles(float deltaTime);
        void LaunchProjectile(bool isMyTank);
		void ReduceLife(bool isMyTank, float dmg);
        bool CheckCollision(glm::vec3 projectilePos, float tankX, float tankY, float hitRadius);
        void UpdateLifeBarInnerMesh(Mesh* lifeBarMesh, float lifePercentage);
        void DeformTerrain(float x, float y, float radius);
        Mesh* CreateCircleMesh(float radius, int segments);
        void DeformTerrain(glm::vec2 impactPoint, float explosionRadius, float deformationDepth);
        bool CheckProjectileTerrainCollision(glm::vec2 projectilePos);
        void UpdateTerrainMesh();
        Mesh* CreateTrajectoryMesh();
        void UpdateTrajectoryMesh(float launchAngle, float initialSpeed, float gravity, float TankX, float TankY);
        void SimulateLandslide(float deltaTimeSeconds);
        Mesh* CreateSunMesh();
        Mesh* CreateExplosionMesh();
        Mesh* CreateCloudMesh();
        Mesh* CreateFlagMesh();


        // Member variables
        float translateX, translateY;
        float scaleX, scaleY;
        float angularStep;
        float cx, cy;
        glm::mat3 modelMatrix;
        float speed;
		float angularSpeed;
		float gravity;
		float projectileSpeed;
		float myTankX, myTankY;
		float myTankAngle;
		float myTankTurretAngle;
		float myTankLife;
		float enemyTankX, enemyTankY;
		float enemyTankAngle;
		float enemyTankTurretAngle;
		float enemyTankLife;
        float maxLife;
		glm::vec3 myTankPosition, enemyTankPosition;
		float explosionRadius, maxDeformation, collisionThreshold;
		bool terrainDeformed;
        float explosionStartTime = 0.0f;
		float enemyexplosionStartTime = 0.0f;
		bool enemyexplosionActive = false;
        bool explosionActive = false;
        float lastTimeHit, enemylastTimeHit;
        float mydamage, enemydamage;

        // Height map for the terrain
        std::vector<float> heightMap;
        int terrainWidth;
        float terrainStep;

        // Terrain mesh
        Mesh* terrainMesh;

		// Sky gradient mesh
		Mesh* skyMesh;

		// Sun mesh
		Mesh* sunMesh;

		// Cloud mesh
		Mesh* cloudMesh;
        glm::vector<glm::vec2> cloudPositions;

		// Explosion mesh
		Mesh* explosionMesh;
		Mesh* enemyexplosionMesh;
        Mesh* explosionMesh1;

        // Tank mesh
        Mesh* myTankBase;
        Mesh* myTankBarrel;
        Mesh* myTankLifeBar;
        Mesh* myTankLifeBarInner;
        Mesh* myhitpointsCircle;
        Mesh* enemyTankBase;
        Mesh* enemyTankBarrel;
        Mesh* enemyTankLifeBar;
        Mesh* enemyTankLifeBarInner;
		Mesh* enemyhitpointsCircle;

		// Projectiles
        struct Projectile {
            glm::vec3 position;
            glm::vec2 velocity;
            bool active;
            bool isFriendly; // true = my tank; false = enemy tank
        };


        Mesh* projectile;
        std::vector<Projectile> projectiles;

		// Trajectory
		Mesh* trajectoryMesh;

        // Flag mesh
        Mesh* flagMesh1;
        Mesh* flagMesh2;
		std::vector<glm::vec2> flagPositions;
        bool flag1 = true;
        bool flag2 = true;
    };
}