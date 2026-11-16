#pragma once

#include <platform.h>
#include <mathematics.h>


/*
    TODO:
        - Mixer
        - Some effects (low pass, high pass)
        - Volume control
        - Implement circular buffer with VirtualAlloc2 and MapViewOfFile3
*/

INTROSPECT
struct Wave_Format {
    unsigned short    format_tag;        /* format type */
    unsigned short    channel_count;     /* number of channels (i.e. mono, stereo...) */
    unsigned long     samples_per_sec;   /* sample rate */
    unsigned long     avg_bytes_per_sec; /* for buffer estimation */
    unsigned short    block_align;       /* block size of data */
    unsigned short    bits_per_sample;   /* number of bits per sample of mono data */
    unsigned short    extra_info_size;   /* The count in bytes of the size of extra information (after extra_info_size) */
};

INTROSPECT
struct Sound {
    String name;
    Wave_Format wave_format;
    uint32 sample_count;
    uint32 played;
    int16* sample_out;
};

struct Sound_Buffer {
    uint32 samples_per_second;
    uint16 size;
    int16* sample_out;
};

bool LoadSound(void* content, Sound* sound);

inline const int16 AMPLITUDE = 4000;

float WriteSineWave(Sound_Buffer* sound_buffer, float hz, float initial_phase);
void FadeIn(Sound_Buffer* sound_buffer, uint8 transition_count);
void FadeOut(Sound_Buffer* sound_buffer, uint8 transition_count);
void Silence(Sound_Buffer* sound_buffer);
void PlayGameSound(Sound* sound, Sound_Buffer* sound_buffer);
