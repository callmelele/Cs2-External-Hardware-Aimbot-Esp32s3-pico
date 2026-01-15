#pragma once
#include <string>

namespace config {
    inline bool show_aimbot = true;
    inline float aim_fov = 200.0f;

    inline bool team_esp = false;
    inline bool show_box_esp = false;
    inline bool show_skeleton_esp = false;
    inline bool show_head_tracker = false;
    inline bool show_extra_flags = false;
    inline bool automatic_update = false;
    inline float render_distance = -1.0f;

    inline bool read() { return true; }
    inline void save() {}
}