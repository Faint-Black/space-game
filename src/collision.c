#include "collision.h"
#include "aabb_bvh.h"
#include "asteroid.h"
#include "ship.h"
#include "utils.h"
#include <stddef.h>

#define PROJECTILE_COLLISION_RADIUS 0.3F

#define CLAW_SHOULDER_Y    (-1.8F)
#define CLAW_SHOULDER_Z    (-3.0F)
#define CLAW_ARM_LENGTH    (10.0F)
#define CLAW_SPHERE_RADIUS ( 2.5F)

/* =========================================================
 * computeClawWorldPos
 * ========================================================= */
static Vec3 computeClawWorldPos(void) {
    Vec3 ship_pos = getShipPosition();
    Vec3 fwd      = getShipForward();
    Vec3 up       = getShipUp();
    Vec3 claw_pos;

    claw_pos.x = ship_pos.x + fwd.x*(CLAW_SHOULDER_Z + CLAW_ARM_LENGTH) + up.x*CLAW_SHOULDER_Y;
    claw_pos.y = ship_pos.y + fwd.y*(CLAW_SHOULDER_Z + CLAW_ARM_LENGTH) + up.y*CLAW_SHOULDER_Y;
    claw_pos.z = ship_pos.z + fwd.z*(CLAW_SHOULDER_Z + CLAW_ARM_LENGTH) + up.z*CLAW_SHOULDER_Y;

    return claw_pos;
}

/* =========================================================
 * sphereVsAABB
 * ========================================================= */
static int sphereVsAABB(Vec3 center, float radius, AABB box) {
    float dx = 0.0F;
    float dy = 0.0F;
    float dz = 0.0F;
    float distSquared;
    float radiusSquared;

    if (center.x < box.min.x) {
        dx = box.min.x - center.x;
    } else if (center.x > box.max.x) {
        dx = center.x - box.max.x;
    }

    if (center.y < box.min.y) {
        dy = box.min.y - center.y;
    } else if (center.y > box.max.y) {
        dy = center.y - box.max.y;
    }

    if (center.z < box.min.z) {
        dz = box.min.z - center.z;
    } else if (center.z > box.max.z) {
        dz = center.z - box.max.z;
    }

    distSquared   = dx*dx + dy*dy + dz*dz;
    radiusSquared = radius * radius;

    return distSquared <= radiusSquared;
}

/* =========================================================
 * bvhSphereQuery
 * ========================================================= */
static int bvhSphereQuery(const BVHNode* node, const Vec3* points, Vec3 center, float radius) {
    int i;

    if (node == NULL) {
        return 0;
    }
    if (!sphereVsAABB(center, radius, node->aabb)) {
        return 0;
    }

    if (bvhNodeIsLeaf(node)) {
        for (i = 0; i < node->num_points; i++) {
            if (vec3Distance(points[node->point_indices[i]], center) <= radius) {
                return 1;
            }
        }
        return 0;
    }

    if (bvhSphereQuery(node->left, points, center, radius)) {
        return 1;
    }
    if (bvhSphereQuery(node->right, points, center, radius)) {
        return 1;
    }
    return 0;
}

/* =========================================================
 * bvhVsBvh  —  ship BVH vs asteroid BVH
 * Traverses both trees in parallel, pruning by AABB overlap.
 * ========================================================= */
static int bvhVsBvh(const BVHNode* a, const BVHNode* b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    if (!aabbVsAABB(a->aabb, b->aabb)) {
        return 0;
    }

    if (bvhNodeIsLeaf(a) && bvhNodeIsLeaf(b)) {
        return 1;
    }

    if (bvhNodeIsLeaf(a)) {
        if (bvhVsBvh(a, b->left)) {
            return 1;
        }
        if (bvhVsBvh(a, b->right)) {
            return 1;
        }
        return 0;
    }

    if (bvhNodeIsLeaf(b)) {
        if (bvhVsBvh(a->left, b)) {
            return 1;
        }
        if (bvhVsBvh(a->right, b)) {
            return 1;
        }
        return 0;
    }

    if (bvhVsBvh(a->left, b->left)) {
        return 1;
    }
    if (bvhVsBvh(a->left, b->right)) {
        return 1;
    }
    if (bvhVsBvh(a->right, b->left)) {
        return 1;
    }
    if (bvhVsBvh(a->right, b->right)) {
        return 1;
    }
    return 0;
}

