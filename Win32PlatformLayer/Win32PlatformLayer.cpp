#include "pch.h"
#include "GameLibrary.h"
#include "Win32PlatformLayer.h"

#if GAME_RENDER_API_OPENGL
    #pragma comment (lib, "opengl32.lib")
    #include "OpenGLRender.h"
#endif

#if GAME_RENDER_API_VULKAN
    #include "VulkanRender.h"
#endif

#pragma comment(lib, "xaudio2.lib")

#include "Tokenizer.h"

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
    - Software renderer with blit speed improvements (BitBlt)
    - GetKeyboardLayout (international wasd support)
    - Restore software renderer as fallback
    - Render using all window space. Remove rounded borders
*/

#define MAX_LOADSTRING 100

// General structs
struct WNDDIMENSION {
    int Width;
    int Height;
};

// Global Variables:
HINSTANCE hInst;                                // current instance
char szTitle[MAX_LOADSTRING];                  // The title bar text
char szWindowClass[MAX_LOADSTRING];            // the main window class name

bool Running;
game_memory GameMemory;

// XInput
#define XINPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef XINPUT_GET_STATE(xinput_get_state);
XINPUT_GET_STATE(XInputGetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
static xinput_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define XINPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef XINPUT_SET_STATE(xinput_set_state);
XINPUT_SET_STATE(XInputSetStateStub) {
    return ERROR_DEVICE_NOT_CONNECTED;
}
static xinput_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

static void LoadXInput(void) {
    HMODULE XInputLibrary = LoadLibrary(_T("xinput1_4.dll"));
    if (XInputLibrary) {
        XInputGetState = (xinput_get_state*)GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (xinput_set_state*)GetProcAddress(XInputLibrary, "XInputSetState");
    }
    else {
        // Diagnostic
    }
}


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND*);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Graphics
struct OFFSCREENBUFFER {
    BITMAPINFO Info;
    void* Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel = 4;
};

OFFSCREENBUFFER BackBuffer;
WINDOWPLACEMENT WindowPosition = { sizeof(WindowPosition) };

