#include "pch.h"

#define STB_IMAGE_IMPLEMENTATION

#include "GameLibrary.h"
#include "GameBuild.h"
// #include "GameShader.h"

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "xxhash.h"

// #if GAME_RENDER_API_DIRECTX
//     #include "DirectX11Render.h"
// #elif GAME_RENDER_API_OPENGL
//     #include "OpenGLRender.h"
// #elif GAME_RENDER_API_VULKAN
//     #include "VulkanRender.h"
// #endif

#pragma comment(lib, "xaudio2.lib")

/* TODO:
    - Saved game locations
    - Getting a handle to our own executable file
    - Multithreading
    - Multiple keyboards?
    - Sleep/timeBeginPeriod
    - ClipCursor() multimonitor support
    - WM_SETCURSOR
    - QueryCancelAutoplay
    - WM_ACTIVEAPP (when we are not the active application)
    - GetKeyboardLayout (international wasd support)
    - Render using all window space. Remove rounded borders
*/

#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE hInst;                                // current instance
char szTitle[MAX_LOADSTRING];                  // The title bar text
char szWindowClass[MAX_LOADSTRING];            // the main window class name

Memory memory;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// Monitors
struct Monitor_Manager {
    Monitor_Info MonitorInfo[MAX_SUPPORTED_MONITORS];
    uint8 nMonitors;
};
static Monitor_Manager MonitorManager = {};

BOOL AddMonitorInfo(HMONITOR hMonitor, Monitor_Manager* Manager) {
    if (0 <= Manager->nMonitors && Manager->nMonitors < MAX_SUPPORTED_MONITORS) {
        MONITORINFOEX Info = {};
        Info.cbSize = sizeof(MONITORINFOEX);

        if (!GetMonitorInfo(hMonitor, &Info)) {
            // Continue enumeration
            return TRUE;
        }

        Monitor_Info Monitor = {};
        strcpy_s(Monitor.device_name, Info.szDevice);
        Monitor.monitor_rect = {
            (float)Info.rcMonitor.left,
            (float)Info.rcMonitor.top,
            (float)(Info.rcMonitor.right - Info.rcMonitor.left),
            (float)(Info.rcMonitor.bottom - Info.rcMonitor.top)
        };
        Monitor.is_primary = (Info.dwFlags & MONITORINFOF_PRIMARY) != 0;
        Monitor.id = Manager->nMonitors;

        Manager->MonitorInfo[Manager->nMonitors++] = Monitor;
        return TRUE;
    }
    return FALSE;
}

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    return AddMonitorInfo(hMonitor, (Monitor_Manager*)dwData);
}

