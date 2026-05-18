#ifndef SHIP_H
#define SHIP_H

#include "utils.h"
#include "objloader.h"

#define MAX_PROJECTILES 50
#define PROJECTILE_SPEED 80.0F
#define PROJECTILE_LIFETIME 3.0F

typedef struct Projectile {
    Vec3 position;
    Vec3 velocity;
    float life;
    int active;
} Projectile;

typedef struct Ship {
    /* Position and orientation */
    Vec3 position;
    Vec3 velocity;
    Vec3 angularVelocity;

    /* Local axes */
    Vec3 forward;
    Vec3 up;
    Vec3 right;

    /* Euler angles */
    float pitch, yaw, roll;

    /* Physics params */
    float thrustPower;
    float rotationSpeed;
    float dampingLinear;
    float dampingAngular;

    /* Mechanical arm */
    float armAngle;
    float clawAngle;
    float armYaw;
    float armPitch;
    int armExtended;

    /* Thrusters */
    float thrusterAngle[2];
    float thrusterGlow;

    /* Scanner */
    float scannerAngle;
    float scannerRange;
    int scannerVisible;

    /* Camera */
    int cameraMode;
    int cameraLocked;
    float cameraDistance;
    float cameraHeight;
    float cameraYaw;
    float cameraPitch;

    /* Projectiles */
    Projectile projectiles[MAX_PROJECTILES];

    /* Model */
    OBJModel* model;

    /* State */
    int thrusting;
} Ship;

/**
 * @brief Initializes the ship state and loads the OBJ model.
 */
extern void initShip();

/**
 * @brief Frees ship resources.
 */
extern void deinitShip();

/**
 * @brief Fetches from global-state the world position of the ship entity.
 *
 * @return Ship's position.
 */
extern Vec3 getShipPosition();

/**
 * @brief Fetches from global-state the direction unit vector of the ship entity.
 *
 * @return Ship's direction vector.
 */
extern Vec3 getShipForward();

/**
 * @brief Advances the ship's state for 1 frame.
 *
 * @param dt Delta Time.
 */
extern void updateShip(float dt);

/**
 * @brief Handles a key press event.
 */
extern void shipKeyDown(int key);

/**
 * @brief Handles a key release event.
 */
extern void shipKeyUp(int key);

/**
 * @brief Fires a projectile from the ship.
 */
extern void shipFireProjectile();

/**
 * @brief Toggles the mechanical arm extension.
 */
extern void shipToggleArm();

/**
 * @brief Toggles the camera between 1st and 3rd person.
 */
extern void shipToggleCamera();

/**
 * @brief Locks the camera to the center of the ship.
 */
extern void shipLockCamera();

/**
 * @brief Toggles scanner visibility.
 */
extern void shipToggleScanner();

/**
 * @brief Handles mouse motion for camera orbit.
 */
extern void shipMouseMotion(int dx, int dy);

/**
 * @brief Sets up camera projection and view matrices.
 *
 * @param windowW Window width in pixels.
 * @param windowH Window height in pixels.
 */
extern void shipSetupCamera(int windowW, int windowH);

/**
 * @brief Renders the ship model, arm, and projectiles.
 */
extern void renderShip();

/**
 * @brief Renders the scanner cone visualization.
 */
extern void renderScanner();

/**
 * @brief Tests if a point is within the scanner cone.
 *
 * @return 1 if point is in scanner cone, 0 otherwise.
 */
extern int shipPointInScanner(Vec3 point);

#endif /* SHIP_H */
