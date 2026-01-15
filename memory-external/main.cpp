#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>

#include "serial.hpp"
#include "classes/utils.h"
#include "memory/memory.hpp"
#include "classes/vector.hpp"
#include "hacks/reader.hpp"
#include "hacks/hack.hpp"
#include "classes/globals.hpp"
#include "classes/auto_updater.hpp"


void read_thread_func() {
    while (true) {
        g_game.loop();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

int main() {
    utils.update_console_title();

    if (config::read()) std::cout << "[config] Success" << std::endl;

    if (updater::check_and_update(true)) {
        if (updater::read()) std::cout << "[updater] Success" << std::endl;
    }
    else {
        std::cout << "[!] Could not update offsets. Using local cache." << std::endl;
        updater::read();
    }

    std::cout << "[cs2] Searching for game process..." << std::endl;
    g_game.init();

    if (esp32.IsConnected()) {
        std::cout << "[hardware] ESP32-S3 Found!" << std::endl;
    }
    else {
        std::cout << "[hardware] ESP32-S3 NOT Found on COM5!" << std::endl;
    }

    // 4. Status Output
    std::cout << "-----------------------------------------------" << std::endl;
    std::cout << "[cs2] Cheat started in SILENT mode (Aimbot Only)" << std::endl;
    std::cout << "[info] No overlay window exists. System is hidden." << std::endl;
    std::cout << "[hotkey] Hold [Aim Key] to target players." << std::endl;
    std::cout << "-----------------------------------------------" << std::endl;

    std::thread read_bus(read_thread_func);
    read_bus.detach();

    while (true) {
        if (GetForegroundWindow() == g_game.process->hwnd_) {
            hack::loop();
        }

        if (GetAsyncKeyState(VK_END) & 0x8000) {
            std::cout << "[info] Shutting down..." << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}