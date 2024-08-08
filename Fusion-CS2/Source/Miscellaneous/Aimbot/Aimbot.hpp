/*

Aimbot.hpp (Finds closest player to the mouse to lock onto enemies)
Authors: 0Zayn (Zayn)

*/

#pragma once

#include <Windows.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>
#include <chrono>
#include <thread>

#include <Entities/Entities.hpp>
#include <Math/Math.hpp>
#include <Globals.hpp>

namespace Aimbot {
    class AimbotSystem {
    public:
        static void Run();
        static constexpr int KeybindCode = 'E';

    private:
        static Vector2 GetClosestPlayer();

        static void MoveMouse(const Vector2& TargetPos);
        static float CalculateDistance(const Vector2& A, const Vector2& B);

        static Vector2 Bezier(const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3, float t);
    };
}