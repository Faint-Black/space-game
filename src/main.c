#include "asteroid.h"
#include "ship.h"
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

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

/* global variables */
static SDL_Window* global_sdl_window;
static SDL_GLContext global_gl_context;
static bool global_app_should_run = true;
static Uint32 global_last_frame_time = 0;

/* returns 0 on success, -1 on failure */
static int initializeSDLandOpenGL(void) {
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

    global_sdl_window = SDL_CreateWindow("Space Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, window_flags);
    if (global_sdl_window == NULL) {
        SDL_LogCritical(SDL_LOG_CATEGORY_ERROR, "ERROR: failed 'SDL_CreateWindow' with error '%s'", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return -1;
    }

    global_gl_context = SDL_GL_CreateContext(global_sdl_window);

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

    return 0;
}

static void deinitializeSDLandOpenGL(void) {
    SDL_GL_DeleteContext(global_gl_context);
    SDL_DestroyWindow(global_sdl_window);
    IMG_Quit();
    SDL_Quit();
}

static void gameInit(void) {
    if (initializeSDLandOpenGL() != 0) exit(EXIT_FAILURE);

    initShip();
    initAsteroids(50);

    global_last_frame_time = SDL_GetTicks();
}

static void gameDeinit(void) {
    deinitShip();
    deinitAsteroids();

    deinitializeSDLandOpenGL();
}

static void gamePollEvents(void) {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
        case SDL_QUIT:
            global_app_should_run = false;
            break;
        case SDL_KEYDOWN:
            if (sdl_event.key.repeat) break;
            shipKeyDown(sdl_event.key.keysym.scancode);
            switch (sdl_event.key.keysym.sym) {
            case SDLK_ESCAPE:
                global_app_should_run = false;
                break;
            case SDLK_SPACE:
                shipFireProjectile();
                break;
            case SDLK_c:
                shipToggleCamera();
                break;
            case SDLK_g:
                shipToggleArm();
                break;
            case SDLK_n:
                shipToggleScanner();
                break;
            default:
                break;
            }
            break;
        case SDL_KEYUP:
            shipKeyUp(sdl_event.key.keysym.scancode);
            break;
        default:
            break;
        }
    }
}

static void gameUpdate(void) {
    Uint32 now = SDL_GetTicks();
    float dt = (float)(now - global_last_frame_time) / 1000.0F;
    global_last_frame_time = now;

    /* Clamp delta time */
    if (dt < 0.001F) dt = 0.001F;
    if (dt > 0.05F) dt = 0.05F;

    updateShip(dt);
}

static void setupLighting(void) {
    GLfloat lightPos[] = {0.5F, 1.0F, 0.3F, 0.0F};
    GLfloat lightDiffuse[] = {0.8F, 0.8F, 0.9F, 1.0F};
    GLfloat lightAmbient[] = {0.2F, 0.2F, 0.25F, 1.0F};
    GLfloat lightSpecular[] = {1.0F, 1.0F, 1.0F, 1.0F};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

static void gameRenderFrame(void) {
    int w, h;

    SDL_GL_GetDrawableSize(global_sdl_window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.02F, 0.02F, 0.05F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Setup camera (sets projection + modelview) */
    shipSetupCamera(w, h);

    /* Scene lighting */
    setupLighting();

    /* Render ship + projectiles */
    renderShip();

    /* Render scanner overlay */
    renderScanner();

    /* swap front and back buffers */
    SDL_GL_SwapWindow(global_sdl_window);
}

int main(void) {
    gameInit();

    while (global_app_should_run) {
        gamePollEvents();
        gameUpdate();
        gameRenderFrame();
    }

    gameDeinit();
    return 0;
}
