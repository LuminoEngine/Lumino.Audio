/*
	@file	AudioDevice.h
*/
#pragma once

#include <Lumino/Base/Cache.h>
#include <Lumino/IO/Stream.h>
#include <Lumino/Audio/Common.h>

namespace Lumino
{
namespace Audio
{
class AudioStream;
class AudioPlayer;

/// �f�o�C�X�Ǘ��ƁAPlayer �̐����E�Ǘ����s���N���X�̃x�[�X�N���X
class AudioDevice
	: public RefObject
{
public:
	AudioDevice() {}
	virtual ~AudioDevice() {}
public:

	/// 3D�T�E���h���X�i�[�̎擾
	SoundListener* getSoundListener() { return &m_soundListener; }

public:

	/// AudioPlayer ���쐬���� (type �� LN_SOUNDPLAYTYPE_AUTO �͎w��ł��Ȃ��̂Œ���)
	virtual AudioPlayer* CreateAudioPlayer(AudioStream* source, bool enable3d, SoundPlayType type) = 0;
	/// �X�V (�X�V�X���b�h����Ă΂��)
	virtual void Update() = 0;
	//virtual void SetListenerState(const Vector3& position, const Vector3& front) = 0;
	/// 3D ��Ԃ�1���[�g�������̋����̐ݒ�
	virtual void SetMetreUnitDistance(float d) = 0;

protected:
	SoundListener		m_soundListener;
};

} // namespace Audio
} // namespace Lumino
