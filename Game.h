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
#include "Water.h"
#include "WaterShader.h"
#include "ReflectionShader.h"
#include "PostProcess.h"

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
	void OnNewAudioDevice() { m_audio.OnNewAudioDevice(); }
private:

    void Update(DX::StepTimer const& timer);
    void UpdateCamera();
    
    // ImGUI
    void UpdateGUI();
    void RenderReflection();
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    
    // ImGui variables
    bool                                    m_show_window = true;
    Vector3                                 debug_vector;
    float                                   debug_float;
	
    // Audio
	AudioModule								m_audio;
	// Input System
	InputSystem								m_input;
	InputCommands							m_inputCommands;
    // Camera                               
    Camera                                  m_camera;
    // World, View, Projection
    Matrix                                  m_world;
    Matrix                                  m_view;
    Matrix                                  m_projection;
    // Light
    Light                                   m_light;

    // Terrain
    Terrain                                             m_terrain;
    TerrainShader							            m_terrainShader;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_mountain_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_walls_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_grass_texture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_water_texture;

    Skydome*                                            m_skydome;
    SkydomeShader*                                      m_skydomeShader;

    ReflectionShader*                                   m_reflectionShader;
    Water*                                              m_water;
    WaterShader*                                        m_waterShader;

    std::unique_ptr<BasicPostProcess>                   m_postProcess;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>             m_sceneTex;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>    m_sceneSRV;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>      m_sceneRT;
    int                                                 postProcessLoop = 0;
};
