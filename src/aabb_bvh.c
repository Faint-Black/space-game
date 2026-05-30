#include "aabb_bvh.h"
#include "utils.h"
#include <stdlib.h>

extern AABB aabbComputeFromPoints(const Vec3* points, int count) {
    AABB result;
    int i;

    if (count == 0) {
        result.min = vec3Null();
        result.max = vec3Null();
        return result;
    }

    result.min = points[0];
    result.max = points[0];
    for (i = 1; i < count; i++) {
        if (points[i].x < result.min.x) result.min.x = points[i].x;
        if (points[i].y < result.min.y) result.min.y = points[i].y;
        if (points[i].z < result.min.z) result.min.z = points[i].z;

        if (points[i].x > result.max.x) result.max.x = points[i].x;
        if (points[i].y > result.max.y) result.max.y = points[i].y;
        if (points[i].z > result.max.z) result.max.z = points[i].z;
    }

    return result;
}

extern AABB aabbMerge(AABB a, AABB b) {
    AABB result;

    result.min.x = minFloat(a.min.x, b.min.x);
    result.min.y = minFloat(a.min.y, b.min.y);
    result.min.z = minFloat(a.min.z, b.min.z);

    result.max.x = maxFloat(a.max.x, b.max.x);
    result.max.y = maxFloat(a.max.y, b.max.y);
    result.max.z = maxFloat(a.max.z, b.max.z);

    return result;
}

/* Internal helper function to partition indices for the BVH */
static int splitIndicesBVH(const Vec3* points, int* indices, int num_indices, int axis, float split_pos) {
    int i = 0;
    int j = num_indices - 1;
    while (i <= j) {
        float val = 0.0F;
        if (axis == 0) {
            val = points[indices[i]].x;
        } else if (axis == 1) {
            val = points[indices[i]].y;
        } else {
            val = points[indices[i]].z;
        }

        if (val <= split_pos) {
            i++;
        } else {
            int temp = indices[i];
            indices[i] = indices[j];
            indices[j] = temp;
            j--;
        }
    }
    return i;
}

extern BVHNode* bvhBuild(const Vec3* points, int* indices, int num_indices, int max_points_per_leaf) {
    BVHNode* node = (BVHNode*)malloc(sizeof(BVHNode));
    int i;
    Vec3 extent;
    int axis;
    float split_pos;
    int split_idx;

    if (num_indices == 0) {
        node->aabb.min = vec3Null();
        node->aabb.max = vec3Null();
        node->left = NULL;
        node->right = NULL;
        node->num_points = 0;
        node->point_indices = NULL;
        return node;
    }

    node->aabb.min = points[indices[0]];
    node->aabb.max = points[indices[0]];
    for (i = 1; i < num_indices; i++) {
        Vec3 p = points[indices[i]];
        if (p.x < node->aabb.min.x) node->aabb.min.x = p.x;
        if (p.y < node->aabb.min.y) node->aabb.min.y = p.y;
        if (p.z < node->aabb.min.z) node->aabb.min.z = p.z;

        if (p.x > node->aabb.max.x) node->aabb.max.x = p.x;
        if (p.y > node->aabb.max.y) node->aabb.max.y = p.y;
        if (p.z > node->aabb.max.z) node->aabb.max.z = p.z;
    }

    /* If the current node contains the limit of points or fewer, turn it into a leaf node */
    if (num_indices <= max_points_per_leaf) {
        node->left = NULL;
        node->right = NULL;
        node->num_points = num_indices;
        node->point_indices = (int*)malloc(num_indices * sizeof(int));
        for (i = 0; i < num_indices; i++) {
            node->point_indices[i] = indices[i];
        }
        return node;
    }

    /* Find the longest axis to split the space (Axis X=0, Y=1, Z=2) */
    extent = vec3SubVector(node->aabb.max, node->aabb.min);
    axis = 0;
    if (extent.y > extent.x && extent.y > extent.z) {
        axis = 1;
    } else if (extent.z > extent.x && extent.z > extent.y) {
        axis = 2;
    }

    split_pos = 0.0F;
    if (axis == 0) {
        split_pos = node->aabb.min.x + extent.x * 0.5F;
    } else if (axis == 1) {
        split_pos = node->aabb.min.y + extent.y * 0.5F;
    } else {
        split_pos = node->aabb.min.z + extent.z * 0.5F;
    }

    split_idx = splitIndicesBVH(points, indices, num_indices, axis, split_pos);

    /* Fallback in case all points end up on the same side (prevents infinite recursion) */
    if (split_idx == 0 || split_idx == num_indices) {
        split_idx = num_indices / 2;
    }

    node->left = bvhBuild(points, indices, split_idx, max_points_per_leaf);
    node->right = bvhBuild(points, indices + split_idx, num_indices - split_idx, max_points_per_leaf);
    node->num_points = 0;
    node->point_indices = NULL;

    return node;
}

