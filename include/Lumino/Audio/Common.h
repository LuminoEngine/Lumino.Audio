/*
	@file	Common.h
*/
#pragma once

#include <Lumino/Base/Common.h>

namespace Lumino
{
namespace Audio
{

/// �����t�@�C���t�H�[�}�b�g
enum StreamFormat
{
	StreamFormat_Unknown = 0,		///< �s���ȃt�@�C��
	StreamFormat_Wave,				///< WAVE
	StreamFormat_MP3,				///< MP3
	StreamFormat_Ogg,				///< OGG
	StreamFormat_MIDI,				///< MIDI

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

} // namespace Audio
} // namespace Lumino
