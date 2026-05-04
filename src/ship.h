#ifndef SHIP_H
#define SHIP_H

#include "utils.h"

typedef struct {
    float angle;
    float clawAngle;
    int   extended;
} Arm;

typedef struct {
    // Posicao e orientacao
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    Vec3 angularVelocity;

    // Eixos locais da nave (forward = -Z, up = Y, right = X)
    Vec3 forward;
    Vec3 up;
    Vec3 right;

    // Rotacaoo em torno dos eixos locais
    float pitch;
    float yaw;
    float roll;

    // Fisica e controle
    float thrustPower;
    float rotationSpeed;
    float dampingLinear;
    float dampingAngular;

    // Scanner
    float scannerAngle;
    float scannerRange;

    int thrusting;

    Arm arm;
} Ship;

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

#endif /* SHIP_H */
