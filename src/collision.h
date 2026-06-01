#ifndef COLLISION_H
#define COLLISION_H

#include "utils.h"

/**
 * @brief Checks if the ship BODY collided with any asteroid (causes damage).
 * The claw region is excluded from this volume.
 *
 * @retval 1 Collision detected.
 * @retval 0 No collision.
 */
extern int checkCollision(Vec3 ship_pos);

/**
 * @brief Tests every active projectile against every asteroid BVH.
 * Returns on the first hit and fills the output indices so game.c
 * can deactivate the projectile and update the score.
 *
 * @param hit_asteroid_idx   (out) Index of the asteroid that was hit, or -1.
 * @param hit_projectile_idx (out) Index of the projectile that hit, or -1.
 * @retval 1 A projectile hit an asteroid.
 * @retval 0 No collision.
 */
extern int checkProjectileCollision(int* hit_asteroid_idx, int* hit_projectile_idx);

/**
 * @brief Checks if the mechanical claw touched any asteroid (causes score).
 * Only active while the arm is extended (getShipArmExtended() == 1).
 *
 * Uses a sphere at the computed world-space claw tip position, so the
 * check tracks the arm animation automatically.
 *
 * @return Index of the hit asteroid in the asteroid array, or -1 if none.
 */
extern int checkClawCollision(void);


#endif /* COLLISION_H */