void RefreshMonitors() {
    for (int i = 0; i < MonitorManager.nMonitors; i++) {
        MonitorManager.MonitorInfo[i] = {};
    }
    MonitorManager.nMonitors = 0;

    // EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&MonitorManager);

    UINT32 numPathArrayElements = 0;
    UINT32 numModeInfoArrayElements = 0;
    LONG ReturnCode = GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &numPathArrayElements, &numModeInfoArrayElements);

    std::vector<DISPLAYCONFIG_PATH_INFO> PathArray(numPathArrayElements);
    std::vector<DISPLAYCONFIG_MODE_INFO> ModeArray(numModeInfoArrayElements);

    ReturnCode = QueryDisplayConfig(
        QDC_ONLY_ACTIVE_PATHS, 
        &numPathArrayElements, PathArray.data(), 
        &numModeInfoArrayElements, ModeArray.data(), 
        NULL
    );

    std::vector<std::wstring> DeviceNames(numPathArrayElements);
    std::vector<std::wstring> DisplayNames(numPathArrayElements);

    for (int i = 0; i < numPathArrayElements; i++) {
        DISPLAYCONFIG_PATH_INFO PathInfo = PathArray[i];

        DISPLAYCONFIG_SOURCE_DEVICE_NAME SourceName = {};
        SourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
        SourceName.header.size = sizeof(SourceName);
        SourceName.header.adapterId = PathInfo.sourceInfo.adapterId;
        SourceName.header.id = PathInfo.sourceInfo.id;
        ReturnCode = DisplayConfigGetDeviceInfo(&SourceName.header);
        if (ReturnCode == ERROR_SUCCESS) {
            DeviceNames[i] = SourceName.viewGdiDeviceName;
        }

        DISPLAYCONFIG_TARGET_DEVICE_NAME TargetName = {};
        TargetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        TargetName.header.size = sizeof(TargetName);
        TargetName.header.adapterId = PathInfo.targetInfo.adapterId;
        TargetName.header.id = PathInfo.targetInfo.id;
        
        if (DisplayConfigGetDeviceInfo(&TargetName.header) == ERROR_SUCCESS) {
            DisplayNames[i] = TargetName.monitorFriendlyDeviceName;
        }
    }

    DISPLAY_DEVICE DisplayDevice = {};
    DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
    for (DWORD i = 0; EnumDisplayDevices(NULL, i, &DisplayDevice, 0); i++) {
        if (DisplayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE) {
            DEVMODE dm = {};
            dm.dmSize = sizeof(DEVMODE);

            if (EnumDisplaySettings(DisplayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &dm)) {
                POINT pt = { dm.dmPosition.x + 1, dm.dmPosition.y + 1 };
                HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);

                if (hMonitor) {
                    MONITORINFOEX MonitorInfo = {};
                    MonitorInfo.cbSize = sizeof(MONITORINFOEX);
                    if (GetMonitorInfo(hMonitor, &MonitorInfo)) {
                        for (int j = 0; j < numPathArrayElements; j++) {
                            std::wstring DeviceName = DeviceNames[j];
                            std::string MonitorDeviceName = MonitorInfo.szDevice;

                            int SizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &MonitorDeviceName[0], (int)MonitorDeviceName.size(), NULL, 0);
                            std::wstring WideMonitorDeviceName(SizeNeeded, 0);
                            MultiByteToWideChar(CP_UTF8, 0, &MonitorDeviceName[0], (int)MonitorDeviceName.size(), &WideMonitorDeviceName[0], SizeNeeded);

                            if (DeviceName.compare(&WideMonitorDeviceName[0]) == 0) {
                                Monitor_Info ResultInfo = {};
                                ResultInfo.id = MonitorManager.nMonitors;
                                ResultInfo.monitor_rect = { 
                                    (float)MonitorInfo.rcMonitor.left, 
                                    (float)MonitorInfo.rcMonitor.top, 
                                    (float)(MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left), 
                                    (float)(MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top)
                                };
                                ResultInfo.work_area = { 
                                    (float)MonitorInfo.rcWork.left, 
                                    (float)MonitorInfo.rcWork.top, 
                                    (float)(MonitorInfo.rcWork.right - MonitorInfo.rcWork.left), 
                                    (float)(MonitorInfo.rcWork.bottom - MonitorInfo.rcWork.top)
                                };
                                strcpy_s(ResultInfo.device_name, MonitorInfo.szDevice);
                                std::wstring DisplayName = DisplayNames[j];
                                int Result = WideCharToMultiByte(
                                    CP_UTF8, 0, &DisplayName[0], (int)DisplayName.size(), 
                                    ResultInfo.display_name, 
                                    sizeof(MonitorManager.MonitorInfo[i].display_name),
                                    NULL, NULL
                                );
                                MonitorManager.MonitorInfo[MonitorManager.nMonitors++] = ResultInfo;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

WINDOWPLACEMENT WindowPosition = { sizeof(WindowPosition) };

// Full screen
VOID ToggleFullScreen(HWND Window) {
    DWORD Style = GetWindowLong(Window, GWL_STYLE);

    if (Style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
        if (GetWindowPlacement(Window, &WindowPosition) &&
            GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &WindowPosition);
        SetWindowPos(Window, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }

}

// Sound
static IXAudio2* pXAudio2 = 0;
static IXAudio2MasteringVoice* pMasterVoice = 0;
static IXAudio2SourceVoice* pSourceVoice = 0;

class voice_callback : public IXAudio2VoiceCallback
{
public:
    HANDLE hBufferEndEvent;
    voice_callback() : hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
    ~voice_callback() { CloseHandle(hBufferEndEvent); }

    //Called when the voice has just finished playing a contiguous audio stream.
    void __stdcall OnStreamEnd() { SetEvent(hBufferEndEvent); }

    //Unused methods are stubs
    void __stdcall OnVoiceProcessingPassEnd() {}
    void __stdcall OnVoiceProcessingPassStart(uint32 SamplesRequired) {}
    void __stdcall OnBufferEnd(void* pBufferContext) {}
    void __stdcall OnBufferStart(void* pBufferContext) {}
    void __stdcall OnLoopEnd(void* pBufferContext) {}
    void __stdcall OnVoiceError(void* pBufferContext, HRESULT Error) {}
};

voice_callback VoiceCallback;

static void InitXAudio2(int nBuffers,
    int BufferSize,
    XAUDIO2_BUFFER* Buffers,
    WAVEFORMATEX* pWaveFormat)
{
    HRESULT hr;
    hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (FAILED(hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
        Log(Log_Level::Error, "ERROR creating XAudio2.");
    }
    else if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice))) {
        Log(Log_Level::Error, "ERROR creating mastering voice.");
    }
    else {
        if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, pWaveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &VoiceCallback))) {
            Log(Log_Level::Error, "ERROR creating source voice.");
        }
        else {
            uint32 AudioBytes = BufferSize * pWaveFormat->nChannels * (pWaveFormat->wBitsPerSample / 8);
            for (int i = 0; i < nBuffers; i++) {
                Buffers[i].AudioBytes = AudioBytes;
                Buffers[i].pAudioData = (BYTE*)Win32AllocateMemory(AudioBytes);
                Buffers[i].PlayBegin = 0;
                Buffers[i].PlayLength = BufferSize;
                Buffers[i].Flags = 0;
                Buffers[i].LoopBegin = 0;
                Buffers[i].LoopLength = 0;
                Buffers[i].LoopCount = 0;
                Buffers[i].pContext = 0;

                //Buffers[i].LoopBegin = 0;
                //Buffers[i].LoopLength = (SamplesPerSecond / 1000) * BufferSizeMilliseconds;
                //Buffers[i].LoopCount = XAUDIO2_LOOP_INFINITE;
            }
        }
    }
}

static void SetWaveFormat(DWORD SamplesPerSecond, WAVEFORMATEX* WaveFormat) {
    WaveFormat->wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat->nChannels = 2;
    WaveFormat->nSamplesPerSec = SamplesPerSecond;
    WaveFormat->wBitsPerSample = 16;
    WaveFormat->nBlockAlign = WaveFormat->nChannels * WaveFormat->wBitsPerSample / 8;
    WaveFormat->nAvgBytesPerSec = SamplesPerSecond * WaveFormat->nBlockAlign;
    WaveFormat->cbSize = 0;
}

static HRESULT SubmitBuffer(XAUDIO2_BUFFER* pBuffer, IXAudio2SourceVoice* pSourceVoice) {
    HRESULT hr;
    XAUDIO2_VOICE_STATE state;
    if (pSourceVoice) {
        //while (pSourceVoice->GetState(&state), state.BuffersQueued > 1) {
        //    WaitForSingleObjectEx(VoiceCallback.hBufferEndEvent, INFINITE, TRUE);
        //}
        hr = pSourceVoice->SubmitSourceBuffer(pBuffer);
        if (FAILED(hr)) {
            OutputDebugStringA("Sound buffer submit went wrong\n");
            return hr;
        }
        else {
            return(0);
        }
    }
    else {
        OutputDebugStringA("No source voice\n");
        return(1);
    }
}

// Recording and playback
void BeginRecordingInput(Record_And_Playback* record_playback, int record_index) {
    record_playback->record_index = record_index;
    LPCSTR filename = "foo.jgzr";
    record_playback->record_file = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    DWORD bytes_written;
    WriteFile(record_playback->record_file, record_playback->game_memory_block, record_playback->total_size, &bytes_written, 0);
}

void EndRecordingInput(Record_And_Playback* record_playback) {
    record_playback->record_index = 0;
    CloseHandle(record_playback->record_file);
}

void RecordInput(Record_And_Playback* record_playback, Input* input) {
    DWORD bytes_written;
    WriteFile(record_playback->record_file, input, sizeof(Input), &bytes_written, 0);
}

void BeginInputPlayback(Record_And_Playback* record_playback, int playback_index) {
    record_playback->playback_index = playback_index;
    LPCSTR filename = "foo.jgzr";
    record_playback->playback_file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    DWORD bytes_to_read = record_playback->total_size;
    DWORD bytes_read;
    if (ReadFile(record_playback->playback_file, record_playback->game_memory_block, bytes_to_read, &bytes_read, 0)) {

    }
    else {
        Log(Log_Level::Error, "Reading game state failed.");
    }
}

void EndInputPlayback(Record_And_Playback* record_playback) {
    record_playback->playback_index = 0;
    CloseHandle(record_playback->playback_file);
}

void PlaybackInput(Record_And_Playback* record_playback, Input* input) {
    DWORD bytes_read = 0;
    if (ReadFile(record_playback->playback_file, input, sizeof(Input), &bytes_read, 0)) {
        if (bytes_read == 0) {
            int Index = record_playback->playback_index;
            EndInputPlayback(record_playback);
            BeginInputPlayback(record_playback, Index);
        }
    }
}

// Message processing
void ProcessPendingMessages(HWND window, Input* input, Record_And_Playback* record_playback) {
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        switch (msg.message) {
            // Mouse
            case WM_LBUTTONDOWN: { PressButton(&input->mouse.left_click); } break;
            case WM_LBUTTONUP:   { LiftButton(&input->mouse.left_click); } break;
            case WM_RBUTTONDOWN: { PressButton(&input->mouse.right_click); } break;
            case WM_RBUTTONUP:   { LiftButton(&input->mouse.right_click); } break;
            case WM_MBUTTONDOWN: { PressButton(&input->mouse.middle_click); } break;
            case WM_MBUTTONUP:   { LiftButton(&input->mouse.middle_click); } break;
            case WM_MOUSEWHEEL:  { input->mouse.wheel = GET_WHEEL_DELTA_WPARAM(msg.wParam); } break;
            
            // Keyboard
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN: {
                uint32 vk_code = msg.wParam;
                bool was_down = (msg.lParam & (1 << 30)) != 0;

                if (!was_down) {
                    if (vk_code >= '0' && vk_code <= '9' || vk_code >= 'A' && vk_code <= 'Z') {
                        PressKey(input, vk_code);
                    }
                    
                    if (vk_code == 'P') {
                        memory.pause = !memory.pause;
                    }
                    
                    if (vk_code == VK_UP)           PressButton(&input->keyboard.up);
                    else if (vk_code == VK_DOWN)    PressButton(&input->keyboard.down);
                    else if (vk_code == VK_LEFT)    PressButton(&input->keyboard.left);
                    else if (vk_code == VK_RIGHT)   PressButton(&input->keyboard.right);
                    else if (vk_code == VK_ESCAPE)  PressButton(&input->keyboard.escape);
                    else if (vk_code == VK_SPACE)   PressButton(&input->keyboard.space);
                    else if (vk_code == VK_RETURN)  PressButton(&input->keyboard.enter);
                    else if (vk_code == VK_CONTROL) PressButton(&input->keyboard.control);
                    else if (vk_code == VK_MENU)    PressButton(&input->keyboard.alt);
                    else if (vk_code == VK_F1)      PressButton(&input->keyboard.F1);
                    else if (vk_code == VK_F2)      PressButton(&input->keyboard.F2);
                    else if (vk_code == VK_F3)      PressButton(&input->keyboard.F3);
                    else if (vk_code == VK_F4)      PressButton(&input->keyboard.F4);
                    else if (vk_code == VK_F5)      PressButton(&input->keyboard.F5);
                    else if (vk_code == VK_F6)      PressButton(&input->keyboard.F6);
                    else if (vk_code == VK_F7)      PressButton(&input->keyboard.F7);
                    else if (vk_code == VK_F8)      PressButton(&input->keyboard.F8);
                    else if (vk_code == VK_F9)      PressButton(&input->keyboard.F9);
                    else if (vk_code == VK_F10)     PressButton(&input->keyboard.F10);
                    else if (vk_code == VK_F11) {
                        PressButton(&input->keyboard.F11);
                        ToggleFullScreen(window);
                    }
                    else if (vk_code == VK_F12)   PressButton(&input->keyboard.F12);
                    else if (vk_code == VK_PRIOR) PressButton(&input->keyboard.page_up);
                    else if (vk_code == VK_NEXT)  PressButton(&input->keyboard.page_down);
                    else if (vk_code == VK_SHIFT) PressButton(&input->keyboard.shift);
                    else if (vk_code == 'L') {
                        if (record_playback->record_index == 0) {
                            BeginRecordingInput(record_playback, 1);
                        }
                        else {
                            EndRecordingInput(record_playback);
                            BeginInputPlayback(record_playback, 1);
                        }
                    }
                }

                // Shortcut for closing Alt+F4
                bool alt_key_was_down = (msg.lParam & ((uint32)1 << 29)) != 0;
                if ((vk_code == VK_F4) && alt_key_was_down) {
                    memory.running = false;
                    PostQuitMessage(0);
                }
            } break;
            case WM_KEYUP:
            case WM_SYSKEYUP: {
                uint32 vk_code = msg.wParam;
                if (vk_code >= '0' && vk_code <= '9' || vk_code >= 'A' && vk_code <= 'Z') {
                    LiftKey(input, vk_code);
                }

                if (vk_code == VK_UP)           LiftButton(&input->keyboard.up);
                else if (vk_code == VK_DOWN)    LiftButton(&input->keyboard.down);
                else if (vk_code == VK_LEFT)    LiftButton(&input->keyboard.left);
                else if (vk_code == VK_RIGHT)   LiftButton(&input->keyboard.right);
                else if (vk_code == VK_ESCAPE)  LiftButton(&input->keyboard.escape);
                else if (vk_code == VK_SPACE)   LiftButton(&input->keyboard.space);
                else if (vk_code == VK_RETURN)  LiftButton(&input->keyboard.enter);
                else if (vk_code == VK_CONTROL) LiftButton(&input->keyboard.control);
                else if (vk_code == VK_MENU)    LiftButton(&input->keyboard.alt);
                else if (vk_code == VK_F1)      LiftButton(&input->keyboard.F1);
                else if (vk_code == VK_F2)      LiftButton(&input->keyboard.F2);
                else if (vk_code == VK_F3)      LiftButton(&input->keyboard.F3);
                else if (vk_code == VK_F4)      LiftButton(&input->keyboard.F4);
                else if (vk_code == VK_F5)      LiftButton(&input->keyboard.F5);
                else if (vk_code == VK_F6)      LiftButton(&input->keyboard.F6);
                else if (vk_code == VK_F7)      LiftButton(&input->keyboard.F7);
                else if (vk_code == VK_F8)      LiftButton(&input->keyboard.F8);
                else if (vk_code == VK_F9)      LiftButton(&input->keyboard.F9);
                else if (vk_code == VK_F10)     LiftButton(&input->keyboard.F10);
                else if (vk_code == VK_F11)     LiftButton(&input->keyboard.F11);
                else if (vk_code == VK_F12)     LiftButton(&input->keyboard.F12);
                else if (vk_code == VK_PRIOR)   LiftButton(&input->keyboard.page_up);
                else if (vk_code == VK_NEXT)    LiftButton(&input->keyboard.page_down);
                else if (vk_code == VK_SHIFT)   LiftButton(&input->keyboard.shift);
            } break;
            case WM_CLOSE:
            case WM_DESTROY:
            {
                memory.running = false;
                PostQuitMessage(0);
            } break;
            default: {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } break;
        }
    }
}

