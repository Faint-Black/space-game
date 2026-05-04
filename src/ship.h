#ifndef SHIP_H
#define SHIP_H

#include "utils.h"


typedef struct {
    float angle;
    float clawAngle;
    int extended;
} Arm;

typedef struct {
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Vec3 angularVelocity;

    Vec3 up;
    Vec3 right;
    Vec3 forward;

    float pitch;
    float yaw;
    float row;

    Arm arm;
} Ship;

/**
 * @brief Fetches from global-state the world position of the ship entity.
 *
 * @return Ship's position.
 */
extern Vec3 getShipPosition(void);

/**
 * @brief Fetches from global-state the direction unit vector of the ship entity.
 *
 * @return Ship's direction vector.
 */
extern Vec3 getShipForward(void);

/**
 * @brief Advances the ship's state for 1 frame.
 *
 * @param dt Delta Time.
 */
extern void updateShip(float dt);

#endif /* SHIP_H */
