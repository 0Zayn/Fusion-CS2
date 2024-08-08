/*

Math.cpp (Simple mathematics utilized by the cheat)
Authors: 0Zayn (Zayn)

*/

#include "Math.hpp"

const float ScreenWidth = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
const float ScreenHeight = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));

bool Vector3::WorldToScreen(Vector2& Out, float(*ViewMatrix)[4][4]) const {
    const float W = (*ViewMatrix)[3][0] * X + (*ViewMatrix)[3][1] * Y + (*ViewMatrix)[3][2] * Z + (*ViewMatrix)[3][3];

    if (W <= 0.01f)
        return false;

    const float InvW = 1.0f / W;

    Out.X = (ScreenWidth / 2.0f) + (((*ViewMatrix)[0][0] * X + (*ViewMatrix)[0][1] * Y + (*ViewMatrix)[0][2] * Z + (*ViewMatrix)[0][3]) * InvW * (ScreenWidth / 2.0f));
    Out.Y = (ScreenHeight / 2.0f) - (((*ViewMatrix)[1][0] * X + (*ViewMatrix)[1][1] * Y + (*ViewMatrix)[1][2] * Z + (*ViewMatrix)[1][3]) * InvW * (ScreenHeight / 2.0f));

    return true;
}