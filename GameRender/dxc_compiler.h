#include <dxcapi.h>

struct DXC_Compiler {
    IDxcUtils* utils;
    IDxcCompiler3* compiler;
    IDxcIncludeHandler* include_handler;
};