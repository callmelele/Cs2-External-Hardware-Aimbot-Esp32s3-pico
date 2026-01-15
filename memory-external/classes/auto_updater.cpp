#include "auto_updater.hpp"
#include <iostream> 
#include <fstream>   
#include <iomanip>   

namespace updater {
    bool check_and_update(bool automatic_update) {
        json commit;
        if (!get_last_commit_date(commit)) {
            std::cout << "[updater] error getting last commit information from GitHub" << std::endl;
            return false;
        }

        if (!file_good(file_path)) {
            std::cout << "[updater] offsets.json missing. Downloading..." << std::endl;
            return download_file(raw_updated_offsets.c_str(), file_path.c_str());
        }
        return true;
    }

    bool get_last_commit_date(json& commit) {
        HINTERNET hInternet = InternetOpen("AutoUpdater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) return false;

        HINTERNET hConnect = InternetOpenUrlA(hInternet, github_repo_api.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) {
            InternetCloseHandle(hInternet);
            return false;
        }

        char buffer[4096];
        DWORD bytesRead;
        std::string commitData;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            commitData.append(buffer, bytesRead);
        }

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);

        try {
            json data = json::parse(commitData);
            if (data.is_array() && !data.empty()) {
                commit = data[0]["commit"]["author"];
                return true;
            }
        }
        catch (...) {}
        return false;
    }

    bool download_file(const char* url, const char* localPath) {
        HINTERNET hInternet = InternetOpen("AutoUpdater", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
        if (!hInternet) return false;

        HINTERNET hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (!hConnect) {
            InternetCloseHandle(hInternet);
            return false;
        }

        std::ofstream outFile(localPath, std::ios::binary);
        char buffer[4096];
        DWORD bytesRead;

        while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            outFile.write(buffer, bytesRead);
        }

        outFile.close();
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return true;
    }

    bool file_good(const std::string& name) {
        std::ifstream f(name.c_str());
        return f.good();
    }

    bool read() {
        if (!file_good(file_path)) return false;

        std::ifstream f(file_path);
        json data;
        try { data = json::parse(f); }
        catch (...) { return false; }

        if (data.empty()) return false;

        build_number = data.value("build_number", 0);

        // Core Aimbot Offsets
        offsets::dwLocalPlayerController = data["dwLocalPlayerController"];
        offsets::dwEntityList = data["dwEntityList"];
        offsets::dwViewMatrix = data["dwViewMatrix"];
        offsets::dwBuildNumber = data["dwBuildNumber"];

        offsets::m_hPlayerPawn = data["m_hPlayerPawn"];
        offsets::m_iTeamNum = data["m_iTeamNum"];
        offsets::m_iHealth = data["m_iHealth"];
        offsets::m_vOldOrigin = data["m_vOldOrigin"];
        offsets::m_pGameSceneNode = data["m_pGameSceneNode"];

        return true;
    }

    void save() {
        json data;
        data["build_number"] = build_number;
        data["dwLocalPlayerController"] = offsets::dwLocalPlayerController;
        data["dwEntityList"] = offsets::dwEntityList;
        data["dwViewMatrix"] = offsets::dwViewMatrix;
        data["dwBuildNumber"] = offsets::dwBuildNumber;
        data["m_hPlayerPawn"] = offsets::m_hPlayerPawn;
        data["m_iTeamNum"] = offsets::m_iTeamNum;
        data["m_iHealth"] = offsets::m_iHealth;
        data["m_vOldOrigin"] = offsets::m_vOldOrigin;
        data["m_pGameSceneNode"] = offsets::m_pGameSceneNode;

        std::ofstream output(file_path);
        output << std::setw(4) << data << std::endl;
    }
}