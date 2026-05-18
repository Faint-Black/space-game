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

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct GameState {
    SDL_Window* sdl_window;
    SDL_GLContext gl_context;
    bool running;
    Uint32 last_frame_time;
} GameState;

extern GameState* gameInit(void);

extern void gamePollEvents(GameState* game);

extern void gameUpdateLogic(GameState* game);

extern void gameRenderFrame(const GameState* game);

extern void gameDeinit(GameState* game);

#endif /* GAME_H */
