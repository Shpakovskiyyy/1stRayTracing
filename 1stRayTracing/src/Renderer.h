#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "Walnut/Image.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

class Renderer
{
public:
    struct Settings
    {
        bool Accumulate { true };
    };

public:
    Renderer() = default;

    void OnResize(uint32_t Width, uint32_t Height);
    void Render(const Scene& Scene, const Camera& Camera);
    std::shared_ptr<Walnut::Image> GetFinalImage() const;

    void ResetFrameIndex() { m_FrameIndex = 1; }
    Settings& GetSettings() { return m_Settings; }

public:
    glm::vec3 LightDirection { -1.f, -1.f, -1.f };
    int Bounces { 5 };
    bool IsMultiThreaded { true };

private:
    struct HitPayload
    {
        float HitDistance;
        glm::vec3 WorldPosition;
        glm::vec3 WorldNormal;
        int ObjectIndex;
    };

private:
    glm::vec4 PerPixel(uint32_t x, uint32_t y);
    HitPayload TraceRay(const Ray& Ray);
    HitPayload ClosestHit(const Ray& Ray, float HitDistance, int ObjectIndex);
    HitPayload Miss(const Ray& Ray);

private:
    std::shared_ptr<Walnut::Image> m_FinalImage;
    uint32_t* m_ImageData  { nullptr };
    glm::vec4* m_AccumulationData { nullptr };

    std::vector<uint32_t> m_ImageHorizontalIter,  m_ImageVerticalIter;

    uint32_t m_FrameIndex { 1 };

    Settings m_Settings;

    const Scene* m_ActiveScene;
    const Camera* m_ActiveCamera;
    
};
