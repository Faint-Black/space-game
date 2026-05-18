#include "asteroid.h"
#include <stddef.h>
#include <stdlib.h>

/* internal global variables */
static Asteroid* asteroid_array = NULL;
static int asteroid_count = 0;

/* frees individual asteroid allocated data */
static void deinitAsteroid(Asteroid asteroid) {
    if (asteroid.faces != NULL) {
        free(asteroid.faces);
    }
}

extern void initAsteroids(int max_asteroid_count) {
    asteroid_count = max_asteroid_count;
    asteroid_array = NULL; /* TODO */
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
