#include "utils.h"
#include <assert.h>
#include <math.h>

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

extern Vec3 vec3Create(float x, float y, float z) {
    Vec3 v = {x, y, z};
    return v;
}

extern Vec3 vec3AddVector(Vec3 a, Vec3 b) {
    return vec3Create(a.x + b.x, a.y + b.y, a.z + b.z);
}

extern Vec3 vec3Scale(Vec3 vec, float scalar) {
    return vec3Create(vec.x * scalar, vec.y * scalar, vec.z * scalar);
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
