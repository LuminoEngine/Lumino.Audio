
#include "Internal.h"
#ifdef LN_WIN32
	#include "XAudio2/XAudio2AudioDevice.h"
	#include "DirectMusic/DirectMusicAudioDevice.h"
#endif
#include <Lumino/Base/Environment.h>
#include <Lumino/IO/AsyncIOTask.h>
#include <Lumino/Audio/AudioManager.h>
#include <Lumino/Audio/Sound.h>
#include "AudioUtils.h"
#include "WaveStream.h"
#include "MidiStream.h"

namespace Lumino
{
namespace Audio
{

class ASyncAudioStreamLoadTask
	: public AsyncIOTask
{
public:
	RefPtr<AudioStream>	m_audioStream;
	RefPtr<Stream>		m_sourceStream;

public:
	virtual void Execute()
	{
		try
		{
			m_audioStream->Create(m_sourceStream);
		}
		catch (Exception* e) {
			m_audioStream->OnCreateFinished(e);
		}
	}
};

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

	// ポーリングスレッド開始
	m_pollingThread.Start(LN_CreateDelegate(this, &AudioManager::Thread_Polling));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioManager::~AudioManager()
{
	// ポーリングスレッドの終了を待つ
	m_endRequested.SetTrue();
	m_pollingThread.Wait();

	// 何か残っていれば削除する
	LN_FOREACH(Sound* sound, m_soundList) {
		sound->Release();
	}
	m_soundList.Clear();

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
	LN_THROW(stream != NULL, ArgumentException);

	StreamFormat format = AudioUtils::CheckFormat(stream);

	Threading::MutexScopedLock lock(m_resourceMutex);

	RefPtr<AudioStream> audioStream;

	switch (format)
	{
		// Wave
	case StreamFormat_Wave:
		audioStream.Attach(LN_NEW WaveStream(), false);
		break;
//#if defined(LNOTE_MSVC)
//		// MP3
//	case AUDIOSOURCE_MP3:
//		audio_source.attach(LN_NEW MP3(this), false);
//		break;
//#endif
//		// OGG
//	case AUDIOSOURCE_OGG:
//		audio_source.attach(LN_NEW Ogg(this), false);
//		break;
		// MIDI
	case StreamFormat_Midi:
		audioStream.Attach(LN_NEW MidiStream(), false);
		break;
	default:
		LN_THROW(stream != NULL, InvalidFormatException);
		return NULL;
	}

	if (1)
	{
		ASyncAudioStreamLoadTask* task = LN_NEW ASyncAudioStreamLoadTask();
		task->m_audioStream = audioStream;
		task->m_sourceStream = stream;
		m_fileManager->RequestASyncTask(task);
	}
	else
	{
		// 入力ファイルを設定する
		audioStream->Create(stream);
	}

	// MP3 と MIDI 以外は登録
	//if (key.isEmpty() == false && (format != AUDIOSOURCE_MP3 && format != AUDIOSOURCE_MIDI))
	//{
	//	Base::CacheManager::registerCachedObject(key, audio_source);
	//}

	audioStream.SafeAddRef();
	return audioStream;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioPlayer* AudioManager::CreateAudioPlayer(AudioStream* stream, SoundPlayType type, bool enable3D)
{
	// 再生方法の選択
	SoundPlayType playerType = AudioUtils::CheckAudioPlayType(type, stream, mOnMemoryLimitSize);

	// 作成
	if (playerType == SoundPlayType_Midi) {
		return m_midiAudioDevice->CreateAudioPlayer(stream, enable3D, playerType);
	}
	else {
		return m_audioDevice->CreateAudioPlayer(stream, enable3D, playerType);
	}


	//Threading::ScopedLock lock(*mLock);

	//SoundPlayType player_type = SOUNDPLAYTYPE_UNKNOWN;

	////-----------------------------------------------------
	//// AudioSourceBase

	//// キャッシュ検索
	//Base::RefPtr<AudioSourceBase> source(
	//	mResourceManager->findAudioSource(key), false);

	//// キャッシュに無かった
	//if (source.getPtr() == NULL)
	//{
	//	// 検索のみ行ったが、見つからなかった
	//	if (stream == NULL) return NULL;

	//	source.attach(
	//		mResourceManager->createAudioSource(stream, key), false);

	//	// 正しい種類をチェック
	//	player_type = AudioUtil::checkAudioPlayType(type, source, mOnMemoryLimitSize);
	//}
	//// キャッシュに見つかった
	//else
	//{
	//	// MIDI はそのまま
	//	if (source->getSourceFormat() == AUDIOSOURCE_MIDI) {
	//		player_type = SOUNDPLAYTYPE_MIDI;
	//	}
	//	// すでにオンメモリロードが完了しているものはすべてオンメモリ
	//	else if (source->getOnmemoryPCMBuffer()) {
	//		player_type = SOUNDPLAYTYPE_ONMEMORY;
	//	}
	//	// それ以外はストリーミング
	//	else {
	//		player_type = SOUNDPLAYTYPE_STREAMING;
	//	}
	//}

	////-----------------------------------------------------
	//// AudioPlayerBase
	//Base::RefPtr<AudioPlayerBase> player(
	//	mAudioDevice->createAudioPlayer(source, enable_3d, player_type));

	//// Sound 作成
	//Sound* sound = LN_NEW Sound(this, player);
	//mSoundList.push_back(sound);
	//return sound;;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Sound* AudioManager::CreateSound(AudioStream* stream, SoundPlayType type, bool enable3D)
{
	RefPtr<Sound> sound(LN_NEW Sound(this, stream, type, enable3D));

	// 管理リストに追加
	Threading::MutexScopedLock lock(m_soundListMutex);
	m_soundList.Add(sound);
	sound.SafeAddRef();	// 管理リストの参照
	sound.SafeAddRef();	// 外に出すための参照
	return sound;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AudioManager::Thread_Polling()
{
#ifdef LN_WIN32
	// COM 初期化
	HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
#endif

	uint64_t lastTime = Environment::GetTickCount();
	while (m_endRequested.IsFalse())
	{
		Threading::Thread::Sleep(10);	// CPU 負荷 100% を避けるため、とりあえず 10ms 待つ

		// 経過時間を求めて全 Sound 更新
		float elapsedTime = static_cast<float>(Environment::GetTickCount() - lastTime) / 1000.0f;
		LN_FOREACH(Sound* sound, m_soundList)
		{
			sound->Polling(elapsedTime);
		}

		// ここからロック
		Threading::MutexScopedLock lock(m_soundListMutex);

		// GC。このリストからしか参照されてなければ Release する。
		ArrayList<Sound*>::iterator itr = m_soundList.begin();
		ArrayList<Sound*>::iterator end = m_soundList.end();
		while (itr != end)
		{
			// TODO: フェード中は開放しない

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
