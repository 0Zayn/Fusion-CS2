/*

Entities.hpp (Finds players in-game)
Authors: 0Zayn (Zayn)

*/

#pragma once

#include <cstdint>
#include <vector>
#include <mutex>
#include <unordered_map>

#include <Math/Math.hpp>
#include <Offsets.hpp>

namespace Entities {
    extern uintptr_t Client;

    struct Entity {
        uintptr_t Base;

        std::string Name;
        int Health;
        int Team;

        Vector3 HeadPos;
        Vector3 FeetPos;

        bool IsValid() const;
    };

    extern std::vector<Entity> VisualsList;
    extern std::vector<Entity> AimbotList;
    extern std::mutex EntityM;

    void UpdateVisuals();
    void UpdateAimbot();
}
