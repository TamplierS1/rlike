#ifndef PATHFINDING_H
#define PATHFINDING_H

#include "map.h"

// Executes `edit_tile` on each processed tile.
void path_cast_ray(Map* map, Vec2 begin, Vec2 end, void(edit_tile)(Tile*));
// Returns true if `checker` returns true for any of the tiles in the checked area.
bool path_cast_ray_and_return(Map* map, Vec2 begin, Vec2 end, bool (*check)(Tile*));
Vec2 path_find_next_move(Map* map, Vec2 begin, Vec2 end);

#endif  // PATHFINDING_H
