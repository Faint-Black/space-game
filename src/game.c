#include "game.h"
#include "asteroid.h"
#include "collision.h"
#include "hud.h"
#include "module.h"
#include "render.h"
#include "score.h"
#include "ship.h"
#include "stars.h"
#include "utils.h"
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

/* Duration of the red screen flash after taking damage (seconds). */
#define HIT_FLASH_DURATION 0.3F

/* How long before the claw can score again after grabbing an asteroid. */
#define CLAW_GRAB_COOLDOWN 0.5F

/* Number of asteroids spawned at the start of a run and on every restart. */
#define ASTEROID_COUNT 75

static int  initializeSDLandOpenGL(GameState* game);
static void deinitializeSDLandOpenGL(GameState* game);
static void setupLighting(void);
static void handleShipCollision(GameState* game, float dt);
static void handleProjectileCollisions(void);
static void handleClawCollision(GameState* game, float dt);
static void toggleFullscreen(SDL_Window* window);

/* =========================================================
 * gameInit
 * ========================================================= */
extern GameState* gameInit(void) {
    GameState* const game = (GameState*)malloc(sizeof(GameState));
    if (initializeSDLandOpenGL(game) != 0) exit(EXIT_FAILURE);

    initStars();
    initShip();
    initAsteroids(ASTEROID_COUNT);
    initModules();
    scoreInit();

    game->running                  = true;
    game->paused                   = false;
    game->last_frame_time          = SDL_GetTicks();
    game->ship_invincibility_timer = 0.0F;
    game->ship_hit_flash_timer     = 0.0F;
    game->show_game_over           = false;
    game->show_title               = true;
    game->claw_grab_cooldown       = 0.0F;

    return game;
}

/* =========================================================
 * gameRestart
 *
 * Resets the whole run to its starting state without
 * recreating the SDL window or GL context. The ship and the
 * asteroids are torn down and rebuilt: initShip() reloads the
 * OBJ models and initAsteroids() rebuilds every BVH, so the
 * matching deinit calls must run first to avoid leaking the
 * previous models, GPU textures and BVH allocations.
 *
 * Stars are left untouched: they are a static background and
 * never freed during a run.
 * ========================================================= */
extern void gameRestart(GameState* game) {
    deinitShip();
    deinitAsteroids();

    initShip();
    initAsteroids(ASTEROID_COUNT);
    initModules();
    scoreInit();

    game->paused                   = false;
    game->show_game_over           = false;
    game->last_frame_time          = SDL_GetTicks();
    game->ship_invincibility_timer = 0.0F;
    game->ship_hit_flash_timer     = 0.0F;
    game->claw_grab_cooldown       = 0.0F;
}

/* =========================================================
 * gamePollEvents  -  unchanged from original
 * ========================================================= */
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
            case SDLK_SPACE:  shipFireProjectile();   break;
            case SDLK_c:      shipToggleCamera();     break;
            case SDLK_g:      shipToggleArm();        break;
            case SDLK_n:      shipToggleScanner();    break;
            case SDLK_k:      shipLockCamera();       break;
            case SDLK_p:      game->paused = !game->paused; break;
            case SDLK_r:      gameRestart(game);      break;
            case SDLK_RETURN: game->show_title = false; break;
            case SDLK_F11:    toggleFullscreen(game->sdl_window); break;
            default: break;
            }
            break;
        case SDL_KEYUP:
            shipKeyUp(sdl_event.key.keysym.scancode);
            break;
        case SDL_MOUSEMOTION:
            shipMouseMotion(sdl_event.motion.xrel, sdl_event.motion.yrel);
            break;
        default: break;
        }
    }
}

/* =========================================================
 * gameUpdateLogic
 * ========================================================= */
extern void gameUpdateLogic(GameState* game) {
    Uint32 now = SDL_GetTicks();
    float dt   = (float)(now - game->last_frame_time) / 1000.0F;
    game->last_frame_time = now;

    /* Clamp delta time to avoid physics spikes */
    if (dt < 0.001F) dt = 0.001F;
    if (dt > 0.05F)  dt = 0.05F;

    /* Freeze on the title screen until the player presses ENTER */
    if (game->show_title) return;

    /* Freeze everything once game is over */
    if (scoreIsGameOver()) return;

    /* Freeze everything while paused. last_frame_time still advances, so dt
     * remains small after unpausing instead of producing a physics spike. */
    if (game->paused) return;

    updateShip(dt);
    updateAsteroids(dt);
    updateModules(dt);

    /* Ship vs asteroid collision (with invincibility cooldown) */
    handleShipCollision(game, dt);

    /* Projectile vs asteroid collision */
    handleProjectileCollisions();

    /* Claw vs asteroid collision */
    handleClawCollision(game, dt);
}

/* =========================================================
 * gameRenderFrame  -  original + red flash on damage
 * ========================================================= */
