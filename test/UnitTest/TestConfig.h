#pragma once

//#include <gtest/gtest.h>
#include <LuminoCore.h>
#include <LuminoAudio.h>
#include <Lumino/Testing/TestUtils.h>

using namespace Lumino;

#define LOCALFILE(fileName) TestUtils::GetFilePath(__FILE__, fileName).GetCStr()
#define LOCALFILEA(fileName) TestUtils::GetFilePathA(__FILE__, fileName).GetCStr()
#define LOCALFILEW(fileName) TestUtils::GetFilePathW(__FILE__, fileName).GetCStr()
