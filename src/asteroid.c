#include "asteroid.h"
#include "aabb_bvh.h"
#include "render.h"
#include "utils.h"
#include <GL/gl.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>

#define MAX_ASTEROID_VELOCITY 5.0F
#define MAX_POINTS_PER_LEAF 4

/* internal global variables */
static Asteroid* asteroid_array = NULL;
static int asteroid_count = 0;
static GLuint asteroid_texture_id = 0;

static Vec3 polarVec3(float theta, float phi) {
    Vec3 result;
    result.x = sin(theta) * cos(phi);
    result.y = cos(theta);
    result.z = sin(theta) * sin(phi);
    return result;
}

static Vertex makeSphereVertex(float theta, float phi) {
    Vertex result;
    result.position = polarVec3(theta, phi);
    return result;
}

static void appendTriangle(TriangleFace* faces, int* index, Vertex v0, Vertex v1, Vertex v2) {
    faces[*index].v[0] = v0;
    faces[*index].v[1] = v1;
    faces[*index].v[2] = v2;
    (*index)++;
}

static Mesh generateSphereMesh(int stacks, int sections) {
    Mesh result;

    int f = 0;
    int i = 0;
    int j = 0;

    float theta_step = PI / stacks;
    float phi_step = 2.0F * (float)PI / (float)sections;

    result.face_count = 2 * sections * stacks;
    result.faces = (TriangleFace*)malloc(result.face_count * sizeof(TriangleFace));

    for (i = 0; i <= stacks; i++) {
        float theta1 = (float)i * theta_step;
        float theta2 = (float)(i + 1) * theta_step;

        /* north pole case */
        if (i == 0) {
            const Vertex north_pole = makeSphereVertex(0.0F, 0.0F);

            for (j = 0; j < sections; j++) {
                const float phi1 = (float)j * phi_step;
                const float phi2 = (float)(j + 1) * phi_step;

                const Vertex v1 = makeSphereVertex(theta2, phi1);
                const Vertex v2 = makeSphereVertex(theta2, phi2);

                appendTriangle(result.faces, &f, v1, north_pole, v2);
            }
        }
        /* south pole case */
        else if (i == stacks) {
            const Vertex south_pole = makeSphereVertex(PI, 0.0F);

            for (j = 0; j < sections; j++) {
                const float phi1 = (float)j * phi_step;
                const float phi2 = (float)(j + 1) * phi_step;

                const Vertex v1 = makeSphereVertex(theta1, phi1);
                const Vertex v2 = makeSphereVertex(theta1, phi2);

                appendTriangle(result.faces, &f, v1, south_pole, v2);
            }
        }
        /* middle stack cases */
        else {
            for (j = 0; j < sections; j++) {
                const float phi1 = (float)j * phi_step;
                const float phi2 = (float)(j + 1) * phi_step;

                const Vertex v00 = makeSphereVertex(theta1, phi1);
                const Vertex v01 = makeSphereVertex(theta1, phi2);
                const Vertex v10 = makeSphereVertex(theta2, phi1);
                const Vertex v11 = makeSphereVertex(theta2, phi2);

                appendTriangle(result.faces, &f, v00, v01, v10);
                appendTriangle(result.faces, &f, v11, v10, v01);
            }
        }
    }

    return result;
}

static Mesh resolveMiscMeshData(Mesh mesh) {
    int i;

    /* resolve normals */
    for (i = 0; i < mesh.face_count; i++) {
        const Vec3 v1 = vec3SubVector(mesh.faces[i].v[1].position, mesh.faces[i].v[0].position);
        const Vec3 v2 = vec3SubVector(mesh.faces[i].v[2].position, mesh.faces[i].v[0].position);
        const Vec3 normal = vec3Cross(v1, v2);
        mesh.faces[i].v[0].normal = normal;
        mesh.faces[i].v[1].normal = normal;
        mesh.faces[i].v[2].normal = normal;
    }

    /* resolve albedo */
    for (i = 0; i < mesh.face_count; i++) {
        mesh.faces[i].v[0].albedo = vec3(1.0F, 1.0F, 1.0F);
        mesh.faces[i].v[1].albedo = vec3(1.0F, 1.0F, 1.0F);
        mesh.faces[i].v[2].albedo = vec3(1.0F, 1.0F, 1.0F);
    }

    /* resolve texture coords */
    for (i = 0; i < mesh.face_count; i++) {
        mesh.faces[i].v[0].texture = vec3(0.0F, 1.0F, 1337.42F);
        mesh.faces[i].v[1].texture = vec3(1.0F, 0.0F, 1337.42F);
        mesh.faces[i].v[2].texture = vec3(0.0F, 0.0F, 1337.42F);
    }

    return mesh;
}

