
#include "Internal.h"
#ifdef LN_WIN32
	#include "XAudio2/XAudio2AudioDevice.h"
	#include "DirectMusic/DirectMusicAudioDevice.h"
#endif
#include <Lumino/Base/Environment.h>
#include <Lumino/IO/ASyncIOObject.h>
#include <Lumino/Audio/AudioManager.h>
#include <Lumino/Audio/Sound.h>
#include "AudioUtils.h"
#include "WaveDecoder.h"
#include "MidiDecoder.h"

namespace Lumino
{
namespace Audio
{

//class ASyncAudioStreamLoadTask
//	: public AsyncIOTask
//{
//public:
//	RefPtr<AudioStream>	m_audioStream;
//	RefPtr<Stream>		m_sourceStream;
//
//public:
//	virtual void Execute()
//	{
//		try
//		{
//			m_audioStream->Create(m_sourceStream);
//		}
//		catch (Exception* e) {
//			m_audioStream->OnCreateFinished(e);
//		}
//	}
//};
//
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
	: m_fileManager(configData.FileManager)
	, m_audioDevice(NULL)
	, m_midiAudioDevice(NULL)
	, mOnMemoryLimitSize(100000)
	, m_audioStreamCache(NULL)
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
	if (m_midiAudioDevice == NULL)
	{
		RefPtr<DirectMusicAudioDevice> device(LN_NEW DirectMusicAudioDevice());
		DirectMusicAudioDevice::ConfigData data;
		data.DMInitMode = configData.DMInitMode;
		data.hWnd = (HWND)configData.hWnd;
		device->Initialize(data);
		device.SafeAddRef();
		m_midiAudioDevice = device;
	}
#else
#endif
	// �L���b�V��������
	m_audioStreamCache = LN_NEW CacheManager(32, 65535);

	// �|�[�����O�X���b�h�J�n
	m_pollingThread.Start(LN_CreateDelegate(this, &AudioManager::Thread_Polling));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioManager::~AudioManager()
{
	// �|�[�����O�X���b�h�̏I����҂�
	m_endRequested.SetTrue();
	m_pollingThread.Wait();

	// �����c���Ă���΍폜����
	LN_FOREACH(Sound* sound, m_soundList) {
		sound->Release();
	}
	m_soundList.Clear();

	if (m_audioStreamCache != NULL) {
		m_audioStreamCache->Finalize();
		LN_SAFE_RELEASE(m_audioStreamCache);
	}
	LN_SAFE_RELEASE(m_audioDevice);
	LN_SAFE_RELEASE(m_midiAudioDevice);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AudioManager::Finalize()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioStream* AudioManager::CreateAudioStream(const TCHAR* filePath)
{
	RefPtr<Stream> stream(m_fileManager->CreateFileStream(filePath));
	return CreateAudioStream(stream, CacheKey(PathName(filePath)));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioStream* AudioManager::CreateAudioStream(Stream* stream, const CacheKey& key)
{
	// �L���b�V������������B
	// �������� AudioStream �́A�܂��񓯊����������ł��邱�Ƃ�����B
	RefPtr<AudioStream> audioStream((AudioStream*)m_audioStreamCache->FindObjectAddRef(key), false);

	// �L���b�V���Ɍ�����Ȃ�������V�������
	if (audioStream.IsNull())
	{
		audioStream.Attach(LN_NEW AudioStream(stream), false);
		audioStream->Create();	// �񓯊��ǂݍ��݊J�n
		/*
			�񓯊��ǂݍ��݂̊J�n�� FileManager �̃^�X�N���X�g�ɓ������B
			�����ŎQ�ƃJ�E���g�� +1 ����A��������������܂ŎQ�Ƃ��ꑱ����B
		*/

		// �L���b�V���ɓo�^
		m_audioStreamCache->RegisterCacheObject(key, audioStream);
	}

	audioStream.SafeAddRef();
	return audioStream;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioPlayer* AudioManager::CreateAudioPlayer(AudioStream* stream, SoundPlayType type, bool enable3D)
{
	// �Đ����@�̑I��
	SoundPlayType playerType = AudioUtils::CheckAudioPlayType(type, stream, mOnMemoryLimitSize);

	// �쐬
	if (playerType == SoundPlayType_Midi) {
		return m_midiAudioDevice->CreateAudioPlayer(stream, enable3D, playerType);
	}
	else {
		return m_audioDevice->CreateAudioPlayer(stream, enable3D, playerType);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Sound* AudioManager::CreateSound(Stream* stream, SoundPlayType type, bool enable3D, const CacheKey& key)
{
	RefPtr<AudioStream> audioStream(CreateAudioStream(stream, key));
	RefPtr<Sound> sound(LN_NEW Sound(this, audioStream, type, enable3D));

	// �Ǘ����X�g�ɒǉ�
	Threading::MutexScopedLock lock(m_soundListMutex);
	m_soundList.Add(sound);
	sound.SafeAddRef();	// �Ǘ����X�g�̎Q��
	sound.SafeAddRef();	// �O�ɏo�����߂̎Q��
	return sound;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AudioManager::Thread_Polling()
{
#ifdef LN_WIN32
	// COM ������
	HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

	uint64_t lastTime = Environment::GetTickCount();
	while (m_endRequested.IsFalse())
	{
		Threading::Thread::Sleep(10);	// CPU ���� 100% ������邽�߁A�Ƃ肠���� 10ms �҂�

		// �o�ߎ��Ԃ����߂đS Sound �X�V
		float elapsedTime = static_cast<float>(Environment::GetTickCount() - lastTime) / 1000.0f;
		LN_FOREACH(Sound* sound, m_soundList)
		{
			sound->Polling(elapsedTime);
		}

		// �������烍�b�N
		Threading::MutexScopedLock lock(m_soundListMutex);

		// GC�B���̃��X�g���炵���Q�Ƃ���ĂȂ���� Release ����B
		ArrayList<Sound*>::iterator itr = m_soundList.begin();
		ArrayList<Sound*>::iterator end = m_soundList.end();
		while (itr != end)
		{
			// TODO: �t�F�[�h���͊J�����Ȃ�

			if ((*itr)->GetRefCount() == 1)
			{
				(*itr)->Release();
				itr = m_soundList.erase(itr);
				end = m_soundList.end();
			}
			else {
				++itr;
			}
		}
	}

#ifdef LN_WIN32
	::CoUninitialize();
#endif
}

} // namespace Audio
} // namespace Lumino
