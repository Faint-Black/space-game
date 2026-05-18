#include "score.h"

/* Valores de pontuação - ajustáveis conforme o jogo evoluir. */
#define POINTS_PER_ASTEROID 10
#define POINTS_PER_MODULE   50

void scoreInit(Score* s) {
    s->points = 0;
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

int scoreGetPoints(const Score* s) {
    return s->points;
}