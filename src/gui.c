#include "libtcod.h"

#include "gui.h"

static Actor* g_engaged_enemy = NULL;
// I can't just use `Actor.is_alive` because once
// all dead actors are removed, `g_engaged_enemy` will point
// to an enemy that is not NULL, which will cause the gui to
// draw that enemy's stats.
static bool g_is_enemy_alive = false;

static void draw_actor_stats(TCOD_Console* console, Actor* actor, Vec2 frame_pos)
{
    Vec2 frame_size = vec2(15, 10);
    int frame_title_offset_x = 1;
    TCOD_console_draw_frame_rgb(console, frame_pos.x, frame_pos.y, frame_size.x,
                                frame_size.y, NULL, &TCOD_light_gray, &TCOD_black,
                                TCOD_BKGND_SET, true);
    TCOD_console_printf_ex(console, frame_pos.x + frame_title_offset_x, frame_pos.y,
                           TCOD_BKGND_DEFAULT, TCOD_LEFT, "%s", actor->name.data);

    Vec2 hp_pos = vec2(frame_pos.x + 1, frame_pos.y + 2);
    TCOD_console_printf_ex(console, hp_pos.x, hp_pos.y, TCOD_BKGND_SET, TCOD_LEFT,
                           "HP: "
                           "%d\n",
                           actor->hp);
    Vec2 dmg_pos = vec2(hp_pos.x, hp_pos.y + 2);
    // TCOD_console_printf_ex(console, dmg_pos.x, dmg_pos.y, TCOD_BKGND_SET, TCOD_LEFT,
    //                        "DMG: "
    //                        "%d\n",
    //                        actor->dmg);

    /* Color the text. libtcod apparently doesn't offer an easier way of doing this. */
    for (int x = hp_pos.x; x < hp_pos.x + frame_size.x - 1; x++)
    {
        if (console->tiles[x + hp_pos.y * console->w].ch == ' ')
            break;
        TCOD_console_set_char_foreground(console, x, hp_pos.y, TCOD_dark_green);
    }
    for (int x = dmg_pos.x; x < dmg_pos.x + frame_size.x - 1; x++)
    {
        if (console->tiles[x + dmg_pos.y * console->w].ch == ' ')
            break;
        TCOD_console_set_char_foreground(console, x, dmg_pos.y, TCOD_dark_red);
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
    draw_actor_stats(console, player, vec2(0, 0));
    if (g_is_enemy_alive)
        draw_actor_stats(console, g_engaged_enemy, vec2(16, 0));
}
