#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <ncurses.h>

#include "map.h"
#include "actor.h"
#include "symbols.h"
#include "event.h"

Map* g_map = NULL;
// The player always has `id` 0.
Actor g_player = {0, {0, 0}, PLAYER, 100, 10, true};
vec_actor_t g_enemies;

void clear_dead_enemies()
{
    int i;
    Actor enemy;
    vec_foreach(&g_enemies, enemy, i)
    {
        if (!enemy.is_alive)
        {
            vec_swapsplice(&g_enemies, i, 1);
        }
    }
}

void draw()
{
    map_draw(g_map);

    int i;
    Actor enemy;
    vec_foreach(&g_enemies, enemy, i)
    {
        actor_draw(&enemy);
    }

    actor_draw(&g_player);
}

void handle_input(int ch)
{
    switch (ch)
    {
        case 'w':
        {
            actor_move(g_map, &g_enemies, &g_player, vec2(0, -1));
            break;
        }
        case 's':
        {
            actor_move(g_map, &g_enemies, &g_player, vec2(0, 1));
            break;
        }
        case 'a':
        {
            actor_move(g_map, &g_enemies, &g_player, vec2(-1, 0));
            break;
        }
        case 'd':
        {
            actor_move(g_map, &g_enemies, &g_player, vec2(1, 0));
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

    vec_init(&g_enemies);
    g_map = map_generate(&g_player.pos, &g_enemies);

    event_system_init();

    event_subscribe(actor_on_event);
}

void update()
{
    int ch;
    while ((ch = getch()) && ch != 'q')
    {
        handle_input(ch);
        clear_dead_enemies();
        draw();
        refresh();
    }
}

void end()
{
    vec_deinit(&g_enemies);
    event_system_deinit();
    map_free(g_map);
    endwin();
}

int main()
{
    init();
    update();
    end();
    return 0;
}
