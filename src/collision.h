#ifndef COLLISION_H
#define COLLISION_H

#include "asteroid.h"
#include "render.h"
#include "utils.h"

/**
 * @brief Axis-Aligned Bounding Box, the building block for BVH leaves and
 *        broad-phase collision queries.
 */
typedef struct {
    Vec3 min;
    Vec3 max;
} AABB;

/**
 * @brief Bounding Volume Hierarchy node. Internal nodes carry only the box
 *        and child pointers; leaves additionally reference the triangle range
 *        they cover.
 */
typedef struct BVHNode {
    AABB box;
    struct BVHNode* left;
    struct BVHNode* right;
    const TriangleFace* faces;
    int face_count;
} BVHNode;

/**
 * @brief Computes the world-space AABB that bounds every vertex of the given
 *        triangle mesh.
 *
 * @param faces Pointer to a contiguous triangle array.
 * @param count Number of triangles in the array.
 *
 * @return The tight AABB enclosing every vertex, or a null box if the input
 *         is empty.
 */
extern AABB computeAABBFromFaces(const TriangleFace* faces, int count);

/**
 * @brief Uses AABB and BVH to check if the ship collided with an asteroid.
 *
 * @retval 0 No collision occured.
 * @retval 1 Some collision occured.
 */
extern int checkCollision(Vec3 ship_pos);

#endif /* COLLISION_H */
