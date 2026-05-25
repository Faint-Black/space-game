#ifndef COLLISION_H
#define COLLISION_H

#include "utils.h"

/**
 * @brief Checks if the ship collided with any asteroid using BVH narrowphase.
 * Required interface for Group 3 (PDF section 3.3).
 *
 * @retval 1 Collision detected.
 * @retval 0 No collision.
 */
extern int checkCollision(Vec3 ship_pos);

/**
 * @brief Tests every active projectile against every asteroid BVH.
 * Deactivates the projectile and fills the output indices on the first hit.
 *
 * @param hit_asteroid_idx   (out) Index of the asteroid that was hit, or -1.
 * @param hit_projectile_idx (out) Index of the projectile that hit, or -1.
 * @retval 1 A projectile hit an asteroid.
 * @retval 0 No collision.
 */
extern int checkProjectileCollision(int* hit_asteroid_idx,
                                    int* hit_projectile_idx);

#endif /* COLLISION_H */