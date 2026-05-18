#ifndef ASTEROID_H
#define ASTEROID_H

#include "render.h"

typedef struct Asteroid {
    TriangleFace* faces;
    int face_count;
} Asteroid;

/**
 * @brief Initializes internal state of asteroids. Must be called once before game begins.
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

#endif /* ASTEROID_H */
