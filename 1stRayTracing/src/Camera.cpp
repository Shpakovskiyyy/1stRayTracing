#include "Camera.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Walnut/Input/Input.h"

Camera::Camera(float VerticalFOVDeg, float NearClip, float FarClip)
    : m_VerticalFOV { VerticalFOVDeg }, m_NearClip { NearClip }, m_FarClip { FarClip }
{
    m_ForwardDirection = glm::vec3{ 0, 0, -1 };
    m_Position = glm::vec3{ 0, 0, 6 };
}

bool Camera::OnUpdate(float TS)
{
    //TODO REFACTOR

    glm::vec2 MousePosition = Walnut::Input::GetMousePosition();
    glm::vec2 Delta = (MousePosition - m_LastMousePosition) * m_MouseSensitivity;
    m_LastMousePosition = MousePosition;

    if(!Walnut::Input::IsMouseButtonDown(Walnut::MouseButton::Right))
    {
        Walnut::Input::SetCursorMode(Walnut::CursorMode::Normal);
        return false;
    }

    Walnut::Input::SetCursorMode(Walnut::CursorMode::Locked);

    bool HasMoved { false };

    constexpr glm::vec3 CameraUpDirection { 0.f, 1.f, 0.f};
    glm::vec3 CameraRightDirection = glm::cross(m_ForwardDirection, CameraUpDirection);

    if(Walnut::Input::IsKeyDown(Walnut::KeyCode::W))
    {
        m_Position += m_ForwardDirection * m_CameraSpeed * TS;
        HasMoved = true;
    }
    if(Walnut::Input::IsKeyDown(Walnut::KeyCode::S))
    {
        m_Position -= m_ForwardDirection * m_CameraSpeed * TS;
        HasMoved = true;
    }
    if(Walnut::Input::IsKeyDown(Walnut::KeyCode::A))
    {
        m_Position -= CameraRightDirection * m_CameraSpeed * TS;
        HasMoved = true;
    }
    if(Walnut::Input::IsKeyDown(Walnut::KeyCode::D))
    {
        m_Position += CameraRightDirection * m_CameraSpeed * TS;
        HasMoved = true;
    }
    if(Walnut::Input::IsKeyDown(Walnut::KeyCode::Q))
    {
        m_Position += CameraUpDirection * m_CameraSpeed * TS;
        HasMoved = true;
    }
    if(Walnut::Input::IsKeyDown(Walnut::KeyCode::E))
    {
        m_Position -= CameraUpDirection * m_CameraSpeed * TS;
        HasMoved = true;
    }

    if(Delta != glm::vec2 {0.f})
    {
        float YawDelta = Delta.x * GetRotationSpeed();
        float PitchDelta = Delta.y * GetRotationSpeed();

        glm::quat RotationQuat { glm::normalize((glm::cross(glm::angleAxis(-PitchDelta, CameraRightDirection), glm::angleAxis(-YawDelta, CameraUpDirection)))) };
        m_ForwardDirection = glm::rotate(RotationQuat, m_ForwardDirection);

        HasMoved = true;
    }

    if(HasMoved)
    {
        RecalculateView();
        RecalculateRayDirections();
    }

    return HasMoved;
}

void Camera::OnResize(uint32_t Width, uint32_t Height)
{
    if(Width == m_ViewportWidth && Height == m_ViewportHeight)
    {
        return;
    }

    m_ViewportWidth = Width;
    m_ViewportHeight = Height;

    RecalculateProjection();
    RecalculateRayDirections();
}

float Camera::GetRotationSpeed() const
{
    return m_CameraSpeed;
}

void Camera::RecalculateProjection()
{
    m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewportWidth, (float)m_ViewportHeight, m_NearClip, m_FarClip);
    m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::RecalculateView()
{
    m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3 {0.f, 1.f, 0.f});
    m_InverseView = glm::inverse(m_View);
}

void Camera::RecalculateRayDirections()
{
    m_RayDirections.resize(m_ViewportWidth * m_ViewportHeight);

    for(uint32_t y = 0; y < m_ViewportHeight; y++)
    {
        for(uint32_t x = 0; x < m_ViewportWidth; x++)
        {
            glm::vec2 Coordinate { (float)x / (float)m_ViewportWidth, (float)y / (float)m_ViewportHeight};
            Coordinate = Coordinate * 2.f - 1.f;

            glm::vec4 Target = m_InverseProjection * glm::vec4 { Coordinate.x, Coordinate.y, 1, 1, };
            glm::vec3 RayDirection = glm::vec3 { m_InverseView * glm::vec4(glm::normalize(glm::vec3 { Target } / Target.w), 0) };
            m_RayDirections[x + y * m_ViewportWidth] = RayDirection;
        }
    }

}