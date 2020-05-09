//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "InputSystem.h"
#include "AudioModule.h"
#include "Camera.h"
#include "TerrainShader.h"
#include "SkydomeShader.h"
#include "Light.h"
#include "Terrain.h"
#include "Skydome.h"

// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
	~Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

	// Audio
	void OnNewAudioDevice() { audio.OnNewAudioDevice(); }
private:

    void Update(DX::StepTimer const& timer);
    void UpdateCamera();
    
    // ImGUI
    void UpdateGUI();
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    
    // ImGui variables
    bool                                    show_window = true;

	// Audio
	AudioModule								audio;
	// Input System
	InputSystem								input;
	InputCommands							inputCommands;
    // Camera                               
    Camera                                  camera;
    // World, View, Projection
    Matrix                                  m_world;
    Matrix                                  m_view;
    Matrix                                  m_projection;
    // Light
    Light                                   light;

    // Terrain
    Terrain                                             terrain;
    TerrainShader							            terrainShader;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_mountain_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_walls_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_grass_texture;

    Skydome*                                            skydome;
    SkydomeShader*                                      skydomeShader;

};
