#include "utils.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

extern float randNormalizedFloat(void) {
    return (float)rand() / (float)RAND_MAX;
}

extern float randCenteredFloat(void) {
    const float normal = randNormalizedFloat();
    return (rand() % 2 == 0) ? (normal) : (-normal);
}

extern float randRangedFloat(float min, float max) {
    return min + randNormalizedFloat() * (max - min);
}

extern Vec3 vec3(float x, float y, float z) {
    Vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

extern Vec3 vec3Null(void) {
    Vec3 result;
    result.x = result.y = result.z = 0.0F;
    return result;
}

extern Vec3 vec3Random(void) {
    const double phi = 2.0 * PI * randNormalizedFloat();
    const float cos_theta = randCenteredFloat();
    const float sin_theta = (float)sqrt(1.0 - cos_theta * cos_theta);
    return vec3(sin_theta * (float)cos(phi), sin_theta * (float)sin(phi), cos_theta);
}

extern Vec3 vec3AddVector(Vec3 a, Vec3 b) {
    Vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

extern Vec3 vec3AddScalar(Vec3 vec, float scalar) {
    Vec3 result;
    result.x = vec.x + scalar;
    result.y = vec.y + scalar;
    result.z = vec.z + scalar;
    return result;
}

extern Vec3 vec3SubVector(Vec3 a, Vec3 b) {
    Vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

extern Vec3 vec3SubScalar(Vec3 vec, float scalar) {
    Vec3 result;
    result.x = vec.x - scalar;
    result.y = vec.y - scalar;
    result.z = vec.z - scalar;
    return result;
}

extern Vec3 vec3HadamardMulVector(Vec3 a, Vec3 b) {
    Vec3 result;
    result.x = a.x * b.x;
    result.y = a.y * b.y;
    result.z = a.z * b.z;
    return result;
}

extern Vec3 vec3MulScalar(Vec3 vec, float scalar) {
    Vec3 result;
    result.x = vec.x * scalar;
    result.y = vec.y * scalar;
    result.z = vec.z * scalar;
    return result;
}

extern Vec3 vec3HadamardDivVector(Vec3 a, Vec3 b) {
    Vec3 result;
    assert(b.x != 0.0F);
    assert(b.y != 0.0F);
    assert(b.z != 0.0F);
    result.x = a.x / b.x;
    result.y = a.y / b.y;
    result.z = a.z / b.z;
    return result;
}

extern Vec3 vec3DivScalar(Vec3 vec, float scalar) {
    Vec3 result;
    assert(scalar != 0.0F);
    result.x = vec.x / scalar;
    result.y = vec.y / scalar;
    result.z = vec.z / scalar;
    return result;
}

extern float vec3Reduce(Vec3 vec) {
    return vec.x + vec.y + vec.z;
}

extern float vec3Magnitude(Vec3 vec) {
    const double x2 = (vec.x * vec.x);
    const double y2 = (vec.y * vec.y);
    const double z2 = (vec.z * vec.z);
    const double result = sqrt(x2 + y2 + z2);
    return (float)result;
}

extern float vec3Dot(Vec3 a, Vec3 b) {
    return vec3Reduce(vec3HadamardMulVector(a, b));
}

extern Vec3 vec3Cross(Vec3 a, Vec3 b) {
    Vec3 result;
    result.x = (a.y * b.z) - (a.z * b.y);
    result.y = (a.z * b.x) - (a.x * b.z);
    result.z = (a.x * b.y) - (a.y * b.x);
    return result;
}

extern Vec3 vec3Normalize(Vec3 vec) {
    return vec3DivScalar(vec, vec3Magnitude(vec));
}

extern float vec3Distance(Vec3 a, Vec3 b) {
    const Vec3 delta = vec3SubVector(b, a);
    return vec3Magnitude(delta);
}

extern Vec3 vec3BarycenterPolygon(const Vec3* vectors, int count) {
    int i;
    Vec3 accumulator = vec3Null();
    for (i = 0; i < count; i++) {
        accumulator = vec3AddVector(accumulator, vectors[i]);
    }
    return vec3DivScalar(accumulator, (float)count);
}

extern Vec3 vec3BarycenterLine(Vec3 v1, Vec3 v2) {
    Vec3 vectors[2];
    vectors[0] = v1;
    vectors[1] = v2;
    return vec3BarycenterPolygon(vectors, 2);
}

extern Vec3 vec3BarycenterTri(Vec3 v1, Vec3 v2, Vec3 v3) {
    Vec3 vectors[3];
    vectors[0] = v1;
    vectors[1] = v2;
    vectors[2] = v3;
    return vec3BarycenterPolygon(vectors, 3);
}

extern Vec3 vec3BarycenterQuad(Vec3 v1, Vec3 v2, Vec3 v3, Vec3 v4) {
    Vec3 vectors[4];
    vectors[0] = v1;
    vectors[1] = v2;
    vectors[2] = v3;
    vectors[3] = v4;
    return vec3BarycenterPolygon(vectors, 4);
}

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
        float val = 0.0f;
        if (axis == 0) val = points[indices[i]].x;
        else if (axis == 1) val = points[indices[i]].y;
        else val = points[indices[i]].z;

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
    if (extent.y > extent.x && extent.y > extent.z) axis = 1;
    else if (extent.z > extent.x && extent.z > extent.y) axis = 2;

    split_pos = 0.0f;
    if (axis == 0) split_pos = node->aabb.min.x + extent.x * 0.5f;
    else if (axis == 1) split_pos = node->aabb.min.y + extent.y * 0.5f;
    else split_pos = node->aabb.min.z + extent.z * 0.5f;

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
    if (!node) return;
    bvhFree(node->left);
    bvhFree(node->right);
    if (node->point_indices != NULL) {
        free(node->point_indices);
    }
    free(node);
}













