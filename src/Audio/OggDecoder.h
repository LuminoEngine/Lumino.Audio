/*
	@file	OggDecoder.h
*/
#pragma once

#include "../../external/libogg/include/ogg/ogg.h"
#include "../../external/libvorbis/include/vorbis/vorbisfile.h"
#include "AudioStream.h"

namespace Lumino
{
namespace Audio
{

/// Ogg �����p Decoder
class OggDecoder
	: public AudioDecoder
{
public:
	OggDecoder();
	virtual ~OggDecoder();

public:
	virtual void Create(Stream* stream);
	virtual StreamFormat GetSourceFormat() const { return StreamFormat_Ogg; }
	virtual const WaveFormat* GetWaveFormat() const { return &mWaveFormat; }
	virtual uint32_t GetSourceDataSize() const { return mSourceDataSize; }
	virtual uint32_t GetTotalUnits() const { return mTotalSamples; }
	virtual byte_t* GetOnmemoryPCMBuffer() const { return mOnmemoryPCMBuffer; }
	virtual uint32_t GetOnmemoryPCMBufferSize() const { return mOnmemoryPCMBufferSize; }
	virtual uint32_t GetBytesPerSec() const { return mWaveFormat.AvgBytesPerSec; }
	virtual void GetLoopState(uint32_t* begin, uint32_t* length) const { *begin = mLoopStart; *length = mLoopLength; }
	virtual void FillOnmemoryBuffer();
	virtual void Read(uint32_t seekPos, void* buffer, uint32_t bufferSize, uint32_t* outReadSize, uint32_t* outWriteSize);
	virtual void Reset() {}


public:	// �ȉ��� ogg API ����̃R�[���o�b�N�Ƃ��ēo�^����֐�

	/// ogg �p read �R�[���o�b�N
	static size_t readOggCallback(void* buffer, size_t element_size, size_t count, void* stream);

	/// ogg �p seek �R�[���o�b�N
	static int seekOggCallback(void* stream, ogg_int64_t offset, int whence);

	/// ogg �p close �R�[���o�b�N
	static int closeOggCallback(void* stream);

	/// ogg �p tell �R�[���o�b�N
	static long tellOggCallback(void* stream);

private:
	static const int WORD_SIZE = sizeof(uint16_t);	/// ��1�T���v��������̃r�b�g��
	static const int WORD_BITS = WORD_SIZE * 8;

	Stream*			m_stream;				///< ���̓X�g���[��
	WaveFormat		mWaveFormat;			///< PCM �p�t�H�[�}�b�g
	uint32_t		mSourceDataSize;		///< �����f�[�^�����̃T�C�Y
	//uint64_t		mDataOffset;			///< �X�g���[������ PCM �f�[�^�̐擪�܂ł̃I�t�Z�b�g�o�C�g��
	uint32_t		mPCMDataSize;			///< �X�g���[������ PCM �f�[�^�̃T�C�Y
	uint32_t		mTotalTime;				///< �S�̂̍Đ����� ( �~���b )
	byte_t*			mOnmemoryPCMBuffer;		///< �I���������Đ��p�̃f�[�^��ǂݍ��ރo�b�t�@
	uint32_t		mOnmemoryPCMBufferSize;	///< mOnmemoryPCMBuffer �̃T�C�Y ( ���̂Ƃ��� mDataOffset �Ɠ��� )
	uint32_t		mTotalSamples;          ///< �S�̂̍Đ��T���v����
	Threading::Mutex	m_mutex;

	OggVorbis_File		mOggVorbisFile;			///< �I�[�v���ς݂� Ogg �t�@�C��
	uint32_t				mLoopStart;             ///< ���[�v�̈�̐擪�T���v����
	uint32_t				mLoopLength;            ///< ���[�v�̈�̒��� ( �T���v���� )

};

} // namespace Audio
} // namespace Lumino
