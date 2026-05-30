#ifndef HUD_H
#define HUD_H

/**
 * @brief Run this every frame to render the Heads Up Display.
 */
extern void renderHUD(int score, int lives);

/**
 * @brief Draws a centered pause indicator (two vertical bars) over the scene.
 *        Should be called after renderHUD when the game is paused.
 */
extern void renderPauseOverlay(void);

/**
 * @brief Draws the centered game-over screen: the "GAME OVER" title, the final
 *        score, the asteroid kill count and the restart hint. Should be called
 *        after renderHUD once the player has run out of lives.
 */
extern void renderGameOverScreen(int final_score, int asteroids_destroyed);

/**
 * @brief Draws the centered title screen ("SPACE GAME" and a start hint) over
 *        the scene. Should be called after renderHUD while the game is waiting
 *        on the title screen.
 */
extern void renderTitleScreen(void);

#endif /* HUD_H */
