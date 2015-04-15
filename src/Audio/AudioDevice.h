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

/// デバイス管理と、Player の生成・管理を行うクラスのベースクラス
class AudioDevice
	: public RefObject
{
public:
	AudioDevice();
	virtual ~AudioDevice();
};

} // namespace Audio
} // namespace Lumino
