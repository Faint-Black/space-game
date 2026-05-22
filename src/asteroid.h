#ifndef ASTEROID_H
#define ASTEROID_H

#include "render.h"
#include "utils.h"
#include <GL/gl.h>

typedef struct Asteroid {
    /* Graphical Vertex data */
    Mesh mesh;
    GLuint texture_id;
    /* Gameplay data */
    float mass;
    Vec3 position;
    Vec3 velocity;
} Asteroid;

/**
 * @brief Initializes internal state of asteroids. Must be called once before game begins.
 * This also loads the asteroid texture into OpenGL.
 */
extern void initAsteroids(int max_asteroid_count);

/**
 * @brief Deinitializes internal state of asteroids. Must be called after game is finished.
 */
extern void deinitAsteroids(void);

/**
 * @brief Fetches from global-state the constant reference to the asteroid entity array.
 *
 * @return Asteroid array.
 */
extern const Asteroid* getAsteroids(void);

/**
 * @brief Fetches from global-state the length of the asteroid entity array.
 *
 * @return Asteroid array length.
 */
extern int getAsteroidCount(void);

/**
 * @brief Generates a list of world points (based on the barycenter of each mesh face).
 * Note: Remember to 'free()' the returned pointer after use.
 * @param asteroid Pointer to the asteroid in question.
 * @param out_count Output variable that will receive the amount of generated points.
 * @return Pointer containing the array of world positions of the triangles.
 */
extern Vec3* asteroidGenerateWorldPoints(const Asteroid* asteroid, int* out_count);

#endif /* ASTEROID_H */