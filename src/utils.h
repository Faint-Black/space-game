#ifndef UTILS_H
#define UTILS_H

typedef struct Vec3 {
    float x;
    float y;
    float z;
} Vec3;

/**
 * @brief Computes the sum of 2 vectors.
 *
 * @return Vector + Vector sum.
 */
extern Vec3 vec3AddVector(Vec3 a, Vec3 b);

/**
 * @brief Sums all vector components with a scalar.
 *
 * @return Vector + Scalar sum.
 */
extern Vec3 vec3AddScalar(Vec3 vec, float scalar);

#endif /* UTILS_H */
