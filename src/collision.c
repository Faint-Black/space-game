#include "collision.h"
#include "aabb_bvh.h"
#include "asteroid.h"
#include "ship.h"
#include "utils.h"
#include <stddef.h>

/* Collision sphere radius for each projectile */
#define PROJECTILE_COLLISION_RADIUS 0.3F

#define SHIP_LOCAL_MIN_X (-2.3F)
#define SHIP_LOCAL_MAX_X ( 2.3F)
#define SHIP_LOCAL_MIN_Y (-2.1F)
#define SHIP_LOCAL_MAX_Y ( 1.1F)
#define SHIP_LOCAL_MIN_Z (-3.3F)   
#define SHIP_LOCAL_MAX_Z ( 2.3F)   

/* =========================================================
 * reconstructAxes
 * ========================================================= */
static void reconstructAxes(Vec3 fwd, Vec3* out_right, Vec3* out_up) {
    Vec3 helper;
    Vec3 right;
    Vec3 up;
    float fy;

    if (fwd.y < 0.0F) {
        fy = -fwd.y;
    } else {
        fy = fwd.y;
    }

    if (fy < 0.9F) {
        helper.x = 0.0F;
        helper.y = 1.0F;
        helper.z = 0.0F;
    } else {
        helper.x = 1.0F;
        helper.y = 0.0F;
        helper.z = 0.0F;
    }

    /* right = normalize(cross(helper, fwd)) */
    right = vec3Normalize(vec3Cross(helper, fwd));

    /* up = cross(fwd, right) */
    up = vec3Cross(fwd, right);

    *out_right = right;
    *out_up    = up;
}

/* =========================================================
 * shipBuildWorldAABB
 * ========================================================= */
static AABB shipBuildWorldAABB(Vec3 pos, Vec3 fwd, Vec3 up, Vec3 right) {
    static const float lx[2] = { SHIP_LOCAL_MIN_X, SHIP_LOCAL_MAX_X };
    static const float ly[2] = { SHIP_LOCAL_MIN_Y, SHIP_LOCAL_MAX_Y };
    static const float lz[2] = { SHIP_LOCAL_MIN_Z, SHIP_LOCAL_MAX_Z };

    AABB box;
    int ix, iy, iz;
    int first = 1;

    for (ix = 0; ix < 2; ix++) {
        for (iy = 0; iy < 2; iy++) {
            for (iz = 0; iz < 2; iz++) {
                Vec3 corner;
                corner.x = pos.x + right.x*lx[ix] + up.x*ly[iy] + fwd.x*lz[iz];
                corner.y = pos.y + right.y*lx[ix] + up.y*ly[iy] + fwd.y*lz[iz];
                corner.z = pos.z + right.z*lx[ix] + up.z*ly[iy] + fwd.z*lz[iz];

                if (first) {
                    box.min = corner;
                    box.max = corner;
                    first = 0;
                } else {
                    if (corner.x < box.min.x) box.min.x = corner.x;
                    if (corner.y < box.min.y) box.min.y = corner.y;
                    if (corner.z < box.min.z) box.min.z = corner.z;
                    if (corner.x > box.max.x) box.max.x = corner.x;
                    if (corner.y > box.max.y) box.max.y = corner.y;
                    if (corner.z > box.max.z) box.max.z = corner.z;
                }
            }
        }
    }
    return box;
}

/* =========================================================
 * aabbVsAsteroidBVH
 * ========================================================= */
static int aabbVsAsteroidBVH(AABB ship_box, const BVHNode* node) {
    if (node == NULL){ 
        return 0;
    }
    if (!aabbVsAABB(ship_box, node->aabb)){ 
        return 0;
    }
    if (bvhNodeIsLeaf(node)){  
        return 1;
    }
    if (aabbVsAsteroidBVH(ship_box, node->left)){  
        return 1;
    }   
    if (aabbVsAsteroidBVH(ship_box, node->right)){
        return 1;
    }
    return 0;
}

/* =========================================================
 * sphereVsAABB  
 * ========================================================= */
static int sphereVsAABB(Vec3 center, float radius, AABB box) {
    float dx = 0.0F, dy = 0.0F, dz = 0.0F;
    float squaredDistance;
    float squaredRadius;

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

    squaredDistance = dx*dx + dy*dy + dz*dz;
    squaredRadius   = radius * radius;

    return squaredDistance <= squaredRadius;
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
 * checkCollision
 * ========================================================= */
extern int checkCollision(Vec3 ship_pos) {
    const Asteroid* asteroids = getAsteroids();
    const int       count     = getAsteroidCount();
    AABB ship_box;
    Vec3 fwd, up, right;
    int i;

    if (!asteroids || count <= 0) {
        return 0;
    }

    fwd = getShipForward();
    reconstructAxes(fwd, &right, &up);

    ship_box = shipBuildWorldAABB(ship_pos, fwd, up, right);

    for (i = 0; i < count; i++) {
        const Asteroid* ast = &asteroids[i];
        
        if (!ast->bvh) {
            continue;
        }
        /* Broadphase: AABB ship vs AABB root */
        if (!aabbVsAABB(ship_box, ast->bvh->aabb)) {
            continue;
        }
        /* Narrowphase: Detailed collision detection between ship and asteroid geometry */
        if (aabbVsAsteroidBVH(ship_box, ast->bvh)) {
            return 1;
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