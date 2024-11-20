/*

Aimbot.hpp (Finds closest player to the mouse to lock onto enemies)
Authors: 0Zayn (Zayn)

*/

#pragma once

#include <Windows.h>
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include <optional>
#include <memory>
#include <mutex>

#include <Entities/Entities.hpp>
#include <Math/Math.hpp>
#include <Globals.hpp>

class CAimbot {
public:
    void Run() noexcept;
    static constexpr int DEFAULT_KEY = 'E';

private:
    struct Target {
        Vector3 Angle;
        float Distance;

        bool operator<(const Target& Other) const noexcept {
            return Distance < Other.Distance;
        }
    };

    std::optional<Vector3> GetPlayer() noexcept;

    Vector3 CalculateAngle(const Vector3& Start, const Vector3& End) const noexcept;
    void ApplySmoothing(const Vector3& Target) noexcept;

    std::optional<Vector3> GetLocalEye() const noexcept;
    std::optional<Vector3> GetEntityEye(const CEntities::Entity& Entity) const noexcept;

    bool SetViewAngles(const Vector3& Angles) noexcept;
    Vector3 CalculateBezier(float T, const std::array<Vector3, 4>& Points) const noexcept;

    static constexpr float PI = 3.14159265358979323846f;
    static constexpr float RAD2DEG = 180.0f / PI;
    static constexpr float ANGLE_EPSILON = 0.1f;
};

inline auto Aimbot = std::make_unique<CAimbot>();