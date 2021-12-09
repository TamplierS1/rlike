#include "inventory.h"
#include "libtcod.h"

#include "gui.h"
#include "libtcod/color.h"
#include "libtcod/console.h"

static Actor* g_engaged_enemy = NULL;
// I can't just use `Actor.is_alive` because once
// all dead actors are removed, `g_engaged_enemy` will point
// to an enemy that is not NULL, which will cause the gui to
// draw that enemy's stats.
static bool g_is_enemy_alive = false;
static bool g_display_inventory = true;

static void draw_frame(TCOD_Console* console, Vec2 frame_pos, Vec2 frame_size,
                       Vec2 title_offset, sds title)
{
    TCOD_console_draw_frame_rgb(console, frame_pos.x, frame_pos.y, frame_size.x,
                                frame_size.y, NULL, &TCOD_light_gray, &TCOD_black,
                                TCOD_BKGND_SET, true);
    TCOD_console_printf_ex(console, frame_pos.x + title_offset.x,
                           frame_pos.y + title_offset.y, TCOD_BKGND_DEFAULT, TCOD_LEFT,
                           "%s", title);
}

static void apply_color(TCOD_Console* console, Vec2 begin, int length, TCOD_color_t color)
{
    for (int x = begin.x; x < begin.x + length - 1; x++)
    {
        if (console->tiles[x + begin.y * console->w].ch == ' ')
            break;
        TCOD_console_set_char_foreground(console, x, begin.y, color);
    }
}

static void draw_actor_stats(TCOD_Console* console, Actor* actor, Vec2 frame_pos,
                             Vec2 frame_size)
{
    draw_frame(console, frame_pos, frame_size, vec2(1, 0), actor->name);

    Vec2 hp_pos = vec2(frame_pos.x + 1, frame_pos.y + 2);
    TCOD_console_printf_ex(console, hp_pos.x, hp_pos.y, TCOD_BKGND_SET, TCOD_LEFT,
                           "HP: "
                           "%d\n",
                           actor->hp);

    apply_color(console, hp_pos, frame_size.x, TCOD_dark_green);
}

static void draw_player_inventory(TCOD_Console* console, Actor* actor, Vec2 frame_pos,
                                  int width)
{
    Inventory* inv = &actor->inventory;

    int entry_height = 2;
    draw_frame(console, frame_pos, vec2(width, inv->items.length * entry_height + 3),
               vec2(1, 0), "Inventory");

    for (int i = 0; i < inv->items.length; i++)
    {
        Item* item = &inv->items.data[i];

        Vec2 entry_pos = vec2(frame_pos.x + 1, frame_pos.y + entry_height * i + 2);
        TCOD_console_printf_ex(console, entry_pos.x, entry_pos.y, TCOD_BKGND_SET,
                               TCOD_LEFT, "[%c]", item->equipped ? 'X' : ' ');
        TCOD_console_printf_ex(console, entry_pos.x + 3, entry_pos.y, TCOD_BKGND_SET,
                               TCOD_LEFT, "%s", item->name);
    }
}

void gui_on_event(Event* event)
{
    switch (event->type)
    {
        case EVENT_ATTACK:
        {
            EventAttack* event_attack = (EventAttack*)(event->data);
            // If the player is the attacker.
            if (event_attack->attacker->id == event_attack->player_id)
            {
                g_engaged_enemy = event_attack->victim;
                g_is_enemy_alive = true;
            }

            break;
        }
        case EVENT_DEATH:
        {
            // TODO: fix the crash that happens when the player dies.
            EventDeath* event_death = (EventDeath*)(event->data);
            if (event_death->dead_actor->id == g_engaged_enemy->id)
                g_is_enemy_alive = false;
            break;
        }
    }
}

bool gui_handle_input(SDL_Keysym key)
{
    return false;
}

void gui_render(TCOD_Console* console, Actor* player)
{
    Vec2 player_stats_pos = vec2(0, 0);
    Vec2 player_stats_size = vec2(15, 5);

    draw_actor_stats(console, player, player_stats_pos, player_stats_size);
    if (g_is_enemy_alive)
        draw_actor_stats(
            console, g_engaged_enemy,
            vec2(player_stats_pos.x + player_stats_size.x + 1, player_stats_pos.y),
            player_stats_size);

    if (g_display_inventory)
        draw_player_inventory(
            console, player,
            vec2(player_stats_pos.x, player_stats_pos.y + player_stats_size.y),
            player_stats_size.x);
}
