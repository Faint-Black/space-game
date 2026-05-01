#ifndef SHIP_H
#define SHIP_H

#include "utils.h"

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
