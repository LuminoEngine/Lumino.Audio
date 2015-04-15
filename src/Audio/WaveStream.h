/*
	@file	WaveStream.h
*/
#pragma once

#include "AudioStream.h"

namespace Lumino
{
namespace Audio
{

/// Wave �f�[�^ AudioStream
class WaveStream
	: public AudioStream
{
protected:
	WaveStream(Stream* stream);
	virtual ~WaveStream();

public:
	virtual StreamFormat GetSourceFormat() const { return StreamFormat_Wave; }
	virtual const WaveFormat* GetWaveFormat() const { return &mWaveFormat; }
	virtual uint32_t GetSourceDataSize() const { return mSourceDataSize; }
	virtual uint32_t GetTotalUnits() const { return mTotalSamples; }
	virtual byte_t* GetOnmemoryPCMBuffer() const { return mOnmemoryPCMBuffer; }
	virtual uint32_t GetOnmemoryPCMBufferSize() const { return mOnmemoryPCMBufferSize; }
	virtual uint32_t GetBytesPerSec() const { return mWaveFormat.AvgBytesPerSec; }
	virtual void GetLoopState(uint32_t* begin, uint32_t* length) const { *begin = 0; *length = 0; }
	virtual void FillOnmemoryBuffer();
	virtual void Read(uint32_t seekPos, void* buffer, uint32_t buffer_size, uint32_t* out_read_size, uint32_t* out_write_size);
	virtual void Reset() {}

private:
	Stream*			mInStream;				///< ���̓X�g���[��
	WaveFormat		mWaveFormat;			///< PCM �p�t�H�[�}�b�g
	uint32_t		mSourceDataSize;		///< �����f�[�^�����̃T�C�Y
	uint32_t		mDataOffset;			///< �X�g���[������ PCM �f�[�^�̐擪�܂ł̃I�t�Z�b�g�o�C�g��
	uint32_t		mPCMDataSize;			///< �X�g���[������ PCM �f�[�^�̃T�C�Y
	uint32_t		mTotalTime;				///< �S�̂̍Đ����� ( �~���b )
	uint8_t*		mOnmemoryPCMBuffer;		///< �I���������Đ��p�̃f�[�^��ǂݍ��ރo�b�t�@
	uint32_t		mOnmemoryPCMBufferSize;	///< mOnmemoryPCMBuffer �̃T�C�Y ( ���̂Ƃ��� mDataOffset �Ɠ��� )
	uint32_t		mTotalSamples;          ///< �S�̂̍Đ��T���v����

};

} // namespace Audio
} // namespace Lumino