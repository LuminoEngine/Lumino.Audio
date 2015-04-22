
#include <Lumino/Base/Exception.h>
#include <Lumino/Base/ByteBuffer.h>
#include <Lumino/IO/BinaryReader.h>
#include "Internal.h"
#include "AudioUtils.h"
#include "Mp3Decoder.h"

namespace Lumino
{
namespace Audio
{

//=============================================================================
// Mp3Decoder
//=============================================================================
	
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Mp3Decoder::Mp3Decoder()
	: m_stream(NULL)
	, m_sourceDataSize(0)
	, m_dataOffset(0)
	, m_totalTime(0)
	, m_onmemoryPCMBuffer(NULL)
	, m_onmemoryPCMBufferSize(0)
	, m_totalSamples(0)
	, m_mutex()
	, m_hACMStream(NULL)
	, m_id3vTagFieldSize(0)
	, m_streamingPCMBufferSize(0)
	, m_mp3SourceBufferParSec()
	, m_resetFlag(true)
{
	memset(&m_waveFormat, 0, sizeof(m_waveFormat));
	memset(&m_acmMP3WaveFormat, 0, sizeof(m_acmMP3WaveFormat));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
Mp3Decoder::~Mp3Decoder()
{
	if (m_hACMStream)
	{
		acmStreamClose(m_hACMStream, 0);
		m_hACMStream = NULL;
	}

	LN_SAFE_DELETE_ARRAY(m_onmemoryPCMBuffer);
	LN_SAFE_RELEASE(m_stream);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Mp3Decoder::Create(Stream* stream)
{
	LN_THROW(stream != NULL, ArgumentException);
	m_stream = stream;
	m_stream->AddRef();

	// �t�@�C���|�C���^��擪�ɖ߂�
	m_stream->Seek(0, SeekOrigin_Begin);

	m_dataOffset = 0;

	// ID3v �m�F
	CheckId3v();
		
	// PCM �t�H�[�}�b�g�擾
	GetPCMFormat();

	// mp3 �� PCM �Ƀf�R�[�h�������� wave �t�H�[�}�b�g���擾
	MPEGLAYER3WAVEFORMAT* mp3_format = &m_acmMP3WaveFormat;
	WAVEFORMATEX wav_fmt_ex;
	wav_fmt_ex.wFormatTag = WAVE_FORMAT_PCM;
	MMRESULT mmr = acmFormatSuggest(NULL, &mp3_format->wfx, &wav_fmt_ex, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG);
	LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);

	// ACM �ϊ��X�g���[�����J�� ( mp3 �� wave )
	//HACMSTREAM* acm = mACMStreamHandle;//(HACMSTREAM*)&mACMStreamHandle;
	mmr = acmStreamOpen(&m_hACMStream, NULL, &mp3_format->wfx, &wav_fmt_ex, NULL, 0, 0, 0);
	LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);

	// WAVEFORMATEX �� Audio::WaveFormat
	AudioUtils::ConvertWAVEFORMATEXToLNWaveFormat(wav_fmt_ex, &m_waveFormat);

	// �S�̂�ϊ��������� PCM �T�C�Y�� m_onmemoryPCMBufferSize �Ɋi�[
    DWORD pcm_size = 0;
	mmr = acmStreamSize(m_hACMStream, m_sourceDataSize, &pcm_size, ACM_STREAMSIZEF_SOURCE);
	LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);
    m_onmemoryPCMBufferSize = pcm_size;

	// 1 �b���� mp3 �f�[�^��ϊ��������́A�œK�ȓ]���� PCM �o�b�t�@�T�C�Y���擾����
	DWORD wave_buffer_size;
	mmr = acmStreamSize(m_hACMStream, mp3_format->wfx.nAvgBytesPerSec, &wave_buffer_size, ACM_STREAMSIZEF_SOURCE);
	LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);
	m_streamingPCMBufferSize = wave_buffer_size;

	//HACMSTREAM acm2;
	//MMRESULT m = acmStreamOpen( &acm2, NULL, &mp3_format->wfx, &mWaveFormat, NULL, 0, 0, 0 );
	//LASSERT_STRONG_RETURN( (m!=0), false, "acmStreamOpen"  );
	//acmStreamSize( m_hACMStream, mMP3BufferSize, &mMP3BufferSize, ACM_STREAMSIZEF_SOURCE );
	//acmStreamClose( acm2, 0 );

