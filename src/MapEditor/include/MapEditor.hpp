#ifndef MAPEDITOR_HPP
#define MAPEDITOR_HPP

#include "Resources.hpp"
#include "Subsystem.hpp"
#include "Exception.hpp"
#include "Gui.hpp"
#include "EditableMap.hpp"
#include "CompileThreadBlock.hpp"
#include "CompileThreadPixel.hpp"
#include "Mutex.hpp"
#include "Configuration.hpp"

class MapEditorException : public Exception {
public:
    MapEditorException(const char *msg) : Exception(msg) { }
    MapEditorException(const std::string& msg) : Exception(msg) { }
};

class MapEditor : public Gui {
public:
    MapEditor(Resources& resources, Subsystem& subsystem, Configuration& config);
    virtual ~MapEditor();

private:
    enum DrawMode {
        DrawModeTile = 0,
        DrawModeObject,
        DrawModeLightSources,
        _DrawModeMAX
    };

    Resources& resources;
    Subsystem& subsystem;
    Configuration& config;
    bool running;
    int top;
    int left;
    int selected_tile_index;
    int selected_object_index;
    int mouse_down_mode;
    bool move_map;
    int move_origin_x;
    int move_origin_y;
    int move_origin_left;
    int move_origin_top;
    EditableMap *wmap;
    Icon *center_icon;
    bool mirrored_drawing;
    bool draw_tile_props;
    bool draw_tile_zorder;
    bool draw_tile_number;
    bool draw_even_only;
    bool drawing_decoration;
    bool draw_decoration_on_screen;
    bool draw_lightmap_on_screen;
    bool draw_map_on_screen;
    bool draw_objects_on_screen;
    bool draw_light_sources;
    DrawMode draw_mode;
    Icon *light_source;
    CompileThread *compile_thread;
    std::string home_workdir;

    int lightmap_w;
    int lightmap_h;
    unsigned char **lightmap;
    gametime_t now;
    gametime_t last;
    GuiWindow *win_compile;
    GuiLabel *lbl_compile;

    /* implementation of Gui::idle() and Gui::on_input_event() */
    virtual void idle() throw (Exception);
    virtual void on_input_event(const InputData& input);
    void hand_draw(int x, int y);

    /* helpers */
    void create_toolbox();
    void add_close_button(GuiWindow *window, GuiButton::OnClick on_click = 0);
    void add_ok_cancel_buttons(GuiWindow *window, GuiButton::OnClick on_click);
    void add_ok_cancel_buttons(GuiWindow *window, GuiButton::OnClick on_ok_click, GuiButton::OnClick on_cancel_click);
    void draw_background();
    void draw_decoration();
    void draw_lightmap();
    void draw_map(bool background);
    void draw_objects();
    void draw_lights();
    void place_tile(int x, int y, bool erasing);
    void place_object(int x, int y, bool erasing);
    void place_light(int x, int y, bool erasing);
    void mouse_move_map(int x, int y);

    static void static_center_map(GuiButton *sender, void *data);
    void center_map();

    static void static_window_close_click(GuiButton *sender, void *data);

    /* mode selector */
    static void static_mode_selector_click(GuiButton *sender, void *data);
    static void static_os_close_click(GuiButton *sender, void *data);
    void mode_selector_click();

    /* map properties */
    GuiTextbox *mp_name;
    GuiTextbox *mp_frog_spawn_init;
    GuiTextbox *mp_author;
    GuiTextbox *mp_description;
    GuiTextbox *mp_width;
    GuiTextbox *mp_height;
    GuiTextbox *mp_deco_brightness;
    GuiTextbox *mp_lightmap_alpha;
    GuiTextbox *mp_parallax;
    GuiListbox *mp_tileset;
    GuiListbox *mp_background;
    GuiCheckbox *mp_cb_dm;
    GuiCheckbox *mp_cb_tdm;
    GuiCheckbox *mp_cb_ctf;
    GuiCheckbox *mp_cb_sr;
    GuiCheckbox *mp_cb_ctc;
    GuiCheckbox *mp_cb_goh;

    static void static_map_properties_click(GuiButton *sender, void *data);
    void map_properties_click();

    static void static_mp_dm_click(GuiCheckbox *sender, void *data, bool state);
    void mp_dm_click();

