
#include <Lumino/Base/Exception.h>
#include <Lumino/IO/BinaryReader.h>
#include "WaveDecoder.h"

namespace Lumino
{
namespace Audio
{

/// wave �t�@�C���̃w�b�_
struct WaveFileHeader
{
	uint32_t	RIFF;
	uint32_t	Size;
	uint32_t	WaveHeader;
};

//=============================================================================
// WaveDecoder
//=============================================================================
	
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
WaveDecoder::WaveDecoder()
	: mInStream(NULL)
	, mSourceDataSize(0)
	, mDataOffset(0)
	, mPCMDataSize(0)
	, mTotalTime(0)
	, mOnmemoryPCMBuffer(NULL)
	, mOnmemoryPCMBufferSize(0)
	, mTotalSamples(0)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
WaveDecoder::~WaveDecoder()
{
	LN_SAFE_DELETE_ARRAY(mOnmemoryPCMBuffer);
	LN_SAFE_RELEASE(mInStream);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void WaveDecoder::Create(Stream* stream)
{
	LN_THROW(stream != NULL, ArgumentException);
	mInStream = stream;
	mInStream->AddRef();

	// �O�̂��߃t�@�C���|�C���^��擪�ɖ߂�
	mInStream->Seek(0, SeekOrigin_Begin);

	BinaryReader reader(mInStream);

	//-------------------------------------------------------------
	// �w�b�_�ǂݍ���ŔO�̂��߃t�H�[�}�b�g�`�F�b�N

	WaveFileHeader rh;
	reader.Read(&rh, sizeof(WaveFileHeader));
	if (memcmp(&rh.RIFF, "RIFF", 4) != 0 ||
		memcmp(&rh.WaveHeader, "WAVE", 4) != 0)
	{
		LN_THROW(0, InvalidFormatException);
	}

	//-------------------------------------------------------------
	// �e�`�����N�`�F�b�N

	char chunk[4];
	while (reader.Read(chunk, 4) == 4)
	{
		if (strncmp(chunk, "fmt ", 4) == 0)
		{
			uint32_t chunkSize = reader.ReadUInt32();	// �`�����N�T�C�Y

			mWaveFormat.FormatTag = reader.ReadUInt16();
			mWaveFormat.Channels = reader.ReadUInt16();
			mWaveFormat.SamplesPerSec = reader.ReadUInt32();
			mWaveFormat.AvgBytesPerSec = reader.ReadUInt32();
			mWaveFormat.BlockAlign = reader.ReadUInt16();
			mWaveFormat.BitsPerSample = reader.ReadUInt16();

			// �g�������̂���t�@�C���̏ꍇ�͓ǂ݂Ƃ΂�
			if (chunkSize > 16) {
				reader.Seek(chunkSize - 16);
				//mWaveFormat.EXSize = FileIO::readU16( mInFile );
				//mInFile->seek( mWaveFormat.EXSize, SEEK_CUR );
			}
			else {
				mWaveFormat.EXSize = 0;
			}
		}
		else if (strncmp(chunk, "data", 4) == 0)
		{
			uint32_t chunkSize = reader.ReadUInt32();

			// �t�@�C���|�C���^�̌��݈ʒu (data �`�����N���̃f�[�^�ʒu) ���L��
			mDataOffset = reader.GetPosition();

			// ���f�[�^�̃T�C�Y�� data �`�����N���̃f�[�^�̃T�C�Y
			mOnmemoryPCMBufferSize = mPCMDataSize = chunkSize;
			mSourceDataSize = mOnmemoryPCMBufferSize;

			// �S�̂̍Đ����Ԃ��v�Z����
			double t = static_cast< double >(mPCMDataSize) / (static_cast< double >(mWaveFormat.AvgBytesPerSec) * 0.001);
			mTotalTime = static_cast< uint32_t >(t);

			// �S�̂̍Đ��T���v����
			uint32_t one_channel_bits = (mOnmemoryPCMBufferSize / mWaveFormat.Channels) * 8;	// 1�`�����l��������̑��r�b�g��
			mTotalSamples = one_channel_bits / mWaveFormat.BitsPerSample;

			break;
		}
		// "fmt " �� "data" �ȊO�͂��ׂēǂݔ�΂�
		else
		{
			uint32_t chunkSize = reader.ReadUInt32();
			reader.Seek(chunkSize);
		}
	}

	// data �`�����N�͌������Ă���͂�
	LN_THROW(mDataOffset != 0, InvalidFormatException);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void WaveDecoder::FillOnmemoryBuffer()
{
	Threading::MutexScopedLock lock(m_mutex);

	if (mOnmemoryPCMBuffer == NULL)
	{
		// �I���������Đ�����Ƃ��ɕK�v�ȃo�b�t�@�̃T�C�Y�͌��f�[�^�̃T�C�Y�Ɠ���
		mOnmemoryPCMBufferSize = mPCMDataSize;

		// �������m��
		mOnmemoryPCMBuffer = LN_NEW byte_t[mOnmemoryPCMBufferSize];

		// ���݂̃V�[�N�ʒu���o���Ă���
		int64_t old_seek = mInStream->GetPosition();

		// �t�@�C���|�C���^���f�[�^������ꏊ�̐擪�ɃZ�b�g
		mInStream->Seek(mDataOffset, SeekOrigin_Begin);

		// �S���ǂݍ���
		int size = mInStream->Read(mOnmemoryPCMBuffer, mOnmemoryPCMBufferSize);

		// �ǂݍ��񂾃T�C�Y���ςȏꍇ�̓G���[
		LN_THROW(size == mOnmemoryPCMBufferSize, InvalidOperationException, "read file size is incorrect.\nThere is a possibility that the file is corrupted.");

		// �V�[�N�ʒu�����ɖ߂�
		mInStream->Seek(old_seek, SeekOrigin_Begin);

		// ��������Ȃ�
		LN_SAFE_RELEASE(mInStream);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void WaveDecoder::Read(uint32_t seekPos, void* buffer, uint32_t buffer_size, uint32_t* out_read_size, uint32_t* out_write_size)
{
	LN_THROW(mInStream != NULL, InvalidOperationException);	// �I���������Đ��ƃX�g���[�~���O�Đ��œ��� AudioStream �����L�����Ƃ��ɂԂ���
	Threading::MutexScopedLock lock(m_mutex);

	mInStream->Seek(mDataOffset + seekPos, SeekOrigin_Begin);

	if (!buffer || !(buffer_size > 0) || !(out_read_size) || !(out_write_size)) {
		return;
	}

	// �ǂݍ��ރT�C�Y
	uint32_t read_size = buffer_size;
	// �\�[�X�̃T�C�Y�𒴂��Ă���ꍇ�̓\�[�X�T�C�Y���ǂ�
	if (mInStream->GetPosition() + buffer_size > mDataOffset + mPCMDataSize)
	{
		read_size = (mDataOffset + mPCMDataSize) - mInStream->GetPosition();
	}

	size_t size = mInStream->Read(buffer, read_size);

	// ���f�[�^����ǂݍ��񂾃T�C�Y�ƁAbuffer_ �֏������񂾃T�C�Y�͓���
	*out_read_size = static_cast<uint32_t>(size);
	*out_write_size = static_cast<uint32_t>(size);
}


} // namespace Audio
} // namespace Lumino
