/*
	@file	AudioManager.h
*/
#pragma once

#include "Common.h"
#include <Lumino/Base/Cache.h>
#include <Lumino/Base/RefObject.h>
#include <Lumino/IO/FileManager.h>

namespace Lumino
{
namespace Audio
{
class GameAudio;
class AudioDevice;
class AudioStream;
class AudioPlayer;
class Sound;

/*
	@brief	�����@�\�̊Ǘ��N���X�ł��B
*/
class AudioManager
	: public RefObject
{
public:
	/// initialize() �ɓn���������f�[�^
	struct ConfigData
	{
		Lumino::FileManager*	FileManager;					///< �t�@�C������̓ǂݍ��݂Ɏg���t�@�C���Ǘ��N���X
		uint32_t				StreamCacheObjectCount;			///< �L���b�V���T�C�Y (�t�@�C����)
		uint32_t				StreamSourceCacheMemorySize;	///< �L���b�V���T�C�Y (��������(byte))
		DirectMusicInitMode		DMInitMode;						///< DirectMusic �̏��������@
		void*					hWnd;							///< DirectMusic �̏������Ɏg���E�B���h�E�n���h��
	};

public:
	static AudioManager* Create(const ConfigData& configData);

public:

	/// �I������
	void Finalize();

	/// �I��������or�X�g���[�~���O�����I���̉����f�[�^�o�C�g��臒l
	void SetAutoPlayTypeSelectThreshold(uint32_t threshold) { mOnMemoryLimitSize = threshold; }

	/// GameAudio �N���X�̎擾
	GameAudio* GetGameAudio() { return mGameAudio; }

	/// �f�o�C�X�N���X�̎擾
	AudioDevice* GetAudioDevice() { return m_audioDevice; }

	/// �L�[�ɑΉ�����I�[�f�B�I�\�[�X���������� (���������ꍇ�� addRef ���ĕԂ�)
	//AudioStream* FindAudioSource(lnSharingKey key);

	/// �I�[�f�B�I�\�[�X�̍쐬 TODO:internal ��
	AudioStream* CreateAudioStream(const TCHAR* filePath);
	AudioStream* CreateAudioStream(Stream* stream, const CacheKey& key);

	AudioPlayer* CreateAudioPlayer(AudioStream* stream, SoundPlayType type, bool enable3D);

	Sound* CreateSound(AudioStream* stream, SoundPlayType type, bool enable3D);

	///// Sound �̍쐬 ( stream_ = NULL �ŃL�[���g�������������s�� )
	//Sound* createSound(FileIO::Stream* stream, SoundPlayType type, bool enable_3d, lnSharingKey key);

	///// Sound �̍쐬 ( �t�@�C�����w�� )
	//Sound* createSound(const lnChar* filename, SoundPlayType type, bool enable_3d);

	///// Sound �̍쐬 ( IAudioSource �w�� )
	//Sound* createSound(AudioSourceBase* source, SoundPlayType type, bool enable_3d);

	/// �O���[�v�̒�~
	//void stopGroup(lnU32 group);

private:
	AudioManager(const ConfigData& configData);
	virtual ~AudioManager();
	void Thread_Polling();

private:

	Lumino::FileManager*	m_fileManager;
	AudioDevice*			m_audioDevice;		///< PCM �Đ��p�f�o�C�X�N���X
	AudioDevice*			m_midiAudioDevice;	///< MIDI �Đ��p�f�o�C�X�N���X (���������� PCM �Ƃ͑S�R�Ⴄ�̂ŁA1�� AudioDevice �ɂ܂Ƃ߂Ȃ������Ǘ����y)
	GameAudio*		mGameAudio;
	uint32_t		mOnMemoryLimitSize;
	Threading::Mutex	m_resourceMutex;

	ArrayList<Sound*>			m_soundList;
	Threading::Mutex			m_soundListMutex;
	Threading::EventFlag		m_endRequested;
	Threading::DelegateThread	m_pollingThread;
};

} // namespace Audio
} // namespace Lumino
