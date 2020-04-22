#pragma once
#include "pch.h"

class AudioModule
{
	public:
		~AudioModule();
		void Initialize();
		void OnSuspending();
		void OnResuming();
		void Update();
		void OnNewAudioDevice() { m_retryAudio = true; }
	private:
		bool m_retryAudio;
		std::unique_ptr<DirectX::AudioEngine> m_audEngine;
};

