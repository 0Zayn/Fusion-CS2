/*

Math.hpp (Simple mathematics utilized by the cheat)
Authors: 0Zayn (Zayn)

*/

#pragma once

#include <Windows.h>
#include <complex>

struct Vector2 {
    constexpr Vector2(
        const float X = 0.f,
        const float Y = 0.f
    ) noexcept : X(X), Y(Y) {}

    float X, Y;
};

struct Vector3 {
    constexpr Vector3(
        const float X = 0.f,
        const float Y = 0.f,
        const float Z = 0.f
    ) noexcept : X(X), Y(Y), Z(Z) {}

    // Overload current operators to work with our aimbot
    Vector3 operator+(const Vector3& Other) const {
        return Vector3(X + Other.X, Y + Other.Y, Z + Other.Z);
    }

    Vector3 operator-(const Vector3& Other) const {
        return Vector3(X - Other.X, Y - Other.Y, Z - Other.Z);
    }

    Vector3 operator*(float Scalar) const {
        return Vector3(X * Scalar, Y * Scalar, Z * Scalar);
    }

    float Length() const {
        return std::sqrt(X * X + Y * Y + Z * Z);
    }

    bool WorldToScreen(Vector2& Out, float(*ViewMatrix)[4][4]) const;

    float X, Y, Z;
};

extern const float ScreenWidth;
extern const float ScreenHeight;