#ifndef SCORE_H
#define SCORE_H

/**
 * @brief Resets the score state to the start-of-game defaults.
 *        Must be called once before any other score function.
 */
extern void scoreInit(void);

/**
 * @brief Awards points and bumps the asteroid kill counter.
 */
extern void scoreAddAsteroidDestroyed(void);

/**
 * @brief Awards points and bumps the rescue-module collection counter.
 */
extern void scoreAddModuleCollected(void);

/**
 * @brief Decrements the lives counter by one. Saturates at zero.
 */
extern void scoreLoseLife(void);

/**
 * @brief Fetches the current point total.
 */
extern int scoreGetPoints(void);

/**
 * @brief Fetches the remaining lives.
 */
extern int scoreGetLives(void);

/**
 * @brief Fetches how many asteroids have been destroyed so far.
 *        Useful for the end-of-game summary screen.
 */
extern int scoreGetAsteroidsDestroyed(void);

/**
 * @brief Fetches how many rescue modules have been collected so far.
 *        Useful for the end-of-game summary screen.
 */
extern int scoreGetModulesCollected(void);

/**
 * @brief Checks whether the player has lost all their lives.
 *
 * @retval 1 The game is over.
 * @retval 0 The player still has at least one life remaining.
 */
extern int scoreIsGameOver(void);

#endif /* SCORE_H */