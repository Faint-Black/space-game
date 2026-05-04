#include "utils.h"

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
