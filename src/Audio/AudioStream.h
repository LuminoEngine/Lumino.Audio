/*
	@file	AudioSource.h
*/
#pragma once

#include <Lumino/Base/Cache.h>
#include <Lumino/IO/Stream.h>
#include <Lumino/Audio/Common.h>

namespace Lumino
{
namespace Audio
{

/// �����f�[�^�̃x�[�X�N���X
class AudioStream
	: public RefObject
	, public ICacheObject
{
	LN_CACHE_OBJECT_DECL;
protected:
	AudioStream();
	virtual ~AudioStream();

public:

	/// �t�@�C���t�H�[�}�b�g�̎擾
	virtual StreamFormat getSourceFormat() const = 0;

	/// PCM �t�H�[�}�b�g�̎擾
	virtual const WaveFormat* getWaveFormat() const = 0;

	/// ���f�[�^�̃T�C�Y�̎擾 ( �X�g���[�~���O�Đ��ł̏I�����蓙�Ŏg�� )
	virtual uint32_t getSourceDataSize() const = 0;

	/// �S�̂̍Đ����Ԃ̎擾 ( �~���b�B��Ŗ����Ȃ邩�� )
	virtual uint32_t getTotalTime() const = 0;

	/// �S�̂̃T���v�����̎擾 ( Midi �̏ꍇ�̓~���[�W�b�N�^�C���P�� )
	virtual uint32_t getTotalUnits() const = 0;

	/// �I���������Đ��p�̃o�b�t�@�̐擪�A�h���X�擾 ( fillBufferAndReleaseStream() ���Ă�ł��Ȃ��ꍇ�� NULL )
	virtual byte_t* getOnmemoryPCMBuffer() const = 0;

	/// �I���������Đ����̑S�o�b�t�@�T�C�Y�̎擾
	virtual uint32_t getOnmemoryPCMBufferSize() const = 0;

	/// 1 �b���̃\�[�X�f�[�^���f�R�[�h�����Ƃ��́A�œK�ȃo�C�g���̎擾
	///	
	///	�ʏ�� PCM �t�H�[�}�b�g����擾�ł��邯�ǁAMP3 �̏ꍇ��
	///	API �̓s��(?)��A�f�R�[�h�ɍœK�� 1 �b���̃T�C�Y�́A���ʂ�PCM�̂���Ƃ͈قȂ�B
	///	���̂��߁A�����ƃ`�F�b�N�ł���悤�ɂ��̃��\�b�h��p�ӁB
	///	���܂̂Ƃ���� MP3 �Ɍ������b�����ǁAGetWaveFormat() ��
	///	�擾�����l���� 1 �b���̃T�C�Y���v�Z����ƃo�O�̂Œ��ӁB
	virtual uint32_t getBytesPerSec() const = 0;

	/// ���[�v�J�n�ʒu�ƏI���ʒu�̎擾
	///
	///	Midi �t�@�C���̏ꍇ�͍ŏ��� CC111 �ʒu�̃f���^�^�C���ƃx�[�X�^�C��
	virtual void getLoopState(uint32_t* begin, uint32_t* length) const = 0;

	/// �I�[�f�B�I�t�@�C���Ƃ��Ĉ����X�g���[����ݒ肷��
	///
	///	�󂯎�����X�g���[���͎Q�ƃJ�E���g���ЂƂ����A
	///	�C���X�^���X���������邩 fillBuffer() ���Ă΂��܂ŕێ�����܂��B
	virtual void setStream(Stream* stream) = 0;


	/// �I���������Đ��p�ɑS�Ẵf�[�^��ǂݍ���
	///
	/// �f�R�[�h��� PCM �f�[�^�T�C�Y���̃o�b�t�@������Ŋm�ۂ��A
	/// �����ɑS�Ẵf�[�^��ǂݍ��݂܂��B
	/// �f�[�^�y�уT�C�Y�� getOnmemoryPCMBuffer()�A
	/// getOnmemoryPCMBufferSize() �Ŏ擾���Ă��������B<br>
	/// <br>
	/// ������Ă΂�Ă��A���łɃo�b�t�@���m�ۂ���Ă���ꍇ��
	/// �Ȃɂ����܂���B<br>
	/// <br>
	/// �ďo����A�X�g���[���͉������A���̃I�[�f�B�I�\�[�X��
	/// �X�g���[�~���O�Đ��ɂ͎g�p�ł��Ȃ��Ȃ�܂��B<br>
	virtual void fillBufferAndReleaseStream() = 0;

	//----------------------------------------------------------------------
	///**
	//  @brief      �f�[�^���f�R�[�h���Abuffer_ �ɏ�������
	//
	//  @param[out] buffer          : PCM �f�[�^���������ރo�b�t�@
	//  @param[in]  buffer_size     : buffer_ �̃T�C�Y ( �o�C�g�P�� )
	//  @param[out] out_read_size   : �\�[�X�f�[�^����ǂݍ��񂾃f�[�^�T�C�Y
	//  @param[out] out_write_size  : ���ۂ� buffer_ �ɏ������񂾃T�C�Y
	//  @par
	//              �ł��邾�� buffer_size �𖞂����悤�Ƀf�[�^���f�R�[�h���A
	//              buffer �������݂܂��B
	//              �ʏ�Abuffer_size �� getBytesPerSec() �Ɠ����l�ł��B<br>
	//              <br>
	//              read_size �̓f�R�[�h�ׂ̈Ƀ\�[�X����ǂݍ��񂾃f�[�^�T�C�Y�ł��B
	//              �ʏ�Amp3 ���̈��k�t�H�[�}�b�g�ł� write_size �����������l�ɂȂ�܂��B
	//              ���݂̃t�@�C���|�C���^�� read_size �̒l�𑫂����l���A
	//              ����̓ǂݍ��݈ʒu�ƂȂ�܂��B<br>
	//              <br>
	//              write_size �́A�ʏ�� buffer_size �Ɠ����l�ł����A
	//              �t�@�C���I�[�Ȃǂł� buffer_size �����������l ( �����f�[�^������Ƃ���܂� )
	//              �ɂȂ�܂��B	
	//*/
	//----------------------------------------------------------------------
	virtual void read(void* buffer, lnU32 buffer_size, lnU32* out_read_size, lnU32* out_write_size);

	/// �t�@�C���|�C���^�ړ� (�擪����̃o�C�g�I�t�Z�b�g)
	/// (���̃N���X�Ŏ������Ă��� read() �� seek() �� getOnmemoryPCMBuffer() �ɓǂݍ���ł��鎖���O��)
	virtual void seek(lnU32 offset);

	/// �f�R�[�h��Ԃ̃��Z�b�g(�Đ��J�n���O�ɌĂ΂��BMP3 �p)
	virtual void reset() = 0;



	/// fillBufferAndReleaseStream() �X���b�h�Z�[�t
	virtual void fillBufferSafe()
	{
		// TODO: fillBufferAndReleaseStream() �𒼐ړǂ�ł�Ƃ���Ȃ����`�F�b�N
		Threading::ScopedLock lock(mMutex);
		this->fillBufferAndReleaseStream();
	}

	/// seek + reed + �X���b�h�Z�[�t
	virtual void readSafe(void* buffer, lnU32 buffer_size, lnU32 offset, lnU32* read_size, lnU32* write_size)
	{
		Threading::ScopedLock lock(mMutex);
		this->seek(offset);
		this->read(buffer, buffer_size, read_size, write_size);
	}
};

} // namespace Audio
} // namespace Lumino
