#include <windows.h>
#include <sapi.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <initguid.h>
#include "voice.h"

// SAPI requires COM initialization
static ISpVoice *g_pVoice = NULL;
static ISpRecognizer *g_pRecognizer = NULL;
static ISpRecoContext *g_pRecoContext = NULL;
static ISpRecoGrammar *g_pGrammar = NULL;
static bool g_comInitialized = false;
static bool g_recognitionInitialized = false;
static char g_lastError[256] = {0};

bool voiceInit(void) {
    if (g_pVoice) return true; // Already initialized
    
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        return false;
    }
    g_comInitialized = true;
    
    hr = CoCreateInstance(&CLSID_SpVoice, NULL, CLSCTX_ALL, &IID_ISpVoice, (void **)&g_pVoice);
    if (FAILED(hr)) {
        if (g_comInitialized) {
            CoUninitialize();
            g_comInitialized = false;
        }
        return false;
    }
    
    return true;
}

void voiceCleanup(void) {
    if (g_pVoice) {
        g_pVoice->lpVtbl->Release(g_pVoice);
        g_pVoice = NULL;
    }
    if (g_comInitialized) {
        CoUninitialize();
        g_comInitialized = false;
    }
}

bool voiceSpeak(const char *text) {
    if (!g_pVoice || !text) return false;
    
    wchar_t wtext[2048];
    int wlen = MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, 2048);
    if (!wlen) {
        wlen = MultiByteToWideChar(CP_ACP, 0, text, -1, wtext, 2048);
        if (!wlen) return false;
    }
    
    return voiceSpeakW(wtext);
}

bool voiceSpeakW(const wchar_t *text) {
    if (!g_pVoice || !text) return false;
    
    HRESULT hr = g_pVoice->lpVtbl->Speak(g_pVoice, text, SPF_ASYNC | SPF_IS_NOT_XML, NULL);
    return SUCCEEDED(hr);
}

bool voiceIsAvailable(void) {
    return g_pVoice != NULL;
}

// Voice Recognition Functions
bool voiceRecognitionInit(void) {
    g_lastError[0] = '\0';
    
    if (g_recognitionInitialized) return true;
    
    if (!g_comInitialized) {
        if (!voiceInit()) {
            strncpy_s(g_lastError, sizeof(g_lastError), "Failed to initialize COM", _TRUNCATE);
            return false;
        }
    }
    
    // Create shared recognizer
    // Note: SpCreateRecognizer is in sphelper.h which requires ATL
    // Instead, we'll use CoCreateInstance with CLSID_SpSharedRecognizer
    HRESULT hr = CoCreateInstance(&CLSID_SpSharedRecognizer, NULL, CLSCTX_ALL, &IID_ISpRecognizer, (void **)&g_pRecognizer);
    if (FAILED(hr)) {
        snprintf(g_lastError, sizeof(g_lastError), "Failed to create recognizer (HRESULT: 0x%08X). Make sure Windows Speech Recognition is enabled.", hr);
        return false;
    }
    
    // Create recognition context
    hr = g_pRecognizer->lpVtbl->CreateRecoContext(g_pRecognizer, &g_pRecoContext);
    if (FAILED(hr)) {
        g_pRecognizer->lpVtbl->Release(g_pRecognizer);
        g_pRecognizer = NULL;
        snprintf(g_lastError, sizeof(g_lastError), "Failed to create recognition context (HRESULT: 0x%08X)", hr);
        return false;
    }
    
    // Set input to default audio device (NULL means use default)
    // The recognizer will use the default audio input device
    hr = g_pRecognizer->lpVtbl->SetInput(g_pRecognizer, NULL, TRUE);
    if (FAILED(hr)) {
        snprintf(g_lastError, sizeof(g_lastError), "Failed to set audio input (HRESULT: 0x%08X). Check microphone permissions.", hr);
        g_pRecoContext->lpVtbl->Release(g_pRecoContext);
        g_pRecoContext = NULL;
        g_pRecognizer->lpVtbl->Release(g_pRecognizer);
        g_pRecognizer = NULL;
        return false;
    }
    
    // Set interest in recognition events
    ULONGLONG ullInterest = SPFEI(SPEI_RECOGNITION);
    hr = g_pRecoContext->lpVtbl->SetInterest(g_pRecoContext, ullInterest, ullInterest);
    if (FAILED(hr)) {
        snprintf(g_lastError, sizeof(g_lastError), "Failed to set recognition interest (HRESULT: 0x%08X)", hr);
        g_pRecoContext->lpVtbl->Release(g_pRecoContext);
        g_pRecoContext = NULL;
        g_pRecognizer->lpVtbl->Release(g_pRecognizer);
        g_pRecognizer = NULL;
        return false;
    }
    
    g_recognitionInitialized = true;
    return true;
}

void voiceRecognitionCleanup(void) {
    if (g_pGrammar) {
        g_pGrammar->lpVtbl->Release(g_pGrammar);
        g_pGrammar = NULL;
    }
    if (g_pRecoContext) {
        g_pRecoContext->lpVtbl->Release(g_pRecoContext);
        g_pRecoContext = NULL;
    }
    if (g_pRecognizer) {
        g_pRecognizer->lpVtbl->Release(g_pRecognizer);
        g_pRecognizer = NULL;
    }
    g_recognitionInitialized = false;
}

bool voiceRecognitionIsAvailable(void) {
    return g_recognitionInitialized && g_pRecognizer != NULL;
}