extern void gameRenderFrame(const GameState* game) {
    int w, h;
    SDL_GL_GetDrawableSize(game->sdl_window, &w, &h);

    glViewport(0, 0, w, h);

    /* Tint the clear color red while the hit flash is active */
    if (game->ship_hit_flash_timer > 0.0F) {
        float t = game->ship_hit_flash_timer / HIT_FLASH_DURATION;
        if (t > 1.0F) t = 1.0F;
        glClearColor(0.02F + 0.38F * t, 0.02F, 0.05F, 1.0F);
    } else {
        glClearColor(0.02F, 0.02F, 0.05F, 1.0F);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shipSetupCamera(w, h);
    setupLighting();
    renderStars();
    renderAsteroids();
    renderModules();

    if (DEBUG_MODE) {
        debugRenderAsteroidLeafBVHs();
        debugRenderShipBVH();
    }

    renderShip();
    renderScanner();
    renderHUD(scoreGetPoints(), scoreGetLives());

    /* Overlays, in priority order: the title screen blocks the game at startup,
     * game over blocks it at the end, and pause sits in between. */
    if (game->show_title) {
        renderTitleScreen();
    } else if (game->show_game_over) {
        renderGameOverScreen(scoreGetPoints(), scoreGetAsteroidsDestroyed());
    } else if (game->paused) {
        renderPauseOverlay();
    }

    SDL_GL_SwapWindow(game->sdl_window);
}

/* =========================================================
 * gameDeinit  -  unchanged from original
 * ========================================================= */
extern void gameDeinit(GameState* game) {
    deinitShip();
    deinitAsteroids();
    deinitializeSDLandOpenGL(game);
}

static void handleShipCollision(GameState* game, float dt) {
    /* Count down both timers every frame */
    if (game->ship_invincibility_timer > 0.0F) {
        game->ship_invincibility_timer -= dt;
        if (game->ship_invincibility_timer < 0.0F)
            game->ship_invincibility_timer = 0.0F;
    }
    if (game->ship_hit_flash_timer > 0.0F) {
        game->ship_hit_flash_timer -= dt;
        if (game->ship_hit_flash_timer < 0.0F)
            game->ship_hit_flash_timer = 0.0F;
    }

    /* Skip collision test during invincibility window */
    if (game->ship_invincibility_timer > 0.0F) return;

    if (checkCollision(getShipPosition())) {
        scoreLoseLife();

        /* Start invincibility window and visual flash */
        game->ship_invincibility_timer = 1.0F;
        game->ship_hit_flash_timer     = HIT_FLASH_DURATION;

        SDL_Log("COLLISION: ship hit! Lives remaining: %d", scoreGetLives());

        if (scoreIsGameOver()) {
            SDL_Log("GAME OVER");
            game->show_game_over = true;
        }
    }
}

/* =========================================================
 * initializeSDLandOpenGL  (private) - unchanged from original
 * ========================================================= */
static int initializeSDLandOpenGL(GameState* game) {
    const int window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
            "ERROR: SDL_Init failed: '%s'", SDL_GetError());
        return -1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
            "ERROR: IMG_Init failed: '%s'", IMG_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    game->sdl_window = SDL_CreateWindow(
        "Space Game",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);

    if (game->sdl_window == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR,
            "ERROR: SDL_CreateWindow failed: '%s'", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    game->gl_context = SDL_GL_CreateContext(game->sdl_window);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
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
    GLfloat diffuse[]  = {0.8F, 0.8F, 0.9F, 1.0F};
    GLfloat ambient[]  = {0.2F, 0.2F, 0.25F, 1.0F};
    GLfloat specular[] = {1.0F, 1.0F, 1.0F, 1.0F};

    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
}

static void handleProjectileCollisions(void) {
    int ast_idx  = -1;
    int proj_idx = -1;
    int max_iter = MAX_PROJECTILES;
    const Projectile* projs = getProjectiles();

    while (max_iter-- > 0 &&
           checkProjectileCollision(&ast_idx, &proj_idx)) {

        if (proj_idx >= 0 && proj_idx < MAX_PROJECTILES) {
            ((Projectile*)projs)[proj_idx].active = 0;
            ((Projectile*)projs)[proj_idx].life   = 0.0F;
        }

        damageAsteroid(ast_idx);

        SDL_Log("COLLISION: projectile %d hit asteroid %d", proj_idx, ast_idx);

        ast_idx  = -1;
        proj_idx = -1;
    }
}

static void handleClawCollision(GameState* game, float dt) {
    int mod_idx;

    if (game->claw_grab_cooldown > 0.0F) {
        game->claw_grab_cooldown -= dt;
        if (game->claw_grab_cooldown < 0.0F) {
            game->claw_grab_cooldown = 0.0F;
        }
        return;
    }

    /* Claw rescues a module: collect it and award points */
    mod_idx = checkModuleGrab();
    if (mod_idx >= 0) {
        collectModule(mod_idx);
        scoreAddModuleCollected();
        game->claw_grab_cooldown = CLAW_GRAB_COOLDOWN;
        SDL_Log("CLAW: rescued module %d, score: %d", mod_idx, scoreGetPoints());
    }
}

static void toggleFullscreen(SDL_Window* window) {
    Uint32 flags = SDL_GetWindowFlags(window);
    if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
        SDL_SetWindowFullscreen(window, 0);
    } else {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}