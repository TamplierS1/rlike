#ifndef ROGUE_H
#define ROGUE_H

#include "map.h"

typedef struct
{
    Vec2 pos;
    char symbol;
} Rogue;

void rogue_move(Map* map, Rogue* rogue, Vec2 dir);

#endif  // ROGUE_H
