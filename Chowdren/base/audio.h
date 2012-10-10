#define SFML_STATIC
#include "SFML/Audio/Music.cpp"
#include "SFML/Audio/SoundFile.cpp"
#include "SFML/Audio/SoundStream.cpp"
#include "SFML/Audio/SoundSource.cpp"
#include "SFML/System/Thread.cpp"
#include "SFML/System/Lock.cpp"
#include "SFML/Audio/AudioDevice.cpp"
#include "SFML/Audio/ALCheck.cpp"
#include "SFML/System/Time.cpp"
#include "SFML/System/Sleep.cpp"
#include "SFML/System/Mutex.cpp"
#include "SFML/System/Err.cpp"

#if defined(SFML_SYSTEM_WINDOWS)
#include "SFML/System/Win32/SleepImpl.cpp"
#include "SFML/System/Win32/ThreadImpl.cpp"
#include "SFML/System/Win32/MutexImpl.cpp"
#else
#include "SFML/System/Unix/SleepImpl.cpp"
#include "SFML/System/Win32/ThreadImpl.cpp"
#include "SFML/System/Win32/MutexImpl.cpp"
#endif

using namespace sf;