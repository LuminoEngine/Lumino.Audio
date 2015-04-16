//==============================================================================
// XA2AudioDevice 
//==============================================================================

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
#include "stdafx.h"
#include "../../../Resource/LNResource.h"
#include "../../../Math/LMath.h"
#include "XA2AudioPlayer.h"
#include "XA2AudioDevice.h"

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
namespace LNote
{
namespace Core
{
namespace Audio
{
namespace XAudio2
{

//==============================================================================
// ■ AudioDevice
//==============================================================================

	// 背面の音源が減衰されるコーンの設定
	static const X3DAUDIO_CONE sListener_DirectionalCone = {
        X3DAUDIO_PI*5.0f/6.0f,
        X3DAUDIO_PI*11.0f/6.0f,
        1.0f, 1.75f,
        0.0f, 0.25f,
        0.708f, 1.0f };

	// 音が完全に聞こえなくなる距離 (XACT 使えばこんなのいらないっぽいけど、なんか面倒なので)
    static const float OuterDistance    = 14.0f;

	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    AudioDevice::AudioDevice()
        : mXAudio                   ( NULL )
        , mMasteringVoice           ( NULL )
        , mDeviceChannels           ( 0 )
        , mX3DAudioModule           ( NULL )
        , mMD_X3DAudioCalculate     ( NULL )
        , mMetreUnitDistanceInv     ( 1.0f )
        , mDirectMusicAudioDevice   ( NULL )
        , mCoInited                 ( false )
    {
        memset( mX3DInstance, 0, sizeof( mX3DInstance ) );
    }

	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    AudioDevice::~AudioDevice()
    {
        LN_LOG_FIN_BEGIN;

        SAFE_DELETE( mDirectMusicAudioDevice );

        if ( mMasteringVoice )
		{
			mMasteringVoice->DestroyVoice();
			mMasteringVoice = NULL;
		}

        if ( mXAudio )
        {
            mXAudio->StopEngine();
		    SAFE_RELEASE( mXAudio );
        }

        if ( mCoInited )
        {
            ::CoUninitialize();
            mCoInited = false;
        }

        if ( mX3DAudioModule )
        {
            ::FreeLibrary( mX3DAudioModule );
            mX3DAudioModule = NULL;
        }

        LN_LOG_FIN_END;
    }

	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    bool AudioDevice::initialize( const ConfigData& configData )
    {
        LN_LOG_INIT_BEGIN;

        if ( SUCCEEDED( ::CoInitializeEx( NULL, COINIT_MULTITHREADED ) ) )
        {
            mCoInited = true;
        }
        
        mX3DAudioModule = ::LoadLibrary( _T( "X3DAudio1_7.dll" ) );
		if ( mX3DAudioModule == NULL )
		{
			LN_LOG_WRITE( _T( "not found X3DAudio1_7.dll" ) );
			return false;
		}

        UINT32 flags = 0;

        if ( configData.EnableDebugFlag )
        {
		    flags |= XAUDIO2_DEBUG_ENGINE;
        }

        // XAudio2 作成
        HRESULT hr = XAudio2Create( &mXAudio, flags );
		if ( FAILED( hr ) )
		{
			LN_LOG_WRITE( _T( "failed XAudio2Create HRESULT:%u" ), hr );
			return false;
		}

		// マスターボイス作成
		hr = mXAudio->CreateMasteringVoice( &mMasteringVoice );
		if ( FAILED( hr ) )
		{
			LN_LOG_WRITE( _T( "failed CreateMasteringVoice HRESULT:%u" ), hr );
			return false;
		}

        LN_LOG_WRITE( "get AudioDevice information..." );

		// デバイス詳細情報の確認
		XAUDIO2_DEVICE_DETAILS details;
		hr = mXAudio->GetDeviceDetails( 0, &details );
        if ( SUCCEEDED( hr ) )
        {
            LN_LOG_WRITE( L"    DeviceID    : %s", details.DeviceID );
            LN_LOG_WRITE( L"    DisplayName : %s", details.DisplayName );
            LN_LOG_WRITE( L"    Role        : %d", details.Role );
            LN_LOG_WRITE( L"    OutputFormat ( WAVEFORMATEX )" );
            LN_LOG_WRITE( "        フォーマットID           : %hu", details.OutputFormat.Format.wFormatTag );
	        LN_LOG_WRITE( "        チャンネル数             : %hu", details.OutputFormat.Format.nChannels );
	        LN_LOG_WRITE( "        サンプリングレート       : %lu", details.OutputFormat.Format.nSamplesPerSec );
	        LN_LOG_WRITE( "        データ速度(Byte/sec)     : %lu", details.OutputFormat.Format.nAvgBytesPerSec );
	        LN_LOG_WRITE( "        ブロックサイズ           : %hu", details.OutputFormat.Format.nBlockAlign );
	        LN_LOG_WRITE( "        サンプルあたりのビット数 : %hu", details.OutputFormat.Format.wBitsPerSample );
	        LN_LOG_WRITE( "        拡張部分のサイズ         : %hu", details.OutputFormat.Format.cbSize );
        }
        else
        {
            LN_LOG_WRITE( "failed to get information" );
        }

        // チャンネル数記憶
        mDeviceChannels = details.OutputFormat.Format.nChannels;
		
        // 関数アドレスを取得して 3D オーディオを初期化 ( グローバル情報の作成 )
        typedef void ( STDAPIVCALLTYPE *DEF_X3DAudioInitialize ) ( UINT32 , FLOAT32 , __out X3DAUDIO_HANDLE );
        DEF_X3DAudioInitialize md_X3DAudioInitialize = (DEF_X3DAudioInitialize)GetProcAddress( mX3DAudioModule, "X3DAudioInitialize" );

	    md_X3DAudioInitialize(
			details.OutputFormat.dwChannelMask,		// Xbox 360 では SPEAKER_XBOX
			X3DAUDIO_SPEED_OF_SOUND, mX3DInstance );
		    //SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER |
            //SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT

        // リスナーの設定
        mListenerState.OrientFront	= D3DXVECTOR3( 0, 0, 1 );	// 前方向の向き
		mListenerState.OrientTop	= D3DXVECTOR3( 0, 1, 0 );	// 上方向の向き ( OrientFront と正規直交であること )
		mListenerState.Position		= D3DXVECTOR3( 0, 0, 0 );
		mListenerState.Velocity		= D3DXVECTOR3( 0, 0, 0 );
		mListenerState.pCone		= NULL;//(X3DAUDIO_CONE*)&sListener_DirectionalCone;

        // X3DAudioCalculate() の関数アドレス
        mMD_X3DAudioCalculate = (DEF_X3DAudioCalculate)GetProcAddress( mX3DAudioModule, "X3DAudioCalculate" );

        // DirectMusic を初期化する場合
        if ( configData.DMInitMode != DMINITMODE_NOTUSE && configData.Window )
        {
            DirectMusic::AudioDevice::ConfigData data;
            data.DMInitMode     = configData.DMInitMode;
            data.Window         = configData.Window;
            mDirectMusicAudioDevice = LN_NEW DirectMusic::AudioDevice();
            mDirectMusicAudioDevice->initialize( data );
        }

        LN_LOG_INIT_END;
        return true;
    }

	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    void AudioDevice::calcEmitterState( EmitterState* emitter )
    {
        if ( emitter )
        {
            static const DWORD calc_flags =
				X3DAUDIO_CALCULATE_MATRIX       |
                X3DAUDIO_CALCULATE_LPF_DIRECT   |
                X3DAUDIO_CALCULATE_LPF_REVERB   |
                X3DAUDIO_CALCULATE_REVERB       |
                X3DAUDIO_CALCULATE_DOPPLER;
                //X3DAUDIO_CALCULATE_EMITTER_ANGLE;
            //if (g_audioState.fUseRedirectToLFE)
		    //{
		    //	// On devices with an LFE channel, allow the mono source data
		    //	// to be routed to the LFE destination channel.
		    //	calc_flags |= X3DAUDIO_CALCULATE_REDIRECT_TO_LFE;
		    //}

            emitter->updateXAudioEmitter( mMetreUnitDistanceInv );
            emitter->DSPSettings.DstChannelCount = mDeviceChannels;

            mMD_X3DAudioCalculate(
                mX3DInstance,
                &mListenerState,
                &emitter->Emitter,
                calc_flags,
                &emitter->DSPSettings );
        }
    }

	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    AudioPlayerBase* AudioDevice::createAudioPlayer( AudioSourceBase* source, bool enable_3d, SoundPlayType type )
    {
		Base::RefPtr<AudioPlayerBase> audio_player;

        // 種類に応じてプレイヤーを作成する
		switch ( type )
		{
			// オンメモリ再生
			case SOUNDPLAYTYPE_ONMEMORY:
            {
                OnMemory* player = LN_NEW OnMemory( this );
				audio_player.attach( player );
                player->initialize( source, enable_3d );
				break;
            }
			// ストリーミング再生
			case SOUNDPLAYTYPE_STREAMING:
            {
                Streaming* player = LN_NEW Streaming( this );
                audio_player.attach( player );
				player->initialize( source, enable_3d );
				break;
            }
			// SMF
			case SOUNDPLAYTYPE_MIDI:
            {
				LN_THROW_InvalidOperation(mDirectMusicAudioDevice, Resource::String::ERR_DirectMusicNotInit);
                if ( mDirectMusicAudioDevice )
                {
					audio_player.attach(
						mDirectMusicAudioDevice->createAudioPlayer( source, enable_3d, type ) );
                }
				break;
            }
		}

		audio_player.addRef();
		return audio_player;
    }

	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    void AudioDevice::update()
    {
        mListenerState.OrientFront.x = this->mSoundListener.Direction.X;
        mListenerState.OrientFront.y = this->mSoundListener.Direction.Y;
        mListenerState.OrientFront.z = this->mSoundListener.Direction.Z;
        mListenerState.OrientTop.x = this->mSoundListener.UpDirection.X;
        mListenerState.OrientTop.y = this->mSoundListener.UpDirection.Y;
        mListenerState.OrientTop.z = this->mSoundListener.UpDirection.Z;
        mListenerState.Position.x = this->mSoundListener.Position.X * mMetreUnitDistanceInv;
        mListenerState.Position.y = this->mSoundListener.Position.Y * mMetreUnitDistanceInv;
        mListenerState.Position.z = this->mSoundListener.Position.Z * mMetreUnitDistanceInv;
        mListenerState.Velocity.x = this->mSoundListener.Velocity.X * mMetreUnitDistanceInv;
        mListenerState.Velocity.y = this->mSoundListener.Velocity.Y * mMetreUnitDistanceInv;
        mListenerState.Velocity.z = this->mSoundListener.Velocity.Z * mMetreUnitDistanceInv;

        if ( mDirectMusicAudioDevice ) mDirectMusicAudioDevice->update();
    }

	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    LNRESULT AudioDevice::setListenerState( const LVector3& position, const LVector3& front )
    {
        // 念のために正規化してから設定する
		LVector3 n = LVector3::Normalize(front);

        mListenerState.OrientFront= LMathUtils::toD3DXVECTOR3(n);
		mListenerState.Position = LMathUtils::toD3DXVECTOR3(position);

		return ResultCode_OK;
    }

//==============================================================================
// ■ EmitterState
//==============================================================================

