/*
	@file	AudioStream.h
*/
#pragma once

#include <Lumino/Base/Exception.h>
#include <Lumino/Base/Cache.h>
#include <Lumino/IO/Stream.h>
#include <Lumino/IO/ASyncIOObject.h>
#include <Lumino/IO/FileManager.h>
#include <Lumino/Audio/Common.h>

namespace Lumino
{
namespace Audio
{
class AudioDecoder;

class AudioStream
	: public ASyncIOObject
	, public ICacheObject
{
	LN_CACHE_OBJECT_DECL;
public:
	AudioStream(Stream* stream);
	virtual ~AudioStream();

	void Create() { InvokeIOProc(true, &FileManager::GetInstance()); }

	/// ���������������Ă��邩�m�F���� (��O���������Ă���΂�������� throw �����)
	bool CheckCreated();

	/// �f�R�[�_�̎擾
	AudioDecoder* GetDecoder() { return m_decoder; }
	const AudioDecoder* GetDecoder() const { return m_decoder; }

protected:
	///�@�񓯊����[�h����
	virtual void OnASyncIOProc();

private:
	Stream*			m_stream;
	AudioDecoder*	m_decoder;
};

/// �����f�[�^�̃x�[�X�N���X
class AudioDecoder
	: public RefObject
{
protected:
	AudioDecoder();
	virtual ~AudioDecoder();

//public:
//	/// Create �ς݂����m�F����B��O���ۑ�����Ă���� throw ���� (�񓯊��ǂݍ��ݗp)
//	bool CheckCreated();
//
//protected:
//	friend class ASyncAudioStreamLoadTask;
//	void OnCreateFinished(Exception* e);

public:
	/// �쐬
	virtual void Create(Stream* stream) = 0;

	/// �t�@�C���t�H�[�}�b�g�̎擾
	virtual StreamFormat GetSourceFormat() const = 0;

	/// PCM �t�H�[�}�b�g�̎擾
	virtual const WaveFormat* GetWaveFormat() const = 0;

	/// ���f�[�^�̃T�C�Y�̎擾 ( �X�g���[�~���O�Đ��ł̏I�����蓙�Ŏg�� )
	virtual uint32_t GetSourceDataSize() const = 0;

	/// �S�̂̍Đ����Ԃ̎擾 ( �~���b�B��Ŗ����Ȃ邩�� )
	//virtual uint32_t getTotalTime() const = 0;

	/// �S�̂̃T���v�����̎擾 ( Midi �̏ꍇ�̓~���[�W�b�N�^�C���P�� )
	virtual uint32_t GetTotalUnits() const = 0;

	/// �I���������Đ��p�̃o�b�t�@�̐擪�A�h���X�擾 ( fillBufferAndReleaseStream() ���Ă�ł��Ȃ��ꍇ�� NULL )
	virtual byte_t* GetOnmemoryPCMBuffer() const = 0;

	/// �I���������Đ����̑S�o�b�t�@�T�C�Y�̎擾
	virtual uint32_t GetOnmemoryPCMBufferSize() const = 0;

	/// 1 �b���̃\�[�X�f�[�^���f�R�[�h����Ƃ��́A�œK�ȃo�C�g���̎擾
	///	
	///	�ʏ�� PCM �t�H�[�}�b�g����擾�ł��邯�ǁAMP3 �̏ꍇ��
	///	API �̓s��(?)��A�f�R�[�h�ɍœK�� 1 �b���̃T�C�Y�́A���ʂ�PCM�̂���Ƃ͈قȂ�B
	///	���̂��߁A�����ƃ`�F�b�N�ł���悤�ɂ��̃��\�b�h��p�ӁB
	///	���܂̂Ƃ���� MP3 �Ɍ������b�����ǁAGetWaveFormat() ��
	///	�擾�����l���� 1 �b���̃T�C�Y���v�Z����ƃo�O�̂Œ��ӁB
	virtual uint32_t GetBytesPerSec() const = 0;

	/// ���[�v�J�n�ʒu�ƏI���ʒu�̎擾
	///
	///	Midi �t�@�C���̏ꍇ�͍ŏ��� CC111 �ʒu�̃f���^�^�C���ƃx�[�X�^�C��
	virtual void GetLoopState(uint32_t* begin, uint32_t* length) const = 0;

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
	//virtual void fillBufferAndReleaseStream() = 0;
	/// ���X���b�h�Z�[�t�Ŏ�������
	virtual void FillOnmemoryBuffer() = 0;

	/*
		�f�[�^���f�R�[�h���Abuffer �ɏ�������
		@param[in]	seekPos			: �V�[�N�ʒu
		@param[out]	buffer			: PCM �f�[�^���������ރo�b�t�@
		@param[in]	bufferSize		: buffer �̃T�C�Y (�o�C�g�P��)
		@param[in]	outReadSize		: �\�[�X�X�g���[������ǂݍ��񂾃f�[�^�T�C�Y (���݂̃V�[�N�ʒu�ɂ��̒l�����Z�����l���A���̓ǂݎ��V�[�N�ʒu�ɂȂ�)
		@param[in]	outWriteSize	: ���ۂ� buffer �ɏ������񂾃T�C�Y (�f�R�[�h���ꂽPCM�f�[�^�̃T�C�Y = �f�o�C�X�ɓn���o�C�g��)
		@details	�ł��邾�� buffer_size �𖞂����悤�Ƀf�[�^���f�R�[�h���Abuffer �������݂܂��B
					outReadSize �̓f�R�[�h�ׂ̈Ƀ\�[�X�X�g���[������ǂݍ��񂾃f�[�^�T�C�Y�ł��B
					�ʏ�Amp3 ���̈��k�t�H�[�}�b�g�ł� outWriteSize �����������l�ɂȂ�܂��B
					���݂̃t�@�C���|�C���^�� outReadSize �̒l�𑫂����l���A����̓ǂݍ��݈ʒu�ƂȂ�܂��B
					���̊֐��̓X���b�h�Z�[�t�ł��B
	*/
	virtual void Read(uint32_t seekPos, void* buffer, uint32_t bufferSize, uint32_t* outReadSize, uint32_t* outWriteSize) = 0;

	/// �t�@�C���|�C���^�ړ� (�擪����̃o�C�g�I�t�Z�b�g)
	/// (���̃N���X�Ŏ������Ă��� read() �� seek() �� getOnmemoryPCMBuffer() �ɓǂݍ���ł��鎖���O��)
	//virtual void seek(uint32_t offset);

	/// �f�R�[�h��Ԃ̃��Z�b�g(�Đ��J�n���O�ɌĂ΂��BMP3 �p)
	virtual void Reset() = 0;

private:
	Exception*	m_exception;
	bool		m_finishedCreate;
};

} // namespace Audio
} // namespace Lumino
