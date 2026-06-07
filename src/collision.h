#ifndef COLLISION_H
#define COLLISION_H

#include "utils.h"

/**
 * @brief Checks if the ship BODY collided with any asteroid (causes damage).
 */
extern int checkCollision(Vec3 ship_pos);

/**
 * @brief Tests every active projectile against every asteroid BVH.
 */
extern int checkProjectileCollision(int* hit_asteroid_idx, int* hit_projectile_idx);

/**
 * @brief Checks if the mechanical claw touched any asteroid (causes score).
 * Only active while the arm is extended (getShipArmExtended() == 1).
 */
extern int checkClawCollision(void);

/**
 * @brief Checks if the mechanical claw reached any active rescue module.
 * Only active while the arm is extended. Returns the module index, or -1.
 */
extern int checkModuleGrab(void);


#endif /* COLLISION_H */