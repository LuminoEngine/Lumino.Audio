/*
	@file	AudioManager.h
*/
#pragma once

#include "Common.h"
#include <Lumino/Base/RefObject.h>
#include <Lumino/IO/FileManager.h>

namespace Lumino
{
namespace Audio
{
class GameAudio;
class AudioDevice;

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
	AudioManager(const ConfigData& configData);
	virtual ~AudioManager();

	/// �I������
	void Finalize();

	/// �I��������or�X�g���[�~���O�����I���̉����f�[�^�o�C�g��臒l
	void SetAutoPlayTypeSelectThreshold(uint32_t threshold) { mOnMemoryLimitSize = threshold; }

	/// GameAudio �N���X�̎擾
	GameAudio* GetGameAudio() { return mGameAudio; }

	/// �f�o�C�X�N���X�̎擾
	AudioDevice* GetAudioDevice() { return mAudioDevice; }

	///// Sound �̍쐬 ( stream_ = NULL �ŃL�[���g�������������s�� )
	//Sound* createSound(FileIO::Stream* stream, SoundPlayType type, bool enable_3d, lnSharingKey key);

	///// Sound �̍쐬 ( �t�@�C�����w�� )
	//Sound* createSound(const lnChar* filename, SoundPlayType type, bool enable_3d);

	///// Sound �̍쐬 ( IAudioSource �w�� )
	//Sound* createSound(AudioSourceBase* source, SoundPlayType type, bool enable_3d);

	/// �O���[�v�̒�~
	//void stopGroup(lnU32 group);

private:
	GameAudio*		mGameAudio;
	AudioDevice*	mAudioDevice;
	uint32_t		mOnMemoryLimitSize;
};

} // namespace Audio
} // namespace Lumino
