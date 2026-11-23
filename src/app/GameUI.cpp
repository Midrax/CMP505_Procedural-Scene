#include "core/pch.h"
#include "app/Game.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

void Game::UpdateGUI()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    if (m_show_window)
    {
        ImGui::Begin("Controls", &m_show_window);
        ImGui::Text("Press R to add Random Noise");
        ImGui::Text("Press F to add Faulting");
        ImGui::Text("Press N to add Perlin Noise");
        ImGui::Text("Press V to add Voronoi Regions");
        ImGui::Text("Press X to add Smoothening");
        ImGui::Text("Press P to loop between Post Processing effects");
        ImGui::Text("Camera Position X: %f", m_camera.GetPosition().x);
        ImGui::Text("Camera Position Y: %f", m_camera.GetPosition().y);
        ImGui::Text("Camera Position Z: %f", m_camera.GetPosition().z);
        float dist = m_camera.GetPosition().y - debug_float + 2;
        ImGui::Text("Ground Distance: %f", dist);
        if (ImGui::Button("Close Me"))
            m_show_window = false;
        ImGui::End();
    }

}
