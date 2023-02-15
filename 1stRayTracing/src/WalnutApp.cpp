#include "Camera.h"
#include "Renderer.h"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

#include "glm/gtc/type_ptr.hpp"

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer() : m_Camera { 45.f, 0.1f, 100.f }
	{
		Material& MainSphere = m_Scene.Materials.emplace_back();
		MainSphere.Albedo = { 1.0f, 0.0f, 1.0f };
		MainSphere.Roughness = 0.f;
		MainSphere.Metallic = 0.f;

		Material& Floor = m_Scene.Materials.emplace_back();
		Floor.Albedo = { 0.2f, 0.3f, 1.0f };
		Floor.Roughness = 0.f;
		Floor.Metallic = 0.1f;

		{
			Sphere ToPush;
			ToPush.Position = { 0.0f, 0.0f, 0.0f };
			ToPush.Radius = 1.f;
			ToPush.MaterialIndex = 0;
			m_Scene.Spheres.push_back(ToPush);
		}

		{
			Sphere ToPush;
			ToPush.Position = { 0.f, -101.f, 0.f };
			ToPush.Radius = 100.f;
			ToPush.MaterialIndex = 1;
			m_Scene.Spheres.push_back(ToPush);
		}
	}

	virtual void OnUpdate(float ts) override
	{
	    if(m_Camera.OnUpdate(ts))
	    {
	        m_Renderer.ResetFrameIndex();
	    }
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Render Time: %.3f", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
		    Render();
		}

		ImGui::Checkbox("Accumulate: ", &m_Renderer.GetSettings().Accumulate);
		if (ImGui::Button("Reset"))
		{
		    m_Renderer.ResetFrameIndex();
		}
		ImGui::Checkbox("Multithreading: ", &m_Renderer.IsMultiThreaded);

	    ImGui::Begin("Scene");
		ImGui::Text("Objects settings: ");
		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& Sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(Sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &Sphere.Radius, 0.1f);
			ImGui::DragInt("Material", &Sphere.MaterialIndex, 1.f, 0, (int)m_Scene.Materials.size() - 1);

			ImGui::Separator();

			ImGui::PopID();
		}
		ImGui::Text("Materials settings: ");
		for (size_t i = 0; i < m_Scene.Materials.size(); i++)
		{
			ImGui::PushID(i);
			Material& Material = m_Scene.Materials[i];

		    ImGui::ColorEdit3("Albedo", glm::value_ptr(Material.Albedo));
		    ImGui::DragFloat("Roughness", &Material.Roughness, 0.01f, 0.f, 1.f);
		    ImGui::DragFloat("Metallic", &Material.Metallic, 0.01f, 0.f, 1.f);

			ImGui::Separator();

			ImGui::PopID();
		}
		ImGui::Text("Lighting settings: ");
		ImGui::SliderFloat("Light Direction X", &m_Renderer.LightDirection.x, -2.f, 2.f);
		ImGui::SliderFloat("Light Direction Y", &m_Renderer.LightDirection.y, -2.f, 2.f);
		ImGui::SliderFloat("Light Direction Z", &m_Renderer.LightDirection.z, -2.f, 2.f);
		ImGui::Text("\n"); 
		ImGui::End();

		
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 {0.f, 0.f});
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto Image = m_Renderer.GetFinalImage();
		if(Image)
			ImGui::Image(Image->GetDescriptorSet(), { (float)Image->GetWidth(), (float)Image->GetHeight() }, ImVec2 {0, 1}, ImVec2 {1, 0});

		ImGui::End();
		ImGui::PopStyleVar();
		Render();
	}

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer{};
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth {0}, m_ViewportHeight{0};
	float m_LastRenderTime {0.f};

};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "1stRayTracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});

	return app;
}