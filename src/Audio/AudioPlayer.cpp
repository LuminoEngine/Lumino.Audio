
#include <Lumino/Base/Exception.h>
#include <Lumino/Math/MathUtils.h>
#include "AudioStream.h"
#include "AudioDevice.h"
#include "AudioPlayer.h"

namespace Lumino
{
namespace Audio
{

//=============================================================================
// AudioPlayer 
//=============================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioPlayer::AudioPlayer(AudioDevice* device)
	: mDevice(device)
	, m_audioStream(NULL)
	, mVolume(100.f)
	, mPitch(100.f)
	, mLoopBegin(0)
	, mLoopLength(0)
	, mIsPlaying(false)
	, mIsPausing(false)
	, mIsLoop(false)
{
	LN_SAFE_ADDREF(mDevice);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioPlayer::~AudioPlayer()
{
	LN_SAFE_RELEASE(m_audioStream);
	LN_SAFE_RELEASE(mDevice);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AudioPlayer::Initialize(AudioStream* audioStream, bool enable3d)
{ 
	LN_THROW(audioStream, ArgumentException);
	LN_REFOBJ_SET(m_audioStream, audioStream);

    // ソースからループ位置を取得
    uint32_t loop_begin, loop_length;
	m_audioStream->GetLoopState(&loop_begin, &loop_length);
    mLoopBegin  = loop_begin;
	mLoopLength = loop_length;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AudioPlayer::setVolume( int volume )
{
    mVolume = static_cast< float >( volume );
    mVolume = Math::Clamp( mVolume, 0.0f, 100.0f );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AudioPlayer::setPitch( int pitch )
{
	mPitch = static_cast< float >( pitch );
	mPitch = Math::Clamp(mPitch, 50.0f, 200.0f);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AudioPlayer::setLoopState(uint32_t loop_begin, uint32_t loop_length)
{
    if ( loop_begin == 0 && loop_length == 0 )
    {
        // ソースからループ位置を取得して設定する
		uint32_t begin, length;
		m_audioStream->GetLoopState(&begin, &length);
        mLoopBegin  = begin;
		mLoopLength = length;
    }
    else
    {
        mLoopBegin = loop_begin;
		mLoopLength = loop_length;
    }
}

} // namespace Audio
} // namespace Lumino
