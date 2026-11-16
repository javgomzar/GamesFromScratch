#include "pch.h"
#include "texture.h"


uint64 GetPixelsSize(int32 width, int32 height, int32 channels) {
    return (uint64)width * (uint64)height * (uint64)channels;
}