/* =========================================================
 * checkCollision  —  ship body vs asteroids (causes damage)
 * ========================================================= */
extern int checkCollision(Vec3 ship_pos) {
    const Asteroid* asteroids = getAsteroids();
    const int       count     = getAsteroidCount();
    const BVHNode*  ship_bvh  = getShipBVH();
    int i;

    (void)ship_pos; /* position is already encoded in the ship BVH AABBs */

    if (!asteroids || count <= 0) {
        return 0;
    }
    if (ship_bvh == NULL) {
        return 0;
    }

    for (i = 0; i < count; i++) {
        const Asteroid* ast = &asteroids[i];

        if (!ast->bvh) {
            continue;
        }
        /* Broadphase: ship BVH root AABB vs asteroid root AABB */
        if (!aabbVsAABB(ship_bvh->aabb, ast->bvh->aabb)) {
            continue;
        }
        /* Narrowphase: ship BVH vs asteroid BVH */
        if (bvhVsBvh(ship_bvh, ast->bvh)) {
            return i + 1;
        }
    }
    return 0;
}

/* =========================================================
 * checkProjectileCollision
 * ========================================================= */
extern int checkProjectileCollision(int* hit_asteroid_idx, int* hit_projectile_idx) {
    const Asteroid*   asteroids = getAsteroids();
    const int         ast_count = getAsteroidCount();
    const Projectile* projs     = getProjectiles();
    int i, j;

    if (hit_asteroid_idx) {
        *hit_asteroid_idx = -1;
    }
    if (hit_projectile_idx) {
        *hit_projectile_idx = -1;
    }
    if (!asteroids || ast_count <= 0 || !projs) {
        return 0;
    }

    for (i = 0; i < MAX_PROJECTILES; i++) {
        Vec3 pos;

        if (!projs[i].active) {
            continue;
        }
        pos = projs[i].position;

        for (j = 0; j < ast_count; j++) {
            const Asteroid* ast = &asteroids[j];

            if (!ast->bvh || !ast->barycenter_array) {
                continue;
            }
            if (!sphereVsAABB(pos, PROJECTILE_COLLISION_RADIUS, ast->bvh->aabb)) {
                continue;
            }
            if (bvhSphereQuery(ast->bvh, ast->barycenter_array, pos, PROJECTILE_COLLISION_RADIUS)) {
                if (hit_asteroid_idx) {
                    *hit_asteroid_idx = j;
                }
                if (hit_projectile_idx) {
                    *hit_projectile_idx = i;
                }
                return 1;
            }
        }
    }
    return 0;
}

/* =========================================================
 * checkClawCollision
 * ========================================================= */
extern int checkClawCollision(void) {
    const Asteroid* asteroids = getAsteroids();
    const int       count     = getAsteroidCount();
    Vec3 claw_pos;
    int i;

    if (!getShipArmExtended()) {
        return -1;
    }
    if (!asteroids || count <= 0) {
        return -1;
    }

    claw_pos = computeClawWorldPos();

    for (i = 0; i < count; i++) {
        const Asteroid* ast = &asteroids[i];

        if (!ast->bvh || !ast->barycenter_array) {
            continue;
        }
        if (!sphereVsAABB(claw_pos, CLAW_SPHERE_RADIUS, ast->bvh->aabb)) {
            continue;
        }
        if (bvhSphereQuery(ast->bvh, ast->barycenter_array, claw_pos, CLAW_SPHERE_RADIUS)) {
            return i;
        }
    }
    return -1;
}