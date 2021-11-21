#include <stdio.h>

#include <json.h>

#include "serialization.h"
#include "error.h"

static char* read_json_from_file(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        fatal("Failed to open file %s\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    unsigned long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    // + 1 is needed for the '\0'.
    char* buffer = malloc((length + 1) * sizeof(char));
    if (fread(buffer, sizeof(char), length, file) != length)
    {
        free(buffer);
        fatal("Failed to read the contents of %s\n", path);
        return NULL;
    }

    fclose(file);
    buffer[length] = '\0';

    return buffer;
}

static bool write_json_to_file(const char* path, struct json_object* object,
                               const char* mode)
{
    FILE* file = fopen(path, mode);
    if (file == NULL)
    {
        fatal("Filed to serialize to %s", path);
        return false;
    }

    fprintf(file, "%s", json_object_get_string(object));

    fclose(file);
    return true;
}

static bool get_jobject_from_jstring(const char* json_string,
                                     struct json_object** out_jobject)
{
    struct json_tokener* tokener = json_tokener_new();

    struct json_object* jobject =
        json_tokener_parse_ex(tokener, json_string, (int)strlen(json_string));
    enum json_tokener_error err = json_tokener_get_error(tokener);
    if (jobject == NULL && err != json_tokener_continue)
    {
        fatal("Failed to parse json when parsing an object: %s.\n",
              json_tokener_error_desc(err));
        json_tokener_free(tokener);
        return false;
    }
    json_tokener_free(tokener);

    *out_jobject = jobject;

    return true;
}

static bool deserialize_int(struct json_object* parent, const char* name, int* out_x)
{
    struct json_object* jint;
    bool result = json_object_object_get_ex(parent, name, &jint);

    *out_x = json_object_get_int(jint);

    return result;
}

static bool deserialize_bool(struct json_object* parent, const char* name, bool* out_b)
{
    struct json_object* jbool;
    bool result = json_object_object_get_ex(parent, name, &jbool);

    *out_b = json_object_get_boolean(jbool);

    return result;
}

static bool deserialize_string(struct json_object* parent, const char* name,
                               vec_char_t* out_string)
{
    struct json_object* jstring;
    bool result = json_object_object_get_ex(parent, name, &jstring);

    vec_init(out_string);
    const char* string = json_object_get_string(jstring);
    vec_pusharr(out_string, string, strlen(string));
    vec_push(out_string, '\0');

    return result;
}

static bool deserialize_color(struct json_object* parent, const char* name,
                              TCOD_color_t* out_color)
{
    struct json_object* jcolor;
    if (!json_object_object_get_ex(parent, name, &jcolor))
        return false;

    struct json_object* r;
    struct json_object* g;
    struct json_object* b;
    if (!json_object_object_get_ex(jcolor, "r", &r) ||
        !json_object_object_get_ex(jcolor, "g", &g) ||
        !json_object_object_get_ex(jcolor, "b", &b))
        return false;

    *out_color = (TCOD_color_t){.r = json_object_get_int(r),
                                .g = json_object_get_int(g),
                                .b = json_object_get_int(b)};

    return true;
}

static bool deserialize_vec(struct json_object* parent, const char* name, Vec2* out_vec)
{
    struct json_object* jvec;
    if (!json_object_object_get_ex(parent, name, &jvec))
        return false;

    struct json_object* x;
    struct json_object* y;
    if (!json_object_object_get_ex(jvec, "x", &x) ||
        !json_object_object_get_ex(jvec, "y", &y))
        return false;

    *out_vec = vec2(json_object_get_int(x), json_object_get_int(y));

    return true;
}

static bool deserialize_map(struct json_object* jmap, Map* out_map)
{
    if (!deserialize_vec(jmap, "size", &out_map->size))
        return false;

    struct json_object* jtiles;
    if (!json_object_object_get_ex(jmap, "tiles", &jtiles))
        return false;
    for (size_t i = 0; i < json_object_array_length(jtiles); i++)
    {
        struct json_object* jtile = json_object_array_get_idx(jtiles, i);

        Tile tile;
        int symbol = 0;
        if (!deserialize_vec(jtile, "pos", &tile.pos) ||
            !deserialize_int(jtile, "symbol", &symbol) ||
            !deserialize_bool(jtile, "is_walkable", &tile.is_walkable) ||
            !deserialize_bool(jtile, "is_visible", &tile.is_visible) ||
            !deserialize_bool(jtile, "was_explored", &tile.was_explored) ||
            !deserialize_color(jtile, "fore_color", &tile.fore_color) ||
            !deserialize_color(jtile, "back_color", &tile.back_color))
            return false;
        tile.symbol = (char)symbol;

        vec_push(&out_map->tiles, tile);
    }

    struct json_object* jrooms;
    if (!json_object_object_get_ex(jmap, "rooms", &jrooms))
        return false;
    for (size_t i = 0; i < json_object_array_length(jrooms); i++)
    {
        struct json_object* jroom = json_object_array_get_idx(jrooms, i);

        Room room;
        if (!deserialize_vec(jroom, "pos", &room.pos) ||
            !deserialize_vec(jroom, "size", &room.size) ||
            !deserialize_vec(jroom, "center", &room.center))
            return false;

        vec_push(&out_map->rooms, room);
    }

    deserialize_vec(jmap, "size", &out_map->size);
    deserialize_int(jmap, "room_density", &out_map->room_density);
    deserialize_vec(jmap, "room_size_min", &out_map->room_size_min);
    deserialize_vec(jmap, "room_size_max", &out_map->room_size_max);
    deserialize_int(jmap, "num_enemies", &out_map->num_enemies);

    return true;
}

static bool deserialize_actor(struct json_object* jactor, Actor* out_actor)
{
    Actor actor;
    int symbol = 0;
    if (!deserialize_int(jactor, "id", &actor.id) ||
        !deserialize_vec(jactor, "pos", &actor.pos) ||
        !deserialize_int(jactor, "symbol", &symbol) ||
        !deserialize_color(jactor, "color", &actor.color) ||
        !deserialize_string(jactor, "name", &actor.name) ||
        !deserialize_int(jactor, "hp", &actor.hp) ||
        !deserialize_int(jactor, "dmg", &actor.dmg) ||
        !deserialize_bool(jactor, "is_alive", &actor.is_alive))
        return false;
    actor.symbol = (char)symbol;

    *out_actor = actor;

    return true;
}

static void serialize_int(struct json_object* parent, const char* name, int x)
{
    json_object_object_add(parent, name, json_object_new_int(x));
}

static void serialize_bool(struct json_object* parent, const char* name, bool b)
{
    json_object_object_add(parent, name, json_object_new_boolean(b));
}

static void serialize_string(struct json_object* parent, const char* name,
                             const vec_char_t* str)
{
    json_object_object_add(parent, name, json_object_new_string(str->data));
}

static void serialize_color(struct json_object* parent, const char* name,
                            TCOD_color_t color)
{
    struct json_object* jcolor = json_object_new_object();
    serialize_int(jcolor, "r", color.r);
    serialize_int(jcolor, "g", color.g);
    serialize_int(jcolor, "b", color.b);

    json_object_object_add(parent, name, jcolor);
}

static void serialize_vec(struct json_object* parent, const char* name, Vec2 vec)
{
    struct json_object* jvec = json_object_new_object();
    serialize_int(jvec, "x", vec.x);
    serialize_int(jvec, "y", vec.y);

    json_object_object_add(parent, name, jvec);
}

static struct json_object* serialize_actor(Actor* actor)
{
    struct json_object* jactor = json_object_new_object();

    serialize_int(jactor, "id", actor->id);
    serialize_vec(jactor, "pos", actor->pos);
    serialize_int(jactor, "symbol", (int)actor->symbol);
    serialize_color(jactor, "color", actor->color);
    serialize_string(jactor, "name", &actor->name);
    serialize_int(jactor, "hp", actor->hp);
    serialize_int(jactor, "dmg", actor->dmg);
    serialize_bool(jactor, "is_alive", actor->is_alive);

    return jactor;
}

static struct json_object* serialize_map(Map* map)
{
    struct json_object* jmap = json_object_new_object();

    serialize_vec(jmap, "size", map->size);

    struct json_object* tiles = json_object_new_array();
    for (int i = 0; i < map->tiles.length; i++)
    {
        Tile* tile = map_tile(map, map->tiles.data[i].pos);
        struct json_object* jtile = json_object_new_object();
        serialize_vec(jtile, "pos", tile->pos);
        serialize_int(jtile, "symbol", (int)tile->symbol);
        serialize_bool(jtile, "is_walkable", tile->is_walkable);
        serialize_bool(jtile, "is_visible", tile->is_visible);
        serialize_bool(jtile, "was_explored", tile->was_explored);
        serialize_color(jtile, "fore_color", tile->fore_color);
        serialize_color(jtile, "back_color", tile->back_color);

        json_object_array_add(tiles, jtile);
    }
    json_object_object_add(jmap, "tiles", tiles);

    struct json_object* rooms = json_object_new_array();
    for (int i = 0; i < map->rooms.length; i++)
    {
        Room* room = &map->rooms.data[i];
        struct json_object* jroom = json_object_new_object();
        serialize_vec(jroom, "pos", room->pos);
        serialize_vec(jroom, "size", room->size);
        serialize_vec(jroom, "center", room->center);

        json_object_array_add(rooms, jroom);
    }
    json_object_object_add(jmap, "rooms", rooms);

    serialize_vec(jmap, "size", map->size);
    serialize_int(jmap, "room_density", map->room_density);
    serialize_vec(jmap, "room_size_min", map->room_size_min);
    serialize_vec(jmap, "room_size_max", map->room_size_max);
    serialize_int(jmap, "num_enemies", map->num_enemies);

    return jmap;
}

bool srz_load_map(const char* path, Map* out_map)
{
    struct json_object* jmap;
    char* json_string = read_json_from_file(path);
    if (!get_jobject_from_jstring(json_string, &jmap))
        return false;

    free(json_string);

    return deserialize_map(jmap, out_map);
}

bool srz_load_player(const char* path, Actor* out_actor)
{
    struct json_object* jactor;
    char* json_string = read_json_from_file(path);
    if (!get_jobject_from_jstring(read_json_from_file(path), &jactor))
        return false;

    // json_string is allocated in `read_json_from_file`.
    free(json_string);

    return deserialize_actor(jactor, out_actor);
}

bool srz_load_enemies(const char* path, vec_actor_t* out_enemies)
{
    struct json_object* jenemies;
    char* json_string = read_json_from_file(path);
    if (!get_jobject_from_jstring(json_string, &jenemies))
        return false;

    vec_init(out_enemies);
    for (size_t i = 0; i < json_object_array_length(jenemies); i++)
    {
        struct json_object* jenemy = json_object_array_get_idx(jenemies, i);

        Actor enemy;
        if (!deserialize_actor(jenemy, &enemy))
            return false;

        vec_push(out_enemies, enemy);
    }

    // json_string is allocated in `read_json_from_file`.
    free(json_string);

    return true;
}

bool srz_save_map(Map* map, const char* path)
{
    struct json_object* jmap = serialize_map(map);
    bool result = write_json_to_file(path, jmap, "w");

    return result;
}

bool srz_save_player(Actor* player, const char* path)
{
    struct json_object* jplayer = serialize_actor(player);
    bool result = write_json_to_file(path, jplayer, "w");

    return result;
}

bool srz_save_enemies(const vec_actor_t* enemies, const char* path)
{
    struct json_object* jenemies = json_object_new_array();
    for (int i = 0; i < enemies->length; i++)
    {
        struct json_object* jenemy = serialize_actor(&enemies->data[i]);
        // TODO: add some error handling.
        if (jenemy == NULL)
            continue;

        json_object_array_add(jenemies, jenemy);
    }

    bool result = write_json_to_file(path, jenemies, "w");

    return result;
}