/*
	@file	Common.h
*/
#pragma once

#include <Lumino/Math/Vector3.h>
#include <Lumino/Base/Common.h>

#define LN_MUSIC_TIME_BASE	768

namespace Lumino
{
namespace Audio
{

/// �����t�@�C���t�H�[�}�b�g
enum StreamFormat
{
	StreamFormat_Unknown = 0,		///< �s���ȃt�@�C��
	StreamFormat_Wave,				///< WAVE
	StreamFormat_Mp3,				///< MP3
	StreamFormat_Ogg,				///< OGG
	StreamFormat_Midi,				///< MIDI

	StreamFormat_Max,				///< (terminator)
};

/// PCM �t�H�[�}�b�g
struct WaveFormat
{
	uint16_t	FormatTag;
	uint16_t	Channels;
	uint32_t	SamplesPerSec;
	uint32_t	AvgBytesPerSec;
	uint16_t	BlockAlign;
	uint16_t	BitsPerSample;
	uint16_t	EXSize;
};

/// DirectMusic �̏��������@
enum DirectMusicInitMode
{
	DirectMusicInitMode_NotUse = 0,			///< DirectMusic ���g�p���Ȃ�
	DirectMusicInitMode_Normal,				///< �ʏ�
	DirectMusicInitMode_ThreadWait,			///< �ʃX���b�h�ŏ��������āA�Đ����ɖ������̏ꍇ�͑҂�
	DirectMusicInitMode_ThreadRequest,		///< �ʃX���b�h�ŏ��������āA�Đ����ɖ������̏ꍇ�͍Đ���\�񂷂�

	DirectMusicInitMode_MAX,
};

/// �Đ����@ (Player �̎��)
enum SoundPlayType
{
	SoundPlayType_Unknown = 0,		///< �s���ȍĐ����@ (�����I��)
	//SoundPlayType_Auto,				///< �����I�� ( �f�t�H���g�ł̓f�R�[�h��̃T�C�Y�� 10000 �o�C�g�ȏ�ɂȂ�ꍇ�̓X�g���[�~���O�A�����łȂ��ꍇ�̓I���������Đ��ɂȂ�܂� )
	SoundPlayType_OnMemory,			///< �I��������
	SoundPlayType_Streaming,	    ///< �X�g���[�~���O
	SoundPlayType_Midi,  			///< SMF

	SoundPlayType_Max,
};

/// �����̍Đ����
enum SoundPlayState
{
	SoundPlayState_Stopped = 0,		///< ��~��
	SoundPlayState_Playing,			///< �Đ���
	SoundPlayState_Pausing,			///< �ꎞ��~��

	SoundPlayState_Max,
};

/// 3D �T�E���h���X�i�[
struct SoundListener
{
	Vector3		Position;
	Vector3		Direction;
	Vector3		UpDirection;
	Vector3		Velocity;

	SoundListener()
		: Position(0, 0, 0)
		, Direction(0, 0, 1.0f)
		, UpDirection(0, 1.0f, 0)
		, Velocity(0, 0, 0)
	{}
};

} // namespace Audio
} // namespace Lumino