// Dynamic library loading
void GameUpdateStub(GAME_UPDATE_INPUTS) {}

struct Game_Code {
    bool is_valid;
    int64 dll_last_write_time;
    HMODULE game_code_dll;
    Game_Update* update;
};

void LoadGameCode(Game_Code* result, LPCSTR source_dll_name, LPCSTR temp_dll_name) {
    result->update = GameUpdateStub;

    char error_text[256];
    DWORD last_error = 0;

    bool copy_result = Win32FileCopy(source_dll_name, temp_dll_name);
    if (!copy_result) {
        last_error = GetLastError();
        if (last_error == ERROR_SHARING_VIOLATION) {
            int retries = 0;
            do {
                Log(Log_Level::Warn, "Retrying game code loading after sharing violation.");
                Sleep(100);
                copy_result = Win32FileCopy(source_dll_name, temp_dll_name);
                retries++;
                if (retries > 100) {
                    Log(Log_Level::Error, "Max number of retries reached.");
                    break;
                }
            } while (!copy_result);
        }
        else {
            sprintf_s(error_text, "Error copying file %s into %s. Error code %d.", source_dll_name, temp_dll_name, last_error);
            Log(Log_Level::Error, error_text);
            return;
        }
    }

    result->game_code_dll = LoadLibraryA(temp_dll_name);
    if (result->game_code_dll) {
        result->update = (Game_Update*)GetProcAddress(result->game_code_dll, "GameUpdate");
        result->is_valid = (result->update);

        result->dll_last_write_time = Win32GetFileInfo(source_dll_name).timestamp;
    }
    else {
        last_error = GetLastError();
        sprintf_s(error_text, "Error loading game code. Error code %d.", last_error);
        Log(Log_Level::Error, error_text); // If error is 126 (dependency error while loading DLL) try using Process Monitor.
    }
}

