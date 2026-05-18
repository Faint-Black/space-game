#include "game.h"

int main(void) {
    GameState* game = gameInit();

    while (game->running) {
        gamePollEvents(game);
        gameUpdateLogic(game);
        gameRenderFrame(game);
    }

    gameDeinit(game);
    return 0;
}
