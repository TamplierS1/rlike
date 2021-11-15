#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <ncurses.h>

#include "map.h"
#include "rogue.h"

Map* g_map = NULL;
Rogue g_rogue = {{0, 0}, '@'};

void draw()
{
    map_draw(g_map);
    mvaddch(g_rogue.pos.y, g_rogue.pos.x, g_rogue.symbol);
}

void handle_input(char ch)
{
    switch (ch)
    {
        case 'w':
        {
            rogue_move(g_map, &g_rogue, vec2(0, -1));
            break;
        }
        case 's':
        {
            rogue_move(g_map, &g_rogue, vec2(0, 1));
            break;
        }
        case 'a':
        {
            rogue_move(g_map, &g_rogue, vec2(-1, 0));
            break;
        }
        case 'd':
        {
            rogue_move(g_map, &g_rogue, vec2(1, 0));
            break;
        }
        case ERR:  // No key was pressed - skip.
            break;
    }
}

void init()
{
    initscr();
    raw();
    intrflush(stdscr, false);
    keypad(stdscr, true);
    noecho();
    nodelay(stdscr, true);
    nonl();
    curs_set(0);

    srand(time(NULL));
    g_map = map_generate(&g_rogue.pos);
}

void run()
{
    int ch;
    while ((ch = getch()) && ch != 'q')
    {
        handle_input(ch);
        draw();
        refresh();
    }
}

void end()
{
    map_free(g_map);
    endwin();
}

int main()
{
    init();
    run();
    end();
    return 0;
}
