//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

namespace
{
    const XMVECTORF32 START_POSITION = { 0.f, 0.0f, 0.f, 0.f };
    const float ROTATION_GAIN = 24.f;
    const float MOVEMENT_GAIN = 1.f;
}

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);

    auto size = m_deviceResources->GetOutputSize();
    float width = size.right;
    float height = size.bottom;

    SetCursorPos((int)(width / 2), (int)(height / 2));
}

Game::~Game()
{
	audio.~AudioModule();
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();


    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

	// Initialize Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, context);

    // Initialize light
    light.setAmbientColour(0.8f, 0.8f, 0.8f, 1.0f);
    light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
    light.setPosition(2.0f, 1.0f, 1.0f);
    light.setDirection(-1.0f, -1.0f, 0.0f);

	// Initialize audio
	audio.Initialize();

	// Initialize input
	input.Initialize(window);

    // Initialize camera
    camera.Initialize(START_POSITION.v);


}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	inputCommands = input.getGameInput();
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });
	UpdateGUI();
    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    float elapsedTime = float(timer.GetElapsedSeconds());

    // Add your game logic here.
    UpdateCamera();
	audio.Update();
	input.Update();

    terrain.Update();

	if (inputCommands.escape) {
		ExitGame();
	}

    elapsedTime;
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.
    m_world = SimpleMath::Matrix::Identity; //set world back to identity
    SimpleMath::Matrix newPosition3 = SimpleMath::Matrix::CreateTranslation(0.f, -3.5f, 15.0f);
    SimpleMath::Matrix newRotation = SimpleMath::Matrix::CreateRotationX(XM_PI);		//scale the terrain down a little. 
    m_world = m_world * newRotation * newPosition3;

    //m_room->Draw(Matrix::Identity, camera.view, camera.projection, Colors::White, m_roomTex.Get());
    basicShaderPair.EnableShader(context);
    basicShaderPair.SetShaderParameters(context, &m_world, &(Matrix)camera.view, &(Matrix)camera.projection, &light, m_texture1.Get(), m_texture2.Get());
    terrain.Render(context);
    
    context;

    m_deviceResources->PIXEndEvent();

    
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // Game is becoming active window.
    input.Reset();
}

void Game::OnDeactivated()
{
    // Game is becoming background window.
}

void Game::OnSuspending()
{
    // Game is being power-suspended (or minimized).
	audio.OnSuspending();
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // Game is being power-resumed (or returning from minimize).
    input.Reset();
	audio.OnResuming();
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Initialize device dependent objects here (independent of window size).

    // Shader
    basicShaderPair.InitStandard(device, L"light_vs.cso", L"light_ps.cso");

    // Textures
    CreateDDSTextureFromFile(device, L"Assets/seafloor.dds", nullptr, m_texture1.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"Assets/grass.dds", nullptr, m_texture2.ReleaseAndGetAddressOf());
    // Terrain
    terrain.Initialize(device, 128, 128);

    device;
    context;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    auto size = m_deviceResources->GetOutputSize();
    float backBufferWidth = size.right;
    float backBufferHeight = size.bottom;

    camera.projection = Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(70.f),
        backBufferWidth / backBufferHeight, 0.01f, 100.f);
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}

void Game::UpdateCamera()
{
    float deltaTime = float(m_timer.GetElapsedSeconds());

    auto mouse = input.mouse->GetState();

    if (!inputCommands.ctrl)
    {
        input.mouse->SetMode(Mouse::MODE_RELATIVE);
    }
    else
    {
        input.mouse->SetMode(Mouse::MODE_ABSOLUTE);
    }
    if (mouse.positionMode == Mouse::MODE_RELATIVE)
    {
        Vector3 delta = Vector3(float(mouse.x), float(mouse.y), 0.f) * ROTATION_GAIN * deltaTime;

        camera.rotation.x -= delta.y * ROTATION_GAIN * deltaTime;
        camera.rotation.y -= delta.x * ROTATION_GAIN * deltaTime;

        // limit pitch to straight up or straight down
        // with a little fudge-factor to avoid gimbal lock
        float limit = 90.0f - 0.01f;
        camera.rotation.x = (std::max)(-limit, camera.rotation.x);
        camera.rotation.x = (std::min)(+limit, camera.rotation.x);

        if (camera.rotation.y > 360.f)
        {
            camera.rotation.y -= 360.f;
        }
        else if (camera.rotation.y < -360.f)
        {
            camera.rotation.y += 360.f;
        }
        
    }

    Vector3 move = Vector3::Zero;

    if (inputCommands.forward)
        move += camera.forward;

    if (inputCommands.back)
        move -= camera.forward;

    if (inputCommands.left)
        move -= camera.right;

    if (inputCommands.right)
        move += camera.right;

    if (inputCommands.space)
        move.y += 1.f;

    if (inputCommands.q_key)
        move.y -= 1.f;

    move *= MOVEMENT_GAIN * deltaTime;

    camera.position += move;
    camera.Update();

    m_view = camera.view;
}


void Game::UpdateGUI() 
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	if (show_window)
	{
		ImGui::Begin("Window", &show_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Camera Pitch: %f", camera.rotation.x);
        ImGui::Text("Camera Yaw: %f", camera.rotation.y);
        ImGui::Text("Camera Position X: %f", camera.position.x);
        ImGui::Text("Camera Position Y: %f", camera.position.y);
        ImGui::Text("Camera Position Z: %f", camera.position.z);

        if (ImGui::Button("Close Me"))
			show_window = false;
		ImGui::End();
	}

}
#pragma endregion