	//// �X�g���[�~���O�Đ��p�̃o�b�t�@�̃T�C�Y���擾 ( mp3_format->wfx.nAvgBytesPerSec �͉��̃T�C�Y )
	//mMP3SourceBufferSizeParSec = ;

	//
	//mMP3SourceBufferParSec = LN_NEW byte_t[ mMP3SourceBufferSizeParSec ];
	// �X�g���[�~���O�Đ��p�� 1 �b���� mp3 �f�[�^�̃T�C�Y���A���������m��
	m_mp3SourceBufferParSec.Resize(mp3_format->wfx.nAvgBytesPerSec);

	// �S�̂̍Đ����Ԃ��v�Z����
	double t = static_cast< double >(m_onmemoryPCMBufferSize) / (static_cast< double >(m_waveFormat.AvgBytesPerSec) * 0.001);
	m_totalTime = static_cast< uint32_t >(t);

    // �S�̂̍Đ��T���v���������߂�
	uint32_t one_channel_bits = (m_onmemoryPCMBufferSize / m_waveFormat.Channels) * 8;	// 1�`�����l��������̑��r�b�g��
	m_totalSamples = one_channel_bits / m_waveFormat.BitsPerSample;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Mp3Decoder::FillOnmemoryBuffer()
{
	Threading::MutexScopedLock lock(m_mutex);

	if (m_onmemoryPCMBuffer == NULL)
	{
		MMRESULT mmr;

		// mp3 �f�[�^�S�̂��i�[����o�b�t�@���쐬���ēǂݍ���
		//lnU8* mp3_buffer = (lnU8*)malloc(m_sourceDataSize);
		//LN_THROW_SystemCall(mp3_buffer);
		ByteBuffer mp3_buffer(m_sourceDataSize);

		m_stream->Seek(m_dataOffset, SeekOrigin_Begin);
		size_t read_size = m_stream->Read(mp3_buffer.GetData(), m_sourceDataSize);
		LN_THROW(read_size == m_sourceDataSize, InvalidFormatException);

		// �S�̂�ϊ��������� PCM �T�C�Y�� mPCMSize �Ɋi�[
		DWORD pcm_size = 0;
		mmr = acmStreamSize(m_hACMStream, m_sourceDataSize, &pcm_size, ACM_STREAMSIZEF_SOURCE);
		LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);
		m_onmemoryPCMBufferSize = pcm_size;

		// �擾�����T�C�Y�Ńo�b�t�@�m��
		m_onmemoryPCMBuffer = LN_NEW byte_t[m_onmemoryPCMBufferSize];

		// ACM �w�b�_�ɕϊ��o�b�t�@�ݒ�
		ACMSTREAMHEADER ash;
		ZeroMemory(&ash, sizeof(ACMSTREAMHEADER));
		ash.cbStruct = sizeof(ACMSTREAMHEADER);
		ash.pbSrc = mp3_buffer.GetData();
		ash.cbSrcLength = m_sourceDataSize;
		ash.pbDst = (LPBYTE)m_onmemoryPCMBuffer;
		ash.cbDstLength = m_onmemoryPCMBufferSize;

		// �R���o�[�g���s
		mmr = acmStreamPrepareHeader(m_hACMStream, &ash, 0);
		LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);
		mmr = acmStreamConvert(m_hACMStream, &ash, 0);
		LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);
		mmr = acmStreamUnprepareHeader(m_hACMStream, &ash, 0);
		LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);

		// ���ۂ� PCM �o�b�t�@�ɏ������񂾃f�[�^�T�C�Y���L������
		m_onmemoryPCMBufferSize = ash.cbDstLengthUsed;

		LN_SAFE_RELEASE(m_stream);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Mp3Decoder::Read(uint32_t seekPos, void* buffer, uint32_t buffer_size, uint32_t* out_read_size, uint32_t* out_write_size)
{
	LN_THROW(m_stream != NULL, InvalidOperationException);	// �I���������Đ��ƃX�g���[�~���O�Đ��œ��� AudioStream �����L�����Ƃ��ɂԂ���
	Threading::MutexScopedLock lock(m_mutex);

	//if (m_onmemoryPCMBuffer)
	//{
	//	AudioSourceBase::read(buffer, buffer_size, out_read_size, out_write_size);
	//}
	//else
	//{

		m_stream->Seek(m_dataOffset + seekPos, SeekOrigin_Begin);

		ZeroMemory(buffer, buffer_size);

		// �t�@�C������f�[�^�ǂݍ���
		size_t read_size = m_stream->Read(m_mp3SourceBufferParSec.GetData(), m_mp3SourceBufferParSec.GetSize());

		DWORD src_length = m_mp3SourceBufferParSec.GetSize();

		// ���ۂɓǂݍ��񂾃T�C�Y���A�ǂނׂ��T�C�Y���������������ꍇ
		if (read_size < m_mp3SourceBufferParSec.GetSize())
		{
			// �Ƃ肠�����A�ǂݍ��߂��T�C�Y���R���o�[�g����
			src_length = read_size;
		}

		// ACM �w�b�_�ɕϊ��o�b�t�@�ݒ�
		ACMSTREAMHEADER ash;
		ZeroMemory(&ash, sizeof(ACMSTREAMHEADER));
		ash.cbStruct = sizeof(ACMSTREAMHEADER);
		ash.pbSrc = m_mp3SourceBufferParSec.GetData();
		ash.cbSrcLength = src_length;
		ash.pbDst = (LPBYTE)buffer;
		ash.cbDstLength = buffer_size;

		// �R���o�[�g���s
		MMRESULT mmr = acmStreamPrepareHeader(m_hACMStream, &ash, 0);
		LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);

		DWORD acm_flag = (m_resetFlag == true) ? ACM_STREAMCONVERTF_START : ACM_STREAMCONVERTF_BLOCKALIGN;
		mmr = acmStreamConvert(m_hACMStream, &ash, acm_flag);
		LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);

		mmr = acmStreamUnprepareHeader(m_hACMStream, &ash, 0);
		LN_THROW(mmr == 0, InvalidOperationException, _T("MMRESULT:%u"), mmr);

		// �R���o�[�g�������ʁA���ۂɎg�����̈��Ԃ�
		*out_read_size = ash.cbSrcLengthUsed;
		*out_write_size = ash.cbDstLengthUsed;

		m_resetFlag = false;
	//}
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
void Mp3Decoder::CheckId3v()
{
	// �Ƃ肠�����ŏ��ɁA�t�@�C���T�C�Y�� mp3 �f�[�^�S�̂̃T�C�Y�Ƃ���
	m_sourceDataSize = m_stream->GetLength();

	// �Ƃ肠���� ID3v2 �Ƃ��ăw�b�_������ǂݍ���
	ID3v2Header header;
	int read_size = m_stream->Read(&header, sizeof(ID3v2Header));
	LN_THROW(read_size == sizeof(ID3v2Header), InvalidFormatException, "mp3 file size is invalid.");

	// Id3v2 �`���̏ꍇ
	if (header.ID[0] == 'I' && header.ID[1] == 'D' && header.ID[2] == '3')
	{
		// �^�O�T�C�Y�擾
		m_id3vTagFieldSize = ((header.Size[0] << 21) | (header.Size[1] << 14) | (header.Size[2] << 7) | (header.Size[3])) + 10;

		// �����f�[�^������ʒu�́A�^�O�̏ꏊ�̎�����Ƃ���
		m_dataOffset = m_id3vTagFieldSize;

		// �����f�[�^�{�̂̃T�C�Y�́A�t�@�C���S�̂̃T�C�Y����^�O�̕�������������
		m_sourceDataSize -= m_id3vTagFieldSize;
	}
	// Id3v2 �`���ȊO ( Id3v1 ) �̏ꍇ
	else
	{
		// �I�[�̃^�O��񂪂Ȃ�
		LN_THROW(m_stream->GetLength() >= 128, InvalidFormatException, "not found mp3 tag.");

		// �^�O�Ȃ��@�f�[�^������ꏊ�̓t�@�C���̐擪����
		m_id3vTagFieldSize = 0;
		m_dataOffset = 0;

		// �t�@�C���I�[���� 128 �o�C�g�߂����Ƃ���𒲂ׂ�
		byte_t data[3];
		m_stream->Seek(-128, SeekOrigin_End);
		read_size = m_stream->Read(data, 3);
		LN_THROW(read_size == 3, InvalidFormatException, "not found mp3 tag.");

		//printf( "%c %c %c %c\n", data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );

		// 'TAG' ����������
		if (data[0] == 'T' && data[1] == 'A' && data[2] == 'G')
		{
			// mp3 �f�[�^�����̃T�C�Y�́A�S�̂���^�O�̕�������������
			m_sourceDataSize -= 128;
		}
	}

	// �O�̂��߁A�t�@�C���|�C���^��擪�ɖ߂��Ă���
	m_stream->Seek(0, SeekOrigin_Begin);
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
void Mp3Decoder::GetPCMFormat()
{
	BYTE  index;
	BYTE  version;
	BYTE  channel;
	BYTE  padding;
	WORD  wBlockSize;
	DWORD dwBitRate;
	DWORD dwSampleRate;
	DWORD dwBitTableLayer3[][16] = {
		{ 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0 },
		{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 }
	};
	DWORD dwSampleTable[][3] = {
		{ 44100, 48000, 32000 },
		{ 22050, 24000, 16000 }
	};

	// �t���[���w�b�_������ǂݍ���
	byte_t data[4];
	m_stream->Seek(m_id3vTagFieldSize, SeekOrigin_Begin);
	m_stream->Read(data, 4);

	// data ���t���[���w�b�_���w���Ă��邩���ׂ�
	if (data[0] != 0xff || data[1] >> 5 != 0x07)
	{
		// �擪�ɂȂ���΃K���K���i�߂Ȃ���T��
		int rs;
		int ends = m_stream->GetLength();
		while (true)
		{
			rs = m_stream->Read(data, 4);
			LN_THROW(rs == 4, InvalidFormatException, "not found mp3 frame header.");

			if (data[0] == 0xff && data[1] >> 5 == 0x07)
			{
				break;
			}
			m_stream->Seek(-3, SeekOrigin_Current);
		}
	}

	// MP3 �̃o�[�W�����́H
	switch (data[1] >> 3 & 0x03)
	{
	case 3:
		version = 1;
		break;
	case 2:
		version = 2;
		break;
	default:
		LN_THROW(0, InvalidFormatException, "not found mp3 frame header.");
		break;
	}

	// ���C���[ 3 �H
	if ((data[1] >> 1 & 0x03) != 1)
	{
		LN_THROW(0, InvalidFormatException, "( ( data[1] >> 1 & 0x03 ) != 1 )");
	}

	// �e�[�u���Œ�`�����r�b�g���[�g�̂����A���Ă͂܂���̂�I��
	index = data[2] >> 4;
	dwBitRate = dwBitTableLayer3[version - 1][index];


	// �����悤�ɁA�T���v�����O���[�g��I��
	index = data[2] >> 2 & 0x03;
	dwSampleRate = dwSampleTable[version - 1][index];

	// �p�f�B���O�̎擾
	padding = data[2] >> 1 & 0x01;

	// �`�����l�����̎擾
	channel = ((data[3] >> 6) == 3) ? 1 : 2;

	wBlockSize = (WORD)((1152 * dwBitRate * 1000 / dwSampleRate) / 8) + padding;

	// MPEGLAYER3WAVEFORMAT �\���̂ɂ��낢��i�[����
	MPEGLAYER3WAVEFORMAT* format = &m_acmMP3WaveFormat;

	format->wfx.wFormatTag = WAVE_FORMAT_MPEGLAYER3;
	format->wfx.nChannels = channel;
	format->wfx.nSamplesPerSec = dwSampleRate;
	format->wfx.nAvgBytesPerSec = (dwBitRate * 1000) / 8;
	format->wfx.nBlockAlign = 1;
	format->wfx.wBitsPerSample = 0;
	format->wfx.cbSize = MPEGLAYER3_WFX_EXTRA_BYTES;

	format->wID = MPEGLAYER3_ID_MPEG;
	format->fdwFlags = padding ? MPEGLAYER3_FLAG_PADDING_ON : MPEGLAYER3_FLAG_PADDING_OFF;
	format->nBlockSize = wBlockSize;
	format->nFramesPerBlock = 1;
	format->nCodecDelay = 0x571;
}

} // namespace Audio
} // namespace Lumino
