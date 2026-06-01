#include "collision.h"
#include "aabb_bvh.h"
#include "asteroid.h"
#include "ship.h"
#include "utils.h"
#include <stddef.h>

/* =========================================================
 * Constants
 * ========================================================= */

/* Collision sphere radius for each projectile */
#define PROJECTILE_COLLISION_RADIUS 0.3F

#define BODY_LOCAL_MIN_X (-2.3F)
#define BODY_LOCAL_MAX_X ( 2.3F)
#define BODY_LOCAL_MIN_Y (-1.2F)
#define BODY_LOCAL_MAX_Y ( 1.5F)
#define BODY_LOCAL_MIN_Z (-2.0F)
#define BODY_LOCAL_MAX_Z ( 3.5F)

#define CLAW_SHOULDER_Y (-1.8F)
#define CLAW_SHOULDER_Z (-3.0F)
#define CLAW_ARM_LENGTH (10.0F)   
#define CLAW_SPHERE_RADIUS ( 2.5)  


static AABB buildWorldAABB(Vec3 pos, Vec3 fwd, Vec3 up, Vec3 right, float lx_min, float lx_max, float ly_min, float ly_max, float lz_min, float lz_max){
    float lx[2], ly[2], lz[2];
    AABB  box;
    int   ix, iy, iz, first;

    lx[0] = lx_min; lx[1] = lx_max;
    ly[0] = ly_min; ly[1] = ly_max;
    lz[0] = lz_min; lz[1] = lz_max;

    first = 1;
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

static Vec3 computeClawWorldPos(void) {
    Vec3 ship_pos = getShipPosition();
    Vec3 fwd = getShipForward();
    Vec3 up = getShipUp();
    Vec3 claw_pos;

    claw_pos.x = ship_pos.x + fwd.x * (CLAW_SHOULDER_Z + CLAW_ARM_LENGTH) + up.x * CLAW_SHOULDER_Y;
    claw_pos.y = ship_pos.y + fwd.y * (CLAW_SHOULDER_Z + CLAW_ARM_LENGTH) + up.y * CLAW_SHOULDER_Y;
    claw_pos.z = ship_pos.z + fwd.z * (CLAW_SHOULDER_Z + CLAW_ARM_LENGTH) + up.z * CLAW_SHOULDER_Y;

    return claw_pos;
}

/* =========================================================
 * aabbVsAsteroidBVH
 * ========================================================= */
static int aabbVsAsteroidBVH(AABB box, const BVHNode* node){
    if (node == NULL){
        return 0;
    }
    if (!aabbVsAABB(box, node->aabb)){
        return 0;
    }
    if (bvhNodeIsLeaf(node)) {
        return 1;
    }
    if (aabbVsAsteroidBVH(box, node->left)){
        return 1;
    }
    if (aabbVsAsteroidBVH(box, node->right)){
        return 1;
    }
    return 0;
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
 * checkCollision  —  ship BODY vs asteroids (causes DAMAGE)
 * ========================================================= */
extern int checkCollision(Vec3 ship_pos) {
    const Asteroid* asteroids = getAsteroids();
    const int count = getAsteroidCount();
    AABB ship_box;
    Vec3 fwd, up, right;
    int i;
    
    if (!asteroids || count <= 0) {
        return 0;
    }
    
    fwd = getShipForward();
    up = getShipUp();
    right = getShipRight();
    
    ship_box = buildWorldAABB(ship_pos, fwd, up, right,
                              BODY_LOCAL_MIN_X, BODY_LOCAL_MAX_X,
                              BODY_LOCAL_MIN_Y, BODY_LOCAL_MAX_Y,
                              BODY_LOCAL_MIN_Z, BODY_LOCAL_MAX_Z);
    
    for (i = 0; i < count; i++) {
        const Asteroid* ast = &asteroids[i];

        if (!ast->bvh){
            continue;
        }
        if (!aabbVsAABB(ship_box, ast->bvh->aabb)){
            continue;
        }
        if (aabbVsAsteroidBVH(ship_box, ast->bvh)) {
            return i + 1;
        }
    }
    return 0;
}

/* =========================================================
 * checkProjectileCollision
 * ========================================================= */
extern int checkProjectileCollision(int* hit_asteroid_idx, int* hit_projectile_idx) {
    const Asteroid* asteroids = getAsteroids();
    const int ast_count = getAsteroidCount();
    const Projectile* projs = getProjectiles();
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
    const int count = getAsteroidCount();
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

        if (!ast->bvh || !ast->barycenter_array){
            continue;
        }
        if (!sphereVsAABB(claw_pos, CLAW_SPHERE_RADIUS, ast->bvh->aabb)){
            continue;
        }
        if (bvhSphereQuery(ast->bvh, ast->barycenter_array, claw_pos, CLAW_SPHERE_RADIUS)){
            return i;
        }
    }
    return -1;
}