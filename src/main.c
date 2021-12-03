#include <stdlib.h>

#include "game.h"

int main()
{
    Game game;

    init(&game);
    update(&game);
    end(&game);

    return EXIT_SUCCESS;
}
