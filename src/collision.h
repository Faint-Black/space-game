#ifndef COLLISION_H
#define COLLISION_H

#include "utils.h"
#include "asteroid.h"

typedef struct{
    Vec3 min;
    Vec3 max;
} AABB;

typedef struct BVHNode {
    AABB box; 
    struct BVHNode *left;
    struct BVHNode *right;
    const TriangleFace *faces;
    int face_count;
} BVHNode;

/**
 * @brief Uses AABB and BVH to check if the ship collided with an asteroid.
 *
 * @retval 0 No collision occured.
 * @retval 1 Some collision occured.
 */
extern int checkCollision(Vec3 ship_pos);

#endif /* COLLISION_H */