    static const X3DAUDIO_DISTANCE_CURVE_POINT Emitter_LFE_CurvePoints[ 3 ]		= { 0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.0f };
	static const X3DAUDIO_DISTANCE_CURVE       Emitter_LFE_Curve				= { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_LFE_CurvePoints[0], 3 };
	static const X3DAUDIO_DISTANCE_CURVE_POINT Emitter_Reverb_CurvePoints[ 3 ]	= { 0.0f, 0.5f, 0.75f, 1.0f, 1.0f, 0.0f };
	static const X3DAUDIO_DISTANCE_CURVE       Emitter_Reverb_Curve				= { (X3DAUDIO_DISTANCE_CURVE_POINT*)&Emitter_Reverb_CurvePoints[0], 3 };

	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    EmitterState::EmitterState( lnU32 input_channels )//, lnU32 output_channels_ )
    {
        
        EmitterAzimuths = LN_NEW FLOAT32[ input_channels ];
		MatrixCoefficients = LN_NEW FLOAT32[ input_channels * OUTPUTCHANNELS ];

		for ( lnU32 i = 0; i < input_channels; ++i )
		{
			EmitterAzimuths[ i ] = 0;
		}

		// サウンドコーンの設定
		EmitterCone.InnerAngle		= 0.0f;		// 内部コーンの角度 ( ラジアン単位 0.0f ～ X3DAUDIO_2PI )
		EmitterCone.OuterAngle		= 0.0f;		// 外部コーンの角度 ( ラジアン単位 0.0f ～ X3DAUDIO_2PI )
		EmitterCone.InnerVolume	= 0.0f;		// 内部コーン上/内のボリュームスケーラ ( 0.0f ～ 2.0f )
		EmitterCone.OuterVolume	= 1.0f;		// 外部コーン上/以降のボリュームスケーラ ( 0.0f ～ 2.0f )
		EmitterCone.InnerLPF		= 0.0f;		// 内部コーン上/内の LPF ダイレクトパスまたはリバーブパスの係数スケーラ ( 0.0f ～ 1.0f )
		EmitterCone.OuterLPF		= 1.0f;		// 外部コーン上/以降の LPF ダイレクトパスまたはリバーブパスの係数スケーラ ( 0.0f ～ 1.0f )
		EmitterCone.InnerReverb	= 0.0f;		// 内部コーン上/内のリバーブ センドレベルスケーラ ( 0.0f ～ 2.0f )
		EmitterCone.OuterReverb	= 1.0f;		// 外部コーン上/以降のリバーブ センドレベルスケーラ ( 0.0f ～ 2.0f )
	
		// エミッタの設定
		Emitter.pCone			= &EmitterCone;			// サウンド コーンへのポインタまたは NULL ( NULL は全方向性 )
		Emitter.OrientFront	    = D3DXVECTOR3( 0, 0, 1 );	// 前方向の向き ( OrientTop と正規直交 )
		Emitter.OrientTop		= D3DXVECTOR3( 0, 1, 0 );	// 上方向の向き ( OrientFront と正規直交 )
		Emitter.Position		= D3DXVECTOR3( 0, 0, 0 );	// ワールド位置
		Emitter.Velocity		= D3DXVECTOR3( 0, 0, 0 );	// ユーザー定義のワールド単位/秒の速度

		Emitter.ChannelCount		= input_channels;		// X3DAUDIO_EMITTER 構造体によって定義されるエミッタの数
		Emitter.ChannelRadius		= 1.0f;					// ChannelCount が 1 より大きい場合にエミッタが配置される Position からの距離 ( 0.0f 以上 )
		Emitter.pChannelAzimuths	= EmitterAzimuths;		// ラジアン単位の方位角で表したチャンネル位置テーブル ( FLOAT32 の配列。各要素 0.0f ～ X3DAUDIO_2PI )
		Emitter.InnerRadius		    = 2.0f;					// 内部半径の計算に使用される値 ( 0.0f ～ MAX_FLT )
		Emitter.InnerRadiusAngle	= X3DAUDIO_PI / 4.0f;	// 内部角度の計算に使用される値 ( 0.0f ～ X3DAUDIO_PI/4.0 )

		Emitter.pVolumeCurve		= (X3DAUDIO_DISTANCE_CURVE*)&X3DAudioDefault_LinearCurve;	// ボリューム レベル距離カーブ ( NULL でデフォルト値を使う )
		Emitter.pLFECurve			= (X3DAUDIO_DISTANCE_CURVE*)&Emitter_LFE_Curve;				// LFE ロールオフ距離カーブ ( NULL でデフォルト値を使う )
		Emitter.pLPFDirectCurve		= NULL;														// ローパスフィルター (LPF) ダイレクトパス係数距離カーブ ( NULL でデフォルト値を使う )
		Emitter.pLPFReverbCurve		= NULL;														// LPF リバーブパス係数距離カーブ ( NULL でデフォルト値を使う )
		Emitter.pReverbCurve		= (X3DAUDIO_DISTANCE_CURVE*)&Emitter_Reverb_Curve;			// リバーブ センド レベル距離カーブ ( NULL でデフォルト値を使う )
		Emitter.CurveDistanceScaler	= 10.0f;													// 正規化された距離カーブをスケーリングするために使用するカーブ距離スケーラ ( FLT_MIN ～ FLT_MAX )
		Emitter.DopplerScaler		= 1.0f;													// ドップラー偏移効果を強調するために使用するドップラー偏移スケーラー ( 0.0f ～ FLT_MAX )
	
		//  X3DAudioCalculate の呼び出し結果を受け取る構造体の初期化
		memset( &DSPSettings, 0, sizeof( DSPSettings ) );
		memset( MatrixCoefficients, 0, sizeof( FLOAT32 ) * input_channels * OUTPUTCHANNELS );
		DSPSettings.SrcChannelCount		= input_channels;
		DSPSettings.DstChannelCount		= 0;//output_channels_;     // calc でセットしておく
		DSPSettings.pMatrixCoefficients	= MatrixCoefficients;
    }

	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    EmitterState::~EmitterState()
    {
        SAFE_DELETE_ARRAY( EmitterAzimuths );
		SAFE_DELETE_ARRAY( MatrixCoefficients );
    }
	//----------------------------------------------------------------------
	//
	//----------------------------------------------------------------------
    void EmitterState::updateXAudioEmitter( lnFloat scale )
    {
        Emitter.Position.x = Position.X * scale;
        Emitter.Position.y = Position.Y * scale;
        Emitter.Position.z = Position.Z * scale;
        Emitter.Velocity.x = Velocity.X * scale;
        Emitter.Velocity.y = Velocity.Y * scale;
        Emitter.Velocity.z = Velocity.Z * scale;
        Emitter.CurveDistanceScaler = Distance * scale;
    }

} // namespace XAudio2
} // namespace Audio
} // namespace Core
} // namespace LNote

//==============================================================================
//
//==============================================================================