/*
	@file	AudioUtils.h
*/
#pragma once

#include <Lumino/IO/Stream.h>
#include <Lumino/Audio/Common.h>

namespace Lumino
{
namespace Audio
{
class AudioStream;

/// Wave �f�[�^ AudioStream
class AudioUtils
{
public:

#ifdef _WIN32
	/// WaveFormat ���� WAVEFORMATEX �ւ̕ϊ�
	static void ConvertLNWaveFormatToWAVEFORMATEX(const WaveFormat& lnFmt, WAVEFORMATEX* wavFmt);

	/// WAVEFORMATEX ���� WaveFormat �ւ̕ϊ�
	static void ConvertWAVEFORMATEXToLNWaveFormat(const WAVEFORMATEX& wavFmt, WaveFormat* lnFmt);

	/// WAVEFORMATEX �\���̂�W���o��
	static void PrintWAVEFORMATEX(const WAVEFORMATEX& wavFmt, const char* str = NULL);
#endif

	/// �����f�[�^�̃t�H�[�}�b�g�`�F�b�N
	///		�����鉹���t�@�C�����ǂ����𒲂ׂāA���̎�ނ�Ԃ��B
	///		���ׂ���A�t�@�C���|�C���^�̓t�@�C���̐擪�ɖ߂�B
	///		MP3 �����`�F�b�N����̂Ƀt�@�C���I�[����� Seek ���邱�Ƃ�����B
	static StreamFormat CheckFormat(Stream* stream);
	
	/// �v�����Ă��� type �� AudioStream ���琳���� type ��Ԃ�
	static SoundPlayType CheckAudioPlayType(SoundPlayType type, AudioStream* audioStream, uint32_t limitSize);
};

} // namespace Audio
} // namespace Lumino
