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
    const XMVECTORF32 START_POSITION = { 0.f, -1.5f, 0.f, 0.f };
    const XMVECTORF32 ROOM_BOUNDS = { 8.f, 6.f, 12.f, 0.f };
    const float ROTATION_GAIN = 0.004f;
    const float MOVEMENT_GAIN = 0.07f;
}

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
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
    auto context = m_deviceResources->GetD3DDeviceContext();

    // TODO: Add your rendering code here.

    camera.Render();
    m_room->Draw(Matrix::Identity, camera.view, camera.projection, Colors::White, m_roomTex.Get());
    
    
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

    // Room
    m_room = GeometricPrimitive::CreateBox(context,
        XMFLOAT3(ROOM_BOUNDS[0], ROOM_BOUNDS[1], ROOM_BOUNDS[2]),
        false, true);

    DX::ThrowIfFailed(
        CreateDDSTextureFromFile(device, L"Assets/roomtexture.dds",
            nullptr, m_roomTex.ReleaseAndGetAddressOf()));

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

    // Room
    m_room.reset();
    m_roomTex.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}

void Game::UpdateCamera()
{
    auto mouse = input.mouse->GetState();

    if (mouse.positionMode == Mouse::MODE_RELATIVE)
    {
        Vector3 delta = Vector3(float(mouse.x), float(mouse.y), 0.f)
            * ROTATION_GAIN;

        camera.pitch -= delta.y;
        camera.yaw -= delta.x;

        // limit pitch to straight up or straight down
        // with a little fudge-factor to avoid gimbal lock
        float limit = XM_PI / 2.0f - 0.01f;
        camera.pitch = (std::max)(-limit, camera.pitch);
        camera.pitch = (std::min)(+limit, camera.pitch);

        // keep longitude in sane range by wrapping
        if (camera.yaw > XM_PI)
        {
            camera.yaw -= XM_PI * 2.0f;
        }
        else if (camera.yaw < -XM_PI)
        {
            camera.yaw += XM_PI * 2.0f;
        }
    }

    input.mouse->SetMode(mouse.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
    Vector3 move = Vector3::Zero;

    if (inputCommands.forward)
        move.z += 1.f;

    if (inputCommands.back)
        move.z -= 1.f;

    if (inputCommands.left)
        move.x += 1.f;

    if (inputCommands.right)
        move.x -= 1.f;

    if (inputCommands.space)
        move.y += 1.f;

    if (inputCommands.q_key)
        move.y -= 1.f;

    Quaternion q = Quaternion::CreateFromYawPitchRoll(camera.yaw, camera.pitch, 0.f);

    move = Vector3::Transform(move, q);

    move *= MOVEMENT_GAIN;

    camera.position += move;

    Vector3 halfBound = (Vector3(ROOM_BOUNDS.v) / Vector3(2.f))
        - Vector3(0.1f, 0.1f, 0.1f);

    camera.position = Vector3::Min(camera.position, halfBound);
    camera.position = Vector3::Max(camera.position, -halfBound);
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
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_window = false;
		ImGui::End();
	}

}
#pragma endregion
