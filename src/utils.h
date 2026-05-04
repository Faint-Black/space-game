#ifndef UTILS_H
#define UTILS_H

typedef struct Vec3 {
    float x;
    float y;
    float z;
} Vec3;


extern Vec3 vec3Create(float x, float y, float z);

/**
 * @brief Computes the sum of 2 vectors.
 *
 * @return Vector + Vector sum.
 */
extern Vec3 vec3AddVector(Vec3 a, Vec3 b);

/**
 * @brief Multiplies all vector components by a scalar.
 *
 * @return Vector * Scalar sum.
 */
extern Vec3 vec3Scale(Vec3 vec, float scalar);

#endif /* UTILS_H */
