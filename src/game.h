#ifndef GAME_H
#define GAME_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

/* Enable BVH leaf debug rendering. Override with -DDEBUG_MODE=1 at compile time. */
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

typedef struct GameState {
    SDL_Window*   sdl_window;
    SDL_GLContext gl_context;
    bool          running;
    bool          paused;
    Uint32        last_frame_time;

    /*
     * ship_invincibility_timer
     *   Seconds of invincibility remaining after the ship takes damage.
     *   Without this, checkCollision() would fire scoreLoseLife() ~60 times
     *   per second while the ship overlaps an asteroid.
     *
     * ship_hit_flash_timer
     *   Seconds remaining for the red screen flash (visual damage feedback).
     */
    float ship_invincibility_timer;
    float ship_hit_flash_timer;

    /*
     * show_game_over
     *   Raised when the player runs out of lives. The window stays open and
     *   the game-over screen is drawn instead of closing the program, so the
     *   player can read the final score and restart with R.
     */
    bool  show_game_over;

    /*
     * show_title
     *   Raised at startup so the title screen is shown and the simulation is
     *   frozen until the player presses ENTER to begin.
     */
    bool  show_title;
} GameState;

extern GameState* gameInit(void);
extern void       gamePollEvents(GameState* game);
extern void       gameUpdateLogic(GameState* game);
extern void       gameRenderFrame(const GameState* game);
extern void       gameDeinit(GameState* game);
extern void       gameRestart(GameState* game);

#endif /* GAME_H */
