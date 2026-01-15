#pragma once
#include "../memory/memory.hpp"
#include "../classes/vector.hpp"
#include <map>
#include <vector>
#include <mutex>

struct view_matrix_t {
    float matrix[4][4];
    float* operator[](int index) { return matrix[index]; }
};

class CPlayer {
public:
    uintptr_t entity;
    int team;
    uintptr_t pCSPlayerPawn;
    uintptr_t gameSceneNode;
    uintptr_t boneArray;
    int health;
    Vector3 origin;
    Vector3 headPos3D;
    struct { std::map<std::string, Vector3> bonePositions; } bones;
};

class CGame {
public:
    std::shared_ptr<pProcess> process;
    ProcessModule base_client, base_engine;
    RECT game_bounds;
    int localTeam;
    std::vector<CPlayer> players;
    void init();
    void loop();
    Vector3 world_to_screen(Vector3* v);
private:
    view_matrix_t view_matrix;
    uintptr_t entity_list, localPlayer;
    uintptr_t localPlayerPawn;
};

inline CGame g_game;
inline std::mutex reader_mutex;