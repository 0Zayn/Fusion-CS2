/*

Visuals.cpp (Simple in-game visuals, includes variations of ESP)
Authors: 0Zayn (Zayn)

*/

#include "Visuals.hpp"

std::mutex DrawM;

ImColor HealthColor(float HealthPercentage) {
    float r = (1.0f - HealthPercentage) * 255.0f;
    float g = HealthPercentage * 255.0f;
    return ImColor(static_cast<int>(r), static_cast<int>(g), 0);
}

void DrawEntity(const Entities::Entity& entity, ImDrawList* DrawList) {
    auto ViewMatrix = reinterpret_cast<float(*)[4][4]>(Entities::Client + Offsets::ViewMatrix);

    Vector2 Feet, Head;
    if (entity.FeetPos.WorldToScreen(Feet, ViewMatrix) && entity.HeadPos.WorldToScreen(Head, ViewMatrix)) {
        float Height = Feet.Y - Head.Y;
        float Width = Height * 0.5f;

        float HealthBarWidth = 4.0f;
        float HealthPadding = 4.0f;
        float HealthHeight = Height * (entity.Health / 100.0f);

        float HealthPercentage = entity.Health / 100.0f;

        ImColor BarColor = HealthColor(HealthPercentage);
        ImColor BackgroundColor = ImColor(0x00, 0x00, 0x00, 76);
        ImColor BorderColor = ImColor(255, 255, 255);
        ImColor TextColor = ImColor(255, 255, 255);

        ImVec2 MousePos = ImGui::GetMousePos();

        ImVec2 BoxMin = { Feet.X - Width / 2, Head.Y };
        ImVec2 BoxMax = { Feet.X + Width / 2, Head.Y + Height };

        if (MousePos.x >= BoxMin.x && MousePos.x <= BoxMax.x &&
            MousePos.y >= BoxMin.y && MousePos.y <= BoxMax.y) {
            BorderColor = ImColor(234, 93, 96);
            TextColor = ImColor(234, 93, 96);
        }

        if (Globals::ESP::Box) {
            DrawList->AddRectFilled(
                BoxMin,
                BoxMax,
                BackgroundColor,
                0.0f
            );
            DrawList->AddRect(
                BoxMin,
                BoxMax,
                BorderColor,
                0.0f
            );
        }

        if (Globals::ESP::Name) {
            ImVec2 TextPos = { Feet.X - Width / 2, Head.Y - 15.0f };

            DrawList->AddText(
                TextPos,
                TextColor,
                entity.Name.c_str()
            );
        }

        if (Globals::ESP::Health) {
            DrawList->AddRectFilled(
                { Feet.X - Width / 2 - HealthBarWidth - HealthPadding, Head.Y },
                { Feet.X - Width / 2 - HealthPadding, Head.Y + Height },
                BackgroundColor,
                10.0f
            );
            DrawList->AddRectFilled(
                { Feet.X - Width / 2 - HealthBarWidth - HealthPadding, Head.Y + Height - HealthHeight },
                { Feet.X - Width / 2 - HealthPadding, Head.Y + Height },
                BarColor,
                10.0f
            );
        }

        if (Globals::ESP::HealthText) {
            ImVec2 TextPos = { Feet.X - Width / 2 + 2.0f, Head.Y + Height - 15.0f };

            DrawList->AddText(
                ImVec2(TextPos.x, TextPos.y),
                TextColor,
                std::to_string(static_cast<int>(entity.Health)).c_str()
            );
        }
    }
}

void Visuals::Players() {
    Entities::UpdateVisuals();

    ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

    std::lock_guard<std::mutex> lock(Entities::EntityM);
    for (const auto& entity : Entities::VisualsList) {
        DrawEntity(entity, DrawList);
    }
}