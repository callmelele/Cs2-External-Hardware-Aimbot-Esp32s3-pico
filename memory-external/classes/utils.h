#pragma once
#include <windows.h>
#include <string>

#include "vector.hpp"
#include "config.hpp"

class Utils {
public:
    void update_console_title();
    bool is_in_bounds(const Vector3& pos, int width, int height);
};

inline Utils utils;