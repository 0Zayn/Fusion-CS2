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

void DrawEntity(const CEntities::Entity& Entity, ImDrawList* DrawList) {
    auto ViewMatrix = reinterpret_cast<float(*)[4][4]>(CEntities::Client + Offsets::ViewMatrix);

    Vector2 Feet, Head;
    if (Entity.FeetPos.WorldToScreen(Feet, ViewMatrix) && Entity.HeadPos.WorldToScreen(Head, ViewMatrix)) {
        float Height = Feet.Y - Head.Y;
        float Width = Height * 0.5f;

        float HealthWidth = 4.0f;
        float HealthPadding = 4.0f;
        float HealthHeight = Height * (Entity.Health / 100.0f);

        float HealthPercentage = Entity.Health / 100.0f;

        ImColor Bar = HealthColor(HealthPercentage);
        ImColor Background = ImColor(0x00, 0x00, 0x00, 76);
        ImColor Border = ImColor(255, 255, 255);
        ImColor Text = ImColor(255, 255, 255);

        ImVec2 MousePos = ImGui::GetMousePos();

        ImVec2 BoxMin = { Feet.X - Width / 2, Head.Y };
        ImVec2 BoxMax = { Feet.X + Width / 2, Head.Y + Height };

        if (MousePos.x >= BoxMin.x && MousePos.x <= BoxMax.x && MousePos.y >= BoxMin.y && MousePos.y <= BoxMax.y) {
            Border = ImColor(234, 93, 96);
            Text = ImColor(234, 93, 96);
        }

        if (Globals::ESP::Box) {
            DrawList->AddRectFilled(BoxMin, BoxMax, Background, 0.0f);
            DrawList->AddRect(BoxMin, BoxMax, Border, 0.0f);
        }

        if (Globals::ESP::Name) {
            ImVec2 TextPos = { Feet.X - Width / 2, Head.Y - 15.0f };
            DrawList->AddText(TextPos, Text, Entity.Name.c_str());
        }

        if (Globals::ESP::Health) {
            DrawList->AddRectFilled({ Feet.X - Width / 2 - HealthWidth - HealthPadding, Head.Y }, { Feet.X - Width / 2 - HealthPadding, Head.Y + Height }, Background, 10.0f);
            DrawList->AddRectFilled({ Feet.X - Width / 2 - HealthWidth - HealthPadding, Head.Y + Height - HealthHeight }, { Feet.X - Width / 2 - HealthPadding, Head.Y + Height }, Bar, 10.0f);
        }

        if (Globals::ESP::HealthText) {
            ImVec2 TextPos = { Feet.X - Width / 2 + 2.0f, Head.Y + Height - 15.0f };
            DrawList->AddText(ImVec2(TextPos.x, TextPos.y), Text, std::to_string(static_cast<int>(Entity.Health)).c_str());
        }
    }
}

void DrawFOV(ImDrawList* DrawList) {
    if (Globals::Aimbot::DrawFOV) {
        ImGuiViewport* Viewport = ImGui::GetMainViewport();
        ImVec2 Center = ImVec2(Viewport->GetCenter().x, Viewport->GetCenter().y);

        DrawList->AddCircle(Center, Globals::Aimbot::FOV, ImColor(255, 255, 255), 32, 1.0f);
    }
}

void Visuals::Players() {
    Entities->Update();

    ImDrawList* DrawList = ImGui::GetBackgroundDrawList();

    DrawFOV(DrawList);

    for (const auto& Entity : Entities->GetEntities())
        DrawEntity(Entity, DrawList);
}