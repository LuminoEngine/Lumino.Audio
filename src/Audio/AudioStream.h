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

/// 音声データのベースクラス
class AudioStream
	: public RefObject
	, public ICacheObject
{
	LN_CACHE_OBJECT_DECL;
protected:
	AudioStream();
	virtual ~AudioStream();

public:

	/// ファイルフォーマットの取得
	virtual StreamFormat getSourceFormat() const = 0;

	/// PCM フォーマットの取得
	virtual const WaveFormat* getWaveFormat() const = 0;

	/// 元データのサイズの取得 ( ストリーミング再生での終了判定等で使う )
	virtual uint32_t getSourceDataSize() const = 0;

	/// 全体の再生時間の取得 ( ミリ秒。後で無くなるかも )
	virtual uint32_t getTotalTime() const = 0;

	/// 全体のサンプル数の取得 ( Midi の場合はミュージックタイム単位 )
	virtual uint32_t getTotalUnits() const = 0;

	/// オンメモリ再生用のバッファの先頭アドレス取得 ( fillBufferAndReleaseStream() を呼んでいない場合は NULL )
	virtual byte_t* getOnmemoryPCMBuffer() const = 0;

	/// オンメモリ再生時の全バッファサイズの取得
	virtual uint32_t getOnmemoryPCMBufferSize() const = 0;

	/// 1 秒分のソースデータをデコードしたときの、最適なバイト数の取得
	///	
	///	通常は PCM フォーマットから取得できるけど、MP3 の場合は
	///	API の都合(?)上、デコードに最適な 1 秒分のサイズは、普通のPCMのそれとは異なる。
	///	そのため、ちゃんとチェックできるようにこのメソッドを用意。
	///	いまのところは MP3 に限った話だけど、GetWaveFormat() で
	///	取得した値から 1 秒分のサイズを計算するとバグので注意。
	virtual uint32_t getBytesPerSec() const = 0;

	/// ループ開始位置と終了位置の取得
	///
	///	Midi ファイルの場合は最初の CC111 位置のデルタタイムとベースタイム
	virtual void getLoopState(uint32_t* begin, uint32_t* length) const = 0;

	/// オーディオファイルとして扱うストリームを設定する
	///
	///	受け取ったストリームは参照カウントがひとつ増え、
	///	インスタンスが解放されるか fillBuffer() が呼ばれるまで保持されます。
	virtual void setStream(Stream* stream) = 0;


	/// オンメモリ再生用に全てのデータを読み込む
	///
	/// デコード後の PCM データサイズ分のバッファを内部で確保し、
	/// そこに全てのデータを読み込みます。
	/// データ及びサイズは getOnmemoryPCMBuffer()、
	/// getOnmemoryPCMBufferSize() で取得してください。<br>
	/// <br>
	/// 複数回呼ばれても、すでにバッファが確保されている場合は
	/// なにもしません。<br>
	/// <br>
	/// 呼出し後、ストリームは解放され、このオーディオソースは
	/// ストリーミング再生には使用できなくなります。<br>
	virtual void fillBufferAndReleaseStream() = 0;

	//----------------------------------------------------------------------
	///**
	//  @brief      データをデコードし、buffer_ に書き込む
	//
	//  @param[out] buffer          : PCM データを書き込むバッファ
	//  @param[in]  buffer_size     : buffer_ のサイズ ( バイト単位 )
	//  @param[out] out_read_size   : ソースデータから読み込んだデータサイズ
	//  @param[out] out_write_size  : 実際に buffer_ に書き込んだサイズ
	//  @par
	//              できるだけ buffer_size を満たすようにデータをデコードし、
	//              buffer 書き込みます。
	//              通常、buffer_size は getBytesPerSec() と同じ値です。<br>
	//              <br>
	//              read_size はデコードの為にソースから読み込んだデータサイズです。
	//              通常、mp3 等の圧縮フォーマットでは write_size よりも小さい値になります。
	//              現在のファイルポインタに read_size の値を足した値が、
	//              次回の読み込み位置となります。<br>
	//              <br>
	//              write_size は、通常は buffer_size と同じ値ですが、
	//              ファイル終端などでは buffer_size よりも小さい値 ( 音声データがあるところまで )
	//              になります。	
	//*/
	//----------------------------------------------------------------------
	virtual void read(void* buffer, lnU32 buffer_size, lnU32* out_read_size, lnU32* out_write_size);

	/// ファイルポインタ移動 (先頭からのバイトオフセット)
	/// (このクラスで実装している read() と seek() は getOnmemoryPCMBuffer() に読み込んでいる事が前提)
	virtual void seek(lnU32 offset);

	/// デコード状態のリセット(再生開始直前に呼ばれる。MP3 用)
	virtual void reset() = 0;



	/// fillBufferAndReleaseStream() スレッドセーフ
	virtual void fillBufferSafe()
	{
		// TODO: fillBufferAndReleaseStream() を直接読んでるところないかチェック
		Threading::ScopedLock lock(mMutex);
		this->fillBufferAndReleaseStream();
	}

	/// seek + reed + スレッドセーフ
	virtual void readSafe(void* buffer, lnU32 buffer_size, lnU32 offset, lnU32* read_size, lnU32* write_size)
	{
		Threading::ScopedLock lock(mMutex);
		this->seek(offset);
		this->read(buffer, buffer_size, read_size, write_size);
	}
};

} // namespace Audio
} // namespace Lumino
