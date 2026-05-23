#include "collision.h"
#include "aabb_bvh.h"
#include "asteroid.h"
#include "utils.h"
#include <stddef.h>

/* Ship is treated as a sphere of this radius around its position for broad-phase tests. */
#define SHIP_COLLISION_RADIUS 1.5F

/* ======================================================================== */
/*  Sphere vs AABB overlap (squared distance, no sqrt)                       */
/* ======================================================================== */

/**
 * @retval 1 The sphere and the box overlap.
 * @retval 0 The sphere and the box are disjoint.
 */
static int sphereIntersectsAabb(Vec3 center, float radius, AABB box) {
    float dx = 0.0F;
    float dy = 0.0F;
    float dz = 0.0F;
    float squared_distance;

    if (center.x < box.min.x)
        dx = box.min.x - center.x;
    else if (center.x > box.max.x)
        dx = center.x - box.max.x;

    if (center.y < box.min.y)
        dy = box.min.y - center.y;
    else if (center.y > box.max.y)
        dy = center.y - box.max.y;

    if (center.z < box.min.z)
        dz = box.min.z - center.z;
    else if (center.z > box.max.z)
        dz = center.z - box.max.z;

    squared_distance = (dx * dx) + (dy * dy) + (dz * dz);
    return (squared_distance <= (radius * radius));
}

/* ======================================================================== */
/*  Public collision query                                                   */
/* ======================================================================== */

extern int checkCollision(Vec3 ship_pos) {
    const Asteroid* asteroids = getAsteroids();
    const int count = getAsteroidCount();
    int i;

    for (i = 0; i < count; i++) {
        const Asteroid asteroid = asteroids[i];
        const AABB box = asteroid.bounding_box;
        if (sphereIntersectsAabb(ship_pos, SHIP_COLLISION_RADIUS, box)) {
            return 1;
        }
    }
    return 0;
}