    static void static_mp_tdm_click(GuiCheckbox *sender, void *data, bool state);
    void mp_tdm_click();

    static void static_mp_ctf_click(GuiCheckbox *sender, void *data, bool state);
    void mp_ctf_click();

    static void static_mp_sr_click(GuiCheckbox *sender, void *data, bool state);
    void mp_sr_click();

    static void static_mp_ctc_click(GuiCheckbox *sender, void *data, bool state);
    void mp_ctc_click();

    static void static_mp_goh_click(GuiCheckbox *sender, void *data, bool state);
    void mp_goh_click();

    static void static_mp_ok_click(GuiButton *sender, void *data);
    void mp_ok_click();

    /* tile selector */
    static void static_tile_selector_click(GuiButton *sender, void *data);
    void tile_selector_click();
    void add_buttons(bool first, GuiWindow *window, int from_tile_index);
    GuiWindow *ts_window;
    GuiPicture *ts_picture;
    int ts_start_index;
    int ts_end_index;
    int ts_buttons_on_page;
    static void static_ts_previous_click(GuiButton *sender, void *data);
    void ts_previous_click();
    static void static_ts_next_click(GuiButton *sender, void *data);
    void ts_next_click();
    static void static_ts_tile_click(GuiButton *sender, void *data);
    void ts_tile_click(GuiButton *sender);
    static void static_ts_close_click(GuiButton *sender, void *data);
    static bool static_ts_keydown(GuiWindow *sender, void *data, int keycode, bool repeat);
    void ts_close_click();
    static void static_ts_properties_click(GuiButton *sender, void *data);
    void ts_properties_click();
    static void static_ts_properties_ok_click(GuiButton *sender, void *data);
    void ts_properties_ok_click();
    GuiCheckbox *tsp_background;
    GuiCheckbox *tsp_light_blocking;
    GuiListbox *tsp_type;
    GuiTextbox *tsp_speed;
    GuiTextbox *tsp_friction;
    void save_tileset() throw (Exception);

    /* object selector */
    static void static_object_selector_click(GuiButton *sender, void *data);
    void object_selector_click();
    GuiWindow *os_window;
    GuiPicture *os_picture;

    static void static_os_object_click(GuiButton *sender, void *data);
    void os_object_click(GuiButton *sender);
    static bool static_os_keydown(GuiWindow *sender, void *data, int keycode, bool repeat);
    void os_close_click();

    /* open map */
    static void static_load_map_click(GuiButton *sender, void *data);
    void load_map_click();
    static void static_load_map_ok_click(GuiButton *sender, void *data);
    void load_map_ok_click();
    GuiListbox *lm_maps;

    /* save map */
    static void static_save_map_click(GuiButton *sender, void *data);
    void save_map_click();

    /* hcopy */
    static void static_hcopy_click(GuiButton *sender, void *data);
    void hcopy_click();

    /* calculate lightmaps */
    static void static_calculate_light_click(GuiButton *sender, void *data);
    static void static_calculate_light_pixel_click(GuiButton *sender, void *data);
    void calculate_light(bool pixel_precise);
    void finalise_lightmap();

    /* zap */
    static void static_zap_click(GuiButton *sender, void *data);
    void zap_click();

    /* options */
    static void static_options_click(GuiButton *sender, void *data);
    void options_click();

    static void static_options_ok_click(GuiButton *sender, void *data);
    void options_ok_click();

    static void static_options_cancel_click(GuiButton *sender, void *data);
    void options_cancel_click();

    static void static_options_value_changed(GuiScroll *sender, void *data, int value);
    void options_value_changed(int value);

    static void static_options_fs_clicked(GuiCheckbox *sender, void *data, bool state);
    void options_fs_clicked(bool state);

    static void static_options_sl_clicked(GuiCheckbox *sender, void *data, bool state);
    void options_sl_clicked(bool state);

    bool opt_old_fullscreen;
    bool opt_old_scanlines;
    float sl_intensity_init_value;
    GuiCheckbox *opt_fullscreen;
    GuiCheckbox *opt_scanlines;
    GuiHScroll *opt_intensity;

    /* exit editor */
    static void static_exit_click(GuiButton *sender, void *data);
    void exit_click();
};

#endif // MAPEDITOR_HPP
