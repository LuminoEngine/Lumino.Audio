
#include "Internal.h"
#ifdef LN_WIN32
	#include "XAudio2/XAudio2AudioDevice.h"
#endif
#include <Lumino/Audio/AudioManager.h>

namespace Lumino
{
namespace Audio
{
//=============================================================================
// AudioManager
//=============================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioManager* AudioManager::Create(const ConfigData& configData)
{
	return LN_NEW AudioManager(configData);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioManager::AudioManager(const ConfigData& configData)
	: m_audioDevice(NULL)
{
#ifdef LN_WIN32
	if (m_audioDevice == NULL)
	{
		RefPtr<XAudio2AudioDevice> device(LN_NEW XAudio2AudioDevice());
		if (device->Initialize()) {
			device.SafeAddRef();
			m_audioDevice = device;
		}
		//mAudioDevice = LN_NEW NullAudioDevice();
	}
#else
#endif
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioManager::~AudioManager()
{
	LN_SAFE_RELEASE(m_audioDevice);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AudioManager::Finalize()
{
}

} // namespace Audio
} // namespace Lumino
