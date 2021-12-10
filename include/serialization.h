#ifndef SERIALIZE_H
#define SERIALIZE_H

#include "map.h"
#include "actor.h"
#include "error.h"

Error srz_load_templates(const char* path, vec_item_t* out_templates);
Error srz_load_map(const char* path, Map* out_map);
Error srz_load_player(const char* path, Actor* out_actor);
Error srz_load_enemies(const char* path, vec_actor_t* out_enemies);

Error srz_save_map(Map* map, const char* path);
Error srz_save_player(Actor* player, const char* path);
Error srz_save_enemies(const vec_actor_t* enemies, const char* path);

#endif  // SERIALIZE_H
