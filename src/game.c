#include "game.h"

#include <stdio.h>
#include <time.h>

#include <SDL.h>

#include "symbols.h"
#include "event.h"
#include "gui.h"
#include "error.h"
#include "serialization.h"

Game* g_game = NULL;

/*********** SAVING ***************/
static bool load_map()
{
    if (srz_load_map("res/saves/map.json", g_game->map) != OK ||
        srz_load_player("res/saves/player.json", &g_game->player) != OK ||
        srz_load_enemies("res/saves/enemies.json", &g_game->enemies) != OK)
        return false;
    return true;
}

static void generate_map()
{
    char name_arr[7] = "Player";
    vec_char_t name;
    vec_init(&name);
    vec_pusharr(&name, name_arr, 7);
    g_game->player = (Actor){0, {0, 0}, PLAYER, PLAYER_COLOR, name, 100, 10, true};

    vec_init(&g_game->enemies);
    g_game->map = map_generate(&g_game->player.pos, &g_game->enemies);
}

/*********** UTILITY ***************/

// returns NULL if no enemy was found at position `pos`.
static Actor* find_enemy_at(Vec2 pos)
{
    for (int i = 0; i < g_game->enemies.length; i++)
    {
        if (vec2_equals(g_game->enemies.data[i].pos, pos))
        {
            return &g_game->enemies.data[i];
        }
    }
    return NULL;
}

// Changes and returns `pos` in relation to the camera.
static Vec2 apply_camera_to_position(Vec2 pos)
{
    return vec2(pos.x - g_game->camera.target.x + g_game->camera.offset.x,
                pos.y - g_game->camera.target.y + g_game->camera.offset.y);
}

/*********** GAMEPLAY **************/

static void player_attack_or_move(SDL_Keycode prev_key, Vec2 dir)
{
    if (prev_key == SDLK_b)
    {
        Vec2 enemy_pos = vec2(g_game->player.pos.x + dir.x, g_game->player.pos.y + dir.y);
        Actor* enemy = find_enemy_at(enemy_pos);
        if (enemy == NULL)
            return;

        EventAttack attack = {&g_game->player, enemy};
        Event event = {EVENT_ATTACK, &attack};
        event_send(&event);
    }
    else
    {
        actor_move(g_game->map, &g_game->enemies, &g_game->player, dir);
    }
}

static void clear_dead_enemies()
{
    int i;
    Actor enemy;
    vec_foreach(&g_game->enemies, enemy, i)
    {
        if (!enemy.is_alive)
        {
            EventDeath event_death = {&enemy};
            Event event = {EVENT_DEATH, &event_death};
            event_send(&event);
            vec_swapsplice(&g_game->enemies, i, 1);
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
    TCOD_console_put_char_ex(g_game->console, screen_pos.x, screen_pos.y, symbol,
                             resulting_fore, resulting_back);
}

static void draw_map()
{
    for (int x = 0; x < g_game->map->size.x; x++)
    {
        for (int y = 0; y < g_game->map->size.y; y++)
        {
            Tile* tile = map_tile(g_game->map, vec2(x, y));
            draw(tile->pos, tile->symbol, tile->fore_color, tile->back_color,
                 tile->is_visible, tile->was_explored);
        }
    }
}

static void draw_actors()
{
    int i;
    Actor enemy;
    vec_foreach(&g_game->enemies, enemy, i)
    {
        // Enemies are not drawn even if they're on an explored tile.
        draw(enemy.pos, enemy.symbol, enemy.color,
             map_tile(g_game->map, enemy.pos)->back_color,
             map_tile(g_game->map, enemy.pos)->is_visible, false);
        // Useful for debugging:
        //        draw(enemy.pos, enemy.symbol, enemy.color,
        //             map_tile(g_map, enemy.pos)->back_color, true, true);
    }

    draw(g_game->player.pos, g_game->player.symbol, g_game->player.color,
         map_tile(g_game->map, g_game->player.pos)->back_color, true, false);
}

/*************** CORE *****************/

static void render()
{
    TCOD_console_clear(g_game->console);

    draw_map();
    draw_actors();
    gui_render(g_game->console, &g_game->player);

    TCOD_context_present(g_game->context, g_game->console, NULL);
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
            g_game->quit = true;
            break;
        default:
            break;
    }
    prev_key = key.sym;
}

static void tcod_init()
{
    atexit(TCOD_quit);

    g_game->console = TCOD_console_new(g_game->width, g_game->height);
    if (!g_game->console)
        fatal(__FILE__, __func__, __LINE__, "Could not open console: %s",
              TCOD_get_error());

    g_game->tileset =
        TCOD_tileset_load("res/tileset.png", 16, 16, 256, TCOD_CHARMAP_CP437);
    if (!g_game->tileset)
        fatal(__FILE__, __func__, __LINE__, "Failed to load tileset: %s",
              TCOD_get_error());

    const TCOD_ContextParams params = {.tcod_version = TCOD_COMPILEDVERSION,
                                       .renderer_type = TCOD_RENDERER_SDL2,
                                       .pixel_width = g_game->width_px,
                                       .pixel_height = g_game->height_px,
                                       .tileset = g_game->tileset,
                                       .vsync = true,
                                       .sdl_window_flags = SDL_WINDOW_RESIZABLE,
                                       .window_title = g_game->title,
                                       .argc = 0,
                                       .argv = NULL,
                                       .window_xy_defined = true,
                                       .console = g_game->console};
    if (TCOD_context_new(&params, &g_game->context) < 0)
        fatal(__FILE__, __func__, __LINE__, "Could not open context: %s",
              TCOD_get_error());
}

void init(Game* game)
{
    g_game = game;
    g_game->title = "rlike";
    g_game->width = 80;
    g_game->height = 60;
    g_game->width_px = 1920;
    g_game->height_px = 1080;
    g_game->player_vision_radius = 12;

    tcod_init();

    srand(time(NULL));

    g_game->map = malloc(sizeof(Map));
    vec_init(&g_game->map->rooms);
    vec_init(&g_game->map->tiles);

    if (!load_map())
    {
        generate_map();
    }

    event_system_init();
    event_subscribe(actor_on_event);
    event_subscribe(gui_on_event);

    g_game->camera.target = g_game->player.pos;
    g_game->camera.offset = vec2(g_game->width / 2, g_game->height / 2);
}

void update()
{
    while (!g_game->quit)
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
                    g_game->quit = true;
                    break;
            }
        }

        g_game->camera.target = g_game->player.pos;

        map_update_fog_of_war(g_game->map, g_game->player.pos,
                              g_game->player_vision_radius);
        clear_dead_enemies();
        render();
    }
}

void end()
{
    srz_save_map(g_game->map, "res/saves/map.json");
    srz_save_enemies(&g_game->enemies, "res/saves/enemies.json");
    srz_save_player(&g_game->player, "res/saves/player.json");

    for (int i = 0; i < g_game->enemies.length; i++)
    {
        vec_deinit(&g_game->enemies.data[i].name);
    }

    vec_deinit(&g_game->enemies);
    event_system_deinit();
    map_free(g_game->map);
}
