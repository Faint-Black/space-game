#ifndef SCORE_H
#define SCORE_H

typedef struct {
    int points;
    int asteroidsDestroyed;
    int modulesCollected;
} Score;

/* Inicializa o placar com valores padrão (chamar uma vez no início do jogo). */
void scoreInit(Score* s);

/* Eventos que alteram a pontuação. */
void scoreAddAsteroidDestroyed(Score* s);
void scoreAddModuleCollected(Score* s);

/* Consultas. */
int scoreGetPoints(const Score* s);

#endif /* SCORE_H */