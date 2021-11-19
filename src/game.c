#include "game.h"

#include <stdio.h>
#include <time.h>

#include <libtcod.h>
#include <SDL.h>

#include "map.h"
#include "actor.h"
#include "symbols.h"
#include "event.h"
#include "gui.h"
#include "error.h"

#define PLAYER_VISION_RADIUS 12

static TCOD_Console* g_console;
static TCOD_Tileset* g_tileset;
static TCOD_Context* g_context;
static bool g_quit = false;

static Map* g_map = NULL;
// The player always has `id` 0.
static Actor g_player;
static vec_actor_t g_enemies;
static Camera g_camera;

/*********** UTILITY ***************/

// Changes and returns `pos` in relation to the camera.
static Vec2 apply_camera_to_position(Vec2 pos)
{
    return vec2(pos.x - g_camera.target.x + g_camera.offset.x,
                pos.y - g_camera.target.y + g_camera.offset.y);
}

/*********** GAMEPLAY **************/

static void clear_dead_enemies()
{
    int i;
    Actor enemy;
    vec_foreach(&g_enemies, enemy, i)
    {
        if (!enemy.is_alive)
        {
            Event event = {.type = EVENT_DEATH, &enemy};
            event_send(&event);
            vec_swapsplice(&g_enemies, i, 1);
        }
    }
}

/************ RENDERING *************/

static void draw(Vec2 pos, char symbol, TCOD_color_t fore, TCOD_color_t back,
                 bool is_visible, bool was_explored)
{
    if (!is_visible && !was_explored)
        return;

    TCOD_color_t resulting_fore = fore;
    TCOD_color_t resulting_back = back;
    if (was_explored && !is_visible)
    {
        resulting_fore = TCOD_color_multiply_scalar(resulting_fore, 0.5f);
        resulting_back = TCOD_color_multiply_scalar(resulting_back, 0.5f);
    }

    Vec2 screen_pos = apply_camera_to_position(pos);
    TCOD_console_put_char_ex(g_console, screen_pos.x, screen_pos.y, symbol,
                             resulting_fore, resulting_back);
}

static void draw_map()
{
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            Tile tile = g_map->tiles[y][x];
            draw(tile.pos, tile.symbol, tile.fore_color, tile.back_color, tile.is_visible,
                 tile.was_explored);
        }
    }
}

static void draw_actors()
{
    int i;
    Actor enemy;
    vec_foreach(&g_enemies, enemy, i)
    {
        // Enemies are not drawn even if they're on an explored tile.
        draw(enemy.pos, enemy.symbol, enemy.color,
             g_map->tiles[enemy.pos.y][enemy.pos.x].back_color,
             g_map->tiles[enemy.pos.y][enemy.pos.x].is_visible, false);
    }

    draw(g_player.pos, g_player.symbol, g_player.color,
         g_map->tiles[enemy.pos.y][enemy.pos.x].back_color, true, false);
}

/*************** CORE *****************/

static void render()
{
    TCOD_console_clear(g_console);

    draw_map();
    draw_actors();
    gui_render(g_console, &g_player);

    TCOD_context_present(g_context, g_console, NULL);
}

static void handle_input(SDL_Keysym key)
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
        case SDLK_ESCAPE:
            g_quit = true;
            break;
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

    g_tileset = TCOD_tileset_load("res/tileset.png", 16, 16, 256, TCOD_CHARMAP_CP437);
    if (!g_tileset)
        fatal("Failed to load tileset: %s", TCOD_get_error());

    const TCOD_ContextParams params = {.tcod_version = TCOD_COMPILEDVERSION,
                                       .renderer_type = TCOD_RENDERER_SDL2,
                                       .pixel_width = SCREEN_WIDTH_PIX,
                                       .pixel_height = SCREEN_HEIGHT_PIX,
                                       .tileset = g_tileset,
                                       .vsync = true,
                                       .sdl_window_flags = SDL_WINDOW_RESIZABLE,
                                       .window_title = TITLE,
                                       .argc = 0,
                                       .argv = NULL,
                                       .window_xy_defined = true,
                                       .console = g_console};
    if (TCOD_context_new(&params, &g_context) < 0)
        fatal("Could not open context: %s", TCOD_get_error());

    char name_arr[7] = "Player";
    vec_char_t name;
    vec_init(&name);
    vec_pusharr(&name, name_arr, 7);
    g_player = (Actor){0, {0, 0}, PLAYER, PLAYER_COLOR, name, 100, 10, true};

    vec_init(&g_enemies);
    srand(time(NULL));
    g_map = map_generate(&g_player.pos, &g_enemies);

    event_system_init();
    event_subscribe(actor_on_event);
    event_subscribe(gui_on_event);

    g_camera.target = g_player.pos;
    g_camera.offset = vec2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
}

void update()
{
    while (!g_quit)
    {
        SDL_Event event;
        // This helps with debugging.
        while (SDL_PollEvent(&event) /*&& SDL_GetMouseFocus() != NULL*/)
        {
            switch (event.type)
            {
                case SDL_KEYDOWN:
                {
                    bool was_key_used = gui_handle_input(event.key.keysym);
                    if (!was_key_used)
                        handle_input(event.key.keysym);
                    break;
                }
                case SDL_QUIT:
                    g_quit = true;
                    break;
            }
        }

        g_camera.target = g_player.pos;

        map_update_fog_of_war(g_map, g_player.pos, PLAYER_VISION_RADIUS);
        clear_dead_enemies();
        render();
    }
}

void end()
{
    for (int i = 0; i < g_enemies.length; i++)
    {
        vec_deinit(&g_enemies.data[i].name);
    }

    vec_deinit(&g_enemies);
    event_system_deinit();
    map_free(g_map);
}
