#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <libtcod.h>
#include <SDL.h>

#include "map.h"
#include "actor.h"
#include "symbols.h"
#include "event.h"
#include "gui.h"
#include "error.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 60

static TCOD_Console* g_console;
static TCOD_Tileset* g_tileset;
static TCOD_Context* g_context;

static Map* g_map = NULL;
// The player always has `id` 0.
static Actor g_player = {0, {0, 0}, PLAYER, 100, 10, true};
static vec_actor_t g_enemies;
static Camera g_camera;

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

void draw_map()
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            // TODO: refactor this camera stuff into a separate function.
            int screen_x = x - g_camera.target.x + g_camera.offset.x;
            int screen_y = y - g_camera.target.y + g_camera.offset.y;
            TCOD_console_put_char(g_console, screen_x, screen_y, g_map->tiles[y][x].symbol,
                                  TCOD_BKGND_DEFAULT);
        }
    }
}

void draw_actor(Actor* actor)
{
    int screen_x = actor->pos.x - g_camera.target.x + g_camera.offset.x;
    int screen_y = actor->pos.y - g_camera.target.y + g_camera.offset.y;
    TCOD_console_put_char(g_console, screen_x, screen_y, actor->symbol,
                          TCOD_BKGND_DEFAULT);
}

void draw_actors()
{
    int i;
    Actor enemy;
    vec_foreach(&g_enemies, enemy, i)
    {
        draw_actor(&enemy);
    }
}

void render()
{
    TCOD_console_clear(g_console);

    draw_map();
    draw_actors();
    draw_actor(&g_player);

    TCOD_context_present(g_context, g_console, NULL);
}

void handle_input(SDL_Keysym key)
{
    switch (key.sym)
    {
        case SDLK_w:
        {
            actor_move(g_map, &g_enemies, &g_player, vec2(0, -1));
            break;
        }
        case SDLK_s:
        {
            actor_move(g_map, &g_enemies, &g_player, vec2(0, 1));
            break;
        }
        case SDLK_a:
        {
            actor_move(g_map, &g_enemies, &g_player, vec2(-1, 0));
            break;
        }
        case SDLK_d:
        {
            actor_move(g_map, &g_enemies, &g_player, vec2(1, 0));
            break;
        }
        default:
            break;
    }
}

void init()
{
    atexit(TCOD_quit);

    g_console = TCOD_console_new(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!g_console)
        fatal("Could not open console: %s", TCOD_get_error());

    g_tileset = TCOD_tileset_load("res/tileset.png", 16, 16, 256, TCOD_CHARMAP_TCOD);
    if (!g_tileset)
        fatal("Failed to load tileset: %s", TCOD_get_error());

    const TCOD_ContextParams params = {.tcod_version = TCOD_COMPILEDVERSION,
                                       .renderer_type = TCOD_RENDERER_SDL2,
                                       .pixel_width = 1920,
                                       .pixel_height = 1080,
                                       .tileset = g_tileset,
                                       .vsync = true,
                                       .sdl_window_flags = SDL_WINDOW_RESIZABLE,
                                       .window_title = "rlike",
                                       .argc = 0,
                                       .argv = NULL,
                                       .window_xy_defined = true,
                                       .console = g_console};
    if (TCOD_context_new(&params, &g_context) < 0)
        fatal("Could not open context: %s", TCOD_get_error());

    gui_init();

    vec_init(&g_enemies);
    srand(time(NULL));
    g_map = map_generate(&g_player.pos, &g_enemies);

    event_system_init();
    event_subscribe(actor_on_event);

    g_camera.target = g_player.pos;
    g_camera.offset = vec2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
}

void update()
{
    bool quit = false;
    while (!quit)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                    handle_input(event.key.keysym);
                    break;
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }

        g_camera.target = g_player.pos;

        clear_dead_enemies();
        render();
    }
}

void end()
{
    vec_deinit(&g_enemies);
    event_system_deinit();
    map_free(g_map);
}

int main()
{
    init();
    update();
    end();
    return 0;
}
