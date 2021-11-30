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
#include "serialization.h"

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

/*********** SAVING ***************/
static bool load_map()
{
    if (srz_load_map("res/saves/map.json", g_map) != OK ||
        srz_load_player("res/saves/player.json", &g_player) != OK ||
        srz_load_enemies("res/saves/enemies.json", &g_enemies) != OK)
        return false;
    return true;
}

static void generate_map()
{
    char name_arr[7] = "Player";
    vec_char_t name;
    vec_init(&name);
    vec_pusharr(&name, name_arr, 7);
    g_player = (Actor){0, {0, 0}, PLAYER, PLAYER_COLOR, name, 100, 10, true};

    vec_init(&g_enemies);
    g_map = map_generate(&g_player.pos, &g_enemies);
}

/*********** UTILITY ***************/

// returns NULL if no enemy was found at position `pos`.
static Actor* find_enemy_at(Vec2 pos)
{
    for (int i = 0; i < g_enemies.length; i++)
    {
        if (vec2_equals(g_enemies.data[i].pos, pos))
        {
            return &g_enemies.data[i];
        }
    }
    return NULL;
}

// Changes and returns `pos` in relation to the camera.
static Vec2 apply_camera_to_position(Vec2 pos)
{
    return vec2(pos.x - g_camera.target.x + g_camera.offset.x,
                pos.y - g_camera.target.y + g_camera.offset.y);
}

/*********** GAMEPLAY **************/

static void player_attack_or_move(SDL_Keycode prev_key, Vec2 dir)
{
    if (prev_key == SDLK_b)
    {
        Vec2 enemy_pos = vec2(g_player.pos.x + dir.x, g_player.pos.y + dir.y);
        Actor* enemy = find_enemy_at(enemy_pos);
        if (enemy == NULL)
            return;

        EventAttack attack = {&g_player, enemy};
        Event event = {EVENT_ATTACK, &attack};
        event_send(&event);
    }
    else
    {
        actor_move(g_map, &g_enemies, &g_player, dir);
    }
}

static void clear_dead_enemies()
{
    int i;
    Actor enemy;
    vec_foreach(&g_enemies, enemy, i)
    {
        if (!enemy.is_alive)
        {
            EventDeath event_death = {&enemy};
            Event event = {EVENT_DEATH, &event_death};
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
    for (int x = 0; x < g_map->size.x; x++)
    {
        for (int y = 0; y < g_map->size.y; y++)
        {
            Tile* tile = map_tile(g_map, vec2(x, y));
            draw(tile->pos, tile->symbol, tile->fore_color, tile->back_color,
                 tile->is_visible, tile->was_explored);
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
        draw(enemy.pos, enemy.symbol, enemy.color, map_tile(g_map, enemy.pos)->back_color,
             map_tile(g_map, enemy.pos)->is_visible, false);
        // Useful for debugging:
        //        draw(enemy.pos, enemy.symbol, enemy.color,
        //             map_tile(g_map, enemy.pos)->back_color, true, true);
    }

    draw(g_player.pos, g_player.symbol, g_player.color,
         map_tile(g_map, g_player.pos)->back_color, true, false);
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
    static SDL_Keycode prev_key = SDLK_ESCAPE;
    switch (key.sym)
    {
        case SDLK_w:
        {
            player_attack_or_move(prev_key, vec2(0, -1));
            break;
        }
        case SDLK_s:
        {
            player_attack_or_move(prev_key, vec2(0, 1));
            break;
        }
        case SDLK_a:
        {
            player_attack_or_move(prev_key, vec2(-1, 0));
            break;
        }
        case SDLK_d:
        {
            player_attack_or_move(prev_key, vec2(1, 0));
            break;
        }
        case SDLK_ESCAPE:
            g_quit = true;
            break;
        default:
            break;
    }
    prev_key = key.sym;
}

static void tcod_init()
{
    atexit(TCOD_quit);

    g_console = TCOD_console_new(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!g_console)
        fatal(__FILE__, __func__, __LINE__, "Could not open console: %s",
              TCOD_get_error());

    g_tileset = TCOD_tileset_load("res/tileset.png", 16, 16, 256, TCOD_CHARMAP_CP437);
    if (!g_tileset)
        fatal(__FILE__, __func__, __LINE__, "Failed to load tileset: %s",
              TCOD_get_error());

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
        fatal(__FILE__, __func__, __LINE__, "Could not open context: %s",
              TCOD_get_error());
}

void init()
{
    tcod_init();

    srand(time(NULL));

    g_map = malloc(sizeof(Map));
    vec_init(&g_map->rooms);
    vec_init(&g_map->tiles);

    if (!load_map())
    {
        generate_map();
    }

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
    srz_save_map(g_map, "res/saves/map.json");
    srz_save_enemies(&g_enemies, "res/saves/enemies.json");
    srz_save_player(&g_player, "res/saves/player.json");

    for (int i = 0; i < g_enemies.length; i++)
    {
        vec_deinit(&g_enemies.data[i].name);
    }

    vec_deinit(&g_enemies);
    event_system_deinit();
    map_free(g_map);
}
