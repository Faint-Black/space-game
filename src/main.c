#include <GL/gl.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <stdbool.h>
#include <stddef.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main(void) {
    SDL_Window* sdl_window;
    SDL_Event sdl_event;
    SDL_GLContext gl_context;
    bool running = true;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return -1;
    }

    /* OpenGL 2.1 */
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    sdl_window = SDL_CreateWindow("Space Game", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    if (sdl_window == NULL) return -1;

    gl_context = SDL_GL_CreateContext(sdl_window);

    while (running) {
        while (SDL_PollEvent(&sdl_event)) {
            if (sdl_event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);

        /* hello world triangle */
        glBegin(GL_TRIANGLES);
        {
            glColor3f(1.0F, 0.0F, 0.0F);
            glVertex2f(-0.5F, -0.5F);

            glColor3f(0.0F, 1.0F, 0.0F);
            glVertex2f(0.5F, -0.5F);

            glColor3f(0.0F, 0.0F, 1.0F);
            glVertex2f(0.0F, 0.5F);
        }
        glEnd();

        SDL_GL_SwapWindow(sdl_window);
    }

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();

    return 0;
}
