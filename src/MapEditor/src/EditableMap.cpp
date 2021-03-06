#include "EditableMap.hpp"
#include "Utils.hpp"
#include "Globals.hpp"

#include <cstdlib>

EditableMap::EditableMap(Resources& resources, Subsystem& subsystem) throw (Exception)
    : Map(subsystem), resources(resources), subsystem(subsystem)
{
    set_background(get_background());
    set_tileset(get_tileset());
    untouch();
}

EditableMap::EditableMap(Resources& resources, Subsystem& subsystem,
    const std::string& filename) throw (Exception)
    : Map(subsystem, filename), resources(resources), subsystem(subsystem)
{
    char buffer[256];

    set_background(get_background());
    set_tileset(get_tileset());
    untouch();

    /* grab objects */
    int num_objects = atoi(get_value("objects").c_str());
    for (int i = 0; i < num_objects; i++) {
        sprintf(buffer, "object_name%d", i);
        const std::string& object_name = get_value(buffer);
        sprintf(buffer, "object_x%d", i);
        int x = atoi(get_value(buffer).c_str());
        sprintf(buffer, "object_y%d", i);
        int y = atoi(get_value(buffer).c_str());
        try {
            Object *obj = resources.get_object(object_name);
            EditableObject *eobj = new EditableObject(obj, x, y);
            objects.push_back(eobj);
        } catch (const Exception& e) {
            subsystem << "WARNING: " << e.what() << std::endl;
        }
    }

    /* grab light sources */
    int num_lights = atoi(get_value("lights").c_str());
    for (int i = 0; i < num_lights; i++) {
        sprintf(buffer, "light_x%d", i);
        int x = atoi(get_value(buffer).c_str());
        sprintf(buffer, "light_y%d", i);
        int y = atoi(get_value(buffer).c_str());
        sprintf(buffer, "light_radius%d", i);
        int radius = atoi(get_value(buffer).c_str());
        lights.push_back(new EditableLight(x, y, radius));
    }

    create_lightmap();
}

EditableMap::~EditableMap() {
    /* delete objects */
    for (Objects::iterator it = objects.begin(); it != objects.end(); it++) {
        delete *it;
    }
    objects.clear();

    /* delete light sources */
    for (Lights::iterator it = lights.begin(); it != lights.end(); it++) {
        delete *it;
    }
    lights.clear();
}

Tileset *EditableMap::get_tileset_ptr() const {
    return ptileset;
}

Background *EditableMap::get_background_ptr() const {
    return pbackground;
}

EditableMap::Objects& EditableMap::get_objects() {
    return objects;
}

EditableMap::Lights& EditableMap::get_light_sources() {
    return lights;
}

void EditableMap::set_parallax_shift(int parallax) {
    set_value("parallax_shift", parallax);
    this->parallax = parallax;
}

void EditableMap::set_decoration_brightness(double brightness) {
    set_value("decoration_brightness", brightness);
    this->decoration_brightness = brightness;
}

void EditableMap::set_lightmap_alpha(double alpha) {
    set_value("lightmap_alpha", alpha);
    this->lightmap_alpha = alpha;
}

void EditableMap::set_tileset(const std::string& tileset) {
    set_value("tileset", tileset);
    this->tileset = tileset;

    ptileset = 0;
    try {
        ptileset = resources.get_tileset(tileset);
    } catch (const ResourcesException& e) {
        subsystem << e.what() << std::endl;
    }
}

void EditableMap::set_background(const std::string& background) {
    set_value("background", background);
    this->background = background;

    pbackground = 0;
    try {
        pbackground = resources.get_background(background);
    } catch (const ResourcesException& e) {
        subsystem << e.what() << std::endl;
    }
}

void EditableMap::resize_map(int new_width, int new_height) {
    touch();

    /* create new map/decoration */
    short **new_map = create_new_array(map, new_width, new_height);
    short **new_decoration = create_new_array(decoration, new_width, new_height);
    cleanup();
    map = new_map;
    decoration = new_decoration;

    width = new_width;
    height = new_height;
}

short **EditableMap::create_new_array(short **from, int new_width, int new_height) {
    short index;
    short **new_map = new short *[new_height];
    for (int y = 0; y < new_height; y++) {
        new_map[y] = new short[new_width];
        for (int x = 0; x < new_width; x++) {
            if (x < width && y < height) {
                index = from[y][x];
            } else {
                index = -1;
            }
            new_map[y][x] = index;
        }
    }

    return new_map;
}

void EditableMap::set_tile(int x, int y, short index) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        touch();
        map[y][x] = index;
    }
}

void EditableMap::set_decoration(int x, int y, short index) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        touch();
        decoration[y][x] = index;
    }
}

