#include "reader.hpp"
#include "../classes/config.hpp"
#include "../classes/auto_updater.hpp"

void CGame::init() {
    std::cout << "[cs2] Waiting for cs2.exe..." << std::endl;

    process = std::make_shared<pProcess>();

    // Using the Hijack method for better security
    while (!process->AttachProcessHj("cs2.exe")) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[cs2] Finding game modules..." << std::endl;
    do {
        base_client = process->GetModule("client.dll");
        base_engine = process->GetModule("engine2.dll");
        if (base_client.base == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    } while (base_client.base == 0 || base_engine.base == 0);

    std::cout << "[cs2] Attached and modules parsed successfully.\n" << std::endl;

    GetClientRect(process->hwnd_, &game_bounds);
}

void CGame::loop() {
    std::lock_guard<std::mutex> lock(reader_mutex);

    localPlayer = process->read<uintptr_t>(base_client.base + updater::offsets::dwLocalPlayerController);
    if (!localPlayer) return;

    entity_list = process->read<uintptr_t>(base_client.base + updater::offsets::dwEntityList);
    view_matrix = process->read<view_matrix_t>(base_client.base + updater::offsets::dwViewMatrix);
    localTeam = process->read<int>(localPlayer + updater::offsets::m_iTeamNum);

    std::vector<CPlayer> list;

    for (int i = 1; i < 64; i++) {
        uintptr_t list_entry = process->read<uintptr_t>(entity_list + (8 * (i & 0x7FFF) >> 9) + 16);
        if (!list_entry) continue;

        uintptr_t controller = process->read<uintptr_t>(list_entry + 112 * (i & 0x1FF));
        if (!controller) continue;

        int team = process->read<int>(controller + updater::offsets::m_iTeamNum);
        if (team == localTeam) continue;

        // 1. Get the Pawn Handle
        uintptr_t playerPawnHandle = process->read<std::uint32_t>(controller + updater::offsets::m_hPlayerPawn);
        if (!playerPawnHandle) continue;

        // 2. Resolve the Pawn Address
        uintptr_t list_entry2 = process->read<uintptr_t>(entity_list + 0x8 * ((playerPawnHandle & 0x7FFF) >> 9) + 16);
        uintptr_t pCSPlayerPawn = process->read<uintptr_t>(list_entry2 + 112 * (playerPawnHandle & 0x1FF));
        if (!pCSPlayerPawn) continue;

        // 3. Health check
        int health = process->read<int>(pCSPlayerPawn + updater::offsets::m_iHealth);
        if (health <= 0 || health > 100) continue;

        CPlayer player;
        player.entity = controller;
        player.pCSPlayerPawn = pCSPlayerPawn;
        player.team = team;
        player.health = health;

        player.origin = process->read<Vector3>(player.pCSPlayerPawn + updater::offsets::m_vOldOrigin);
        player.gameSceneNode = process->read<uintptr_t>(player.pCSPlayerPawn + updater::offsets::m_pGameSceneNode);
        player.boneArray = process->read<uintptr_t>(player.gameSceneNode + 0x210);

        Vector3 headPos = process->read<Vector3>(player.boneArray + 6 * 32);

        player.headPos3D = headPos;

        if (headPos.x == 0 && headPos.y == 0) continue;

        if (player.headPos3D.DistTo(player.origin) > 100.0f) continue;

        player.bones.bonePositions["head"] = world_to_screen(&headPos);

        list.push_back(player);
    }
    players = list;
}

Vector3 CGame::world_to_screen(Vector3* v) {
    float w = view_matrix[3][0] * v->x + view_matrix[3][1] * v->y + view_matrix[3][2] * v->z + view_matrix[3][3];
    if (w < 0.01f) return { 0,0,0 };

    float inv_w = 1.f / w;
    float _x = (view_matrix[0][0] * v->x + view_matrix[0][1] * v->y + view_matrix[0][2] * v->z + view_matrix[0][3]) * inv_w;
    float _y = (view_matrix[1][0] * v->x + view_matrix[1][1] * v->y + view_matrix[1][2] * v->z + view_matrix[1][3]) * inv_w;

    float x = game_bounds.right * .5f;
    float y = game_bounds.bottom * .5f;
    x += 0.5f * _x * game_bounds.right + 0.5f;
    y -= 0.5f * _y * game_bounds.bottom + 0.5f;

    return { x, y, w };
}