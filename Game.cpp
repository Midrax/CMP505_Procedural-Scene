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
    const XMVECTORF32 START_POSITION = { 0.f, 0.0f, -1.f, 0.f };
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
    light.setAmbientColour(0.5f, 0.5f, 0.5f, 1.0f);
    light.setDiffuseColour(0.5f, 0.5f, 0.5f, 1.0f);
    light.setPosition(0.0f, -15.0f, 1.0f);
    light.setDirection(-1.0f, 1.0f, 1.0f);

    // Initialize Skydome
    skydome = new Skydome;
    skydome->Initialize(device);
    skydomeShader = new SkydomeShader;
    skydomeShader->Initialize(device, window);

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
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.

    // Render Skybox
    m_deviceResources->TurnOffCulling();
    m_deviceResources->TurnZBufferOff();
    m_world = SimpleMath::Matrix::Identity * Matrix::CreateScale(10.f) * SimpleMath::Matrix::CreateTranslation(camera.GetPosition());
    skydome->Render(context);
    skydomeShader->Render(context, skydome->GetIndexCount(), m_world, m_view, m_projection, skydome->GetApexColor(), skydome->GetCenterColor());
    m_deviceResources->TurnOnCulling();
    m_deviceResources->TurnZBufferOn();

    // Render Terrain
    m_world = SimpleMath::Matrix::Identity; //set world back to identity
    SimpleMath::Matrix newPosition3 = Matrix::CreateScale(0.2f) * SimpleMath::Matrix::CreateTranslation(-10.5f, -2.0f, 10.f);
    SimpleMath::Matrix newRotation = SimpleMath::Matrix::CreateRotationX(XM_PI);
    m_world = m_world * newRotation * newPosition3;
    terrainShader.EnableShader(context);
    terrainShader.SetShaderParameters(context, &m_world, &(Matrix)m_view, &(Matrix)m_projection, &light, m_grass_texture.Get(), m_mountain_texture.Get(), m_walls_texture.Get());
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
    terrainShader.InitStandard(device, L"terrain_vs.cso", L"terrain_ps.cso");
    // Textures
    CreateDDSTextureFromFile(device, L"Assets/grass.dds", nullptr, m_grass_texture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"Assets/mountain.dds", nullptr, m_mountain_texture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"Assets/rock_diffuse.dds", nullptr, m_walls_texture.ReleaseAndGetAddressOf());
    // Terrain
    terrain.Initialize(device, 256, 256);
    terrain.GenerateHeightMap(device);
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

    m_projection = Matrix::CreatePerspectiveFieldOfView(XMConvertToRadians(90.f),
        backBufferWidth / backBufferHeight, 0.01f, 1000.f);
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

    // Release the sky dome shader object.
    if (skydomeShader)
    {
        skydomeShader->Shutdown();
        delete skydomeShader;
        skydomeShader = 0;
    }

    // Release the sky dome object.
    if (skydome)
    {
        skydome->Shutdown();
        delete skydome;
        skydome = 0;
    }

    terrain.Shutdown();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}

void Game::UpdateCamera()
{
    auto device = m_deviceResources->GetD3DDevice();

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

        camera.SetPitch(camera.GetRotation().x - delta.y * ROTATION_GAIN * deltaTime);
        camera.SetYaw(camera.GetRotation().y - delta.x * ROTATION_GAIN * deltaTime);

        // limit pitch to straight up or straight down
        // with a little fudge-factor to avoid gimbal lock
        float limit = 90.0f - 0.01f;
        camera.SetPitch((std::max)(-limit, camera.GetRotation().x));
        camera.SetPitch((std::min)(+limit, camera.GetRotation().x));

        if (camera.GetRotation().y > 360.f)
        {
            camera.SetYaw(camera.GetRotation().y - 360.f);
        }
        else if (camera.GetRotation().y < -360.f)
        {
            camera.SetYaw(camera.GetRotation().y + 360.f);
        }
        
    }

    Vector3 move = Vector3::Zero;

    if (inputCommands.forward)
        move += camera.forward;

    if (inputCommands.back)
        move -= camera.forward;

    if (inputCommands.left)
        move -= camera.right*MOVEMENT_GAIN;

    if (inputCommands.right)
        move += camera.right*MOVEMENT_GAIN;

    if (inputCommands.space)
        move.y += MOVEMENT_GAIN;

    if (inputCommands.q_key)
        move.y -= MOVEMENT_GAIN;

    move *= MOVEMENT_GAIN * deltaTime;

    if (inputCommands.v_key)
        terrain.VoronoiDungeon(device);
    if (inputCommands.n_key)
        terrain.NoiseHeightMap(device);
    if (inputCommands.f_key)
        terrain.Faulting(device);
    if (inputCommands.r_key)
        terrain.RandomHeightMap(device);
    if (inputCommands.x_key)
        terrain.SmoothenHeightMap(device, 1.25);


    camera.SetPosition(camera.GetPosition() + move);
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
        ImGui::Text("Camera Pitch: %f", camera.GetRotation().x);
        ImGui::Text("Camera Yaw: %f", camera.GetRotation().y);
        ImGui::Text("Camera Position X: %f", camera.GetPosition().x);
        ImGui::Text("Camera Position Y: %f", camera.GetPosition().y);
        ImGui::Text("Camera Position Z: %f", camera.GetPosition().z);

        if (ImGui::Button("Close Me"))
			show_window = false;
		ImGui::End();
	}

}
#pragma endregion
