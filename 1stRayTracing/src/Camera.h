#pragma once

#include <vector>
#include "glm/glm.hpp"


class Camera
{
public:
    Camera(float VerticalFOVDeg, float NearClip, float FarClip);

    bool OnUpdate(float TS);
    void OnResize(uint32_t Width, uint32_t Height);

    const glm::mat4& GetProjection() const                 { return m_Projection; };
    const glm::mat4& GetInverseProjection() const          { return m_InverseProjection; }
    const glm::mat4& GetView() const                       { return m_View; }
    const glm::mat4& GetInverseView() const                { return m_InverseView; }

    const glm::vec3& GetPosition() const                   { return m_Position; }
    const glm::vec3& GetDirection() const                  { return m_ForwardDirection; }

    const std::vector<glm::vec3>& GetRayDirections() const { return m_RayDirections; }

    float GetRotationSpeed() const;

private:
    void RecalculateProjection();
    void RecalculateView();
    void RecalculateRayDirections();

private:
    float m_VerticalFOV           { 45.f };
    float m_NearClip              { 0.1f };
    float m_FarClip               { 100.f };
    float m_CameraSpeed           { 5.f };
    float m_MouseSensitivity      { 0.002f };
    float m_RotationSpeed         { 0.3f };

    glm::mat4 m_Projection        { 1.f };
    glm::mat4 m_InverseProjection { 1.f };
    glm::mat4 m_View              { 1.f };
    glm::mat4 m_InverseView       { 1.f };

    glm::vec3 m_Position          { 0.f };
    glm::vec3 m_ForwardDirection  { 0.f };
    glm::vec2 m_LastMousePosition { 0.f };

    uint32_t m_ViewportWidth      { 0 };
    uint32_t m_ViewportHeight     { 0 };

    std::vector<glm::vec3> m_RayDirections;

};



