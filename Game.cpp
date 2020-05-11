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
	m_audio.~AudioModule();
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
    m_light.setAmbientColour(0.5f, 0.5f, 0.5f, 1.0f);
    m_light.setDiffuseColour(0.5f, 0.5f, 0.5f, 1.0f);
    m_light.setPosition(0.0f, -15.0f, 1.0f);
    m_light.setDirection(-1.0f, 1.0f, 1.0f);

    // Initialize Skydome
    m_skydome = new Skydome;
    m_skydome->Initialize(device);
    m_skydomeShader = new SkydomeShader;
    m_skydomeShader->Initialize(device, window);

    m_reflectionShader = new ReflectionShader;
    m_reflectionShader->Initialize(device, window);

    m_water = new Water;
    m_water->Initialize(device, L"Assets/water_normal.dds", 3.75f, 110.0f);
    m_waterShader = new WaterShader;
    m_waterShader->Initialize(device, window);

	// Initialize audio
	m_audio.Initialize();

	// Initialize input
	m_input.Initialize(window);

    // Initialize camera
    Vector3 start = Vector3(m_terrain.GetRooms()[0]->vPoint->x,
                            -10,
                            -m_terrain.GetRooms()[0]->vPoint->z);
    m_camera.Initialize(start*0.2);
    // camera.Initialize(START_POSITION.v);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	m_inputCommands = m_input.getGameInput();
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
	m_audio.Update();
	m_input.Update();
    m_water->Update();
	if (m_inputCommands.escape) {
		ExitGame();
	}

    elapsedTime;
}

void Game::UpdateCamera()
{
    auto device = m_deviceResources->GetD3DDevice();

    float deltaTime = float(m_timer.GetElapsedSeconds());

    auto mouse = m_input.mouse->GetState();

    if (!m_inputCommands.ctrl)
    {
        m_input.mouse->SetMode(Mouse::MODE_RELATIVE);
    }
    else
    {
        m_input.mouse->SetMode(Mouse::MODE_ABSOLUTE);
    }
    if (mouse.positionMode == Mouse::MODE_RELATIVE)
    {
        Vector3 delta = Vector3(float(mouse.x), float(mouse.y), 0.f) * ROTATION_GAIN * deltaTime;

        m_camera.SetPitch(m_camera.GetRotation().x - delta.y * ROTATION_GAIN * deltaTime);
        m_camera.SetYaw(m_camera.GetRotation().y - delta.x * ROTATION_GAIN * deltaTime);

        // limit pitch to straight up or straight down
        // with a little fudge-factor to avoid gimbal lock
        float limit = 90.0f - 0.01f;
        m_camera.SetPitch((std::max)(-limit, m_camera.GetRotation().x));
        m_camera.SetPitch((std::min)(+limit, m_camera.GetRotation().x));

        if (m_camera.GetRotation().y > 360.f)
        {
            m_camera.SetYaw(m_camera.GetRotation().y - 360.f);
        }
        else if (m_camera.GetRotation().y < -360.f)
        {
            m_camera.SetYaw(m_camera.GetRotation().y + 360.f);
        }

    }

    Vector3 move = Vector3::Zero;

    if (m_inputCommands.forward)
        move += m_camera.forward;

    if (m_inputCommands.back)
        move -= m_camera.forward;

    if (m_inputCommands.left)
        move -= m_camera.right * MOVEMENT_GAIN;

    if (m_inputCommands.right)
        move += m_camera.right * MOVEMENT_GAIN;

    if (m_inputCommands.space)
        move.y += MOVEMENT_GAIN;

    if (m_inputCommands.q_key)
        move.y -= MOVEMENT_GAIN;

    move *= MOVEMENT_GAIN * deltaTime;

    if (m_inputCommands.v_key)
        m_terrain.VoronoiDungeon(device);
    if (m_inputCommands.n_key)
        m_terrain.NoiseHeightMap(device);
    if (m_inputCommands.f_key)
        m_terrain.Faulting(device);
    if (m_inputCommands.r_key)
        m_terrain.RandomHeightMap(device);
    if (m_inputCommands.x_key)
        m_terrain.SmoothenHeightMap(device, 1.25);

    m_camera.SetPosition(m_camera.GetPosition() + move);

    bool result, foundHeight;
    float posX, posY, posZ, rotX, rotY, rotZ;
    float height;
    bool canWalk = false;
    //foundHeight = m_quadTree->GetHeightAtPosition(m_camera.GetPosition().x, m_camera.GetPosition().z, height, canWalk);
    float newPosY = m_camera.GetPosition().y;

    //if (foundHeight)
    //{
    //    // If there was a triangle under the camera then position the camera just above it by two units.
    //    if (posY < 3.0f) {
    //        newPosY = height + 2.0f;
    //        if (!canWalk) {
    //            //collision
    //            posX = m_camera.oldPosition.x;
    //            posZ = m_camera.oldPosition.z;
    //            m_camera.SetPosition(posX, posY, posZ);
    //        }
    //    }
    //}

    

    if (m_camera.GetPosition().y < -2.5f)
    { 
        m_light.setAmbientColour(0.0f, 0.5f, 0.5f, 1.0f);
    }
    else
        m_light.setAmbientColour(0.5f, 0.5f, 0.5f, 1.0f);
    m_camera.Update();

    m_view = m_camera.view;
}

