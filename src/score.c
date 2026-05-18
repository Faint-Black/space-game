#include "score.h"

/* Valores de pontuação - ajustáveis conforme o jogo evoluir. */
#define POINTS_PER_ASTEROID 10
#define POINTS_PER_MODULE   50
#define INITIAL_LIVES       3

void scoreInit(Score* s) {
    s->points = 0;
    s->lives = INITIAL_LIVES;
    s->asteroidsDestroyed = 0;
    s->modulesCollected = 0;
}

void scoreAddAsteroidDestroyed(Score* s) {
    s->points += POINTS_PER_ASTEROID;
    s->asteroidsDestroyed++;
}

void scoreAddModuleCollected(Score* s) {
    s->points += POINTS_PER_MODULE;
    s->modulesCollected++;
}

void scoreLoseLife(Score* s) {
    if (s->lives > 0) {
        s->lives--;
    }
}

int scoreGetPoints(const Score* s) {
    return s->points;
}

int scoreGetLives(const Score* s) {
    return s->lives;
}

int scoreIsGameOver(const Score* s) {
    return s->lives <= 0;
}