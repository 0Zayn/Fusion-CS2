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

    template <typename ReadType>
    BOOL Read(uintptr_t Address, ReadType& Value, size_t Size)
    {
        if (Address == 0 || Address < 0xAA70AA8 || Address > 0x7FFD5DC070000)
            return false;

        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(reinterpret_cast<LPCVOID>(Address), &mbi, sizeof(mbi)) == 0)
            return false;

        if (mbi.State != MEM_COMMIT)
            return false;

        if ((mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) || !(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)))
            return false;

        if (Size > (static_cast<size_t>(mbi.RegionSize) - (Address - reinterpret_cast<uintptr_t>(mbi.BaseAddress))))
            return false;

        memcpy(&Value, reinterpret_cast<void*>(Address), Size);

        return true;
    }
}
