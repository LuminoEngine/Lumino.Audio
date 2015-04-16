/*
	@file	AudioManager.h
*/
#pragma once

#include "Common.h"
#include <Lumino/Base/RefObject.h>
#include <Lumino/IO/FileManager.h>

namespace Lumino
{
namespace Audio
{
class GameAudio;
class AudioDevice;

/*
	@brief	音声機能の管理クラスです。
*/
class AudioManager
	: public RefObject
{

public:
	/// initialize() に渡す初期化データ
	struct ConfigData
	{
		Lumino::FileManager*	FileManager;					///< ファイルからの読み込みに使うファイル管理クラス
		uint32_t				StreamCacheObjectCount;			///< キャッシュサイズ (ファイル数)
		uint32_t				StreamSourceCacheMemorySize;	///< キャッシュサイズ (メモリ量(byte))
		DirectMusicInitMode		DMInitMode;						///< DirectMusic の初期化方法
		void*					hWnd;							///< DirectMusic の初期化に使うウィンドウハンドル
	};

public:
	AudioManager(const ConfigData& configData);
	virtual ~AudioManager();

	/// 終了処理
	void Finalize();

	/// オンメモリorストリーミング自動選択の音声データバイト数閾値
	void SetAutoPlayTypeSelectThreshold(uint32_t threshold) { mOnMemoryLimitSize = threshold; }

	/// GameAudio クラスの取得
	GameAudio* GetGameAudio() { return mGameAudio; }

	/// デバイスクラスの取得
	AudioDevice* GetAudioDevice() { return mAudioDevice; }

	///// Sound の作成 ( stream_ = NULL でキーを使った検索だけ行う )
	//Sound* createSound(FileIO::Stream* stream, SoundPlayType type, bool enable_3d, lnSharingKey key);

	///// Sound の作成 ( ファイル名指定 )
	//Sound* createSound(const lnChar* filename, SoundPlayType type, bool enable_3d);

	///// Sound の作成 ( IAudioSource 指定 )
	//Sound* createSound(AudioSourceBase* source, SoundPlayType type, bool enable_3d);

	/// グループの停止
	//void stopGroup(lnU32 group);

private:
	GameAudio*		mGameAudio;
	AudioDevice*	mAudioDevice;
	uint32_t		mOnMemoryLimitSize;
};

} // namespace Audio
} // namespace Lumino
