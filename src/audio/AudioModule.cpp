#include "core/pch.h"
#include "audio/AudioModule.h"
#include <filesystem>
#include <fstream>
using namespace DirectX;

AudioModule::~AudioModule()
{
	if (m_audEngine)
	{
		m_audEngine->Suspend();
	}
	m_ambientInstance.reset();
}

void AudioModule::Initialize()
{
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | AudioEngine_Debug;
#endif
	m_audEngine = std::make_unique<AudioEngine>(eflags);
	m_retryAudio = false;

	const auto trackPath = std::filesystem::absolute(L"Assets/Nine Inch Nails - 36 Ghosts IV.wav");
	m_ambient = std::make_unique<SoundEffect>(m_audEngine.get(), trackPath.c_str());
	m_ambientInstance = m_ambient->CreateInstance();
	m_ambientInstance->SetVolume(0.5);
	m_ambientInstance->Play(true);
	const auto format = m_ambient->GetFormat();
	std::ofstream("audio-playback.log")
		<< "Loaded: " << trackPath.string() << '\n'
		<< "Duration: " << m_ambient->GetSampleDurationMS() << " ms\n"
		<< "Format: " << format->nSamplesPerSec << " Hz, "
		<< format->wBitsPerSample << " bit, " << format->nChannels << " channels\n";
}

void AudioModule::OnSuspending()
{
	m_audEngine->Suspend();
}

void AudioModule::OnResuming()
{
	m_audEngine->Resume();
}

void AudioModule::Update()
{
	if (m_retryAudio)
	{
		m_retryAudio = false;
		if (m_audEngine->Reset())
		{
			// TODO: restart any looped sounds here
			if (m_ambientInstance)
				m_ambientInstance->Play(true);
		}
	}
	else if (!m_audEngine->Update())
	{
		if (m_audEngine->IsCriticalError())
		{
			m_retryAudio = true;
		}
	}
}
