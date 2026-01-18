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
    inline void loop() {
        Vector3 targetHeadPos = { 0, 0, 0 };
        float closestDistance = FLT_MAX;
        bool foundTarget = false;

        int centerX = (g_game.game_bounds.right - g_game.game_bounds.left) / 2;
        int centerY = (g_game.game_bounds.bottom - g_game.game_bounds.top) / 2;

        {
            std::lock_guard<std::mutex> lock(reader_mutex);
            for (const auto& player : g_game.players) {
                if (player.headPos3D.DistTo(player.origin) > 100.0f) continue;
                if (player.bones.bonePositions.find("head") == player.bones.bonePositions.end()) continue;

                Vector3 screenHead = player.bones.bonePositions.at("head");
                if (screenHead.z < 0.01f || !utils.is_in_bounds(screenHead, g_game.game_bounds.right, g_game.game_bounds.bottom)) continue;

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