bool voiceRecognitionListen(char *buffer, size_t bufferSize, int timeoutSeconds) {
    g_lastError[0] = '\0';
    
    if (!g_recognitionInitialized || !g_pRecoContext) {
        if (!voiceRecognitionInit()) {
            return false;
        }
    }
    
    // Create a dictation grammar
    if (!g_pGrammar) {
        ULONGLONG ullGrammarId = 0;
        HRESULT hr = g_pRecoContext->lpVtbl->CreateGrammar(g_pRecoContext, ullGrammarId, &g_pGrammar);
        if (FAILED(hr)) {
            snprintf(g_lastError, sizeof(g_lastError), "Failed to create grammar (HRESULT: 0x%08X)", hr);
            return false;
        }
        
        // Load dictation grammar
        hr = g_pGrammar->lpVtbl->LoadDictation(g_pGrammar, NULL, SPLO_STATIC);
        if (FAILED(hr)) {
            snprintf(g_lastError, sizeof(g_lastError), "Failed to load dictation grammar (HRESULT: 0x%08X). Windows Speech Recognition may not be set up.", hr);
            g_pGrammar->lpVtbl->Release(g_pGrammar);
            g_pGrammar = NULL;
            return false;
        }
    }
    
    // Set grammar state to active
    HRESULT hr = g_pGrammar->lpVtbl->SetDictationState(g_pGrammar, SPRS_ACTIVE);
    if (FAILED(hr)) {
        snprintf(g_lastError, sizeof(g_lastError), "Failed to activate dictation (HRESULT: 0x%08X)", hr);
        return false;
    }
    
    // Wait for recognition result
    SPEVENT event;
    ULONG fetched = 0;
    HANDLE hEvent = NULL;
    
    // GetNotifyEventHandle returns HANDLE directly
    hEvent = g_pRecoContext->lpVtbl->GetNotifyEventHandle(g_pRecoContext);
    if (hEvent == NULL || hEvent == INVALID_HANDLE_VALUE) {
        g_pGrammar->lpVtbl->SetDictationState(g_pGrammar, SPRS_INACTIVE);
        strncpy_s(g_lastError, sizeof(g_lastError), "Failed to get notification event handle", _TRUNCATE);
        return false;
    }
    
    // Wait for event with timeout
    DWORD waitResult = WaitForSingleObject(hEvent, timeoutSeconds * 1000);
    if (waitResult != WAIT_OBJECT_0) {
        g_pGrammar->lpVtbl->SetDictationState(g_pGrammar, SPRS_INACTIVE);
        if (waitResult == WAIT_TIMEOUT) {
            strncpy_s(g_lastError, sizeof(g_lastError), "Timeout: No voice input detected. Make sure microphone is working and speak clearly.", _TRUNCATE);
        } else {
            snprintf(g_lastError, sizeof(g_lastError), "Wait failed (error: %lu)", GetLastError());
        }
        return false; // Timeout
    }
    
    // Get recognition result
    ULONG ulCount = 1;
    while (g_pRecoContext->lpVtbl->GetEvents(g_pRecoContext, ulCount, &event, &fetched) == S_OK && fetched > 0) {
        if (event.eEventId == SPEI_RECOGNITION) {
            ISpRecoResult *pResult = (ISpRecoResult *)event.lParam;
            if (pResult) {
                WCHAR *pwszText = NULL;
                hr = pResult->lpVtbl->GetText(pResult, SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &pwszText, NULL);
                if (SUCCEEDED(hr) && pwszText) {
                    // Convert wide char to UTF-8
                    int len = WideCharToMultiByte(CP_UTF8, 0, pwszText, -1, buffer, (int)bufferSize, NULL, NULL);
                    CoTaskMemFree(pwszText);
                    pResult->lpVtbl->Release(pResult);
                    g_pGrammar->lpVtbl->SetDictationState(g_pGrammar, SPRS_INACTIVE);
                    return len > 0;
                }
                pResult->lpVtbl->Release(pResult);
            }
        }
    }
    
    g_pGrammar->lpVtbl->SetDictationState(g_pGrammar, SPRS_INACTIVE);
    strncpy_s(g_lastError, sizeof(g_lastError), "No recognition result received", _TRUNCATE);
    return false;
}

const char* voiceRecognitionGetLastError(void) {
    return g_lastError[0] != '\0' ? g_lastError : NULL;
}

bool voiceRecognitionTestMicrophone(void) {
    g_lastError[0] = '\0';
    
    // Test if we can initialize recognition
    if (!voiceRecognitionInit()) {
        return false;
    }
    
    // Check if recognizer is available
    if (!g_pRecognizer) {
        strncpy_s(g_lastError, sizeof(g_lastError), "Recognizer not available", _TRUNCATE);
        return false;
    }
    
    // Try to get recognition status
    SPRECOSTATE state;
    HRESULT hr = g_pRecognizer->lpVtbl->GetRecoState(g_pRecognizer, &state);
    if (FAILED(hr)) {
        snprintf(g_lastError, sizeof(g_lastError), "Failed to get recognition state (HRESULT: 0x%08X)", hr);
        return false;
    }
    
    return true;
}

bool voiceRecognitionStartListening(void) {
    if (!voiceRecognitionInit()) return false;
    if (g_pGrammar) {
        g_pGrammar->lpVtbl->SetDictationState(g_pGrammar, SPRS_ACTIVE);
        return true;
    }
    return false;
}

bool voiceRecognitionStopListening(void) {
    if (g_pGrammar) {
        g_pGrammar->lpVtbl->SetDictationState(g_pGrammar, SPRS_INACTIVE);
        return true;
    }
    return false;
}

