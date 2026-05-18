#ifndef UTILS_H
#define UTILS_H

typedef struct Vec3 {
    float x;
    float y;
    float z;
} Vec3;

/**
 * @brief Creates a new vector from individual components.
 *
 * @return Vector(x, y, z)
 */
extern Vec3 vec3(float x, float y, float z);

/**
 * @brief Creates a null vector.
 *
 * @return Vector(0, 0, 0)
 */
extern Vec3 vec3Null(void);

/**
 * @brief Computes the sum of 2 vectors.
 *
 * @return Vector((a.x + b.x), (a.y + b.y), (a.z + b.z))
 */
extern Vec3 vec3AddVector(Vec3 a, Vec3 b);

/**
 * @brief Sums all vector components by a scalar.
 *
 * @return Vector((x + scalar), (y + scalar), (z + scalar))
 */
extern Vec3 vec3Scale(Vec3 vec, float scalar);

/**
 * @brief Computes the difference of 2 vectors.
 *
 * @return Vector((a.x - b.x), (a.y - b.y), (a.z - b.z))
 */
extern Vec3 vec3SubVector(Vec3 a, Vec3 b);

/**
 * @brief Subtracts all vector components by a scalar.
 *
 * @return Vector((x - scalar), (y - scalar), (z - scalar))
 */
extern Vec3 vec3SubScalar(Vec3 vec, float scalar);

/**
 * @brief Multiplies 2 vectors, element-wise.
 *
 * @return Vector((a.x * b.x), (a.y * b.y), (a.z * b.z))
 */
extern Vec3 vec3HadamardMulVector(Vec3 a, Vec3 b);

/**
 * @brief Multiplies all vector components by a scalar.
 *
 * @return Vector((x * scalar), (y * scalar), (z * scalar))
 */
extern Vec3 vec3MulScalar(Vec3 vec, float scalar);

/**
 * @brief Divides 2 vectors, element-wise. Throws error if any B component is zero.
 *
 * @return Vector((a.x / b.x), (a.y / b.y), (a.z / b.z))
 */
extern Vec3 vec3HadamardDivVector(Vec3 a, Vec3 b);

/**
 * @brief Divides all vector components by a scalar. Throws error if scalar is zero.
 *
 * @return Vector((x / scalar), (y / scalar), (z / scalar))
 */
extern Vec3 vec3DivScalar(Vec3 vec, float scalar);

/**
 * @brief Reduces a vector into a scalar (sum).
 *
 * @return x + y + z
 */
extern float vec3Reduce(Vec3 vec);

/**
 * @brief Computes the length/size/magnitude of a vector.
 *
 * @return ||vec||
 */
extern float vec3Magnitude(Vec3 vec);

/**
 * @brief Computes the dot product of two vectors.
 *
 * @return dot(A, B)
 */
extern float vec3Dot(Vec3 a, Vec3 b);

/**
 * @brief Computes the cross product of two vectors.
 *
 * @return cross(A, B)
 */
extern Vec3 vec3Cross(Vec3 a, Vec3 b);

/**
 * @brief Normalizes a vector.
 *
 * @return \frac{ vec }{ ||vec|| }
 */
extern Vec3 vec3Normalize(Vec3 vec);

/**
 * @brief Calculates the distance between two vectors.
 *
 * @return ||A->B||
 */
extern float vec3Distance(Vec3 a, Vec3 b);

/**
 * @brief Calculates the barycenter of the input vectors.
 *
 * @param vectors The vector array.
 * @param count The length of the vector array.
 *
 * @return Vector(average(vectors.x), average(vectors.y), average(vectors.z))
 */
extern Vec3 vec3BarycenterPolygon(const Vec3* vectors, int count);

/**
 * @brief Convenience wrapper of vec3BarycenterPolygon() for 2 vectors.
 *
 * @return barycenter([v1, v2])
 */
extern Vec3 vec3BarycenterLine(Vec3 v1, Vec3 v2);

/**
 * @brief Convenience wrapper of vec3BarycenterPolygon() for 3 vectors.
 *
 * @return barycenter([v1, v2, v3])
 */
extern Vec3 vec3BarycenterTri(Vec3 v1, Vec3 v2, Vec3 v3);

/**
 * @brief Convenience wrapper of vec3BarycenterPolygon() for 4 vectors.
 *
 * @return barycenter([v1, v2, v3, v4])
 */
extern Vec3 vec3BarycenterQuad(Vec3 v1, Vec3 v2, Vec3 v3, Vec3 v4);

#endif /* UTILS_H */
