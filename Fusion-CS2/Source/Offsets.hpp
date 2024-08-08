/*

Offsets.hpp (Offsets utilized by the cheat)
Authors: 0Zayn (Zayn), a2x (cs2-dumper)

Dumper: https://github.com/a2x/cs2-dumper

*/

#pragma once

#include <cstddef>

namespace Offsets {
	// These change almost every update
	constexpr std::ptrdiff_t EntityList = 0x1964198;	// dwEntityList			(offsets.hpp)
	constexpr std::ptrdiff_t LocalPlayer = 0x17CF698;	// dwLocalPlayerPawn	(offsets.hpp)
	constexpr std::ptrdiff_t ViewMatrix = 0x19C6250;	// dwViewMatrix			(offsets.hpp)

	// These don't change as often
	constexpr std::ptrdiff_t Player = 0x7DC;			// m_hPlayerPawn		(client_dll.hpp)

	constexpr std::ptrdiff_t Health = 0x324;			// m_iHealth			(client_dll.hpp)
	constexpr std::ptrdiff_t Team = 0x3C3;				// m_iTeamNum			(client_dll.hpp)

	constexpr std::ptrdiff_t OldOrigin = 0x1274;		// m_vOldOrigin			(client_dll.hpp)
}