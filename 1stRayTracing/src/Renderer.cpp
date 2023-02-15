#include "Renderer.h"

#include "Walnut/Random.h"
#include <execution>


namespace Utils
{
    static uint32_t ConvertToRGBA(const glm::vec4& Color)
    {
        auto R = (uint8_t)(Color.r * 255.f);
        auto G = (uint8_t)(Color.g * 255.f);
        auto B = (uint8_t)(Color.b * 255.f);
        auto A = (uint8_t)(Color.a * 255.f);

        return A << 24 | B << 16 | G << 8 | R;
    }
}

void Renderer::OnResize(uint32_t Width, uint32_t Height)
{
    if(m_FinalImage)
    {
        if(m_FinalImage->GetWidth() == Width && m_FinalImage->GetHeight() == Height)
            return;

        m_FinalImage->Resize(Width, Height);
    }
    else
    {
        m_FinalImage = std::make_shared<Walnut::Image>(Width, Height, Walnut::ImageFormat::RGBA);
    }

    delete[] m_ImageData;
    m_ImageData = new uint32_t[Width * Height];

    delete[] m_AccumulationData;
    m_AccumulationData = new glm::vec4[Width * Height];

    m_ImageHorizontalIter.resize(Width);
	m_ImageVerticalIter.resize(Height);
	for (uint32_t i = 0; i < Width; i++)
		m_ImageHorizontalIter[i] = i;
	for (uint32_t i = 0; i < Height; i++)
		m_ImageVerticalIter[i] = i;
}

void Renderer::Render(const Scene& Scene, const Camera& Camera)
{
    m_ActiveScene = &Scene;
    m_ActiveCamera = &Camera;

    if(m_FrameIndex == 1)
    {
        memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));
    }

    if(IsMultiThreaded)
    {
        std::for_each(std::execution::par, m_ImageVerticalIter.begin(), m_ImageVerticalIter.end(),
            [this](uint32_t y)
            {
               std::for_each(std::execution::par, m_ImageHorizontalIter.begin(), m_ImageHorizontalIter.end(),
               [this, y](uint32_t x)
               {
                   glm::vec4 PixelColor = PerPixel(x, y);
                   m_AccumulationData[x + y * m_FinalImage->GetWidth()] += PixelColor;
                   
                   glm::vec4 AccumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
                   AccumulatedColor /= m_FrameIndex;
                   
                   // Clamping values so that it's easier to deal with colors 
                   AccumulatedColor = glm::clamp(AccumulatedColor, glm::vec4 { 0.f }, glm::vec4 { 1.f });
                   
                   m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(AccumulatedColor);
               }); 
            });
    }
    else
    {
        for(uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
        {
            for(uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
            {
                glm::vec4 PixelColor = PerPixel(x, y);
                m_AccumulationData[x + y * m_FinalImage->GetWidth()] += PixelColor;
        
                glm::vec4 AccumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
                AccumulatedColor /= m_FrameIndex;
        
                // Clamping values so that it's easier to deal with colors 
                AccumulatedColor = glm::clamp(AccumulatedColor, glm::vec4 { 0.f }, glm::vec4 { 1.f });
        
                m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(AccumulatedColor);
            }
        }
    }

    m_FinalImage->SetData(m_ImageData);

    if(m_Settings.Accumulate) m_FrameIndex++;
    else ResetFrameIndex();
}

std::shared_ptr<Walnut::Image> Renderer::GetFinalImage() const
{
    return m_FinalImage;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
    Ray ray;
    ray.Origin = m_ActiveCamera->GetPosition();
    ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

    glm::vec3 FinalColor { 0.f };
    float Multiplier { 1.f }; 

    for(int i = 0; i < Bounces; i++)
    {
        Renderer::HitPayload Payload { TraceRay(ray) };
        if(Payload.HitDistance < 0.f) 
        {
            glm::vec3 SkyColor { 0.6f, 0.7f, 0.9f };
            FinalColor += SkyColor * Multiplier;
            break;
        }

        LightDirection = glm::normalize(LightDirection);
        float LightIntensity = glm::max(glm::dot(Payload.WorldNormal, -LightDirection), 0.0f);
        
        const Sphere& Sphere = m_ActiveScene->Spheres[Payload.ObjectIndex];
        const Material& Material = m_ActiveScene->Materials[Sphere.MaterialIndex];
        
        glm::vec3 SphereColor { Material.Albedo };
        SphereColor *= LightIntensity;
        FinalColor += SphereColor * Multiplier;
        Multiplier *= 0.5f;

        ray.Origin = Payload.WorldPosition + Payload.WorldNormal * 0.0001f;
        ray.Direction = glm::reflect(ray.Direction, Payload.WorldNormal + Material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
    }

    return glm::vec4 { FinalColor, 1.f };
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& Ray, float HitDistance, int ObjectIndex)
{
    Renderer::HitPayload Payload;
    Payload.HitDistance = HitDistance;
    Payload.ObjectIndex = ObjectIndex;

    const Sphere& ClosestSphere = m_ActiveScene->Spheres[ObjectIndex];

    glm::vec3 Origin = Ray.Origin - ClosestSphere.Position;
    Payload.WorldPosition = Origin + Ray.Direction * HitDistance;
    Payload.WorldNormal = glm::normalize(Payload.WorldPosition);

    Payload.WorldPosition += ClosestSphere.Position;

    return Payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& Ray)
{
    Renderer::HitPayload Payload;
    Payload.HitDistance = -1.f;

    return Payload;
}

Renderer::HitPayload Renderer::TraceRay(const Ray& Ray)
{
    // (bx^2 + by^2 + bz^2)t^2 + (ax^2 + ay^2 + az^2)t + (ax^2 + ay^2 + az^2 - r^2) = 0
    // Where
    // a = ray origin
    // b = ray direction
    // r = radius
    // t = hit distance

    int ClosestSphereIndex = -1;
    float HitDistance = FLT_MAX;

    for(size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
    {
        const Sphere& Sphere = m_ActiveScene->Spheres[i];

        glm::vec3 Origin = Ray.Origin - Sphere.Position;

        float A = glm::dot(Ray.Direction, Ray.Direction);
        float B = 2.f * glm::dot(Origin, Ray.Direction);
        float C = glm::dot(Origin, Origin) - Sphere.Radius * Sphere.Radius;
        
        // Discriminant = b^2 - 4ac
        const float Discriminant = B * B - 4 * A * C;
        
        // Ray didn't hit any sphere
        if(Discriminant < 0.f) continue;
        
        // t = -b +- sqrt(discriminant) / 2a
        // Where t = hit distance
        float ClosestT = (-B - glm::sqrt(Discriminant)) / (2.f * A);

        if(ClosestT > 0.f && ClosestT < HitDistance)
        {
            HitDistance = ClosestT;
            ClosestSphereIndex = (int)i;
        }
    }

    if(ClosestSphereIndex < 0) return Miss(Ray);

    return ClosestHit(Ray, HitDistance, ClosestSphereIndex);
}
