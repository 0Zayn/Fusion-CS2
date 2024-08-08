/*

Offsets.hpp (Offsets utilized by the cheat)
Authors: 0Zayn (Zayn), a2x (cs2-dumper)

Dumper: https://github.com/a2x/cs2-dumper

*/

#pragma once

#include <cstddef>

namespace Offsets {
	constexpr std::ptrdiff_t EntityList = 0x19BEED0;
	constexpr std::ptrdiff_t LocalPlayer = 0x1824A18;
	constexpr std::ptrdiff_t ViewMatrix = 0x1A20CF0;

	constexpr std::ptrdiff_t Player = 0x7DC;

	constexpr std::ptrdiff_t Health = 0x324;
	constexpr std::ptrdiff_t Team = 0x3C3;

	constexpr std::ptrdiff_t OldOrigin = 0x1274;
}