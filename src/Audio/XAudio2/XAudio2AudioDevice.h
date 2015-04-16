﻿/*
	@file	XAudio2AudioDevice.h
*/
#pragma once

#include <xaudio2.h>
#include <x3daudio.h>
//#include "../../../Base/Container/NodeList.h"
//#include "../../../Base/NonCopyable.h"
//#include "../../../System/Interface.h"
//#include "../../Interface.h"
//#include "../DirectMusic/DirectMusicAudioDevice.h"
#include <Lumino/Math/Vector3.h>
#include "../AudioDevice.h"

namespace Lumino
{
namespace Audio
{

/// 3D オーディオの計算に必要なパラメータ
struct EmitterState
	: public Base::NonCopyable
{
public:
	static const int OUTPUTCHANNELS = 8;
	
public:
    LVector3        Position;
    LVector3        Velocity;
    lnFloat         Distance; 

public:
    EmitterState( lnU32 input_channels_ );
    ~EmitterState();

	/// Position から X3DAUDIO_EMITTER に正しい座標を設定する
    void updateXAudioEmitter( lnFloat scale );

public:
    X3DAUDIO_EMITTER		Emitter;
    X3DAUDIO_CONE			EmitterCone;
    FLOAT32*				EmitterAzimuths;
    X3DAUDIO_DSP_SETTINGS	DSPSettings;            // この中の DstChannelCount は AudioDevice::update3DState でセットされる
    FLOAT32*				MatrixCoefficients;
};

/// XAudio2
class XAudio2AudioDevice
    : public AudioDevice
{
public:

	/// initialize() に渡す初期化データ
    //struct ConfigData
    //{
    //    bool                    EnableDebugFlag;    ///< XAudio2 のデバッグフラグを有効にする場合 true ( Release ビルドの場合は無効 )
    //    DirectMusicInitMode		DMInitMode;         ///< DirectMusic を初期化する時の方法
    //    System::Window*        Window;             ///< DirectMusic の初期化に使うウィンドウハンドルを持つウィンドウクラス
    //};

public:
	XAudio2AudioDevice();
	virtual ~XAudio2AudioDevice();

public:

	/// 初期化
    bool Initialize(/* const ConfigData& configData */);

	/// 終了処理 ( デストラクタでも呼ばれます )
    void Finalize();

	/// IXAudio2 インスタンスの取得
	IXAudio2* GetXAudio2() const { return mXAudio; }

	/// マスタリングボイスの取得
	IXAudio2MasteringVoice* GetMasteringVoice() { return mMasteringVoice; }

	/// 3D 音源の計算をする ( AudioPlayer::polling() から呼ばれる )
	void CalcEmitterState(EmitterState* emitter);

public:
    virtual AudioPlayer* CreateAudioPlayer( AudioSourceBase* source, bool enable_3d, SoundPlayType type );
    virtual void Update();
    virtual void SetListenerState( const Vector3& position, const Vector3& front );
    virtual void SetMetreUnitDistance( lnFloat d ) { mMetreUnitDistanceInv = 1.0f / d; }

private:

    typedef void ( *DEF_X3DAudioCalculate ) ( __in const X3DAUDIO_HANDLE, __in const X3DAUDIO_LISTENER*, __in const X3DAUDIO_EMITTER*, UINT32, __inout X3DAUDIO_DSP_SETTINGS* );
    typedef Base::NodeList< AudioPlayerBase > AudioPlayerList;

    IXAudio2*				    mXAudio;			    ///< IXAudio2 インスタンスへのポインタ
	IXAudio2MasteringVoice*	    mMasteringVoice;	    ///< IXAudio2MasteringVoice インスタンスへのポインタ
	X3DAUDIO_HANDLE			    mX3DInstance;           ///< 3D オーディオの管理クラス
    lnU32						mDeviceChannels;        ///< アクティブなオーディオデバイスの出力チャンネル数
    
    X3DAUDIO_LISTENER	        mListenerState;
    HMODULE                     mX3DAudioModule;        ///< "X3DAudio1_7.dll"
    DEF_X3DAudioCalculate       mMD_X3DAudioCalculate;  ///< X3DAudioCalculate()
    lnFloat                     mMetreUnitDistanceInv;  ///< 3D 空間の1メートル相当の距離の逆数
    
    DirectMusic::AudioDevice*   mDirectMusicAudioDevice;
    bool                        mCoInited;
};

} // namespace Audio
} // namespace Lumino
