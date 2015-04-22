/*
	@file	MidiDecoder.h
*/
#pragma once

#include <Lumino/Base/ByteBuffer.h>
#include <Lumino/IO/BinaryReader.h>
#include "AudioStream.h"

namespace Lumino
{
namespace Audio
{

/// MIDI �f�[�^ AudioStream
class MidiDecoder
	: public AudioDecoder
{
public:
	MidiDecoder();
	virtual ~MidiDecoder();

public:
	/// ��������ɓW�J���ꂽ MIDI �f�[�^�̎擾
	const byte_t* GetSourceData() const { return m_midiFileData.GetData(); }

	/// ���ʂ𐳋K�����ēǂݍ��ނ��̐ݒ� ( fillBufferAndReleaseStream() �̑O�ɌĂԂ��� )
	void SetEnableVolumeNormalize(bool flag) { m_volumeNormalize = flag; }

public:
	virtual void				Create(Stream* stream);
	virtual StreamFormat		GetSourceFormat() const { return StreamFormat_Midi; }
	virtual const WaveFormat*	GetWaveFormat() const { return NULL; }
	virtual uint32_t			GetSourceDataSize() const { return m_midiFileData.GetSize(); }
	virtual uint32_t			GetTotalUnits() const { printf("Midi::getTotalUnits() Undefined."); return 0; }
	virtual byte_t*				GetOnmemoryPCMBuffer() const { return NULL; }
	virtual uint32_t			GetOnmemoryPCMBufferSize() const { return 0; }
	virtual uint32_t			GetBytesPerSec() const { return 0; }
	virtual void				GetLoopState(uint32_t* begin, uint32_t* length) const;
	virtual void				FillOnmemoryBuffer();
	virtual void				Read(uint32_t seekPos, void* buffer, uint32_t bufferSize, uint32_t* outReadSize, uint32_t* outWriteSize);
	virtual void				Reset();

private:

	/// midi �f�[�^�̒��̕K�v�ȃf�[�^���`�F�b�N����
	void SearchData();

	/// �f���^�^�C���̓ǂݍ���
	uint32_t ReadDelta(BinaryReader& reader);

	/// �g���b�N���� CC111 ����
	bool SearchTrack(BinaryReader& reader, uint32_t* cc111_time);

private:

	/// Midi �t�@�C���̃w�b�_
	struct MidiHeader
	{
		uint8_t		mChunktype[4];	///< �`�����N�^�C�v (MThd)
		uint32_t	mLength;		///< �f�[�^��
		uint16_t	mFormat;		///< �t�H�[�}�b�g�^�C�v
		uint16_t	mNumtrack;		///< �g���b�N��
		uint16_t	mTimebase;		///< �^�C���x�[�X
	};

	/// �{�����[���`�F���W�̈ʒu�ƒl (���K���Ɏg��)
	struct VolumeEntry
	{
		uint32_t	mPosition;		///< �{�����[���`�F���W�̒l�̈ʒu [ 00 B0 07 46 ] �� 07
		uint32_t	mVolume;		///< �{�����[���l
	};

private:
	Stream*					m_stream;		///< ���̓X�g���[��
	ByteBuffer				m_midiFileData;	///< MIDI �t�@�C���S�̂̃f�[�^ (m_stream �����ׂă������ɓǂݍ��񂾃o�b�t�@)
	Threading::Mutex		m_mutex;
	uint32_t				m_cc111Time;
	uint32_t				m_baseTime;
	ArrayList<VolumeEntry>	m_volumeEntryList;
	bool					m_volumeNormalize;

};

} // namespace Audio
} // namespace Lumino
