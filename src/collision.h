#ifndef COLLISION_H
#define COLLISION_H

#include "asteroid.h"
#include "render.h"
#include "utils.h"

/* AABB and BVHNode are defined in utils.h */

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
