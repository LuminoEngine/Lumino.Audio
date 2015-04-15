/*
	@file	AudioDevice.h
*/
#pragma once

#include <Lumino/Base/Cache.h>
#include <Lumino/IO/Stream.h>
#include <Lumino/Audio/Common.h>

namespace Lumino
{
namespace Audio
{

/// �f�o�C�X�Ǘ��ƁAPlayer �̐����E�Ǘ����s���N���X�̃x�[�X�N���X
class AudioDevice
	: public RefObject
{
public:
	AudioDevice();
	virtual ~AudioDevice();
};

} // namespace Audio
} // namespace Lumino