void Game::UpdateGUI()
{
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    if (m_show_window)
    {
        ImGui::Begin("Window", &m_show_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Camera Pitch: %f", m_camera.GetRotation().x);
        ImGui::Text("Camera Yaw: %f", m_camera.GetRotation().y);
        ImGui::Text("Camera Position X: %f", m_camera.GetPosition().x);
        ImGui::Text("Camera Position Y: %f", m_camera.GetPosition().y);
        ImGui::Text("Camera Position Z: %f", m_camera.GetPosition().z);
        ImGui::Text("Room Position X: %f", m_terrain.GetRooms()[0]->vPoint->x*0.2);
        ImGui::Text("Room Position Y: %f", m_terrain.GetRooms()[0]->vPoint->y*0.2);
        ImGui::Text("Room Position Z: %f", m_terrain.GetRooms()[0]->vPoint->z*0.2);

        if (ImGui::Button("Close Me"))
            m_show_window = false;
        ImGui::End();
    }

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

    SimpleMath::Matrix newPosition;
    SimpleMath::Matrix newRotation;
    // TODO: Add your rendering code here.

    // Render Skybox
    m_deviceResources->TurnOffCulling();
    m_deviceResources->TurnZBufferOff();
    m_world = SimpleMath::Matrix::Identity * Matrix::CreateScale(10.f) * SimpleMath::Matrix::CreateTranslation(m_camera.GetPosition());
    m_skydome->Render(context);
    m_skydomeShader->Render(context, m_skydome->GetIndexCount(), m_world, m_view, m_projection, m_skydome->GetApexColor(), m_skydome->GetCenterColor());
    m_deviceResources->TurnOnCulling();
    m_deviceResources->TurnZBufferOn();
    
    // Render Water 
    m_world = SimpleMath::Matrix::Identity;
    newPosition = SimpleMath::Matrix::CreateTranslation(0.0f, -2.5f, 10.f);
    newRotation = SimpleMath::Matrix::CreateRotationX(XM_PI);
    m_world = m_world * newRotation * newPosition;
    RenderReflection();
    m_water->Render(context);
    m_waterShader->Render(context, m_water->GetIndexCount(), m_world, m_view, m_projection, m_camera.view,
        m_water_texture.Get(), m_water_texture.Get(), m_water->GetTexture(),
        m_camera.GetPosition(), m_water->GetNormalMapTiling(), m_water->GetWaterTranslation(), m_water->GetReflectRefractScale(),
        m_water->GetRefractionTint(), m_light.getDirection(), m_water->GetSpecularShininess());
    context;

    // Render Terrain
    m_world = SimpleMath::Matrix::Identity; //set world back to identity
    newPosition = Matrix::CreateScale(0.2f) * SimpleMath::Matrix::CreateTranslation(0, -2.0f, 0.f);
    newRotation = SimpleMath::Matrix::CreateRotationX(XM_PI);
    m_world = m_world * newRotation * newPosition;
    m_terrainShader.EnableShader(context);
    m_terrainShader.SetShaderParameters(context, &m_world, &(Matrix)m_view, &(Matrix)m_projection, &m_light, m_grass_texture.Get(), m_mountain_texture.Get(), m_walls_texture.Get());
    m_terrain.Render(context);

    // Render Water 
    m_world = SimpleMath::Matrix::Identity;
    newPosition = SimpleMath::Matrix::CreateTranslation(0.0f, -2.5f, 10.f);
    newRotation = SimpleMath::Matrix::CreateRotationX(XM_PI);
    m_world = m_world * newRotation * newPosition;
    RenderReflection();
    m_water->Render(context);
    m_waterShader->Render(context, m_water->GetIndexCount(), m_world, m_view, m_projection, m_camera.view,
        m_water_texture.Get(), m_water_texture.Get(), m_water->GetTexture(),
        m_camera.GetPosition(), m_water->GetNormalMapTiling(), m_water->GetWaterTranslation(), m_water->GetReflectRefractScale(),
        m_water->GetRefractionTint(), m_light.getDirection(), m_water->GetSpecularShininess());
    context;

    m_deviceResources->PIXEndEvent();

    
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Show the new frame.
    m_deviceResources->Present();
}

void Game::RenderReflection()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto context = m_deviceResources->GetD3DDeviceContext();

    Matrix reflectionViewMatrix, worldMatrix, projectionMatrix;

    Vector4 clipPlane = XMFLOAT4(0.0f, 1.0f, 0.0f, -m_water->GetWaterHeight());

    m_camera.RenderReflection(m_water->GetWaterHeight());

    reflectionViewMatrix = m_camera.reflectionViewMatrix;

    worldMatrix = m_world;
    projectionMatrix = m_projection;

    Vector3 cameraPosition = m_camera.GetPosition();

    cameraPosition.y = -cameraPosition.y + (m_water->GetWaterHeight() * 2.0f);

    m_deviceResources->TurnOffCulling();
    m_deviceResources->TurnZBufferOff();

    m_reflectionShader->Render(context, m_terrain.GetIndexCount(), worldMatrix, reflectionViewMatrix, projectionMatrix,
        m_water_texture.Get(), m_water_texture.Get(), m_light.getDiffuseColour(), m_light.getDirection(), 2.0f,
        clipPlane);
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
    m_input.Reset();
}

