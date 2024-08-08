/*

Math.hpp (Simple mathematics utilized by the cheat)
Authors: 0Zayn (Zayn)

*/

#pragma once

#include <Windows.h>

struct Vector2 {
    constexpr Vector2(
        const float X = 0.f,
        const float Y = 0.f
    ) noexcept : X(X), Y(Y) {}

    float X, Y;

    // Overload current operators, since they wouldnt work with our aimbot
    Vector2 operator-(const Vector2& other) const {
        return Vector2(X - other.X, Y - other.Y);
    }

    Vector2 operator*(float scalar) const {
        return Vector2(X * scalar, Y * scalar);
    }

    Vector2 operator+(const Vector2& other) const {
        return Vector2(X + other.X, Y + other.Y);
    }

    Vector2& operator+=(const Vector2& other) {
        X += other.X;
        Y += other.Y;
        return *this;
    }
};

struct Vector3 {
    constexpr Vector3(
        const float X = 0.f,
        const float Y = 0.f,
        const float Z = 0.f
    ) noexcept : X(X), Y(Y), Z(Z) {}

    bool WorldToScreen(Vector2& Out, float(*ViewMatrix)[4][4]) const;

    float X, Y, Z;
};

extern const float ScreenWidth;
extern const float ScreenHeight;