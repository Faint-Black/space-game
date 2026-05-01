#include "utils.h"

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