extern void bvhRefit(BVHNode* node, const Vec3* current_points) {
    if (!node) return;

    /* If it's a leaf node, recompute AABB directly from its assigned points */
    if (node->point_indices != NULL && node->num_points > 0) {
        int i;
        node->aabb.min = current_points[node->point_indices[0]];
        node->aabb.max = current_points[node->point_indices[0]];

        for (i = 1; i < node->num_points; i++) {
            Vec3 p = current_points[node->point_indices[i]];
            if (p.x < node->aabb.min.x) node->aabb.min.x = p.x;
            if (p.y < node->aabb.min.y) node->aabb.min.y = p.y;
            if (p.z < node->aabb.min.z) node->aabb.min.z = p.z;

            if (p.x > node->aabb.max.x) node->aabb.max.x = p.x;
            if (p.y > node->aabb.max.y) node->aabb.max.y = p.y;
            if (p.z > node->aabb.max.z) node->aabb.max.z = p.z;
        }
    }
    /* If it's an internal node, recursively update children, then merge their AABBs */
    else {
        bvhRefit(node->left, current_points);
        bvhRefit(node->right, current_points);

        if (node->left && node->right) {
            node->aabb = aabbMerge(node->left->aabb, node->right->aabb);
        } else if (node->left) {
            node->aabb = node->left->aabb;
        } else if (node->right) {
            node->aabb = node->right->aabb;
        }
    }
}

extern void bvhFree(BVHNode* node) {
    if (node != NULL) {
        bvhFree(node->left);
        bvhFree(node->right);
        if (node->point_indices != NULL) {
            free(node->point_indices);
        }
        free(node);
    }
}

extern int bvhNodeIsLeaf(const BVHNode* node) {
    return (node->left == NULL && node->right == NULL);
}

#define ABSF(x) ((x) < 0.0F ? -(x) : (x))

/* ================================================================
 * aabbVsAABB
 * ================================================================ */
extern int aabbVsAABB(AABB a, AABB b) {
    if (a.max.x < b.min.x || a.min.x > b.max.x) {
        return 0;
    }
    if (a.max.y < b.min.y || a.min.y > b.max.y) {
        return 0;
    }
    if (a.max.z < b.min.z || a.min.z > b.max.z) {
        return 0;
    }
    return 1;
}

/* ================================================================
 * triVsAABB
 * ================================================================ */
