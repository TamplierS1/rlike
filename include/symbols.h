#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <libtcod.h>

// TODO: remove unused defines. Move the rest to map.c

#define PLAYER '@'
#define WALL '#'
#define FLOOR ' '
#define ENEMY '%'

// clang-format off
#define FLOOR_BACK_COLOR (TCOD_color_t){35, 35, 35}
#define FLOOR_FORE_COLOR (TCOD_color_t){0, 0, 0}
#define WALL_BACK_COLOR (TCOD_color_t){0, 0, 0}
#define WALL_FORE_COLOR (TCOD_color_t){79, 73, 67}
#define PLAYER_COLOR (TCOD_color_t){204, 194, 184}
#define ENEMY_COLOR (TCOD_color_t){137, 29, 29}
// clang-format on

#endif  // SYMBOLS_H
