#include "utils.h"

extern Vec3 vec3AddVector(Vec3 a, Vec3 b) {
    return (Vec3){
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
}

extern Vec3 vec3AddScalar(Vec3 vec, float scalar) {
    return (Vec3){
        vec.x + scalar,
        vec.y + scalar,
        vec.z + scalar,
    };
}