// Software render
VOID ResizeDIBSection(OFFSCREENBUFFER* Buffer, int Width, int Height) {

    //if (Buffer->Memory) {
    //    VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    //}

    Buffer->Width = Width;
    Buffer->Height = Height;

    //Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    //Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    //Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    //Buffer->Info.bmiHeader.biPlanes = 1;
    //Buffer->Info.bmiHeader.biBitCount = 32;
    //Buffer->Info.bmiHeader.biCompression = BI_RGB;
    //Buffer->Info.bmiHeader.biSizeImage = 0;

    //int BitmapMemorySize = Buffer->BytesPerPixel * Width * Height;
    //Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

VOID DisplayBufferToWindow(
    OFFSCREENBUFFER* Buffer, HDC DeviceContext, int Width, int Height
) {
    /*
    StretchDIBits(DeviceContext,
        0, 0, Width, Height,
        0, 0, Buffer->Width, Buffer->Height,
        Buffer->Memory,
        &Buffer->Info,
        DIB_RGB_COLORS,
        SRCCOPY
    );*/

    SwapBuffers(DeviceContext);
}

#if GAME_RENDER_API_OPENGL
    openGL RendererContext;
#endif
#if GAME_RENDER_API_VULKAN
    vulkan RendererContext;
#endif

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
        Log(Error, "ERROR creating XAudio2.");
    }
    else if (FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasterVoice))) {
        Log(Error, "ERROR creating mastering voice.");
    }
    else {
        if (FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, pWaveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &VoiceCallback))) {
            Log(Error, "ERROR creating source voice.");
        }
        else {
            uint32 AudioBytes = BufferSize * pWaveFormat->nChannels * (pWaveFormat->wBitsPerSample / 8);
            for (int i = 0; i < nBuffers; i++) {
                Buffers[i].AudioBytes = AudioBytes;
                Buffers[i].pAudioData = (BYTE*)VirtualAlloc(0, AudioBytes, MEM_COMMIT, PAGE_READWRITE);
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
void BeginRecordingInput(record_and_playback* RecordPlayback, int RecordIndex) {
    RecordPlayback->RecordIndex = RecordIndex;
    LPCSTR Filename = "foo.jgzr";
    RecordPlayback->RecordFile = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    DWORD BytesWritten;
    WriteFile(RecordPlayback->RecordFile, RecordPlayback->GameMemoryBlock, RecordPlayback->TotalSize, &BytesWritten, 0);
}

void EndRecordingInput(record_and_playback* RecordPlayback) {
    RecordPlayback->RecordIndex = 0;
    CloseHandle(RecordPlayback->RecordFile);
}

void RecordInput(record_and_playback* RecordPlayback, game_input* Input) {
    DWORD BytesWritten;
    WriteFile(RecordPlayback->RecordFile, Input, sizeof(*Input), &BytesWritten, 0);
}

void BeginInputPlayback(record_and_playback* RecordPlayback, int PlaybackIndex) {
    RecordPlayback->PlaybackIndex = PlaybackIndex;
    LPCSTR Filename = "foo.jgzr";
    RecordPlayback->PlaybackFile = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    DWORD BytesToRead = RecordPlayback->TotalSize;
    DWORD BytesRead;
    if (ReadFile(RecordPlayback->PlaybackFile, RecordPlayback->GameMemoryBlock, BytesToRead, &BytesRead, 0)) {

    }
    else {
        Log(Error, "Reading game state failed.");
    }
}

void EndInputPlayback(record_and_playback* RecordPlayback) {
    RecordPlayback->PlaybackIndex = 0;
    CloseHandle(RecordPlayback->PlaybackFile);
}

void PlaybackInput(record_and_playback* RecordPlayback, game_input* Input) {
    DWORD BytesRead = 0;
    if (ReadFile(RecordPlayback->PlaybackFile, Input, sizeof(*Input), &BytesRead, 0)) {
        if (BytesRead == 0) {
            int Index = RecordPlayback->PlaybackIndex;
            EndInputPlayback(RecordPlayback);
            BeginInputPlayback(RecordPlayback, Index);
        }
    }
}

static bool Pause = false;
static bool Minimized = false;
// Message processing
void ProcessPendingMessages(HWND Window, game_input* pInput, record_and_playback* RecordPlayback) {
    MSG msg;
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
        switch (msg.message) {
            // Mouse
            case WM_LBUTTONDOWN: { PressButton(&pInput->Mouse.LeftClick); } break;
            case WM_LBUTTONUP:   { LiftButton(&pInput->Mouse.LeftClick); } break;
            case WM_RBUTTONDOWN: { PressButton(&pInput->Mouse.RightClick); } break;
            case WM_RBUTTONUP:   { LiftButton(&pInput->Mouse.RightClick); } break;
            case WM_MBUTTONDOWN: { PressButton(&pInput->Mouse.MiddleClick); } break;
            case WM_MBUTTONUP:   { LiftButton(&pInput->Mouse.MiddleClick); } break;
            case WM_MOUSEWHEEL:  { pInput->Mouse.Wheel = GET_WHEEL_DELTA_WPARAM(msg.wParam); } break;
            case WM_KEYDOWN:
            
            // Keyboard
            case WM_SYSKEYDOWN: {
                uint32 VKCode = msg.wParam;
                bool WasDown = (msg.lParam & (1 << 30)) != 0;

                if (!WasDown) {
                    if (VKCode >= '0' && VKCode <= '9' || VKCode >= 'A' && VKCode <= 'Z') {
                        PressKey(pInput, VKCode);
                    }
                    
                    if (VKCode == 'P') {
                        Pause = !Pause;
                    }
                    
                    if (VKCode == VK_UP)          PressButton(&pInput->Keyboard.Up);
                    else if (VKCode == VK_DOWN)   PressButton(&pInput->Keyboard.Down);
                    else if (VKCode == VK_LEFT)   PressButton(&pInput->Keyboard.Left);
                    else if (VKCode == VK_RIGHT)  PressButton(&pInput->Keyboard.Right);
                    else if (VKCode == VK_ESCAPE) PressButton(&pInput->Keyboard.Escape);
                    else if (VKCode == VK_SPACE)  PressButton(&pInput->Keyboard.Space);
                    else if (VKCode == VK_RETURN) PressButton(&pInput->Keyboard.Enter);
                    else if (VKCode == VK_F1)     PressButton(&pInput->Keyboard.F1);
                    else if (VKCode == VK_F2)     PressButton(&pInput->Keyboard.F2);
                    else if (VKCode == VK_F3)     PressButton(&pInput->Keyboard.F3);
                    else if (VKCode == VK_F4)     PressButton(&pInput->Keyboard.F4);
                    else if (VKCode == VK_F5)     PressButton(&pInput->Keyboard.F5);
                    else if (VKCode == VK_F6)     PressButton(&pInput->Keyboard.F6);
                    else if (VKCode == VK_F7)     PressButton(&pInput->Keyboard.F7);
                    else if (VKCode == VK_F8)     PressButton(&pInput->Keyboard.F8);
                    else if (VKCode == VK_F9)     PressButton(&pInput->Keyboard.F9);
                    else if (VKCode == VK_F10)    PressButton(&pInput->Keyboard.F10);
                    else if (VKCode == VK_F11) {
                        PressButton(&pInput->Keyboard.F11);
                        ToggleFullScreen(Window);
                    }
                    else if (VKCode == VK_F12)   PressButton(&pInput->Keyboard.F12);
                    else if (VKCode == VK_PRIOR) PressButton(&pInput->Keyboard.PageUp);
                    else if (VKCode == VK_NEXT)  PressButton(&pInput->Keyboard.PageDown);
                    else if (VKCode == VK_SHIFT) PressButton(&pInput->Keyboard.Shift);
                    else if (VKCode == 'L') {
                        if (RecordPlayback->RecordIndex == 0) {
                            BeginRecordingInput(RecordPlayback, 1);
                        }
                        else {
                            EndRecordingInput(RecordPlayback);
                            BeginInputPlayback(RecordPlayback, 1);
                        }
                    }
                }

                // Shortcut for closing Alt+F4
                bool AltKeyWasDown = (msg.lParam & ((uint32)1 << 29)) != 0;
                if ((VKCode == VK_F4) && AltKeyWasDown) {
                    Running = false;
                    PostQuitMessage(0);
                }
            } break;
            case WM_KEYUP:
            case WM_SYSKEYUP: {
                uint32 VKCode = msg.wParam;
                if (VKCode >= '0' && VKCode <= '9' || VKCode >= 'A' && VKCode <= 'Z') {
                    LiftKey(pInput, VKCode);
                }

                if (VKCode == VK_UP)          LiftButton(&pInput->Keyboard.Up);
                else if (VKCode == VK_DOWN)   LiftButton(&pInput->Keyboard.Down);
                else if (VKCode == VK_LEFT)   LiftButton(&pInput->Keyboard.Left);
                else if (VKCode == VK_RIGHT)  LiftButton(&pInput->Keyboard.Right);
                else if (VKCode == VK_ESCAPE) LiftButton(&pInput->Keyboard.Escape);
                else if (VKCode == VK_SPACE)  LiftButton(&pInput->Keyboard.Space);
                else if (VKCode == VK_RETURN) LiftButton(&pInput->Keyboard.Enter);
                else if (VKCode == VK_F1)     LiftButton(&pInput->Keyboard.F1);
                else if (VKCode == VK_F2)     LiftButton(&pInput->Keyboard.F2);
                else if (VKCode == VK_F3)     LiftButton(&pInput->Keyboard.F3);
                else if (VKCode == VK_F4)     LiftButton(&pInput->Keyboard.F4);
                else if (VKCode == VK_F5)     LiftButton(&pInput->Keyboard.F5);
                else if (VKCode == VK_F6)     LiftButton(&pInput->Keyboard.F6);
                else if (VKCode == VK_F7)     LiftButton(&pInput->Keyboard.F7);
                else if (VKCode == VK_F8)     LiftButton(&pInput->Keyboard.F8);
                else if (VKCode == VK_F9)     LiftButton(&pInput->Keyboard.F9);
                else if (VKCode == VK_F10)    LiftButton(&pInput->Keyboard.F10);
                else if (VKCode == VK_F11)    LiftButton(&pInput->Keyboard.F11);
                else if (VKCode == VK_F12)    LiftButton(&pInput->Keyboard.F12);
                else if (VKCode == VK_PRIOR)  LiftButton(&pInput->Keyboard.PageUp);
                else if (VKCode == VK_NEXT)   LiftButton(&pInput->Keyboard.PageDown);
                else if (VKCode == VK_SHIFT)  LiftButton(&pInput->Keyboard.Shift);
            } break;
            case WM_CLOSE:
            case WM_DESTROY:
            {
                Running = false;
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

struct game_code {
    bool IsValid;
    FILETIME DLLLastWriteTime;
    HMODULE GameCodeDLL;
    game_update* Update;
};

FILETIME GetLastWriteTime(LPCSTR FilePath) {
    FILETIME Result = {};

    WIN32_FIND_DATAA FindData;
    HANDLE FileHandle = FindFirstFileA(FilePath, &FindData);
    if (FileHandle != INVALID_HANDLE_VALUE) {
        Result = FindData.ftLastWriteTime;
        FindClose(FileHandle);
    }

    return Result;
}

void LoadGameCode(game_code* Result, LPCSTR SourceDLLName, LPCSTR TempDLLName) {
    Result->Update = GameUpdateStub;

    char ErrorText[256];
    DWORD LastError = 0;

    bool CopyResult = CopyFileA(SourceDLLName, TempDLLName, FALSE);
    if (!CopyResult) {
        LastError = GetLastError();
        if (LastError == ERROR_SHARING_VIOLATION) {
            int Retries = 0;
            do {
                Log(Warn, "Retrying game code loading after sharing violation.");
                Sleep(100);
                CopyResult = CopyFileA(SourceDLLName, TempDLLName, FALSE);
                Retries++;
                if (Retries > 100) {
                    Log(Error, "Max number of retries reached.");
                    break;
                }
            } while (!CopyResult);
        }
        else {
            sprintf_s(ErrorText, "Error copying file %s into %s. Error code %d.", SourceDLLName, TempDLLName, LastError);
            Log(Error, ErrorText);
            return;
        }
    }

    Result->GameCodeDLL = LoadLibraryA(TempDLLName);
    if (Result->GameCodeDLL) {
        Result->Update = (game_update*)GetProcAddress(Result->GameCodeDLL, "GameUpdate");
        Result->IsValid = (Result->Update);

        FILETIME LastWriteTime = GetLastWriteTime(SourceDLLName);
        Result->DLLLastWriteTime = LastWriteTime;
    }
    else {
        LastError = GetLastError();
        sprintf_s(ErrorText, "Error loading game code. Error code %d.", LastError);
        Log(Error, ErrorText); // If error is 126 (dependency error while loading DLL) try using Process Monitor.
    }
}

void UnloadGameCode(game_code* GameCode) {
    if (GameCode->GameCodeDLL) {
        bool UnloadResult = FreeLibrary(GameCode->GameCodeDLL);
    }

    GameCode->IsValid = false;
    GameCode->Update = GameUpdateStub;
}

// Performance
static uint64 PerfCountFrequency;

inline LARGE_INTEGER GetWallClock() {
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline float GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End) {
    return((float)(End.QuadPart - Start.QuadPart) / (float)PerfCountFrequency);
}

// Debugging
void DebugDrawVertical(game_offscreen_buffer* Buffer, int X, int Top, int Bottom, uint32 Color) {
    uint8* Pixel = ((uint8*)Buffer->Memory + X * Buffer->BytesPerPixel + Top * Buffer->Pitch);

    for (int Y = Top; Y < Bottom; Y++) {
        *(uint32*)Pixel = Color;
        Pixel += Buffer->Pitch;
    }
}

void LogDebugRecords(render_group* Group, memory_arena* TransientArena);

// Main window callback
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //TestThreads();

    // Loading XInputLibrary
    LoadXInput();

    // Audio initialization
    int SamplesPerSecond = 48000;
    const int nBuffers = 5;                         // Number of audio buffers to rotate
    int BufferSize = SamplesPerSecond / 60;         // Buffer size in samples
    WAVEFORMATEX WaveFormat;
    SetWaveFormat(SamplesPerSecond, &WaveFormat);

    XAUDIO2_BUFFER XAudio2Buffers[nBuffers];
    game_sound_buffer Buffers[nBuffers];
    InitXAudio2(nBuffers, BufferSize, XAudio2Buffers, &WaveFormat);

    game_sound_buffer GameSoundBuffers[nBuffers];
    for (int i = 0; i < nBuffers; i++) {
        GameSoundBuffers[i].SamplesPerSecond = SamplesPerSecond;
        GameSoundBuffers[i].BufferSize = BufferSize;
        GameSoundBuffers[i].SampleOut = (int16*)XAudio2Buffers[i].pAudioData;
    }

    // Performance counting initialization
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    PerfCountFrequency = PerfCountFrequencyResult.QuadPart;

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
    HWND Window;
    if (!InitInstance(hInstance, nCmdShow, &Window)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PLATFORMLAYER));

    // Set up for main loop
    const LPCSTR SourceDLLName = "GameLibrary.dll";
    const LPCSTR TempDLLName = "GameLibraryTemp.dll";

    game_code GameCode = { 0 };
    LoadGameCode(&GameCode, SourceDLLName, TempDLLName);
    LPVOID BaseAddress = 0;
    GameMemory.PermanentStorageSize = Megabytes(64);
    void* GameMemoryBlock = VirtualAlloc(BaseAddress, GameMemory.PermanentStorageSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    GameMemory.PermanentStorage = GameMemoryBlock;

    GameMemory.Platform.FreeFileMemory = PlatformFreeFileMemory;
    GameMemory.Platform.ReadEntireFile = PlatformReadEntireFile;
    GameMemory.Platform.WriteEntireFile = PlatformWriteEntireFile;
    GameMemory.Platform.AppendToFile = PlatformAppendToFile;

    // Assets
    const char* AssetsPath = "..\\GameAssets\\game_assets";
    WriteAssetsFile(AssetsPath);
    LoadAssetsFromFile(GameMemory.Platform.ReadEntireFile, &GameMemory.Assets, AssetsPath);

    game_state* pGameState = (game_state*)GameMemory.PermanentStorage;
    render_group* Group = &GameMemory.RenderGroup;

    pGameState->EntityList.Assets = &GameMemory.Assets;

    // Recording and playback
    record_and_playback RecordPlayback;
    RecordPlayback.PlaybackIndex = 0;
    RecordPlayback.RecordIndex = 0;
    RecordPlayback.GameMemoryBlock = GameMemoryBlock;
    RecordPlayback.TotalSize = GameMemory.PermanentStorageSize;

    memory_index MemoryUsed = InitializeRenderGroup(
        &GameMemory.RenderGroup, 
        &GameMemory.Assets, 
        (uint8*)GameMemory.PermanentStorage + sizeof(game_state)
    );

    // Memory arenas
    uint8* ArenaStart = (uint8*)GameMemory.PermanentStorage + sizeof(game_state) + MemoryUsed;
    GameMemory.TransientArena = MemoryArena(Megabytes(1), ArenaStart);
    ArenaStart += GameMemory.TransientArena.Size;
    GameMemory.StringsArena = MemoryArena(Kilobytes(1), ArenaStart);
    ArenaStart += GameMemory.StringsArena.Size;
    GameMemory.GeneralPurposeArena = MemoryArena(Megabytes(1), ArenaStart);
    ArenaStart += GameMemory.GeneralPurposeArena.Size;

    // Input
    game_input Input = {};
    Input.Mode = Keyboard;

    // Sound
    int currentBuffer = 1;

    // Enforcing framerate
    int MonitorRefreshHz = 60;
    float TargetSecondsPerFrame = 1.0f / (float)MonitorRefreshHz;

    // Performance
    LARGE_INTEGER LastCounter = GetWallClock();
    uint64 LastCycleCount = __rdtsc();

    // Initilize render API
    InitializeRenderer(
        &RendererContext,
        &GameMemory.RenderGroup.VertexBuffer,
        &GameMemory.Assets,
        Window, 
        hInstance
    );

    Running = true;
    bool FirstFrame = true;
    // Main message loop:
    while (Running) {
        // Loading game code
        FILETIME NewDLLWriteTime = GetLastWriteTime(SourceDLLName);
        LONG DebugFiletime = CompareFileTime(&GameCode.DLLLastWriteTime, &NewDLLWriteTime);

        if (DebugFiletime != 0) {
            static int Loads = 0;
            UnloadGameCode(&GameCode);
            LoadGameCode(&GameCode, SourceDLLName, TempDLLName);
            if (GameCode.IsValid) {
                Log(Info, "New game code loaded.");
            }
        }

        // Clear transient memory
        ClearArena(&GameMemory.TransientArena);

        // Previous input
        UpdatePreviousInput(&Input);

        // Peek and dispatch messages
        if (!FirstFrame) {
            ProcessPendingMessages(Window, &Input, &RecordPlayback);
        }

        Input.Keyboard.Any = false;
        for (int i = 0; i < NUMBER_OF_KEYS; i++) {
            if (Input.Keyboard.Keys[i].IsDown) {
                Input.Keyboard.Any = true;
                break;
            }
        }

        if ((Input.Mode != Keyboard) && Input.Keyboard.Any) {
            Input.Mode = Keyboard;
        }

        // XInput Controller
        for (DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex) {
            XINPUT_STATE ControllerState;
            if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS) {
                XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;

                // Button mapping
                Input.Controller.PadUp.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                Input.Controller.PadDown.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                Input.Controller.PadLeft.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                Input.Controller.PadRight.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                Input.Controller.LB.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                Input.Controller.RB.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                Input.Controller.AButton.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_A);
                Input.Controller.BButton.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_B);
                Input.Controller.XButton.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_X);
                Input.Controller.YButton.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_Y);
                Input.Controller.Start.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_START);
                Input.Controller.Back.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_BACK);
                Input.Controller.LS.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
                Input.Controller.RS.IsDown = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
                Input.Controller.LT.IsDown = Pad->bLeftTrigger > 0;
                Input.Controller.RT.IsDown = Pad->bRightTrigger > 0;

                for (int i = 0; i < NUMBER_OF_CONTROLLER_BUTTONS; i++) {
                    game_button_state* Button = &Input.Controller.Buttons[i];
                    Button->JustPressed = Button->IsDown && !Button->WasDown;
                    Button->JustPressed = !Button->IsDown && Button->WasDown;
                }

                Input.Controller.Any = false;
                for (int i = 0; i < 16; i++) {
                    if (Input.Controller.Buttons[i].IsDown) {
                        Input.Controller.Any = true;
                        break;
                    }
                }

                if (Input.Mode != Controller && Input.Controller.Any) {
                    Input.Mode = Controller;
                }

                SHORT LeftStickX = (float)Pad->sThumbLX;
                SHORT LeftStickY = (float)Pad->sThumbLY;
                SHORT RightStickX = (float)Pad->sThumbRX;
                SHORT RightStickY = (float)Pad->sThumbRY;

                // Normalizing joystick values
                Input.Controller.LeftJoystick.X = LeftStickX / (LeftStickX < 0 ? 32768.0f : 32767.0f);
                Input.Controller.LeftJoystick.Y = LeftStickY / (LeftStickY < 0 ? 32768.0f : 32767.0f);
                Input.Controller.RightJoystick.X = RightStickX / (RightStickX < 0 ? 32768.0f : 32767.0f);
                Input.Controller.RightJoystick.Y = RightStickY / (RightStickY < 0 ? 32768.0f : 32767.0f);

                char text[256];
                sprintf_s(text, "L: X %f, Y %f\nR: X %f, Y %f\n",
                    Input.Controller.LeftJoystick.X, Input.Controller.LeftJoystick.Y,
                    Input.Controller.RightJoystick.X, Input.Controller.RightJoystick.Y);
                //OutputDebugStringA(text);
            }
        }

        // Mouse
        //OpenInputDesktop(0, TRUE, READ_CONTROL);
        POINT MouseP;
        GetCursorPos(&MouseP);
        ScreenToClient(Window, &MouseP);

        RECT rect;
        GetWindowRect(Window, &rect);
        int ScreenX = rect.left;
        int ScreenY = rect.bottom;

        Input.Mouse.LastCursor.X = Input.Mouse.Cursor.X;
        Input.Mouse.LastCursor.Y = Input.Mouse.Cursor.Y;

        Input.Mouse.Cursor.X = MouseP.x;
        Input.Mouse.Cursor.Y = MouseP.y;

        /*char MouseTextBuffer[256];
        sprintf_s(MouseTextBuffer, "X: %d\nY: %d\n", Input.Mouse.Cursor.X, Input.Mouse.Cursor.Y);
        OutputDebugStringA(MouseTextBuffer);*/

        // Render image and sound on buffer
        game_offscreen_buffer Buffer = {};
        Buffer.Memory = BackBuffer.Memory;
        Buffer.Width = BackBuffer.Width;
        Buffer.Height = BackBuffer.Height;
        Buffer.Pitch = BackBuffer.Pitch;
        Buffer.BytesPerPixel = 4;

        if (RecordPlayback.RecordIndex) {
            RecordInput(&RecordPlayback, &Input);
        }
        if (RecordPlayback.PlaybackIndex) {
            PlaybackInput(&RecordPlayback, &Input);
        }

        // Game function
        if (GameCode.IsValid) {
            RECT Rect = { 0 };
            GetClientRect(Window, &Rect);

            if (FirstFrame) {
                Group->Width = Rect.right - Rect.left;
                Group->Height = Rect.bottom - Rect.top;
            }
            else {
                int32 NewWidth = Rect.right - Rect.left;
                int32 NewHeight = Rect.bottom - Rect.top;

                if ((NewWidth != GameMemory.RenderGroup.Width || NewHeight != GameMemory.RenderGroup.Height)) {
                    Group->Width = NewWidth;
                    Group->Height = NewHeight;
                    ResizeWindow(&RendererContext, NewWidth, NewHeight);
                }
            }

            if (!Pause) {
                // Clear render group
                ClearEntries(Group);

                GameCode.Update(&GameMemory, &GameSoundBuffers[currentBuffer], &GameSoundBuffers[currentBuffer], &Input);

                if (pGameState->Exit) {
                    Running = false; PostQuitMessage(0);
                }
            }

            if (Input.Keyboard.F10.IsDown && !Input.Keyboard.F11.WasDown) {
                ScreenCapture(&RendererContext, Group->Width, Group->Height);
            }

            LogDebugRecords(Group, &GameMemory.TransientArena);
            Render(Window, Group, &RendererContext, pGameState->Time);
            ClearVertexBuffer(&GameMemory.RenderGroup.VertexBuffer);
        }
        else {
            Log(Error, "Could not update state due to invalid game code.");
        }

        // DebugSyncDisplay(&Buffer, &GameSoundBuffers[currentBuffer]);
        // DisplayBufferToWindow(&BackBuffer, DeviceContext, Dimension.Width, Dimension.Height);

        XAUDIO2_VOICE_STATE VoiceState;
        pSourceVoice->GetState(&VoiceState);
        if (FAILED(SubmitBuffer(&XAudio2Buffers[currentBuffer], pSourceVoice))) {
            Log(Error, "Buffer playing went wrong.");
        }
        else {
            currentBuffer = (currentBuffer + 1) % nBuffers;
        }

        // Performance computations
        uint64 EndCycleCount = __rdtsc();
        uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
        GameMemory.DebugInfo.UsedMCyclesPerFrame = CyclesElapsed / 1000000.0f;

        LARGE_INTEGER WorkCounter = GetWallClock();
        float WorkSecsElapsed = GetSecondsElapsed(LastCounter, WorkCounter);
        GameMemory.DebugInfo.UsedTime = 1000.0f * WorkSecsElapsed;

        float SecsElapsedPerFrame = WorkSecsElapsed;
        if (SecsElapsedPerFrame < TargetSecondsPerFrame) {
            while (SecsElapsedPerFrame < (TargetSecondsPerFrame - 0.0005f)) {
                // if sleep granular : DWORD SleepMs = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecsElapsedPerFrame));
                //Sleep(SleepMs);
                SecsElapsedPerFrame = GetSecondsElapsed(LastCounter, GetWallClock());
            }
        }
        else {
            // Missed a frame!
            Log(Warn, "Missed a frame!");
        }

        float ActualSecsElapsed = SecsElapsedPerFrame + 0.0005f;
        GameMemory.DebugInfo.BudgetTime = 1000.0f * ActualSecsElapsed; // (in ms)
        GameMemory.DebugInfo.FPS = 1.0f / ActualSecsElapsed;

        pGameState->dt = ActualSecsElapsed;
        pGameState->Time += ActualSecsElapsed;

        if (FirstFrame) {
            pSourceVoice->Start(0, 0);
            FirstFrame = false;
        }

        while (SecsElapsedPerFrame < TargetSecondsPerFrame) {
            // if sleep granular : DWORD SleepMs = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecsElapsedPerFrame));
            //Sleep(SleepMs);
            SecsElapsedPerFrame = GetSecondsElapsed(LastCounter, GetWallClock());
        }

        LARGE_INTEGER EndCounter = GetWallClock();
        LastCounter = EndCounter;
        LastCycleCount = EndCycleCount;
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
                case IDM_ABOUT:
                    { DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), Window, About); } break;
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

            if (GameMemory.IsInitialized) {
                RECT Rect = { 0 };
                GetClientRect(Window, &Rect);

                int32 NewWidth = Rect.right - Rect.left;
                int32 NewHeight = Rect.bottom - Rect.top;

                render_group* Group = &GameMemory.RenderGroup;

                if (NewWidth != Group->Width || NewHeight != Group->Height) {
                    Group->Width = NewWidth;
                    Group->Height = NewHeight;
                    ResizeWindow(&RendererContext, NewWidth, NewHeight);
                }

                Render(Window, Group, &RendererContext, 0.0);
            }

            EndPaint(Window, &ps);
            ReleaseDC(Window, hdc);
        } break;
        case WM_CLOSE:
        case WM_DESTROY:
            { Running = false; PostQuitMessage(0); } break;
        default:
            return DefWindowProc(Window, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
    }
    return (INT_PTR)FALSE;
}

