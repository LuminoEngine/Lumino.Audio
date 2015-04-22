/*
	@file	Mp3Decoder.h
*/
#pragma once

#include <Lumino/Base/ByteBuffer.h>
#include "AudioStream.h"

namespace Lumino
{
namespace Audio
{

/// MP3 �f�[�^ AudioStream
///
/// ���� AudioStream �́A�X�g���[�~���O�Đ��ŋ��L���邱�Ƃ͂ł��Ȃ��B
/// ����́A�f�R�[�h�ɂ� WinAPI ���g�p���Ă��邪�A����͌��� MP3 �f�[�^�ɑ΂���
///	�V�[�P���V�����ɃA�N�Z�X���Ȃ���΂Ȃ�Ȃ����߁B
/// �܂�A�X�g���[�~���O�� Player �N���X 2 �ȏォ�瓯���� Read (�f�R�[�h����) ���Ă΂��ƁA
/// �X�g���[���̃V�[�N�ʒu���O�ɖ߂����肷��B
/// ���̏�ԂŃf�R�[�h API ���Ăяo���ƁA������񂾂肷��B
class Mp3Decoder
	: public AudioDecoder
{
public:
	Mp3Decoder();
	virtual ~Mp3Decoder();

public:
	virtual void Create(Stream* stream);
	virtual StreamFormat GetSourceFormat() const { return StreamFormat_Mp3; }
	virtual const WaveFormat* GetWaveFormat() const { return &m_waveFormat; }
	virtual uint32_t GetSourceDataSize() const { return m_sourceDataSize; }
	virtual uint32_t GetTotalUnits() const { return m_totalSamples; }
	virtual byte_t* GetOnmemoryPCMBuffer() const { return m_onmemoryPCMBuffer; }
	virtual uint32_t GetOnmemoryPCMBufferSize() const { return m_onmemoryPCMBufferSize; }
	virtual uint32_t GetBytesPerSec() const { return m_streamingPCMBufferSize; }
	virtual void GetLoopState(uint32_t* begin, uint32_t* length) const { *begin = 0; *length = 0; }
	virtual void FillOnmemoryBuffer();
	virtual void Read(uint32_t seekPos, void* buffer, uint32_t bufferSize, uint32_t* outReadSize, uint32_t* outWriteSize);
	virtual void Reset() { m_resetFlag = true; }

private:

	/// �ǂ̃t�H�[�}�b�g�� mp3 �����ׂāA�f�[�^�܂ł̃I�t�Z�b�g��^�O�t�B�[���h�̃T�C�Y�������o�Ɋi�[
	void CheckId3v();

	/// mp3 �� PCM �t�H�[�}�b�g�𒲂ׂă����o�Ɋi�[����
	void GetPCMFormat();

private:

	/// ID3v2 �`���̃w�b�_���
	struct ID3v2Header
	{
		uint8_t	ID[3];
		uint8_t	Version[2];
		uint8_t	Flag;
		uint8_t	Size[4];
	};

private:
	Stream*					m_stream;					///< ���̓X�g���[��
	WaveFormat				m_waveFormat;				///< ���C�u�����p PCM �p�t�H�[�}�b�g
	uint32_t				m_sourceDataSize;			///< �����f�[�^�����̃T�C�Y
	uint64_t				m_dataOffset;				///< �X�g���[������ PCM �f�[�^�̐擪�܂ł̃I�t�Z�b�g�o�C�g��
	uint32_t				m_totalTime;				///< �S�̂̍Đ����� ( �~���b )
	uint8_t*				m_onmemoryPCMBuffer;		///< �I���������Đ��p�̃f�[�^��ǂݍ��ރo�b�t�@
	uint32_t				m_onmemoryPCMBufferSize;	///< mOnmemoryPCMBuffer �̃T�C�Y ( ���̂Ƃ��� mDataOffset �Ɠ��� )
	uint32_t				m_totalSamples;				///< �S�̂̍Đ��T���v����
	Threading::Mutex		m_mutex;

	MPEGLAYER3WAVEFORMAT	m_acmMP3WaveFormat;
	HACMSTREAM				m_hACMStream;
	size_t					m_id3vTagFieldSize;			///< Id3v2 �`���̏ꍇ�̃^�O�t�B�[���h(�w�b�_���)�����̃T�C�Y
	uint32_t				m_streamingPCMBufferSize;	///< 1 �b���� mp3 �f�[�^��ϊ��������́A�œK�ȓ]���� PCM �o�b�t�@�T�C�Y
	ByteBuffer				m_mp3SourceBufferParSec;	///< �f�R�[�h���Ƀt�@�C������ǂ� 1 �b���� mp3 �f�[�^���ꎞ�I�Ɋi�[����o�b�t�@
	bool					m_resetFlag;				///< �f�R�[�h��Ԃ̃��Z�b�g��v������t���O ( read() �ł̃f�R�[�h���̃t���O�w��Ɏg�� )

};

} // namespace Audio
} // namespace Lumino
