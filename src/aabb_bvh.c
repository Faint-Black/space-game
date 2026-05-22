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
    result.min.x = (a.min.x < b.min.x) ? a.min.x : b.min.x;
    result.min.y = (a.min.y < b.min.y) ? a.min.y : b.min.y;
    result.min.z = (a.min.z < b.min.z) ? a.min.z : b.min.z;

    result.max.x = (a.max.x > b.max.x) ? a.max.x : b.max.x;
    result.max.y = (a.max.y > b.max.y) ? a.max.y : b.max.y;
    result.max.z = (a.max.z > b.max.z) ? a.max.z : b.max.z;
    return result;
}

/* Internal helper function to partition indices for the BVH */
static int splitIndicesBVH(const Vec3* points, int* indices, int num_indices, int axis, float split_pos) {
    int i = 0;
    int j = num_indices - 1;
    while (i <= j) {
        float val = 0.0F;
        if (axis == 0)
            val = points[indices[i]].x;
        else if (axis == 1)
            val = points[indices[i]].y;
        else
            val = points[indices[i]].z;

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
    if (extent.y > extent.x && extent.y > extent.z)
        axis = 1;
    else if (extent.z > extent.x && extent.z > extent.y)
        axis = 2;

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

/**
 * @brief Expands the bounding box limits to include the given point.
 */
static void aabbExpandPoint(AABB* box, Vec3 p) {
    if (p.x < box->min.x) box->min.x = p.x;
    if (p.y < box->min.y) box->min.y = p.y;
    if (p.z < box->min.z) box->min.z = p.z;
    if (p.x > box->max.x) box->max.x = p.x;
    if (p.y > box->max.y) box->max.y = p.y;
    if (p.z > box->max.z) box->max.z = p.z;
}

/**
 * @brief Computes the AABB that bounds a single triangle face.
 */
static AABB aabbFromFace(const TriangleFace* f) {
    AABB box;
    box.min = f->v[0].position;
    box.max = f->v[0].position;
    aabbExpandPoint(&box, f->v[1].position);
    aabbExpandPoint(&box, f->v[2].position);
    return box;
}

/**
 * @brief Returns the smallest AABB that fully contains both input boxes.
 */
static AABB aabbUnion(AABB a, AABB b) {
    AABB result;
    result.min.x = (a.min.x < b.min.x) ? a.min.x : b.min.x;
    result.min.y = (a.min.y < b.min.y) ? a.min.y : b.min.y;
    result.min.z = (a.min.z < b.min.z) ? a.min.z : b.min.z;
    result.max.x = (a.max.x > b.max.x) ? a.max.x : b.max.x;
    result.max.y = (a.max.y > b.max.y) ? a.max.y : b.max.y;
    result.max.z = (a.max.z > b.max.z) ? a.max.z : b.max.z;
    return result;
}

extern AABB computeAABBFromFaces(const TriangleFace* faces, int count) {
    AABB box;
    int i;

    box.min = vec3Null();
    box.max = vec3Null();

    if (faces == NULL || count <= 0) {
        return box;
    }

    box = aabbFromFace(&faces[0]);
    for (i = 1; i < count; i++) {
        box = aabbUnion(box, aabbFromFace(&faces[i]));
    }
    return box;
}