void UnloadGameCode(Game_Code* code) {
    if (code->game_code_dll) {
        bool UnloadResult = FreeLibrary(code->game_code_dll);
    }

    code->is_valid = false;
    code->update = GameUpdateStub;
}

// Main window callback
int APIENTRY wWinMain(
    _In_ HINSTANCE     hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR        lpCmdLine,
    _In_ int           nCmdShow
) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Loading XInputLibrary
    LoadXInput();

    // Audio initialization
    // int SamplesPerSecond = 48000;
    // const int nBuffers = 5;                         // Number of audio buffers to rotate
    // int BufferSize = SamplesPerSecond / 60;         // Buffer size in samples
    // WAVEFORMATEX WaveFormat;
    // SetWaveFormat(SamplesPerSecond, &WaveFormat);

    // XAUDIO2_BUFFER XAudio2Buffers[nBuffers];
    // game_sound_buffer Buffers[nBuffers];
    // InitXAudio2(nBuffers, BufferSize, XAudio2Buffers, &WaveFormat);

    // game_sound_buffer GameSoundBuffers[nBuffers];
    // for (int i = 0; i < nBuffers; i++) {
    //     GameSoundBuffers[i].SamplesPerSecond = SamplesPerSecond;
    //     GameSoundBuffers[i].BufferSize = BufferSize;
    //     GameSoundBuffers[i].SampleOut = (int16*)XAudio2Buffers[i].pAudioData;
    // }

    // Performance counting initialization
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    Platform.performance_counter_frequency = PerfCountFrequencyResult.QuadPart;

    // Initialize global strings
    LoadStringA(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    DWORD WinError = GetLastError();

    LoadStringA(hInstance, IDC_WIN32PLATFORMLAYER, szWindowClass, MAX_LOADSTRING);
    WinError = GetLastError();

    MyRegisterClass(hInstance);

    // Dummy window for OpenGL context creation
#if GAME_RENDER_API_OPENGL
    HWND DummyWindow = CreateWindowA(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0,
        100, 100, nullptr, nullptr, hInstance, nullptr);

    GetWGLFunctions(DummyWindow);
    DestroyWindow(DummyWindow);
#endif

    // Perform application initialization:
    HWND window;
    if (!InitInstance(hInstance, nCmdShow, &window)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PLATFORMLAYER));

    // Console for logging
    AllocConsole();
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD ConsoleMode = 0;
    GetConsoleMode(hConsole, &ConsoleMode);
    SetConsoleMode(hConsole, ConsoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    // Set up for main loop
    const LPCSTR source_dll_name = "bin\\GameLibrary.dll";
    const LPCSTR temp_dll_name = "bin\\GameLibraryTemp.dll";

    Game_Code code = { 0 };
    LoadGameCode(&code, source_dll_name, temp_dll_name);
    memory_size permanent_storage_size = Megabytes(128);
    void* game_memory_block = Win32AllocateMemory(permanent_storage_size);
    memory.permanent = MemoryArena(permanent_storage_size, (uint8*)game_memory_block);
    time_records = (Time_Record*)&memory.time_records_platform;
    memory.hot_reload = true;

    // Game_State* pGameState = PushStruct(&Memory.Permanent, game_state);
    // Memory.GameState = pGameState;

    // Memory arenas
    memory.transient = SuballocateMemoryArena(memory.permanent, Megabytes(64));

    // Assets
    //Memory.AssetManager = InitializeAssetManager(&Memory.Permanent, &Memory.Transient);

    // Recording and playback
    Record_And_Playback record_playback;
    record_playback.playback_index = 0;
    record_playback.record_index = 0;
    record_playback.game_memory_block = game_memory_block;
    record_playback.total_size = permanent_storage_size;
    
    Render_Group* group = &memory.render_group;
    RECT rect = { 0 };
    GetClientRect(window, &rect);
    InitializeRenderGroup(
        group,
        &memory.permanent,
        &memory.asset_manager,
        rect.right - rect.left,
        rect.bottom - rect.top
    );

    // Input
    Input* input = &memory.input;
    input->mode = Keyboard;

    // Sound
    int currentBuffer = 1;

    // Enforcing framerate
    int monitor_refresh_hz = 60;
    float target_seconds_per_frame = 1.0f / (float)monitor_refresh_hz;

    // Performance
    uint64 last_counter = Win32GetWallClock();

    HDC device_context = GetDC(window);

    // Initilize render API
    // InitializeRenderer(
    //     &memory.permanent,
    //     group,
    //     window,
    //     hInstance,
    //     device_context
    // );

    RefreshMonitors();

    ReleaseDC(window, device_context);

    // Delete old hot reloading files
    WIN32_FIND_DATAA FindData = {};
    HANDLE OldPDBFile = FindFirstFileA("bin\\GameLibrary*.pdb", &FindData);
    bool FindResult = true;
    while (FindResult && OldPDBFile != INVALID_HANDLE_VALUE) {
        if (
            strcmp(FindData.cFileName, "GameLibrary.pdb") != 0 && 
           !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        ) {
            char Buffer[64] = {};
            sprintf_s(Buffer, "bin\\%s", FindData.cFileName);
            bool Result = Win32FileDelete(Buffer);
            if (Result) {
                sprintf_s(Buffer, "Deleted old PDB file bin\\%s.", FindData.cFileName);
                Log(Log_Level::Info, Buffer);
            }
        }
        FindResult = FindNextFileA(OldPDBFile, &FindData);
    }

    // Code compilation setup
    Build_Configuration build_config = ReadBuildConfiguration(memory.permanent, "GameBuild\\build.conf");
    Process_Info LibraryCompilation = {};
    uint64 LibraryCompilationStart = 0;
    char MetaFile[] = "bin\\Meta.exe";
    Process_Info MetaprogrammingCompilation = {};
    uint64 MetaprogrammingCompilationStart = 0;
    Process_Info MetaprogrammingExecution = {};
    uint64 MetaprogrammingExecutionStart = 0;
    char LogBuffer[64] = {};

    // Slang setup
    // InitializeSlang(&Memory.Permanent, BuildConfig.Renderer);

    int nArgs = 0;
    LPWSTR* Args = CommandLineToArgvW(lpCmdLine, &nArgs);
    if (nArgs == 1 && wcscmp(Args[0], L"-test") == 0) {
        memory.test = true;
    }
    else if (nArgs > 0) {
        for (int i = 0; i < nArgs; i++) {
            OutputDebugStringW(L"Invalid argument: ");
            OutputDebugStringW(Args[i]);
            OutputDebugStringW(L"\n");
        }
    }
    LocalFree(Args);

    memory.running = true;
    bool FirstFrame = true;

    // Main loop
    Log(Log_Level::Info, "Running.");

    while (memory.running) {
        // Hot reloading code
        if (
            !LibraryCompilation.running && !MetaprogrammingCompilation.running && !MetaprogrammingExecution.running &&
            (input->keyboard.control.just_pressed && input->keyboard.H.is_down ||
             input->keyboard.control.is_down      && input->keyboard.H.just_pressed)
        ) {
            int64 MetaprogrammingSourceTimestamp = Win32GetFileInfo(build_config.metaprogramming_code_path.content).timestamp;
            int64 MetaprogrammingBinaryTimestamp = Win32GetFileInfo(MetaFile).timestamp;
    
            if (MetaprogrammingSourceTimestamp > MetaprogrammingBinaryTimestamp) {
                MetaprogrammingCompilationStart = Win32GetWallClock();
                MetaprogrammingCompilation = CompileMetaprogramming(memory.transient, &build_config);
            }
            else {
                MetaprogrammingExecutionStart = Win32GetWallClock();
                MetaprogrammingExecution = Win32RunCommand(MetaFile);
            }
        }

        if (MetaprogrammingCompilation.running) {
            int32 WaitResult = Win32WaitForProcess(&MetaprogrammingCompilation, 0);
            if (WaitResult >= 0) {
                uint64 End = Win32GetWallClock();
                LogCompilationResult(memory.transient, "Metaprogramming", WaitResult, MetaprogrammingCompilationStart, End);
                MetaprogrammingCompilationStart = 0;

                if (WaitResult == 0) {
                    MetaprogrammingExecutionStart = Win32GetWallClock();
                    MetaprogrammingExecution = Win32RunCommand(MetaFile);
                }
            }
        }

        if (MetaprogrammingExecution.running) {
            int32 WaitResult = Win32WaitForProcess(&MetaprogrammingExecution, 0);
            if (WaitResult >= 0) {
                uint64 End = Win32GetWallClock();
                float Time = GetSecondsElapsed(MetaprogrammingExecutionStart, End);
                Log_Level Level = WaitResult == 0 ? Log_Level::Info : Log_Level::Error;
                if (WaitResult == 0) sprintf_s(LogBuffer, "Metaprogramming executed in %.2f milliseconds.", 1000.0f * Time);
                else                 sprintf_s(LogBuffer, "Metaprogramming execution failed with code '%d'.", WaitResult);
                Log(Level, LogBuffer);
                MetaprogrammingExecutionStart = 0;

                if (WaitResult == 0) {
                    LibraryCompilationStart = Win32GetWallClock();
                    LibraryCompilation = CompileGameLibraryHot(memory.transient, build_config);
                }
            }
        }

        if (LibraryCompilation.running) {
            int32 WaitResult = Win32WaitForProcess(&LibraryCompilation, 0);
            if (WaitResult >= 0) {
                uint64 End = Win32GetWallClock();
                LogCompilationResult(memory.transient, "Game library", WaitResult, LibraryCompilationStart, End);
                LibraryCompilationStart = 0;
            }
        }
        
        int64 NewDLLWriteTime = Win32GetFileInfo(source_dll_name).timestamp;
        if (NewDLLWriteTime > code.dll_last_write_time) {
            static int Loads = 0;
            UnloadGameCode(&code);
            LoadGameCode(&code, source_dll_name, temp_dll_name);
            if (code.is_valid) {
                Log(Log_Level::Info, "New game code loaded.");
                memory.hot_reload = true;
            }
        }

        // Hot reloading for shaders
        // ReloadShaders();

        // Clear transient memory
        Clear(memory.transient);

        // Previous input
        UpdatePreviousInput(&memory.input);

        // Peek and dispatch messages
        if (!FirstFrame) {
            ProcessPendingMessages(window, &memory.input, &record_playback);
        }

        input->keyboard.any = false;
        for (int i = 0; i < NUMBER_OF_KEYS; i++) {
            if (input->keyboard.keys[i].is_down) {
                input->keyboard.any = true;
                break;
            }
        }

        if (input->mode != Keyboard && input->keyboard.any) {
            input->mode = Keyboard;
        }

        // XInput Controller
        for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex) {
            XINPUT_STATE ControllerState;
            if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
                XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;

                // Button mapping
                input->controller.pad_up.is_down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                input->controller.pad_down.is_down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                input->controller.pad_left.is_down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                input->controller.pad_right.is_down = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                input->controller.LB.is_down = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                input->controller.RB.is_down = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                input->controller.A_button.is_down = (Pad->wButtons & XINPUT_GAMEPAD_A);
                input->controller.B_button.is_down = (Pad->wButtons & XINPUT_GAMEPAD_B);
                input->controller.X_button.is_down = (Pad->wButtons & XINPUT_GAMEPAD_X);
                input->controller.Y_button.is_down = (Pad->wButtons & XINPUT_GAMEPAD_Y);
                input->controller.start.is_down = (Pad->wButtons & XINPUT_GAMEPAD_START);
                input->controller.back.is_down = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                input->controller.LS.is_down = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
                input->controller.RS.is_down = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
                input->controller.LT.is_down = Pad->bLeftTrigger > 0;
                input->controller.RT.is_down = Pad->bRightTrigger > 0;

                for (int i = 0; i < NUMBER_OF_CONTROLLER_BUTTONS; i++) {
                    Button_State* button = &input->controller.buttons[i];
                    button->just_pressed = button->is_down && !button->was_down;
                    button->just_pressed = !button->is_down && button->was_down;
                }

                input->controller.any = false;
                for (int i = 0; i < 16; i++) {
                    if (input->controller.buttons[i].is_down) {
                        input->controller.any = true;
                        break;
                    }
                }

                if (input->mode != Controller && input->controller.any) {
                    input->mode = Controller;
                }

                SHORT LeftStickX = (float)Pad->sThumbLX;
                SHORT LeftStickY = (float)Pad->sThumbLY;
                SHORT RightStickX = (float)Pad->sThumbRX;
                SHORT RightStickY = (float)Pad->sThumbRY;

                // Normalizing joystick values
                input->controller.left_joystick.x = LeftStickX / (LeftStickX < 0 ? 32768.0f : 32767.0f);
                input->controller.left_joystick.y = LeftStickY / (LeftStickY < 0 ? 32768.0f : 32767.0f);
                input->controller.right_joystick.x = RightStickX / (RightStickX < 0 ? 32768.0f : 32767.0f);
                input->controller.right_joystick.y = RightStickY / (RightStickY < 0 ? 32768.0f : 32767.0f);
            }
        }

        // Mouse
        //OpenInputDesktop(0, TRUE, READ_CONTROL);
        POINT MouseP;
        GetCursorPos(&MouseP);
        ScreenToClient(window, &MouseP);

        RECT rect;
        GetWindowRect(window, &rect);
        int ScreenX = rect.left;
        int ScreenY = rect.bottom;

        input->mouse.last_cursor.x = input->mouse.cursor.x;
        input->mouse.last_cursor.y = input->mouse.cursor.y;

        input->mouse.cursor.x = MouseP.x;
        input->mouse.cursor.y = MouseP.y;

        if (record_playback.record_index) {
            RecordInput(&record_playback, &memory.input);
        }
        if (record_playback.playback_index) {
            PlaybackInput(&record_playback, &memory.input);
        }

        // Game function
        if (code.is_valid) {
            GetClientRect(window, &rect);

            if (FirstFrame) {
                group->width = rect.right - rect.left;
                group->height = rect.bottom - rect.top;
            }
            else {
                int32 NewWidth = rect.right - rect.left;
                int32 NewHeight = rect.bottom - rect.top;

                if ((NewWidth != memory.render_group.width || NewHeight != memory.render_group.height)) {
                    group->width = NewWidth;
                    group->height = NewHeight;
                    //ResizeWindow(NewWidth, NewHeight);
                }
            }

            if (!memory.pause) {
                // Clear render group
                Clear(group);

                //code.Update(&memory, &GameSoundBuffers[currentBuffer], &GameSoundBuffers[currentBuffer]);

                // if (pGameState->Exit) {
                //     Memory.Running = false; PostQuitMessage(0);
                // }
            }

            // if (input->keyboard.F10.just_pressed) {
            //     ScreenCapture(group->width, group->height);
            // }

            // Basis CameraBasis = GetCameraBasis(pGameState->ActiveCamera->Angle, pGameState->ActiveCamera->Pitch);
            // Matrix4 View = GetViewMatrix(CameraBasis, pGameState->ActiveCamera->Distance, pGameState->ActiveCamera->Anchor);
            // Render(Group, View, &Memory.Input, Window, pGameState->Time);
        }
        else {
            Log(Log_Level::Error, "Could not update state due to invalid game code.");
        }

        // XAUDIO2_VOICE_STATE VoiceState;
        // pSourceVoice->GetState(&VoiceState);
        // if (FAILED(SubmitBuffer(&XAudio2Buffers[currentBuffer], pSourceVoice))) {
        //     Log(Log_Level::Error, "Buffer playing went wrong.");
        // }
        // else {
        //     currentBuffer = (currentBuffer + 1) % nBuffers;
        // }

        Debug_Info* debug_info = &memory.debug_info;

        PROCESS_MEMORY_COUNTERS_EX PMC = {};
        uint64 UsedMemory = 0;
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&PMC, sizeof(PMC))) {
            UsedMemory = PMC.WorkingSetSize / Megabytes(1);
        }
        debug_info->used_memory = UsedMemory;
        DEBUG_VALUE(UsedMemory, uint64);

        // float WorkSecsElapsed = GetSecondsElapsed(LastCounter, Win32GetWallClock());
        // float UsedTime_ms = 1000.0f * WorkSecsElapsed;

        // float SecsElapsedPerFrame = WorkSecsElapsed;
        // if (SecsElapsedPerFrame >= TargetSecondsPerFrame) {
        //     Log(Log_Level::Warn, "Missed a frame!");
        // }

        // float ActualSecsElapsed = SecsElapsedPerFrame + 0.0005f;
        // float FPS = 1.0f / ActualSecsElapsed;
        // float BudgetTime_ms = 1000.0f * TargetSecondsPerFrame;
        
        // DebugInfo->FPS                 = MonitorRefreshHz;
        // DebugInfo->UsedTime            = UsedTime_ms;
        // DebugInfo->UsedMCyclesPerFrame = UsedMCyclesPerFrame;

        // DEBUG_VALUE(FPS, float);
        // DEBUG_VALUE(BudgetTime_ms, float);
        // DEBUG_VALUE(UsedTime_ms, float);
        // DEBUG_VALUE(UsedMCyclesPerFrame, float);

        // float Time = pGameState->Time;
        // DEBUG_VALUE(Time, float);

        // if (FirstFrame) {
        //     pSourceVoice->Start(0, 0);
        //     FirstFrame = false;
        // }

        // while (SecsElapsedPerFrame < TargetSecondsPerFrame) {
        //     // if sleep granular : DWORD SleepMs = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecsElapsedPerFrame));
        //     //Sleep(SleepMs);
        //     SecsElapsedPerFrame = GetSecondsElapsed(LastCounter, Win32GetWallClock());
        // }

        // pGameState->dt = SecsElapsedPerFrame;
        // pGameState->Time += SecsElapsedPerFrame;

        // uint64 EndCounter = Win32GetWallClock();
        // LastCounter = EndCounter;
        // LastCycleCount = EndCycleCount;

        debug_info->entry_count = 0;

        memory.time_records_platform_count = __COUNTER__;
    }

    return 0;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXA wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PLATFORMLAYER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = 0; // MAKEINTRESOURCEW(IDC_TESTPROJECT);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExA(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND* WindowPtr)
{
    hInst = hInstance; // Store instance handle in our global variable

    BOOL DPIAwarenessResult = SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (DPIAwarenessResult) {
        Log(Log_Level::Info, "DPI awareness activated.");
    }
    else {
        Log(Log_Level::Error, "Couldn't set DPI awareness.");
    }

    // This code starts the window centered
    HWND hWnd = CreateWindowA(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 0, 0,
        100, 100, nullptr, nullptr, hInstance, nullptr);

    DWORD Error = GetLastError();

    if (!hWnd) return FALSE;
    else *WindowPtr = hWnd;

    // Starting resolution
    int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    // These are not the actual dimensions of the buffer
    int WindowWidth = 1280;
    int WindowHeight = 720;

    // This code starts the window centered
    int X = (ScreenWidth / 2) - (WindowWidth / 2);
    int Y = (ScreenHeight / 2) - (WindowHeight / 2);

    MoveWindow(hWnd, X, Y, WindowWidth, WindowHeight, 0);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId) {
                case IDM_EXIT:
                    { DestroyWindow(Window); } break;
                default:
                    return DefWindowProc(Window, message, wParam, lParam);
            }
        }
        break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(Window, &ps);

            if (memory.initialized) {
                RECT Rect = { 0 };
                GetClientRect(Window, &Rect);

                int32 NewWidth = Rect.right - Rect.left;
                int32 NewHeight = Rect.bottom - Rect.top;

                Render_Group* Group = &memory.render_group;

                if (NewWidth != Group->width || NewHeight != Group->height) {
                    Group->width = NewWidth;
                    Group->height = NewHeight;
                    // ResizeWindow(NewWidth, NewHeight);
                }

#if GAME_RENDER_API_OPENGL
                Render(Group, Memory.GameState->ActiveCamera, &Memory.Input, Window, 0.0);
#endif
            }

            EndPaint(Window, &ps);
            ReleaseDC(Window, hdc);
        } break;
        case WM_CLOSE:
        case WM_DESTROY:
            { memory.running = false; PostQuitMessage(0); } break;
        default:
            return DefWindowProc(Window, message, wParam, lParam);
    }
    return 0;
}
