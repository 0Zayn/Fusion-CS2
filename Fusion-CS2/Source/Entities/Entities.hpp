#pragma once

#include <Windows.h>
#include <vector>
#include <memory>
#include <string>

#include <Globals.hpp>
#include <Offsets.hpp>
#include <Math/Math.hpp>

class CEntities {
public:
    struct Entity {
        uintptr_t Base{};
        std::string Name;

        int Health{};
        int Team{};

        Vector3 HeadPos;
        Vector3 FeetPos;

        bool IsValid() const { return Base && Health > 0 && Health <= 100; }
    };

    static inline const uintptr_t Client = reinterpret_cast<uintptr_t>(GetModuleHandle("client.dll"));

    void Update();
    const std::vector<Entity>& GetEntities() const { return Entities; }

private:
    struct EntityData {
        uintptr_t Controller{};
        uintptr_t Pawn{};
        std::string Name;
    };

    EntityData GetEntityData(uintptr_t ListEntry, int Index) const;
   
    Vector3 GetPosition(uintptr_t Pawn) const;
    bool IsValidEntity(const EntityData& Data, int LocalTeam) const;

    std::vector<Entity> Entities;
};

inline auto Entities = std::make_unique<CEntities>();