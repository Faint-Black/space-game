#include "score.h"

/* Point and life tuning knobs. Tweak as the game's balance evolves. */
#define POINTS_PER_ASTEROID 10
#define POINTS_PER_MODULE 50
#define INITIAL_LIVES 3

typedef struct {
    int points;
    int lives;
    int asteroidsDestroyed;
    int modulesCollected;
} Score;

/* Single per-process score instance, mirroring the global_ship pattern used
 * elsewhere in the project. */
static Score global_score;

extern void scoreInit(void) {
    global_score.points = 0;
    global_score.lives = INITIAL_LIVES;
    global_score.asteroidsDestroyed = 0;
    global_score.modulesCollected = 0;
}

extern void scoreAddAsteroidDestroyed(void) {
    global_score.points += POINTS_PER_ASTEROID;
    global_score.asteroidsDestroyed++;
}

extern void scoreAddModuleCollected(void) {
    global_score.points += POINTS_PER_MODULE;
    global_score.modulesCollected++;
}

extern void scoreLoseLife(void) {
    if (global_score.lives > 0) {
        global_score.lives--;
    }
}

extern int scoreGetPoints(void) {
    return global_score.points;
}

extern int scoreGetLives(void) {
    return global_score.lives;
}

extern int scoreGetAsteroidsDestroyed(void) {
    return global_score.asteroidsDestroyed;
}

extern int scoreGetModulesCollected(void) {
    return global_score.modulesCollected;
}

extern int scoreIsGameOver(void) {
    return (global_score.lives <= 0);
}