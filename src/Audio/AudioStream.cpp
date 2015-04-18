
#include "AudioStream.h"

namespace Lumino
{
namespace Audio
{

//=============================================================================
// AudioManager
//=============================================================================

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioStream::AudioStream()
	: m_exception(NULL)
	, m_finishedCreate(false)
{
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AudioStream::~AudioStream()
{
	LN_SAFE_DELETE(m_exception);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
bool AudioStream::CheckCreated()
{
	if (m_exception != NULL) {
		throw m_exception;
	}
	return m_finishedCreate;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AudioStream::OnCreateFinished(Exception* e)
{
	m_exception = e;
	m_finishedCreate = true;
}


} // namespace Audio
} // namespace Lumino
