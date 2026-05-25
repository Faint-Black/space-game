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

#endif /* HUD_H */
