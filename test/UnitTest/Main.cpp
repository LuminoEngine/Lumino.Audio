#include <TestConfig.h>
#include <Lumino/Platform/PlatformSupport.h>
#include "../../src/Audio/AudioStream.h"
#include "../../src/Audio/AudioPlayer.h"
using namespace Lumino::Audio;

//#pragma comment(lib, "dinput8.lib")
int main()
{
	//FileManager::

	
	Platform::ApplicationSettings s;
	Platform::Application app(s);

	Logger::Initialize(_T("log.txt"));

	AudioManager::ConfigData data;
	data.FileManager = &FileManager::GetInstance();
	data.StreamCacheObjectCount = 32;
	data.StreamSourceCacheMemorySize = 0;
	data.DMInitMode = DirectMusicInitMode_Normal;
	data.hWnd = Platform::PlatformSupport::GetWindowHandle(app.GetMainWindow());
	RefPtr<AudioManager> manager(AudioManager::Create(data));
	
	//RefPtr<AudioStream> stream(manager->CreateAudioStream(_T("D:/MMD/オーディオ/ZIGG-ZAGG.wav")));
	//RefPtr<AudioPlayer> player(manager->CreateAudioPlayer(stream, SoundPlayType_OnMemory, false));
	//player->play();

#if 0
	RefPtr<AudioStream> stream(manager->CreateAudioStream(_T("D:/MMD/オーディオ/ZIGG-ZAGG.wav")));
	//RefPtr<AudioStream> stream(manager->CreateAudioStream(_T("D:/tmp/ln21.mid")));
	RefPtr<Sound> sound(manager->CreateSound(stream, SoundPlayType_Unknown, false));
	sound->SetPitch(110);
	sound->SetLoopEnabled(true);
	sound->Play();
#endif

#if 0
	uint64_t st = Environment::GetTickCount();
	RefPtr<FileStream> stream(LN_NEW FileStream(_T("D:/MMD/オーディオ/ZIGG-ZAGG.wav"), FileOpenMode_Read/* | FileOpenMode_Deferring*/));
	
	RefPtr<Sound> sound(manager->CreateSound(stream, SoundPlayType_Unknown, false, CacheKey::Null));
#endif
	//RefPtr<Sound> sound(Sound::Create(LOCALFILE("Audio/TestData/UnitTest1.mid")));
	RefPtr<Sound> sound(Sound::Create(_T("D:/tmp/ln21.mid")));
	sound->SetPitch(110);
	sound->SetLoopEnabled(true);
	sound->Play();

	//printf("t:%llu\n", Environment::GetTickCount() - st);

	getchar();

	//Platform::ApplicationSettings s;
	//Platform::Application app(s);

	//HWND hWnd = Platform::PlatformSupport::GetWindowHandle(app.GetMainWindow());
	//RefPtr<InputManager> manager(InputManager::Create(hWnd));

	//Mouse* mouse = manager->GetMouse();
	//Keyboard* keyboard = manager->GetKeyboard();
	//while (app.DoEvents())
	//{
	//	manager->UpdateFrame();

	//	if (mouse->IsPress(MouseButton_1)) {
	//		printf("on\n");
	//	}
	//	if (mouse->GetWheelState()) {
	//		printf("%d\n", mouse->GetWheelState());
	//	}

	//	int keyCode = (int)keyboard->CheckPressedVirtualButton();
	//	if (keyCode) {
	//		printf("key:%d\n", keyCode);
	//	}


	//	Threading::Thread::Sleep(32);
	//}

	return 0;
}

