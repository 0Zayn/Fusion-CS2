/* 

Entities.cpp (Finds players in-game)
Authors: 0Zayn (Zayn)

*/

#include "Entities.hpp"

namespace Entities {
    std::vector<Entity> VisualsList;
    std::vector<Entity> AimbotList;
    std::mutex EntityM;

    uintptr_t Client = reinterpret_cast<uintptr_t>(GetModuleHandle("client.dll"));

    bool Entity::IsValid() const {
        return Base && Health > 0 && Health <= 100;
    }

    void UpdateVisuals() {
        std::lock_guard<std::mutex> lock(EntityM);
        VisualsList.clear();

        auto ViewMatrix = reinterpret_cast<float(*)[4][4]>(Client + Offsets::ViewMatrix);
        auto LocalPlayer = *reinterpret_cast<uintptr_t*>(Client + Offsets::LocalPlayer);
        if (!LocalPlayer) return;

        auto LocalTeam = *reinterpret_cast<int*>(Client + Offsets::Team);
        auto EntityArray = *reinterpret_cast<uintptr_t*>(Client + Offsets::EntityList);
        if (!EntityArray) return;

        for (int i = 0; i < 64; ++i) {
            auto LE1 = *reinterpret_cast<uintptr_t*>(EntityArray + (8 * (i & 0x7FFF) >> 9) + 16);
            if (!LE1) continue;

            auto PlayerController = *reinterpret_cast<uintptr_t*>(LE1 + 120 * (i & 0x1FF));
            if (!PlayerController) continue;

            auto Player = *reinterpret_cast<uint32_t*>(PlayerController + Offsets::Player);
            if (!Player) continue;

            auto LE2 = *reinterpret_cast<uintptr_t*>(EntityArray + 0x8 * ((Player & 0x7FFF) >> 9) + 16);
            if (!LE2) continue;

            auto PlayerPawn = *reinterpret_cast<uintptr_t*>(LE2 + 120 * (Player & 0x1FF));
            if (!PlayerPawn || PlayerPawn == LocalPlayer) continue;

            int Health = *reinterpret_cast<int*>(PlayerPawn + Offsets::Health);
            if (Health <= 0 || Health > 100)
                continue;

            int Team = *reinterpret_cast<int*>(PlayerPawn + Offsets::Team);
            if (Team == LocalTeam) continue;

            Vector3 FeetPos = *reinterpret_cast<Vector3*>(PlayerPawn + Offsets::OldOrigin);
            Vector3 HeadPos = { FeetPos.X, FeetPos.Y, FeetPos.Z + 75.0f };

            VisualsList.push_back({ PlayerPawn, "Player", Health, Team, HeadPos, FeetPos });
        }
    }

    void UpdateAimbot() {
        std::lock_guard<std::mutex> lock(EntityM);
        AimbotList.clear();

        auto ViewMatrix = reinterpret_cast<float(*)[4][4]>(Client + Offsets::ViewMatrix);
        auto LocalPlayer = *reinterpret_cast<uintptr_t*>(Client + Offsets::LocalPlayer);
        if (!LocalPlayer) return;

        auto LocalTeam = *reinterpret_cast<int*>(Client + Offsets::Team);
        auto EntityArray = *reinterpret_cast<uintptr_t*>(Client + Offsets::EntityList);
        if (!EntityArray) return;

        for (int i = 0; i < 64; ++i) {
            auto LE1 = *reinterpret_cast<uintptr_t*>(EntityArray + (8 * (i & 0x7FFF) >> 9) + 16);
            if (!LE1) continue;

            auto PlayerController = *reinterpret_cast<uintptr_t*>(LE1 + 120 * (i & 0x1FF));
            if (!PlayerController) continue;

            auto Player = *reinterpret_cast<uint32_t*>(PlayerController + Offsets::Player);
            if (!Player) continue;

            auto LE2 = *reinterpret_cast<uintptr_t*>(EntityArray + 0x8 * ((Player & 0x7FFF) >> 9) + 16);
            if (!LE2) continue;

            auto PlayerPawn = *reinterpret_cast<uintptr_t*>(LE2 + 120 * (Player & 0x1FF));
            if (!PlayerPawn || PlayerPawn == LocalPlayer) continue;

            int Health = *reinterpret_cast<int*>(PlayerPawn + Offsets::Health);
            if (Health <= 0 || Health > 100)
                continue;

            int Team = *reinterpret_cast<int*>(PlayerPawn + Offsets::Team);
            if (Team == LocalTeam) continue;

            Vector3 FeetPos = *reinterpret_cast<Vector3*>(PlayerPawn + Offsets::OldOrigin);
            Vector3 HeadPos = { FeetPos.X, FeetPos.Y, FeetPos.Z + 65.0f };

            AimbotList.push_back({ PlayerPawn, "Player", Health, Team, HeadPos, FeetPos });
        }
    }
}