debug_record DebugRecordArray_Win32[__COUNTER__];

void LogDebugRecords(render_group* Group, memory_arena* Arena) {
    char Buffer[512];
    int Height = 350;
    game_font* Font = GetAsset(&GameMemory.Assets, Font_Cascadia_Mono_ID);
    for (int i = 0; i < ArrayCount(DebugRecordArray_Win32); i++) {
        debug_record* DebugRecord = DebugRecordArray_Win32 + i;

        if (DebugRecord->HitCount) {
            if (DebugRecord->HitCount == 1) {
                sprintf_s(Buffer, "%s: (%i hit) %.02f Mcycles (%s:%i).", 
                    DebugRecord->FunctionName, DebugRecord->HitCount, 
                    DebugRecord->CycleCount / 1000000.0f, DebugRecord->FileName, DebugRecord->LineNumber);
            }
            else {
                sprintf_s(Buffer, "%s: (%i hits) Total: %.02f Mcycles, Average: %.02f ms (%s:%i).", 
                    DebugRecord->FunctionName, DebugRecord->HitCount, 
                    DebugRecord->CycleCount / 1000000.0f, 
                    DebugRecord->CycleCount / (1000000.0f * DebugRecord->HitCount), 
                    DebugRecord->FileName, DebugRecord->LineNumber);
            }
            string String = PushString(Arena, 512, Buffer);
            Log(Info, Buffer);
            Height += 18;
            DebugRecord->HitCount = 0;
            DebugRecord->CycleCount = 0;
        }
    }
}