extern int triVsAABB(Vec3 v0, Vec3 v1, Vec3 v2, AABB box) {
    Vec3  bc, be;
    Vec3  f0, f1, f2;
    Vec3  n;
    float p0, p1, p2, r;
    float abs_f0_z, abs_f0_y, abs_f0_x;
    float abs_f1_z, abs_f1_y, abs_f1_x;
    float abs_f2_z, abs_f2_y, abs_f2_x;

    #define TSAT_X(ey, ez, fay, faz) \
        p0 =  (ey)*v0.y - (ez)*v0.z; \
        p2 =  (ey)*v2.y - (ez)*v2.z; \
        r  = (fay)*be.y + (faz)*be.z; \
        if (p0 < p2) { \
            if (p0 > r || p2 < -r) return 0; \
        } else { \
            if (p2 > r || p0 < -r) return 0; \
        }

    #define TSAT_Y(ex, ez, fax, faz) \
        p0 = -(ex)*v0.x + (ez)*v0.z; \
        p2 = -(ex)*v2.x + (ez)*v2.z; \
        r  = (fax)*be.x + (faz)*be.z; \
        if (p0 < p2) { \
            if (p0 > r || p2 < -r) return 0; \
        } else { \
            if (p2 > r || p0 < -r) return 0; \
        }

    #define TSAT_Z(ex, ey, fax, fay) \
        p0 = (ex)*v0.x - (ey)*v0.y; \
        p1 = (ex)*v1.x - (ey)*v1.y; \
        r  = (fax)*be.x + (fay)*be.y; \
        if (p0 < p1) { \
            if (p0 > r || p1 < -r) return 0; \
        } else { \
            if (p1 > r || p0 < -r) return 0; \
        }

    bc.x = (box.min.x + box.max.x) * 0.5F;
    bc.y = (box.min.y + box.max.y) * 0.5F;
    bc.z = (box.min.z + box.max.z) * 0.5F;
    be.x = (box.max.x - box.min.x) * 0.5F;
    be.y = (box.max.y - box.min.y) * 0.5F;
    be.z = (box.max.z - box.min.z) * 0.5F;

    v0 = vec3SubVector(v0, bc);
    v1 = vec3SubVector(v1, bc);
    v2 = vec3SubVector(v2, bc);

    f0 = vec3SubVector(v1, v0);
    f1 = vec3SubVector(v2, v1);
    f2 = vec3SubVector(v0, v2);

    if (f0.z < 0.0F) {
        abs_f0_z = -f0.z;
    } else {
        abs_f0_z = f0.z;
    }
    
    if (f0.y < 0.0F) {
        abs_f0_y = -f0.y;
    } else {
        abs_f0_y = f0.y;
    }
    
    if (f0.x < 0.0F) {
        abs_f0_x = -f0.x;
    } else {
        abs_f0_x = f0.x;
    }

    if (f1.z < 0.0F) {
        abs_f1_z = -f1.z;
    } else {
        abs_f1_z = f1.z;
    }
    
    if (f1.y < 0.0F) {
        abs_f1_y = -f1.y;
    } else {
        abs_f1_y = f1.y;
    }
    
    if (f1.x < 0.0F) {
        abs_f1_x = -f1.x;
    } else {
        abs_f1_x = f1.x;
    }

    if (f2.z < 0.0F) {
        abs_f2_z = -f2.z;
    } else {
        abs_f2_z = f2.z;
    }
    
    if (f2.y < 0.0F) {
        abs_f2_y = -f2.y;
    } else {
        abs_f2_y = f2.y;
    }
    
    if (f2.x < 0.0F) {
        abs_f2_x = -f2.x;
    } else {
        abs_f2_x = f2.x;
    }

    TSAT_X( f0.z, f0.y, abs_f0_z, abs_f0_y)
    TSAT_Y(-f0.z, f0.x, abs_f0_z, abs_f0_x)
    TSAT_Z( f0.y,-f0.x, abs_f0_y, abs_f0_x)

    TSAT_X( f1.z, f1.y, abs_f1_z, abs_f1_y)
    TSAT_Y(-f1.z, f1.x, abs_f1_z, abs_f1_x)
    TSAT_Z( f1.y,-f1.x, abs_f1_y, abs_f1_x)

    TSAT_X( f2.z, f2.y, abs_f2_z, abs_f2_y)
    TSAT_Y(-f2.z, f2.x, abs_f2_z, abs_f2_x)
    TSAT_Z( f2.y,-f2.x, abs_f2_y, abs_f2_x)

    #undef TSAT_X
    #undef TSAT_Y
    #undef TSAT_Z

    if (v0.x > be.x && v1.x > be.x && v2.x > be.x) {
        return 0;
    }
    if (v0.x < -be.x && v1.x < -be.x && v2.x < -be.x) {
        return 0;
    }
    if (v0.y > be.y && v1.y > be.y && v2.y > be.y) {
        return 0;
    }
    if (v0.y < -be.y && v1.y < -be.y && v2.y < -be.y) {
        return 0;
    }
    if (v0.z > be.z && v1.z > be.z && v2.z > be.z) {
        return 0;
    }
    if (v0.z < -be.z && v1.z < -be.z && v2.z < -be.z) {
        return 0;
    }

    n = vec3Cross(f0, f1);
    
    if (n.x < 0.0F) {
        abs_f0_x = -n.x;
    } else {
        abs_f0_x = n.x;
    }
    
    if (n.y < 0.0F) {
        abs_f0_y = -n.y;
    } else {
        abs_f0_y = n.y;
    }
    
    if (n.z < 0.0F) {
        abs_f0_z = -n.z;
    } else {
        abs_f0_z = n.z;
    }
    
    r = abs_f0_x * be.x + abs_f0_y * be.y + abs_f0_z * be.z;
    
    if (vec3Dot(n, v0) < 0.0F) {
        p0 = -vec3Dot(n, v0);
    } else {
        p0 = vec3Dot(n, v0);
    }
    
    if (p0 > r) {
        return 0;
    }
    return 1;
}