void EditableMap::set_object(Object *obj, int x, int y) {
    bool found = false;
    for (Objects::iterator it = objects.begin(); it != objects.end(); it++) {
        EditableObject *eobj = *it;
        if (eobj->x == x && eobj->y == y) {
            found = true;
            break;
        }
    }

    if (!found) {
        EditableObject *eobj = new EditableObject(obj, x, y);
        eobj->spawn_counter = 30;
        objects.push_back(eobj);
        touch();
    }

}

void EditableMap::set_game_play_type(GamePlayType type) {
    game_play_type = type;
    touch();
}

void EditableMap::set_frog_spawn_init(int t) {
    frog_spawn_init = t;
    touch();
}

void EditableMap::erase_object(int x, int y) {
    for (Objects::iterator it = objects.begin(); it != objects.end(); it++) {
        EditableObject *eobj = *it;
        if (eobj->x == x && eobj->y == y) {
            delete eobj;
            objects.erase(it);
            touch();
            break;
        }
    }
}

void EditableMap::set_light(int x, int y) {
    bool found = false;
    for (Lights::iterator it = lights.begin(); it != lights.end(); it++) {
        EditableLight *elgt = *it;
        if (elgt->x == x && elgt->y == y) {
            found = true;
            break;
        }
    }

    if (!found) {
        lights.push_back(new EditableLight(x, y, 64));
        touch();
    }

}

void EditableMap::erase_light(int x, int y) {
    for (Lights::iterator it = lights.begin(); it != lights.end(); it++) {
        EditableLight *elgt = *it;
        if (elgt->x == x && elgt->y == y) {
            delete elgt;
            lights.erase(it);
            touch();
            break;
        }
    }
}

void EditableMap::save() throw (Exception) {
    char buffer[256];

    /* drop header informations */
    int height = atoi(get_value("height").c_str());
    for (int i = 0; i < height; i++) {
        sprintf(buffer, "tiles%d", i);
        set_value(buffer, "");
        sprintf(buffer, "decoration%d", i);
        set_value(buffer, "");
    }

    /* drop objects */
    int cnt_objects = atoi(get_value("objects").c_str());
    for (int i = 0; i < cnt_objects; i++) {
        sprintf(buffer, "object_name%d", i);
        set_value(buffer, "");
        sprintf(buffer, "object_x%d", i);
        set_value(buffer, "");
        sprintf(buffer, "object_y%d", i);
        set_value(buffer, "");
    }

    /* drop light sources */
    int cnt_lights = atoi(get_value("lights").c_str());
    for (int i = 0; i < cnt_lights; i++) {
        sprintf(buffer, "light_x%d", i);
        set_value(buffer, "");
        sprintf(buffer, "light_y%d", i);
        set_value(buffer, "");
        sprintf(buffer, "light_radius%d", i);
        set_value(buffer, "");
    }

    int width = get_width();
    height = get_height();
    set_value("width", width);
    set_value("height", height);
    save_array("tiles", get_map(), width, height);
    save_array("decoration", get_decoration(), width, height);

    set_value("game_play_type", static_cast<int>(game_play_type));
    set_value("frog_spawn_init", frog_spawn_init);

    cnt_objects = static_cast<int>(objects.size());
    set_value("objects", cnt_objects);
    for (int i = 0; i < cnt_objects; i++) {
        EditableObject *eobj = objects[i];
        sprintf(buffer, "object_name%d", i);
        set_value(buffer, eobj->object->get_name());
        sprintf(buffer, "object_x%d", i);
        set_value(buffer, eobj->x);
        sprintf(buffer, "object_y%d", i);
        set_value(buffer, eobj->y);
    }

    cnt_lights = static_cast<int>(lights.size());
    set_value("lights", cnt_lights);
    for (int i = 0; i < cnt_lights; i++) {
        EditableLight *elgt = lights[i];
        sprintf(buffer, "light_x%d", i);
        set_value(buffer, elgt->x);
        sprintf(buffer, "light_y%d", i);
        set_value(buffer, elgt->y);
        sprintf(buffer, "light_radius%d", i);
        set_value(buffer, elgt->radius);
    }

    std::string save_dir = get_home_directory() + dir_separator + UserDirectory;
    create_directory("maps", save_dir);
    KeyValue::save(save_dir + dir_separator + "maps" + dir_separator + get_name() + ".map");
}

void EditableMap::save_array(const char *prefix, short **from, int width, int height) {
    char buffer[128];
    for (int y = 0; y < height; y++) {
        std::string t;
        for (int x = 0; x < width; x++) {
            int index = from[y][x];
            if (x < width - 1) {
                sprintf(buffer, "%d,", index);
            } else {
                sprintf(buffer, "%d", index);
            }
            t += buffer;
        }
        sprintf(buffer, "%s%d", prefix, y);
        set_value(buffer, t);
    }
}
