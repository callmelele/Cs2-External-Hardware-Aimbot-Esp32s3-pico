#include "utils.h"

void Utils::update_console_title() {
    std::string title = "CS2 Silent - Bezier Active";

    title += " | FOV: " + std::to_string((int)config::aim_fov);

    SetConsoleTitle(title.c_str());
}

bool Utils::is_in_bounds(const Vector3& pos, int width, int height) {
    return pos.x >= 0 && pos.x <= width && pos.y >= 0 && pos.y <= height;
}