//
// Game.cpp
//

#include "core/pch.h"
#include "app/Game.h"

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

Game::~Game() = default;

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
    if (!m_skydome->Initialize(device))
        throw std::runtime_error("Failed to initialize the sky model: Assets/skydome.txt");
    m_skydomeShader = new SkydomeShader;
    if (!m_skydomeShader->Initialize(device, window))
        throw std::runtime_error("Failed to initialize the sky shaders.");

    m_reflectionShader = new ReflectionShader;
    if (!m_reflectionShader->Initialize(device, window))
        throw std::runtime_error("Failed to initialize the reflection shaders.");

    m_water = new Water;
    if (!m_water->Initialize(device, L"Assets/water_normal.dds", 3.75f, 110.0f))
        throw std::runtime_error("Failed to initialize water: Assets/water_normal.dds");
    m_waterShader = new WaterShader;
    if (!m_waterShader->Initialize(device, window))
        throw std::runtime_error("Failed to initialize the water shaders.");

	// Initialize audio
	m_audio.Initialize();
	SetWindowTextW(window, L"Procedural Scene - 36 Ghosts IV");

	// Initialize input
	m_input.Initialize(window);

    // Initialize camera
    const auto rooms = m_terrain.GetRooms();
    if (rooms.empty() || !rooms.front() || !rooms.front()->vPoint)
        throw std::runtime_error("Terrain generation produced no room for the starting camera.");
    Vector3 start = Vector3(rooms.front()->vPoint->x,
                            -10,
                            -rooms.front()->vPoint->z);
    m_camera.Initialize(start*0.2);

    m_postProcess = std::make_unique<BasicPostProcess>(device);

    CD3D11_TEXTURE2D_DESC sceneDesc(
        DXGI_FORMAT_R16G16B16A16_FLOAT, width, height,
        1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

    DX::ThrowIfFailed(
        device->CreateTexture2D(&sceneDesc, nullptr, m_sceneTex.GetAddressOf())
    );

    DX::ThrowIfFailed(
        device->CreateShaderResourceView(m_sceneTex.Get(), nullptr,
            m_sceneSRV.ReleaseAndGetAddressOf())
    );

    DX::ThrowIfFailed(
        device->CreateRenderTargetView(m_sceneTex.Get(), nullptr,
            m_sceneRT.ReleaseAndGetAddressOf()
        ));
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
    if (m_inputCommands.p_key)
    {
        if (postProcessLoop < 4)
            postProcessLoop++;
        else
            postProcessLoop = 0;
    }

    m_camera.SetPosition(m_camera.GetPosition() + move);

    bool result, foundHeight;
    float posX = m_camera.GetPosition().x; 
    float posY = m_camera.GetPosition().y;
    float posZ = -m_camera.GetPosition().z;
    
    float height;
    bool canWalk = false;
    foundHeight = m_terrain.GetHeightAtPosition(posX / 0.2, posZ / 0.2, height, canWalk);
    float newPosY = m_camera.GetPosition().y;

    debug_float = -(height)*0.2;
    float distanceFromGround = m_camera.GetPosition().y - debug_float + 2;
    if (foundHeight)
    {
        // If there was a triangle under the camera then position the camera just above it by two units.
        if (distanceFromGround < 0.02f) {
            posX = m_camera.oldPosition.x;
            posZ = m_camera.oldPosition.z;
            m_camera.SetPositionY(newPosY);
            newPosY = m_camera.GetPosition().y + 0.05f;
            if (!canWalk) {
                //collision
                m_camera.SetPosition(posX, newPosY, posZ);
            }
        }
    }

    

    if (m_camera.GetPosition().y < -2.5f)
    { 
        m_light.setAmbientColour(0.0f, 0.5f, 0.5f, 1.0f);
    }
    else
        m_light.setAmbientColour(0.5f, 0.5f, 0.5f, 1.0f);
    m_camera.Update();

    m_view = m_camera.view;
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
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();
    // The scene texture was sampled by post-processing during the previous frame.
    ID3D11ShaderResourceView* unboundViews[3] = {};
    context->PSSetShaderResources(0, 3, unboundViews);
    context->ClearRenderTargetView(m_sceneRT.Get(), Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, m_sceneRT.GetAddressOf(), depthStencil);

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
    m_terrainShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_light, m_grass_texture.Get(), m_mountain_texture.Get(), m_walls_texture.Get());
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

    m_deviceResources->TurnOnCulling();
    m_deviceResources->TurnZBufferOn();

    m_deviceResources->PIXEndEvent();

    // Show the new frame.

    context->OMSetRenderTargets(1, &renderTarget, depthStencil);
    if (postProcessLoop == 0)
    {
        m_postProcess->SetEffect(BasicPostProcess::Copy);
    }
    if (postProcessLoop == 1)
    {
        m_postProcess->SetEffect(BasicPostProcess::BloomBlur);
    }
    if (postProcessLoop == 2)
    {
        m_postProcess->SetEffect(BasicPostProcess::Monochrome);
    }
    if (postProcessLoop == 3)
    {
        m_postProcess->SetEffect(BasicPostProcess::Sepia);
    }
    m_postProcess->SetSourceTexture(m_sceneSRV.Get());
    m_postProcess->Process(context);

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

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

    // ReflectionShader issues its own indexed draw, so bind the matching mesh.
    // A previously bound water or sky index buffer cannot use the terrain count.
    m_terrain.Bind(context);
    worldMatrix = Matrix::CreateRotationX(XM_PI) * Matrix::CreateScale(0.2f)
        * Matrix::CreateTranslation(0.0f, -2.0f, 0.0f);
    m_reflectionShader->Render(context, m_terrain.GetIndexCount(), worldMatrix, reflectionViewMatrix, projectionMatrix,
        m_water_texture.Get(), m_water_texture.Get(), m_light.getDiffuseColour(), m_light.getDirection(), 2.0f,
        clipPlane);
    m_deviceResources->TurnOnCulling();
    m_deviceResources->TurnZBufferOn();
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
    width = 1024;
    height = 768;
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
    if (!m_terrainShader.InitStandard(device, L"terrain_vs.cso", L"terrain_ps.cso"))
        throw std::runtime_error("Failed to initialize the compiled terrain shaders.");
    // Textures
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Assets/grass.dds", nullptr, m_grass_texture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Assets/mountain.dds", nullptr, m_mountain_texture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Assets/rock.dds", nullptr, m_walls_texture.ReleaseAndGetAddressOf()));
    DX::ThrowIfFailed(CreateDDSTextureFromFile(device, L"Assets/water.dds", nullptr, m_water_texture.ReleaseAndGetAddressOf()));
    // Terrain
    if (!m_terrain.Initialize(device, 256, 256) || !m_terrain.GenerateHeightMap(device))
        throw std::runtime_error("Failed to initialize the procedural terrain.");
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
