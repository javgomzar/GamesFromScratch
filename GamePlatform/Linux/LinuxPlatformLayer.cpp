#include "GameInput.h"
#include "GameLibrary.h"
#include "GameBuild.h"
#include <linux/input-event-codes.h>
#include <GLFW/glfw3.h>

#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include "xxhash.h"

#if GAME_RENDER_API_DIRECTX
    #include "DirectX11Render.h"
#elif GAME_RENDER_API_OPENGL
    #include "OpenGLRender.h"
#elif GAME_RENDER_API_VULKAN
    // #include "VulkanRender.h"
#endif

game_memory Memory;

struct game_code {
    bool IsValid;
    int64 SOLastWriteTime;
    void* GameCodeSO;
    game_update* Update;
};

GAME_UPDATE(GameUpdateStub) {}

void LoadGameCode(game_code* Result, const char* SourceFileName, const char* TemporaryFileName) {
    Result->Update = GameUpdateStub;

    bool CopyResult = Platform.FileCopy(SourceFileName, TemporaryFileName);
    if (!CopyResult) {
        Raise("Error copying game code file.");
    }

    Result->GameCodeSO = dlopen(TemporaryFileName, RTLD_NOW);
    if (Result->GameCodeSO) {
        Result->Update = (game_update*)dlsym(Result->GameCodeSO, "GameUpdate");
        if (Result->Update) {
            Result->IsValid = true;
        }
    }
    else {
        Raise("Error loading update function from game library.");
    }
}

