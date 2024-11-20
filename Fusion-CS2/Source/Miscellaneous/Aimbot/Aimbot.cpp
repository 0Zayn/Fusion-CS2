#include "Aimbot.hpp"

void CAimbot::Run() noexcept {
    if (!Globals::Aimbot::Enabled || !(GetAsyncKeyState(DEFAULT_KEY) & 0x8000))
        return;

    const auto Target = GetPlayer();
    if (!Target.has_value())
        return;

    if (Globals::Aimbot::StickyAim && Target.has_value())
        SetViewAngles(Target.value());
    else if (Target.has_value())
        ApplySmoothing(Target.value());
}

std::optional<Vector3> CAimbot::GetPlayer() noexcept {
    if (!CEntities::Client || !Entities)
        return std::nullopt;

    Entities->Update();

    const auto LocalEye = GetLocalEye();
    if (!LocalEye.has_value())
        return std::nullopt;

    auto* ViewAngles = reinterpret_cast<Vector3*>(CEntities::Client + Offsets::ViewAngles);

    const auto CurrentAngle = *ViewAngles;
    std::vector<Target> Targets;
    Targets.reserve(32);

    const auto& EntityList = Entities->GetEntities();
    for (const auto& Entity : EntityList) {
        if (!Entity.IsValid() || !Entity.Base)
            continue;

        const auto EntityEye = GetEntityEye(Entity);
        if (!EntityEye.has_value())
            continue;

        const auto Angle = CalculateAngle(LocalEye.value(), EntityEye.value());
        if (!std::isfinite(Angle.X) || !std::isfinite(Angle.Y) || !std::isfinite(Angle.Z))
            continue;

        const float Distance = std::hypot(Angle.X - CurrentAngle.X, Angle.Y - CurrentAngle.Y);
        if (!std::isfinite(Distance) || Distance > Globals::Aimbot::FOV)
            continue;

        Targets.push_back({ Angle, Distance });
    }

    if (Targets.empty())
        return std::nullopt;

    std::sort(Targets.begin(), Targets.end());
    return Targets[0].Angle;
}

void CAimbot::ApplySmoothing(const Vector3& Target) noexcept {
    auto* ViewAngles = reinterpret_cast<Vector3*>(CEntities::Client + Offsets::ViewAngles);
    if (!ViewAngles)
        return;

    auto CurrentAngle = *ViewAngles;
    const Vector3 Delta = Target - CurrentAngle;

    if (!std::isfinite(Delta.X) || !std::isfinite(Delta.Y) || !std::isfinite(Delta.Z))
        return;

    std::array<Vector3, 4> ControlPoints = {
        CurrentAngle,
        CurrentAngle + Delta * 0.3f,
        Target - Delta * 0.5f,
        Target
    };

    for (const auto& Point : ControlPoints) {
        if (!std::isfinite(Point.X) || !std::isfinite(Point.Y) || !std::isfinite(Point.Z))
            return;
    }

    const float Smoothing = std::clamp(Globals::Aimbot::Smoothing * 0.8f, 0.15f, 0.85f);
    auto NewAngle = CalculateBezier(Smoothing, ControlPoints);

    if (!std::isfinite(NewAngle.X) || !std::isfinite(NewAngle.Y) || !std::isfinite(NewAngle.Z))
        return;

    NewAngle.X = std::clamp(NewAngle.X, -89.0f, 89.0f);
    NewAngle.Y = std::fmod(NewAngle.Y + 180.0f, 360.0f) - 180.0f;
    NewAngle.Z = 0.0f;

    if (std::isfinite(NewAngle.X) && std::isfinite(NewAngle.Y) && std::isfinite(NewAngle.Z))
        SetViewAngles(NewAngle);
}

std::optional<Vector3> CAimbot::GetLocalEye() const noexcept {
    if (!CEntities::Client)
        return std::nullopt;

    auto* LocalPawnPtr = reinterpret_cast<uintptr_t*>(CEntities::Client + Offsets::LocalPlayer);

    uintptr_t LocalPawn = *LocalPawnPtr;
    if (!LocalPawn)
        return std::nullopt;

    auto* Origin = reinterpret_cast<Vector3*>(LocalPawn + Offsets::OldOrigin);
    auto* ViewOffset = reinterpret_cast<Vector3*>(LocalPawn + Offsets::ViewOffset);

    Vector3 LocalEye = *Origin + *ViewOffset;
    if (!std::isfinite(LocalEye.X) || !std::isfinite(LocalEye.Y) || !std::isfinite(LocalEye.Z))
        return std::nullopt;

    if (LocalEye.Length() < ANGLE_EPSILON)
        return std::nullopt;

    return LocalEye;
}

std::optional<Vector3> CAimbot::GetEntityEye(const CEntities::Entity& Entity) const noexcept {
    if (!Entity.Base)
        return std::nullopt;

    auto* Origin = reinterpret_cast<Vector3*>(Entity.Base + Offsets::OldOrigin);
    auto* ViewOffset = reinterpret_cast<Vector3*>(Entity.Base + Offsets::ViewOffset);

    Vector3 Result = *Origin + *ViewOffset;
    if (!std::isfinite(Result.X) || !std::isfinite(Result.Y) || !std::isfinite(Result.Z))
        return std::nullopt;

    return Result;
}

bool CAimbot::SetViewAngles(const Vector3& Angles) noexcept {
    if (!CEntities::Client)
        return false;

    auto* ViewAngles = reinterpret_cast<Vector3*>(CEntities::Client + Offsets::ViewAngles);

    if (!ViewAngles)
        return false;

    if (!std::isfinite(Angles.X) || !std::isfinite(Angles.Y) || !std::isfinite(Angles.Z))
        return false;

    Vector3 SafeAngles = Angles;
    SafeAngles.X = std::clamp(SafeAngles.X, -89.0f, 89.0f);
    SafeAngles.Y = std::fmod(SafeAngles.Y + 180.0f, 360.0f) - 180.0f;
    SafeAngles.Z = 0.0f;

    std::memcpy(ViewAngles, &SafeAngles, sizeof(Vector3));
    return true;
}

Vector3 CAimbot::CalculateAngle(const Vector3& Start, const Vector3& End) const noexcept {
    const Vector3 Delta = End - Start;
    const float Length = Delta.Length();

    if (Length < ANGLE_EPSILON || !std::isfinite(Length))
        return Vector3();

    float Pitch = -std::asin(Delta.Z / Length) * RAD2DEG;
    float Yaw = std::atan2(Delta.Y, Delta.X) * RAD2DEG;

    if (!std::isfinite(Pitch) || !std::isfinite(Yaw))
        return Vector3();

    return Vector3(Pitch, Yaw, 0.0f);
}

Vector3 CAimbot::CalculateBezier(float T, const std::array<Vector3, 4>& Points) const noexcept {
    T = std::clamp(T, 0.0f, 1.0f);

    const float U = 1.0f - T;
    const float TT = T * T;
    const float UU = U * U;

    Vector3 Result = Points[0] * (UU * U) + Points[1] * (3.0f * UU * T) + Points[2] * (3.0f * U * TT) + Points[3] * (TT * T);

    if (!std::isfinite(Result.X) || !std::isfinite(Result.Y) || !std::isfinite(Result.Z))
        return Vector3();

    return Result;
}