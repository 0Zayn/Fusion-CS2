/* 

Entities.cpp (Finds players in-game)
Authors: 0Zayn (Zayn)

*/

#include "Entities.hpp"

void CEntities::Update() {
    Entities.clear();

    if (!Client) return;

    const auto EntityList = *reinterpret_cast<uintptr_t*>(Client + Offsets::EntityList);
    const auto LocalPawn = *reinterpret_cast<uintptr_t*>(Client + Offsets::LocalPlayer);
    const auto LocalTeam = *reinterpret_cast<int*>(Client + Offsets::Team);

    if (!EntityList || !LocalPawn) return;

    for (int i = 0; i < 64; ++i) {
        const auto EntityData = GetEntityData(EntityList, i);
        if (!IsValidEntity(EntityData, LocalTeam)) continue;

        const auto Health = *reinterpret_cast<int*>(EntityData.Pawn + Offsets::Health);
        if (Health <= 0 || Health > 100) continue;

        const auto Origin = *reinterpret_cast<Vector3*>(EntityData.Pawn + Offsets::OldOrigin);
        const auto ViewOffset = *reinterpret_cast<Vector3*>(EntityData.Pawn + Offsets::ViewOffset);

        Entity CurrentEntity{};

        CurrentEntity.Base = EntityData.Pawn;
        CurrentEntity.Name = EntityData.Name;

        CurrentEntity.Health = Health;
        CurrentEntity.Team = *reinterpret_cast<int*>(EntityData.Pawn + Offsets::Team);

        CurrentEntity.FeetPos = Origin;
        CurrentEntity.HeadPos = Vector3(Origin.X + ViewOffset.X, Origin.Y + ViewOffset.Y, Origin.Z + ViewOffset.Z + 10.0f);

        Entities.push_back(std::move(CurrentEntity));
    }
}

CEntities::EntityData CEntities::GetEntityData(uintptr_t ListEntry, int Index) const {
    EntityData Data{};

    const auto E1 = *reinterpret_cast<uintptr_t*>(ListEntry + (0x8 * (Index & 0x7FFF) >> 0x9) + 0x10);
    if (!E1) return Data;

    Data.Controller = *reinterpret_cast<uintptr_t*>(E1 + 0x78 * (Index & 0x1FF));
    if (!Data.Controller) return Data;

    const auto NamePtr = reinterpret_cast<char*>(Data.Controller + Offsets::Name);
    if (NamePtr) Data.Name = std::string(NamePtr);

    const auto Player = *reinterpret_cast<uint32_t*>(Data.Controller + Offsets::Player);
    if (!Player) return Data;

    const auto Entry2 = *reinterpret_cast<uintptr_t*>(ListEntry + 0x8 * ((Player & 0x7FFF) >> 0x9) + 0x10);
    if (!Entry2) return Data;

    Data.Pawn = *reinterpret_cast<uintptr_t*>(Entry2 + 0x78 * (Player & 0x1FF));
    return Data;
}

Vector3 CEntities::GetPosition(uintptr_t Pawn) const {
    return *reinterpret_cast<Vector3*>(Pawn + Offsets::OldOrigin);
}

bool CEntities::IsValidEntity(const EntityData& Data, int LocalTeam) const {
    if (!Data.Controller || !Data.Pawn)
        return false;

    const auto LocalPawn = *reinterpret_cast<uintptr_t*>(Client + Offsets::LocalPlayer);
    if (Data.Pawn == LocalPawn)
        return false;

    if (Globals::Misc::TeamCheck) {
        const auto EntityTeam = *reinterpret_cast<int*>(Data.Controller + Offsets::Team);
        if (EntityTeam == LocalTeam)
            return false;
    }

    return true;
}