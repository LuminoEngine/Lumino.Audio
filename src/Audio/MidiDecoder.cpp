
#include <Lumino/Base/Exception.h>
#include <Lumino/IO/BinaryReader.h>
#include "MidiDecoder.h"

namespace Lumino
{
namespace Audio
{

//=============================================================================
// MidiDecoder
//=============================================================================
	
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MidiDecoder::MidiDecoder()
	: m_stream(NULL)
	, m_midiFileData()
	, m_mutex()
	, m_cc111Time(0)
	, m_baseTime(0)
	, m_volumeEntryList()
	, m_volumeNormalize(true)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
MidiDecoder::~MidiDecoder()
{
	LN_SAFE_RELEASE(m_stream);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MidiDecoder::Create(Stream* stream)
{
	LN_REFOBJ_SET(m_stream, stream);
	m_volumeEntryList.Clear();

	SearchData();
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MidiDecoder::GetLoopState(uint32_t* cc111time, uint32_t* base_time) const
{
	*cc111time = m_cc111Time;
	*base_time = m_baseTime;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MidiDecoder::FillOnmemoryBuffer()
{
	Threading::MutexScopedLock lock(m_mutex);

	if (m_stream != NULL)
	{
		m_midiFileData.Resize((size_t)m_stream->GetLength(), false);
		m_stream->Seek(0, SeekOrigin_Begin);
		m_stream->Read(m_midiFileData.GetData(), m_midiFileData.GetSize());

		if (m_volumeNormalize)
		{
			// �{�����[���̍ő�l��T��
			uint32_t maxVolume = 0;
			LN_FOREACH(VolumeEntry& v, m_volumeEntryList)
			{
				maxVolume = std::max(maxVolume, v.mVolume);
			}

			// MIDI �f�[�^���̍ő�{�����[���� 127 �ɂ���̂ɕK�v�Ȓl�����߁A�S�Ẵ{�����[���ɉ��Z����
			int sub = 127 - maxVolume;
			LN_FOREACH(VolumeEntry& v, m_volumeEntryList)
			{
				m_midiFileData[v.mPosition] += sub;
			}
		}

		LN_SAFE_RELEASE(m_stream);
	}
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MidiDecoder::Read(uint32_t seekPos, void* buffer, uint32_t bufferSize, uint32_t* outReadSize, uint32_t* outWriteSize)
{
	LN_THROW(0, InvalidOperationException);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void MidiDecoder::Reset()
{
	LN_THROW(0, InvalidOperationException);
}
//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
void MidiDecoder::SearchData()
{
	// �t�@�C���|�C���^��擪�ɖ߂��Ă���
	m_stream->Seek(0, SeekOrigin_Begin);

	BinaryReader reader(m_stream);

	// Midi �t�@�C���̃w�b�_�ǂݍ���
	MidiHeader header;
	size_t size = reader.Read(&(header.mChunktype), 4);
	header.mLength = reader.ReadUInt32(ByteOrder_Big);
	header.mFormat = reader.ReadUInt16(ByteOrder_Big);
	header.mNumtrack = reader.ReadUInt16(ByteOrder_Big);
	header.mTimebase = reader.ReadUInt16(ByteOrder_Big);

	// �x�[�X�^�C���i�[
	m_baseTime = header.mTimebase;

	m_cc111Time = 0;
	uint32_t cc111time = 0;

	// �g���b�N�̐��������[�v���āAcc111 �ƃ{�����[���`�F���W��T��
	for (int i = 0; i < header.mNumtrack; ++i)
	{
		SearchTrack(reader, &cc111time);

		if (m_cc111Time == 0)
		{
			m_cc111Time = cc111time;
		}
	}
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
uint32_t MidiDecoder::ReadDelta(BinaryReader& reader)
{
	uint8_t t;
	uint32_t dtime = 0;
	for (int i = 0; i < sizeof(uint32_t); ++i)
	{
		size_t size = reader.Read(&t, sizeof(uint8_t));
		dtime = (dtime << 7) | (t & 0x7f);

		// MSB�������Ă��Ȃ��Ȃ�΁A���̃o�C�g�̓f���^�^�C���ł͂Ȃ��̂Ŕ�����
		if (!(t & 0x80)) break;
	}
	return dtime;
}

//----------------------------------------------------------------------
//
//----------------------------------------------------------------------
bool MidiDecoder::SearchTrack(BinaryReader& reader, uint32_t* cc111_time)
{
	// �g���b�N�`�����N�̃`�F�b�N
	uint8_t chunk[4];
	size_t read_size = reader.Read(chunk, 4);
	LN_THROW(read_size == 4, InvalidFormatException);
	LN_THROW(memcmp(chunk, "MTrk", 4) == 0, InvalidFormatException);

	// �g���b�N�̒����ǂݍ���
	uint32_t   track_length;
	track_length = reader.ReadUInt32(ByteOrder_Big);

	uint8_t prev_state = 0; // �ЂƂO�̃C�x���g�̃X�e�[�^�X�o�C�g���L������ϐ�
	uint8_t state;
	uint8_t data1;
	uint8_t data2;
	uint32_t track_time = 0;

	while (1)
	{
		// �f���^�^�C����ǂݍ���
		track_time += ReadDelta(reader);

		// �X�e�[�^�X�o�C�g��ǂݍ���
		read_size = reader.Read(&state, sizeof(uint8_t));
		LN_THROW(read_size == sizeof(uint8_t), InvalidFormatException);

		// �����j���O�X�e�[�^�X�̏ꍇ
		if (!(state & 0x80))
		{
			// ��O�̃C�x���g�̃X�e�[�^�X�o�C�g����
			state = prev_state;
			// �t�@�C���|�C���^����߂�
			reader.Seek(-1);
		}

		// �X�e�[�^�X�o�C�g����ɂǂ̃C�x���g������
		switch (state & 0xf0)
		{
			// �f�[�^�o�C�g�� 2 �o�C�g�̃C�x���g
		case 0x80:	// �m�[�g�I�t
		case 0x90:	// �m�[�g�I��
		case 0xA0:	// �|���t�H�j�b�N�E�L�[�E�v���b�V��
		case 0xE0:	// �s�b�`�x���h
			reader.Seek(2);
			break;

		case 0xB0:	// �R���g���[���`�F���W
			read_size = reader.Read(&data1, sizeof(uint8_t));
			LN_THROW(read_size == sizeof(uint8_t), InvalidFormatException);

			read_size = reader.Read(&data2, sizeof(uint8_t));
			LN_THROW(read_size == sizeof(uint8_t), InvalidFormatException);

			// cc111
			if (data1 == 0x6F)
			{
				*cc111_time = track_time;
			}
			// �{�����[���`�F���W
			else if (data1 == 0x07)
			{
				// �f�[�^�̈ʒu�ƃ{�����[�����L���[�ɓ���ĕۑ�
				VolumeEntry entry;
				entry.mPosition = (uint32_t)reader.GetPosition() - 1;
				entry.mVolume = data2;
				m_volumeEntryList.Add(entry);
				//printf("�{�����[���`�F���W %d\n", data2);
			}
			break;

			// �f�[�^�o�C�g�� 1 �o�C�g�̃C�x���g
		case 0xC0:	// �v���O�����`�F���W
		case 0xD0:	// �`�����l���v���b�V��
			reader.Seek(1);
			break;

			// �f�[�^�o�C�g���ϒ��̃C�x���g
		case 0xF0:
			// SysEx�C�x���g
			if (state == 0xF0)
			{
				int data_length = 0;
				// �f�[�^���ǂݍ���
				read_size = reader.Read(&data_length, sizeof(uint8_t));
				LN_THROW(read_size == sizeof(uint8_t), InvalidFormatException);

				reader.Seek(data_length);
			}
			// ���^�C�x���g
			else if (state == 0xFF)
			{
				uint8_t type;

				// type�̎擾
				read_size = reader.Read(&type, sizeof(uint8_t));
				LN_THROW(read_size == sizeof(uint8_t), InvalidFormatException);

				uint32_t data_length = -1;

				// type �ʂɃf�[�^�o�C�g�̒������擾
				switch (type)
				{
				case 0x00:
					data_length = 2; break;
				case 0x01:
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
				case 0x06:
				case 0x07:
					break;
				case 0x20:
					data_length = 1; break;
				case 0x21:
					data_length = 1; break;
				case 0x2F:
					data_length = 0; break; // �G���h�I�u�g���b�N
				case 0x51:
					data_length = 3; break; // �Z�b�g�e���|
				case 0x54:
					data_length = 5; break;
				case 0x58:
					data_length = 4; break;
				case 0x59:
					data_length = 2; break;
				case 0x7F:
					break;
				default:
					LN_THROW(0, InvalidFormatException, "invalid meta event.");
					return false;
				}

				uint32_t temp = data_length;

				// �f�[�^�����Œ�̏ꍇ
				if (data_length != -1)
				{
					data_length = ReadDelta(reader);
					if (data_length != temp)
					{
						LN_THROW(0, InvalidFormatException, "invalid meta event data lendth.");
						return false;
					}
				}
				else
				{
					// �C�ӂ̃f�[�^�����擾
					data_length = ReadDelta(reader);
				}

				reader.Seek(data_length);

				// �g���b�N�̏I�[�����������ꍇ�͏I��
				if (type == 0x2F)
				{
					return true;
				}
			}

			break;

		default:
			LN_THROW(0, InvalidFormatException, "invalid status byte.");
			return false;

		}
		// ���̃C�x���g���O�̃C�x���g�̃X�e�[�^�X�o�C�g���m�F�ł���悤�ɕۑ�����
		prev_state = state;
	}
	return true;
}

} // namespace Audio
} // namespace Lumino
