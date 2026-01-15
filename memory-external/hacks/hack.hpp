#pragma once
#include <thread>
#include <cmath>
#include <climits>
#include <vector>
#include <mutex>
#include "reader.hpp"
#include "../classes/config.hpp"
#include "../classes/globals.hpp"
#include "algorithm"

namespace hack {
    inline std::vector<std::pair<std::string, std::string>> boneConnections = {
                        {"neck_0", "spine_1"}, {"spine_1", "spine_2"}, {"spine_2", "pelvis"},
                        {"spine_1", "arm_upper_L"}, {"arm_upper_L", "arm_lower_L"}, {"arm_lower_L", "hand_L"},
                        {"spine_1", "arm_upper_R"}, {"arm_upper_R", "arm_lower_R"}, {"arm_lower_R", "hand_R"},
                        {"pelvis", "leg_upper_L"}, {"leg_upper_L", "leg_lower_L"}, {"leg_lower_L", "ankle_L"},
                        {"pelvis", "leg_upper_R"}, {"leg_upper_R", "leg_lower_R"}, {"leg_lower_R", "ankle_R"}
    };

    inline void loop() {
        std::lock_guard<std::mutex> lock(reader_mutex);

        float closestDistance = FLT_MAX;
        Vector3 targetHeadPos = { 0, 0, 0 };
        bool foundTarget = false;

        int centerX = (g_game.game_bounds.right - g_game.game_bounds.left) / 2;
        int centerY = (g_game.game_bounds.bottom - g_game.game_bounds.top) / 2;

        for (auto it = g_game.players.begin(); it != g_game.players.end(); ++it) {
            auto& player = *it;

            if (player.headPos3D.DistTo(player.origin) > 100.0f)
                continue;

            if (player.bones.bonePositions.find("head") == player.bones.bonePositions.end())
                continue;

            Vector3 screenHead = player.bones.bonePositions.at("head");

            if (screenHead.z < 0.01f || !utils.is_in_bounds(screenHead, g_game.game_bounds.right, g_game.game_bounds.bottom))
                continue;

            if (config::show_aimbot && g_game.localTeam != player.team) {
                float dx = screenHead.x - centerX;
                float dy = screenHead.y - centerY;
                float distFromCrosshair = std::sqrt(dx * dx + dy * dy);

                if (distFromCrosshair < config::aim_fov && distFromCrosshair < closestDistance) {
                    closestDistance = distFromCrosshair;
                    targetHeadPos = screenHead;
                    foundTarget = true;
                }
            }
        }

        if (foundTarget && (GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
            float targetRelX = targetHeadPos.x - centerX;
            float targetRelY = targetHeadPos.y - centerY;
            float moveX = 0, moveY = 0;

            if (closestDistance > 2.0f) {
                float t = 0.55f;
                float u = 1.0f - t;
                float tt = t * t;
                float arcStrength = 0.35f;

                float controlX = (targetRelX * 0.5f) + (targetRelY * arcStrength);
                float controlY = (targetRelY * 0.5f) - (targetRelX * arcStrength);

                moveX = (2 * u * t * controlX) + (tt * targetRelX);
                moveY = (2 * u * t * controlY) + (tt * targetRelY);
            }
            else {
                moveX = targetRelX;
                moveY = targetRelY;
            }

            float sensitivityScale = 2.0f; 

            int8_t finalX = (int8_t)std::clamp((int)(moveX * sensitivityScale), -127, 127);
            int8_t finalY = (int8_t)std::clamp((int)(moveY * sensitivityScale), -127, 127);

            esp32.SendMove(finalX, finalY);
        }
    }
}