int main() {
    game_code GameCode = {};
    LoadGameCode(&GameCode, "bin/GameLibrary.so", "bin/GameLibraryTemp.so");

    memory_index PermanentStorageSize = Megabytes(64);
    void* GameMemoryBlock = Platform.AllocateMemory(PermanentStorageSize);
    Memory.Permanent = MemoryArena(PermanentStorageSize, GameMemoryBlock);
    TimeRecords = (time_record*)&Memory.TimeRecordsPlatform;
    Memory.HotReload = true;

    game_state* pGameState = PushStruct(&Memory.Permanent, game_state);
    Memory.GameState = pGameState;

    // Memory arenas
    Memory.Transient = SuballocateMemoryArena(&Memory.Permanent, Megabytes(1));
    memory_arena FontsArena = SuballocateMemoryArena(&Memory.Permanent, Megabytes(1));

    // Assets
    game_assets* Assets = &Memory.Assets;
    const char* AssetsPath = "GameAssets/game_assets";
    // WriteAssetsFile(AssetsPath);
    // LoadAssetsFromFile(&FontsArena, Assets, AssetsPath);

    // Recording and playback
    record_and_playback RecordPlayback;
    RecordPlayback.PlaybackIndex = 0;
    RecordPlayback.RecordIndex = 0;
    RecordPlayback.GameMemoryBlock = GameMemoryBlock;
    RecordPlayback.TotalSize = PermanentStorageSize;

    render_group* Group = &Memory.RenderGroup;
    rectangle Rect = {};
    // GetClientRect(Window, &Rect);
    // InitializeRenderGroup(
    //     &Memory.Permanent,
    //     Group,
    //     Assets,
    //     Rect.right - Rect.left,
    //     Rect.bottom - Rect.top
    // );

    // Input
    char TextBuffer[128];
    int KeyboardFD = 0;
    game_input* Input = &Memory.Input;
    Input->Mode = Keyboard;
    DIR* Inputs = opendir("/dev/input");
    if (!Inputs) {
        Log(Error, "No input devices folder.");
    }
    else {
        dirent* Device;
        while ((Device = readdir(Inputs)) != nullptr) {
            if (strncmp(Device->d_name, "event", 5) != 0) {
                continue;
            }

            sprintf(TextBuffer, "/dev/input/%s", Device->d_name);
            int FD = open(TextBuffer, O_RDONLY | O_NONBLOCK);
            if (FD < 0) {
                ReportError();
                continue;
            }

            uint8 EVBits[EV_MAX / 8 + 1] = {};
            if (ioctl(FD, EVIOCGBIT(0, sizeof(EVBits)), EVBits) >= 0) {
                if (EVBits[EV_KEY / 8] & (1 << (EV_KEY % 8))) {
                    if (ioctl(FD, EVIOCGBIT(EV_KEY, sizeof(EVBits)), EVBits) >= 0) {
                        if (EVBits[KEY_A / 8] & (1 << (KEY_A % 8))) {
                            KeyboardFD = FD;
                            continue;
                        }
                    }
                }
            }
            
            close(FD);
        }
    }

    Assert(KeyboardFD);

    // Sound
    // TODO

    // Compilation setup
    build_configuration BuildConfiguration = {};
    ReadBuildConfiguration("GameBuild/build.conf", &BuildConfiguration);
    process_info LibraryCompilation = {};
    uint64 LibraryCompilationStart = 0;
    char MetaFile[32] = "bin/Meta";
    process_info MetaprogrammingCompilation = {};
    uint64 MetaprogrammingCompilationStart = 0;
    process_info MetaprogrammingExecution = {};
    uint64 MetaprogrammingExecutionStart = 0;

    // Window creation
    // InitializeRenderer(Group);
    if (!glfwInit()) {
        Log(Error, "Failed to initialize GLFW.");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* Window = glfwCreateWindow(
        800, 600,
        "RunGame",
        nullptr,
        nullptr
    );

    if (!Window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(Window);
    glfwSwapInterval(1); // VSYNC

    Memory.Running = true;
    bool FirstFrame = true;

    // Main loop
    while (Memory.Running) {
        // Hot reloading

        // Hot reloading for shaders

        ClearArena(&Memory.Transient);

        // Input
        UpdatePreviousInput(&Memory.Input);
        input_event Event;
        if (KeyboardFD) {
            while (read(KeyboardFD, &Event, sizeof(Event)) == sizeof(Event)) {
                if (Event.type == EV_KEY) {
                    char Key = 0;
                    switch (Event.code) {
                        case KEY_0: { Key = '0'; } break;
                        case KEY_1: { Key = '1'; } break;
                        case KEY_2: { Key = '2'; } break;
                        case KEY_3: { Key = '3'; } break;
                        case KEY_4: { Key = '4'; } break;
                        case KEY_5: { Key = '5'; } break;
                        case KEY_6: { Key = '6'; } break;
                        case KEY_7: { Key = '7'; } break;
                        case KEY_8: { Key = '8'; } break;
                        case KEY_9: { Key = '9'; } break;
                        case KEY_A: { Key = 'A'; } break;
                        case KEY_B: { Key = 'B'; } break;
                        case KEY_C: { Key = 'C'; } break;
                        case KEY_D: { Key = 'D'; } break;
                        case KEY_E: { Key = 'E'; } break;
                        case KEY_F: { Key = 'F'; } break;
                        case KEY_G: { Key = 'G'; } break;
                        case KEY_H: { Key = 'H'; } break;
                        case KEY_J: { Key = 'J'; } break;
                        case KEY_K: { Key = 'K'; } break;
                        case KEY_L: { Key = 'L'; } break;
                        case KEY_M: { Key = 'M'; } break;
                        case KEY_N: { Key = 'N'; } break;
                        case KEY_O: { Key = 'O'; } break;
                        case KEY_P: { Key = 'P'; } break;
                        case KEY_Q: { Key = 'Q'; } break;
                        case KEY_R: { Key = 'R'; } break;
                        case KEY_S: { Key = 'S'; } break;
                        case KEY_T: { Key = 'T'; } break;
                        case KEY_U: { Key = 'U'; } break;
                        case KEY_V: { Key = 'V'; } break;
                        case KEY_W: { Key = 'W'; } break;
                        case KEY_X: { Key = 'X'; } break;
                        case KEY_Y: { Key = 'Y'; } break;
                        case KEY_Z: { Key = 'Z'; } break;
                    }
                    game_button_state* Button = GetKey(Input, Key);
                    if (Button == nullptr) {
                        switch(Event.code) {
                            case KEY_LEFTSHIFT:
                            case KEY_RIGHTSHIFT: { Button = &Input->Keyboard.Shift; } break;
                            case KEY_LEFTCTRL:
                            case KEY_RIGHTCTRL: { Button = &Input->Keyboard.Control; } break;
                            case KEY_UP:        { Button = &Input->Keyboard.Up; } break;
                            case KEY_DOWN:      { Button = &Input->Keyboard.Down; } break;
                            case KEY_LEFT:      { Button = &Input->Keyboard.Left; } break;
                            case KEY_RIGHT:     { Button = &Input->Keyboard.Right; } break;
                            case KEY_ESC:       { Button = &Input->Keyboard.Escape; } break;
                            case KEY_SPACE:     { Button = &Input->Keyboard.Space; } break;
                            case KEY_ENTER:     { Button = &Input->Keyboard.Enter; } break;
                            case KEY_MENU:      { Button = &Input->Keyboard.Alt; } break;
                            case KEY_F1:        { Button = &Input->Keyboard.F1; } break;
                            case KEY_F2:        { Button = &Input->Keyboard.F2; } break;
                            case KEY_F3:        { Button = &Input->Keyboard.F3; } break;
                            case KEY_F4:        { Button = &Input->Keyboard.F4; } break;
                            case KEY_F5:        { Button = &Input->Keyboard.F5; } break;
                            case KEY_F6:        { Button = &Input->Keyboard.F6; } break;
                            case KEY_F7:        { Button = &Input->Keyboard.F7; } break;
                            case KEY_F8:        { Button = &Input->Keyboard.F8; } break;
                            case KEY_F9:        { Button = &Input->Keyboard.F9; } break;
                            case KEY_F10:       { Button = &Input->Keyboard.F10; } break;
                            case KEY_F11:       { Button = &Input->Keyboard.F11; } break;
                            case KEY_F12:       { Button = &Input->Keyboard.F12; } break;
                            case KEY_PAGEUP:    { Button = &Input->Keyboard.PageUp; } break;
                            case KEY_PAGEDOWN:  { Button = &Input->Keyboard.PageDown; } break;
                        }
                    }
                    if (Button) {
                        if (Event.value == 0) {
                            LiftButton(Button);
                            Log(Info, "Key lifted");
                        }
                        else if (Event.value == 1) {
                            PressButton(Button);
                            Log(Info, "Key pressed");
                        }
                    }
                }
            }
        }

        // Update
        if (GameCode.IsValid) {
            Clear(Group);

            // GameCode.Update(&Memory, nullptr, nullptr);
            // Log(Info, "UPDATE");

            if (pGameState->Exit) {
                Memory.Running = false;
            }
        }
        else {
            Log(Error, "Could not update state due to invalid game code.");
        }

        if (Input->Keyboard.F10.JustPressed) {
            // Screen capture
        }
    }

    glfwDestroyWindow(Window);
    glfwTerminate();

    return 0;
}