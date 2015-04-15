
#include <Lumino/Base/Exception.h>
#include "WaveStream.h"

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
// WaveStream
//=============================================================================
	
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
WaveStream::WaveStream(Stream* stream)
	: mInStream(NULL)
	, mSourceDataSize(0)
	, mDataOffset(0)
	, mPCMDataSize(0)
	, mTotalTime(0)
	, mOnmemoryPCMBuffer(NULL)
	, mOnmemoryPCMBufferSize(0)
	, mTotalSamples(0)
{
	LN_THROW(stream != NULL, ArgumentException);
	mInStream = stream;
	mInStream->AddRef();

	// �O�̂��߃t�@�C���|�C���^��擪�ɖ߂�
	mInStream->Seek(0, SeekOrigin_Begin);

	//-------------------------------------------------------------
	// �w�b�_�ǂݍ���ŔO�̂��߃t�H�[�}�b�g�`�F�b�N

	WaveFileHeader rh;
	mInStream->Read(&rh, sizeof(WaveFileHeader));
	if (memcmp(&rh.RIFF, "RIFF", 4) != 0 ||
		memcmp(&rh.WaveHeader, "WAVE", 4) != 0)
	{
		LN_THROW(0, InvalidFormatException);
	}

	//-------------------------------------------------------------
	// �e�`�����N�`�F�b�N

	char chunk[4];
	while (mInStream->Read(chunk, 4) == 4)
	{
		if (strncmp(chunk, "fmt ", 4) == 0)
		{
			uint32_t size = FileIO::FileUtils::readU32(mInFile);

			mWaveFormat.FormatTag = FileIO::FileUtils::readU16(mInFile);
			mWaveFormat.Channels = FileIO::FileUtils::readU16(mInFile);
			mWaveFormat.SamplesPerSec = FileIO::FileUtils::readU32(mInFile);
			mWaveFormat.AvgBytesPerSec = FileIO::FileUtils::readU32(mInFile);
			mWaveFormat.BlockAlign = FileIO::FileUtils::readU16(mInFile);
			mWaveFormat.BitsPerSample = FileIO::FileUtils::readU16(mInFile);

			// �g�������̂���t�@�C���̏ꍇ
			if (size > 16)
			{
				mInFile->seek(size - 16, SEEK_CUR);
				//mWaveFormat.EXSize = FileIO::readU16( mInFile );
				//mInFile->seek( mWaveFormat.EXSize, SEEK_CUR );
			}
			else
			{
				mWaveFormat.EXSize = 0;
			}
		}
		else if (strncmp(chunk, "data", 4) == 0)
		{
			uint32_t size = FileIO::FileUtils::readU32(mInFile);

			// �t�@�C���|�C���^�̌��݈ʒu ( data �`�����N���̃f�[�^�ʒu ) ���L��
			mDataOffset = mInFile->getPosition();

			// ���f�[�^�̃T�C�Y�� data �`�����N���̃f�[�^�̃T�C�Y
			mOnmemoryPCMBufferSize = mPCMDataSize = size;

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
			uint32_t size = FileIO::FileUtils::readU32(mInFile);
			//printf( "chunc:%c %c %c %c\n", chunk[0], chunk[1], chunk[2], chunk[3] );
			//printf( "size:%u\n", size );

			mInFile->seek(size, SEEK_CUR);
		}
	}

	//-------------------------------------------------------------
	// �㏈��

	if (mDataOffset == 0)
	{
		LN_THROW_InvalidFormat(0, "not found 'data' chunk.");
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
WaveStream::~WaveStream()
{
	LN_SAFE_DELETE_ARRAY(mOnmemoryPCMBuffer);
	LN_SAFE_RELEASE(mInStream);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void WaveStream::FillOnmemoryBuffer()
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void WaveStream::Read(uint32_t seekPos, void* buffer, uint32_t buffer_size, uint32_t* out_read_size, uint32_t* out_write_size)
{
}


} // namespace Audio
} // namespace Lumino
