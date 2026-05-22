#ifndef COLLISION_H
#define COLLISION_H

#include "render.h"
#include "utils.h"

/**
 * @brief Uses AABB and BVH to check if the ship collided with an asteroid.
 *
 * @retval 0 No collision occured.
 * @retval 1 Some collision occured.
 */
extern int checkCollision(Vec3 ship_pos);

#endif /* COLLISION_H */
