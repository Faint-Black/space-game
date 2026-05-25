#include "collision.h"
#include "aabb_bvh.h"
#include "asteroid.h"
#include "ship.h"
#include "utils.h"
#include <stddef.h>

/* Collision sphere radius for the ship */
#define SHIP_COLLISION_RADIUS       1.5F
/* Collision sphere radius for each projectile */
#define PROJECTILE_COLLISION_RADIUS 0.3F

/* =========================================================
 * sphereVsAABB
 *
 * Computes the squared distance from the sphere center to the
 * nearest point on the AABB. No sqrt needed.
 * ========================================================= */
static int sphereVsAABB(Vec3 center, float radius, AABB box) {
    float dx = 0.0F, dy = 0.0F, dz = 0.0F;

    if      (center.x < box.min.x) dx = box.min.x - center.x;
    else if (center.x > box.max.x) dx = center.x  - box.max.x;

    if      (center.y < box.min.y) dy = box.min.y - center.y;
    else if (center.y > box.max.y) dy = center.y  - box.max.y;

    if      (center.z < box.min.z) dz = box.min.z - center.z;
    else if (center.z > box.max.z) dz = center.z  - box.max.z;

    return (dx*dx + dy*dy + dz*dz) <= (radius * radius);
}

/* =========================================================
 * bvhSphereQuery
 *
 * Recursive BVH traversal with spatial pruning 
 *   - Sphere misses node AABB -> PRUNE (return 0 immediately)
 *   - Sphere hits  + leaf     -> test distance to each stored point
 *   - Sphere hits  + internal -> recurse into children
 * ========================================================= */
static int bvhSphereQuery(const BVHNode* node,
                          const Vec3*    points,
                          Vec3           center,
                          float          radius)
{
    int i;
    if (node == NULL) return 0;

    /* Pruning step: early-out if sphere does not touch this node's AABB */
    if (!sphereVsAABB(center, radius, node->aabb)) return 0;

    /* Leaf node: test the actual stored points */
    if (bvhNodeIsLeaf(node)) {
        for (i = 0; i < node->num_points; i++) {
            if (vec3Distance(points[node->point_indices[i]], center) <= radius)
                return 1;
        }
        return 0;
    }

    /* Internal node: visit both children */
    if (bvhSphereQuery(node->left,  points, center, radius)) return 1;
    if (bvhSphereQuery(node->right, points, center, radius)) return 1;
    return 0;
}

/* =========================================================
 * checkCollision  -  Required interface 
 * Returns 1 if the ship sphere overlaps any asteroid BVH.
 * ========================================================= */
extern int checkCollision(Vec3 ship_pos) {
    const Asteroid* asteroids = getAsteroids();
    const int       count     = getAsteroidCount();
    int i;

    if (!asteroids || count <= 0) return 0;

    for (i = 0; i < count; i++) {
        const Asteroid* ast = &asteroids[i];
        if (!ast->bvh || !ast->barycenter_array) continue;

        /* Broadphase: sphere vs root AABB */
        if (!sphereVsAABB(ship_pos, SHIP_COLLISION_RADIUS, ast->bvh->aabb))
            continue;

        /* Narrowphase: full BVH traversal with pruning */
        if (bvhSphereQuery(ast->bvh, ast->barycenter_array,
                           ship_pos, SHIP_COLLISION_RADIUS))
            return 1;
    }
    return 0;
}

/* =========================================================
 * checkProjectileCollision
 *
 * Reads active projectiles via getProjectiles() 
 *
 * Iterates projectiles x asteroids; returns on the first hit
 * and fills the output indices so game.c can deactivate the
 * projectile and update the score.
 * ========================================================= */
extern int checkProjectileCollision(int* hit_asteroid_idx,
                                    int* hit_projectile_idx)
{
    const Asteroid*   asteroids = getAsteroids();
    const int         ast_count = getAsteroidCount();
    const Projectile* projs     = getProjectiles();
    int i, j;

    if (hit_asteroid_idx)   *hit_asteroid_idx   = -1;
    if (hit_projectile_idx) *hit_projectile_idx = -1;

    if (!asteroids || ast_count <= 0 || !projs) return 0;

    for (i = 0; i < MAX_PROJECTILES; i++) {
        Vec3 pos;
        if (!projs[i].active) continue;
        pos = projs[i].position;

        for (j = 0; j < ast_count; j++) {
            const Asteroid* ast = &asteroids[j];
            if (!ast->bvh || !ast->barycenter_array) continue;

            /* Broadphase */
            if (!sphereVsAABB(pos, PROJECTILE_COLLISION_RADIUS, ast->bvh->aabb))
                continue;

            /* Narrowphase */
            if (bvhSphereQuery(ast->bvh, ast->barycenter_array,
                               pos, PROJECTILE_COLLISION_RADIUS)) {
                if (hit_asteroid_idx)   *hit_asteroid_idx   = j;
                if (hit_projectile_idx) *hit_projectile_idx = i;
                return 1;
            }
        }
    }
    return 0;
}