static Vec3* generateBarycenters(Mesh mesh, Vec3 initial_pos) {
    /* each triangular face has its own barycenter */
    Vec3* points = malloc(mesh.face_count * sizeof(Vec3));

    int i;
    for (i = 0; i < mesh.face_count; i++) {
        const Vec3 v1 = mesh.faces[i].v[0].position;
        const Vec3 v2 = mesh.faces[i].v[1].position;
        const Vec3 v3 = mesh.faces[i].v[2].position;
        const Vec3 model_coord = vec3BarycenterTri(v1, v2, v3);
        const Vec3 world_coord = vec3AddVector(model_coord, initial_pos);
        points[i] = world_coord;
    }

    return points;
}

static int* generateBvhIndices(int count) {
    int* result = malloc(count * sizeof(int));
    int i;
    for (i = 0; i < count; i++) {
        result[i] = i;
    }
    return result;
}

static Asteroid initAsteroid(void) {
    Asteroid result;

    result.mass = 1.0F;
    result.position = vec3MulScalar(vec3Random(), randNormalizedFloat() * INTERACTION_BOUNDS_RADIUS);
    result.velocity = vec3MulScalar(vec3Random(), MAX_ASTEROID_VELOCITY);

    result.mesh = resolveMiscMeshData(generateSphereMesh(20, 20));
    result.texture_id = asteroid_texture_id;

    result.barycenter_array = generateBarycenters(result.mesh, result.position);
    result.barycenter_count = result.mesh.face_count;

    result.bvh_index_count = result.barycenter_count;
    result.bvh_index_array = generateBvhIndices(result.bvh_index_count);
    result.bvh = bvhBuild(result.barycenter_array, result.bvh_index_array, result.bvh_index_count, MAX_POINTS_PER_LEAF);

    return result;
}

/* frees individual asteroid allocated data */
static void deinitAsteroid(Asteroid asteroid) {
    if (asteroid.mesh.faces != NULL) {
        free(asteroid.mesh.faces);
    }
    if (asteroid.barycenter_array != NULL) {
        free(asteroid.barycenter_array);
    }
    if (asteroid.bvh_index_array != NULL) {
        free(asteroid.bvh_index_array);
    }
    if (asteroid.bvh != NULL) {
        bvhFree(asteroid.bvh);
    }
}

static void updateBVHpos(BVHNode* node, Vec3 vel) {
    node->aabb.min = vec3AddVector(node->aabb.min, vel);
    node->aabb.max = vec3AddVector(node->aabb.max, vel);
    if (node->left != NULL) updateBVHpos(node->left, vel);
    if (node->right != NULL) updateBVHpos(node->right, vel);
}

static void updateAsteroid(Asteroid* asteroid, float dt) {
    int i;
    const Vec3 vel = vec3MulScalar(asteroid->velocity, dt);

    asteroid->position = vec3AddVector(asteroid->position, vel);
    for (i = 0; i < asteroid->barycenter_count; i++) {
        asteroid->barycenter_array[i] = vec3AddVector(asteroid->barycenter_array[i], vel);
    }
    updateBVHpos(asteroid->bvh, vel);
}

extern void updateAsteroids(float dt) {
    int i;
    for (i = 0; i < asteroid_count; i++) {
        updateAsteroid(&asteroid_array[i], dt);
    }
}

extern void initAsteroids(int max_asteroid_count) {
    int i;
    asteroid_texture_id = loadTexture("./data/asteroid-texture.jpg");
    asteroid_count = max_asteroid_count;
    asteroid_array = (Asteroid*)malloc(asteroid_count * sizeof(Asteroid));
    for (i = 0; i < asteroid_count; i++) {
        asteroid_array[i] = initAsteroid();
    }
}

extern void deinitAsteroids(void) {
    if (asteroid_array != NULL) {
        int i;
        for (i = 0; i < asteroid_count; i++) {
            deinitAsteroid(asteroid_array[i]);
        }
        free(asteroid_array);
    }
    asteroid_count = 0;
}

extern const Asteroid* getAsteroids(void) {
    return asteroid_array;
}

extern int getAsteroidCount(void) {
    return asteroid_count;
}
