/*

Aimbot.cpp (Finds closest player to the mouse to lock onto enemies)
Authors: 0Zayn (Zayn)

*/

#include "Aimbot.hpp"

namespace Aimbot {
    void AimbotSystem::Run() {
        if (Globals::Aimbot::Enabled) {
            if (GetAsyncKeyState(KeybindCode) & 0x8000) {
                Entities::UpdateAimbot();

                auto ClosestEntity = GetClosestPlayer();

                if (ClosestEntity.X != 0.0f && ClosestEntity.Y != 0.0f)
                    MoveMouse(ClosestEntity);
            }
        }
    }

    Vector2 AimbotSystem::GetClosestPlayer() {
        POINT MousePos;
        GetCursorPos(&MousePos);

        Vector2 ClosestPos{ 0.0f, 0.0f };
        float ClosestDistance = 1e30f;

        std::lock_guard<std::mutex> Lock(Entities::EntityM);
        for (const auto& Entity : Entities::AimbotList) {
            if (!Entity.IsValid()) continue;

            Vector2 ScreenPos;
            if (Entity.HeadPos.WorldToScreen(ScreenPos, reinterpret_cast<float(*)[4][4]>(Entities::Client + Offsets::ViewMatrix))) {
                float Distance = CalculateDistance({ static_cast<float>(MousePos.x), static_cast<float>(MousePos.y) }, ScreenPos);

                if (Distance < ClosestDistance) {
                    ClosestDistance = Distance;
                    ClosestPos = ScreenPos;
                }
            }
        }

        return ClosestPos;
    }

    void AimbotSystem::MoveMouse(const Vector2& TargetPos) {
        POINT CurrentPos;
        GetCursorPos(&CurrentPos);

        Vector2 StartPos(static_cast<float>(CurrentPos.x), static_cast<float>(CurrentPos.y));

        if (Globals::Aimbot::StickyAim) { // Needs fixing ong
            Vector2 Delta(
                TargetPos.X - static_cast<float>(CurrentPos.x),
                TargetPos.Y - static_cast<float>(CurrentPos.y)
            );

            Delta.X *= Globals::Aimbot::Smoothing;
            Delta.Y *= Globals::Aimbot::Smoothing;

            mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(Delta.X), static_cast<DWORD>(Delta.Y), 0, 0);
        }
        else {
            Vector2 ControlPoint1 = StartPos + (TargetPos - StartPos) * 0.25f;
            Vector2 ControlPoint2 = StartPos + (TargetPos - StartPos) * 0.75f;

            const int Steps = 50;
            const float SmoothingFactor = Globals::Aimbot::Smoothing;

            for (int i = 1; i <= Steps; ++i) {
                float t = static_cast<float>(i) / Steps;
                Vector2 Point = Bezier(StartPos, ControlPoint1, ControlPoint2, TargetPos, t);

                Vector2 Smoothed = StartPos + (Point - StartPos) * SmoothingFactor;

                Vector2 Delta(
                    Smoothed.X - static_cast<float>(CurrentPos.x),
                    Smoothed.Y - static_cast<float>(CurrentPos.y)
                );

                mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(Delta.X), static_cast<DWORD>(Delta.Y), 0, 0);

                CurrentPos.x += static_cast<LONG>(Delta.X);
                CurrentPos.y += static_cast<LONG>(Delta.Y);

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    float AimbotSystem::CalculateDistance(const Vector2& A, const Vector2& B) {
        return std::hypot(B.X - A.X, B.Y - A.Y);
    }

    Vector2 AimbotSystem::Bezier(const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3, float t) {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        Vector2 p = p0 * uuu;       // (1-t)^3 * P0
        p = p + p1 * (3 * uu * t);  // 3 * (1-t)^2 * t * P1
        p = p + p2 * (3 * u * tt);  // 3 * (1-t) * t^2 * P2
        p = p + p3 * ttt;           // t^3 * P3

        return p;
    }
}