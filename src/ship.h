#ifndef SHIP_H
#define SHIP_H

#include "objloader.h"
#include "utils.h"

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

    /* Models */
    OBJModel* model;
    OBJModel* armModel;

    /* State */
    int thrusting;
} Ship;

extern void initShip();
extern void deinitShip();

/** @brief World position of the ship. */
extern Vec3 getShipPosition();

/** @brief Forward unit vector of the ship (-Z local). */
extern Vec3 getShipForward();

/** @brief Up unit vector of the ship (+Y local). */
extern Vec3 getShipUp();

/** @brief Right unit vector of the ship (+X local). */
extern Vec3 getShipRight();

/** @brief Returns 1 if the mechanical arm is currently extended, 0 otherwise. */
extern int getShipArmExtended();

extern void updateShip(float dt);
extern void shipKeyDown(int key);
extern void shipKeyUp(int key);
extern void shipFireProjectile();
extern void shipToggleArm();
extern void shipToggleCamera();
extern void shipLockCamera();
extern void shipToggleScanner();
extern void shipMouseMotion(int dx, int dy);
extern void shipSetupCamera(int windowW, int windowH);
extern void renderShip();
extern void renderScanner();
extern int  shipPointInScanner(Vec3 point);

/**
 * @brief Returns a read-only pointer to the projectile array (MAX_PROJECTILES entries).
 * Check .active on each entry before using it.
 */
extern const Projectile* getProjectiles(void);

#endif /* SHIP_H */