#pragma once
#include <stdbool.h>
#include <windows.h>

// Initialize voice/TTS system
bool voiceInit(void);

// Cleanup voice/TTS system
void voiceCleanup(void);

// Speak text (synchronous)
bool voiceSpeak(const char *text);

// Speak wide text (synchronous)
bool voiceSpeakW(const wchar_t *text);

// Check if voice is available
bool voiceIsAvailable(void);

// Voice Recognition (Speech-to-Text)
bool voiceRecognitionInit(void);
void voiceRecognitionCleanup(void);
bool voiceRecognitionIsAvailable(void);
bool voiceRecognitionListen(char *buffer, size_t bufferSize, int timeoutSeconds);
bool voiceRecognitionStartListening(void);
bool voiceRecognitionStopListening(void);
const char* voiceRecognitionGetLastError(void);
bool voiceRecognitionTestMicrophone(void);

