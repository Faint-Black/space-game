#include "game.h"
#include "asteroid.h"
#include "collision.h"
#include "hud.h"
#include "render.h"
#include "score.h"
#include "ship.h"
#include "stars.h"
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
#include <stdlib.h>

static int initializeSDLandOpenGL(GameState* game);
static void deinitializeSDLandOpenGL(GameState* game);
static void setupLighting(void);

extern GameState* gameInit(void) {
    GameState* const game = malloc(sizeof(GameState));
    if (initializeSDLandOpenGL(game) != 0) exit(EXIT_FAILURE);

    initStars();
    initShip();
    initAsteroids(50);
    scoreInit();

    game->running = true;
    game->last_frame_time = SDL_GetTicks();
    return game;
}

extern void gamePollEvents(GameState* game) {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
        case SDL_QUIT: game->running = false; break;
        case SDL_KEYDOWN:
            if (sdl_event.key.repeat) break;
            shipKeyDown(sdl_event.key.keysym.scancode);
            switch (sdl_event.key.keysym.sym) {
            case SDLK_ESCAPE: game->running = false; break;
            case SDLK_SPACE: shipFireProjectile(); break;
            case SDLK_c: shipToggleCamera(); break;
            case SDLK_g: shipToggleArm(); break;
            case SDLK_n: shipToggleScanner(); break;
            case SDLK_k: shipLockCamera(); break;
            default: break;
            }
            break;
        case SDL_KEYUP: shipKeyUp(sdl_event.key.keysym.scancode); break;
        case SDL_MOUSEMOTION: shipMouseMotion(sdl_event.motion.xrel, sdl_event.motion.yrel); break;
        default: break;
        }
    }
}

extern void gameUpdateLogic(GameState* game) {
    Uint32 now = SDL_GetTicks();
    float dt = (float)(now - game->last_frame_time) / 1000.0F;
    game->last_frame_time = now;

    /* Clamp delta time */
    if (dt < 0.001F) dt = 0.001F;
    if (dt > 0.05F) dt = 0.05F;

    updateShip(dt);

    if (checkCollision(getShipPosition())) {
        scoreLoseLife();
    }
}

extern void gameRenderFrame(const GameState* game) {
    int w;
    int h;
    SDL_GL_GetDrawableSize(game->sdl_window, &w, &h);

    glViewport(0, 0, w, h);
    glClearColor(0.02F, 0.02F, 0.05F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Setup camera (sets projection + modelview) */
    shipSetupCamera(w, h);
    /* Scene lighting */
    setupLighting();
    /* Render asteroids */
    renderAsteroids();
    /* Stars (camera-relative background) */
    renderStars();
    /* Render ship + projectiles */
    renderShip();
    /* Render scanner overlay */
    renderScanner();
    /* Render HUD on top */
    renderHUD(scoreGetPoints(), scoreGetLives());
    /* swap front and back buffers */
    SDL_GL_SwapWindow(game->sdl_window);
}

extern void gameDeinit(GameState* game) {
    deinitShip();
    deinitAsteroids();

    deinitializeSDLandOpenGL(game);
}

/*----- PRIVATE FUNCTIONS BELOW -----*/

static int initializeSDLandOpenGL(GameState* game) {
    const int window_flags = (SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ERROR: failed 'SDL_Init' with error '%s'", SDL_GetError());
        return -1;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ERROR: failed 'IMG_Init' with error '%s'", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    /* OpenGL 1.1 (fixed function pipeline only) */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    game->sdl_window = SDL_CreateWindow("Space Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
    if (game->sdl_window == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ERROR: failed 'SDL_CreateWindow' with error '%s'", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    game->gl_context = SDL_GL_CreateContext(game->sdl_window);

    /* Enable depth testing */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    /* Enable backface culling */
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    /* Enable lighting */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    /* Relative mouse mode */
    SDL_SetRelativeMouseMode(SDL_TRUE);

    return 0;
}

static void deinitializeSDLandOpenGL(GameState* game) {
    SDL_GL_DeleteContext(game->gl_context);
    SDL_DestroyWindow(game->sdl_window);
    IMG_Quit();
    SDL_Quit();
}

static void setupLighting(void) {
    GLfloat position[] = {0.5F, 1.0F, 0.3F, 0.0F};
    GLfloat diffuse[] = {0.8F, 0.8F, 0.9F, 1.0F};
    GLfloat ambient[] = {0.2F, 0.2F, 0.25F, 1.0F};
    GLfloat specular[] = {1.0F, 1.0F, 1.0F, 1.0F};

    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}
