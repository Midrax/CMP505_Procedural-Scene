#include "pch.h"
#include "AudioModule.h"
using namespace DirectX;

AudioModule::~AudioModule()
{
	if (m_audEngine)
	{
		m_audEngine->Suspend();
	}
	nin_together.reset();
}

void AudioModule::Initialize()
{
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | AudioEngine_Debug;
#endif
	m_audEngine = std::make_unique<AudioEngine>(eflags);
	m_retryAudio = false;

	m_ambient = std::make_unique<SoundEffect>(m_audEngine.get(),L"Assets/Nine Inch Nails - Together.wav");
	nin_together = m_ambient->CreateInstance();
	nin_together->Play(true);
	nin_together->SetVolume(0.5);
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
			if (nin_together)
				nin_together->Play(true);
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
