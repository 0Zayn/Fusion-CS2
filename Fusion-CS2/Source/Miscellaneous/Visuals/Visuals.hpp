/*

Visuals.cpp (Simple in-game visuals, includes variations of ESP)
Authors: 0Zayn (Zayn)

*/

#pragma once

#include <cstdint>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <string>

#include <Math/Math.hpp>
#include <Offsets.hpp>
#include <Entities/Entities.hpp>

#include <Globals.hpp>
#include <ImGui/imgui.h>

namespace Visuals {
	void Players();
}