#ifndef COLLISION_H
#define COLLISION_H

#include "utils.h"

/**
 * @brief Checks if the ship collided with any asteroid using BVH vs BVH narrowphase.
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

#endif /* COLLISION_H */