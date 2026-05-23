#ifndef AABB_BVH_H
#define AABB_BVH_H

#include "render.h"
#include "utils.h"

typedef struct AABB {
    Vec3 min;
    Vec3 max;
} AABB;

typedef struct BVHNode {
    AABB aabb;
    struct BVHNode* left;
    struct BVHNode* right;
    int* point_indices; /* Array of indices if leaf node, otherwise NULL */
    int num_points;     /* Number of points in this leaf (0 if internal node) */
} BVHNode;

/**
 * @brief Computes the overall AABB from a list of points.
 */
extern AABB aabbComputeFromPoints(const Vec3* points, int count);

/**
 * @brief Merges two AABBs into a larger AABB that encompasses both.
 */
extern AABB aabbMerge(AABB a, AABB b);

/**
 * @brief Builds the BVH tree top-down using the provided points.
 * @param points Array with all world points.
 * @param indices Array of indices corresponding to the points that should enter the tree.
 * @param num_indices Size of the indices array.
 * @param max_points_per_leaf Maximum points allowed in a leaf node.
 * @return Pointer to the dynamically allocated BVH root.
 */
extern BVHNode* bvhBuild(const Vec3* points, int* indices, int num_indices, int max_points_per_leaf);

/**
 * @brief Recursively frees all memory allocated for the BVH.
 */
extern void bvhFree(BVHNode* root);

/**
 * @brief Refits/updates the AABBs of an existing BVH tree.
 * Useful when the object moves but its internal geometry remains the same.
 *
 * @param node The current BVH node to update.
 * @param current_points The updated world-space points array.
 */
extern void bvhRefit(BVHNode* node, const Vec3* current_points);

/**
 * @brief Helper that checks if a given node is a leaf or not
 */
extern int bvhNodeIsLeaf(const BVHNode* node);

#endif /* AABB_BVH_H */
