#include "collision.h"
#include "utils.h"

//Expands the bounding box limits to include the given point
static void aabbExpandPoint(AABB *box, Vec3 p) {
    //Update min bounds if point is outside
    if (p.x < box->min.x) box->min.x = p.x;
    if (p.y < box->min.y) box->min.y = p.y;
    if (p.z < box->min.z) box->min.z = p.z;
    //Update max bounds if point is outside
    if (p.x > box->max.x) box->max.x = p.x;
    if (p.y > box->max.y) box->max.y = p.y;
    if (p.z > box->max.z) box->max.z = p.z;
}

//Computes the AABB that bounds a single triangle face
static AABB aabbFromFace(const TriangleFace *f) {
    AABB box;
    box.min = box.max = f->v[0].position; //Initialize box to the first vertex position
    //Expand box to include the other two vertices of the face
    aabbExpandPoint(&box, f->v[1].position);
    aabbExpandPoint(&box, f->v[2].position);
    return box;
}

//Creates a new AABB that is the union of two AABBs
static AABB aabbUnion(AABB a, AABB b) {
    AABB result;
    //check  the smaller min bounds
    result.min.x = a.min.x < b.min.x ? a.min.x : b.min.x;
    result.min.y = a.min.y < b.min.y ? a.min.y : b.min.y;
    result.min.z = a.min.z < b.min.z ? a.min.z : b.min.z;
    //check the larger max bounds
    result.max.x = a.max.x > b.max.x ? a.max.x : b.max.x;
    result.max.y = a.max.y > b.max.y ? a.max.y : b.max.y;
    result.max.z = a.max.z > b.max.z ? a.max.z : b.max.z;
    return result;
}

// 
extern AABB computeAABBFromFaces(const TriangleFace *faces, int count){
    AABB box;
    int i;
}

/* TODO */
extern int checkCollision(Vec3 ship_pos) {
    (void)ship_pos;
    return 0;
}
