#ifndef GAME_DEBUG
#define GAME_DEBUG

// +---------------------------------------------------------------------------------------------------------------------------------+
// | Debug info                                                                                                                      |
// +---------------------------------------------------------------------------------------------------------------------------------+

struct debug_info {
    float FPS;
    float BudgetTime;
    float UsedTime;
    float UsedMCyclesPerFrame;
};

inline void Assert(bool assertion, const char* Message = "") {
    if (!assertion) {
        int* i = 0;
        int j = *i;
    }
}

#endif