void Game::OnDeactivated()
{
    // Game is becoming background window.
}

void Game::OnSuspending()
{
    // Game is being power-suspended (or minimized).
	m_audio.OnSuspending();
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // Game is being power-resumed (or returning from minimize).
    m_input.Reset();
	m_audio.OnResuming();
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
    m_terrainShader.InitStandard(device, L"terrain_vs.cso", L"terrain_ps.cso");
    // Textures
    CreateDDSTextureFromFile(device, L"Assets/grass.dds", nullptr, m_grass_texture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"Assets/mountain.dds", nullptr, m_mountain_texture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"Assets/rock_diffuse.dds", nullptr, m_walls_texture.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"Assets/water.dds", nullptr, m_water_texture.ReleaseAndGetAddressOf());
    // Terrain
    m_terrain.Initialize(device, 256, 256);
    m_terrain.GenerateHeightMap(device);
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
    if (m_skydomeShader)
    {
        m_skydomeShader->Shutdown();
        delete m_skydomeShader;
        m_skydomeShader = 0;
    }

    // Release the sky dome object.
    if (m_skydome)
    {
        m_skydome->Shutdown();
        delete m_skydome;
        m_skydome = 0;
    }

    if (m_reflectionShader)
    {
        m_reflectionShader->Shutdown();
        delete m_reflectionShader;
        m_reflectionShader = 0;
    }

    if (m_waterShader)
    {
        m_waterShader->Shutdown();
        delete m_waterShader;
        m_waterShader = 0;
    }

    if (m_water)
    {
        m_water->Shutdown();
        delete m_water;
        m_water = 0;
    }

    m_terrain.Shutdown();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
